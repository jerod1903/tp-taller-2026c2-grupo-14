#include "common/map/persistence/MapRequirementsLoader.h"

#include <cstdint>
#include <limits>
#include <set>
#include <stdexcept>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <toml++/toml.hpp>

#include "common/map/persistence/TomlTableReader.h"

namespace stardew::maps {
namespace {

/// Lee un numero de piso opcional y comprueba que sea positivo y representable como int.
std::optional<int> optionalLevel(const detail::TomlTableReader& table, std::string_view key) {
    const auto value = table.optionalInt(key);
    if (value.has_value() && *value < 1) {
        throw std::runtime_error("transition level must be a positive integer");
    }
    return value;
}

}  // namespace

/// Carga y valida las reglas del mapa desde un archivo TOML.
/// @param path Ruta del archivo TOML de requisitos.
/// @return Reglas tipadas que puede consumir el validador de mapas.
/// @throws std::runtime_error Si el archivo no se puede parsear o contiene reglas incompletas,
///                            desconocidas o contradictorias.
MapRequirements loadMapRequirements(const std::string& path) {
    // Parsear una sola vez permite reutilizar el documento para leer todas sus reglas.
    toml::table document;
    try {
        document = toml::parse_file(path);
    } catch (const toml::parse_error& error) {
        throw std::runtime_error("cannot parse map requirements '" + path +
                                 "': " + std::string(error.description()));
    }

    // Lee la version y las opciones generales antes de procesar las listas de requisitos.
    const detail::TomlTableReader reader(document, "requirements");
    MapRequirements requirements{
            reader.requiredInt("format_version"),
            reader.requiredValue<bool>("allow_structure_overlap"),
            reader.requiredValue<bool>("require_outgoing_transition_for_each_area"),
            {},
            {},
            {},
    };
    if (requirements.formatVersion <= 0) {
        throw std::runtime_error("requirements format_version must be positive");
    }

    // Carga las cantidades requeridas por tipo de area y sus numeros de piso, si corresponden.
    std::set<AreaType> areaTypes;
    const auto* requiredAreas = reader.requiredArray("required_areas");
    for (const auto& node: *requiredAreas) {
        const auto& table = detail::requiredTable(node, "each required_areas entry");
        const detail::TomlTableReader areaReader(table, "required area");
        const auto type = areaReader.requiredEnum<AreaType>("type", areaTypeFromString);
        const int count = areaReader.requiredInt("count");
        if (count < 0 || !areaTypes.insert(type).second) {
            throw std::runtime_error("required area count is negative or its type is duplicated");
        }

        // Los pisos deben ser positivos, unicos y coincidir con la cantidad declarada.
        std::vector<int> levelNumbers;
        if (const auto* levels = areaReader.optionalArray("level_numbers")) {
            std::set<int> uniqueLevels;
            for (const auto& levelNode: *levels) {
                const auto level = levelNode.value<std::int64_t>();
                if (!level.has_value() || *level < 1 || *level > std::numeric_limits<int>::max() ||
                    !uniqueLevels.insert(static_cast<int>(*level)).second) {
                    throw std::runtime_error(
                            "required area has an invalid or duplicate level number");
                }
                levelNumbers.push_back(static_cast<int>(*level));
            }
            if (!levelNumbers.empty() && levelNumbers.size() != static_cast<std::size_t>(count)) {
                throw std::runtime_error("required level_numbers count does not match area count");
            }
        }

        requirements.areas.push_back({type, count, std::move(levelNumbers)});
    }

    // Una regla por tipo de elemento y tipo de area: donde va, cuanto mide y cuantos puede haber.
    std::set<std::tuple<AreaType, ElementType>> elementRules;
    const auto* elements = reader.requiredArray("elements");
    for (const auto& node: *elements) {
        const auto& table = detail::requiredTable(node, "each elements entry");
        const detail::TomlTableReader elementReader(table, "element rule");
        ElementRule rule;
        rule.areaType = elementReader.requiredEnum<AreaType>("area_type", areaTypeFromString);
        rule.type = elementReader.requiredEnum<ElementType>("type", elementTypeFromString);
        rule.layer = elementReader.requiredEnum<ElementLayer>("layer", elementLayerFromString);
        rule.width = elementReader.requiredInt("width");
        rule.height = elementReader.requiredInt("height");
        rule.maxPerArea = elementReader.optionalInt("max_per_area");
        rule.required = elementReader.requiredValue<bool>("required");
        if (rule.width <= 0 || rule.height <= 0 || (rule.maxPerArea && *rule.maxPerArea <= 0)) {
            throw std::runtime_error("element rule size and max_per_area must be positive");
        }
        if (!elementRules.emplace(rule.areaType, rule.type).second) {
            throw std::runtime_error("duplicate element rule");
        }
        requirements.elements.push_back(rule);
    }

    // Carga las conexiones que cada tipo de area o piso debe ofrecer.
    using TransitionKey = std::tuple<AreaType, std::optional<int>, AreaType, std::optional<int>>;
    std::set<TransitionKey> transitionRequirements;
    const auto* requiredTransitions = reader.requiredArray("required_transitions");
    for (const auto& node: *requiredTransitions) {
        const auto& table = detail::requiredTable(node, "each required_transitions entry");
        const detail::TomlTableReader transitionReader(table, "required transition");
        const auto fromType =
                transitionReader.requiredEnum<AreaType>("from_area_type", areaTypeFromString);
        const auto fromLevel = optionalLevel(transitionReader, "from_level");
        const auto toType =
                transitionReader.requiredEnum<AreaType>("to_area_type", areaTypeFromString);
        const auto toLevel = optionalLevel(transitionReader, "to_level");
        if ((fromLevel.has_value() && fromType != AreaType::DungeonLevel) ||
            (toLevel.has_value() && toType != AreaType::DungeonLevel)) {
            throw std::runtime_error("only dungeon transition endpoints can specify a level");
        }
        if (!transitionRequirements.emplace(fromType, fromLevel, toType, toLevel).second) {
            throw std::runtime_error("duplicate required transition rule");
        }
        requirements.transitions.push_back({fromType, fromLevel, toType, toLevel});
    }

    return requirements;
}

}  // namespace stardew::maps
