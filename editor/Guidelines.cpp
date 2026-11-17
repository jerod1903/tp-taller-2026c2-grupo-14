#include "editor/Guidelines.h"

#include <map>
#include <vector>

namespace maps = stardew::maps;

namespace {
// Como se nombra cada tipo de area: cuantas tiene el mapa y quien necesita los elementos
struct AreaWords {
    std::string singular;
    std::string plural;
    std::string owner;
};

const std::map<maps::AreaType, AreaWords> AREA_WORDS = {
        {maps::AreaType::Farm, {"granja", "granjas", "La granja"}},
        {maps::AreaType::Village, {"pueblo", "pueblos", "El pueblo"}},
        {maps::AreaType::DungeonLevel, {"piso", "pisos", "Cada piso de la mina"}},
};

// Los elementos que pueden ser obligatorios; los demas se nombran con su id
const std::map<maps::ElementType, std::string> ELEMENT_NAMES = {
        {maps::ElementType::Cabin, "la cabaña"},
        {maps::ElementType::Market, "el mercado"},
        {maps::ElementType::CommunityCenter, "el centro comunitario"},
        {maps::ElementType::DungeonEntrance, "la entrada a la mina"},
};

std::string areaName(maps::AreaType type) {
    auto words = AREA_WORDS.find(type);
    return words != AREA_WORDS.end() ? words->second.singular : std::string(maps::toString(type));
}

std::string countOf(const maps::AreaRequirement& area) {
    // Los pisos son una sola mina
    if (area.type == maps::AreaType::DungeonLevel) {
        return "una mina de " + std::to_string(area.count) + " pisos";
    }
    auto words = AREA_WORDS.find(area.type);
    const std::string name = words == AREA_WORDS.end() ? std::string(maps::toString(area.type)) :
                             area.count == 1           ? words->second.singular :
                                                         words->second.plural;
    return std::to_string(area.count) + " " + name;
}

std::string elementName(maps::ElementType type) {
    auto name = ELEMENT_NAMES.find(type);
    return name != ELEMENT_NAMES.end() ? name->second : std::string(maps::toString(type));
}

// "a", "a y b", "a, b y c"
std::string listOf(const std::vector<std::string>& items) {
    std::string text;
    for (std::size_t i = 0; i < items.size(); ++i) {
        if (i > 0) {
            text += i + 1 == items.size() ? " y " : ", ";
        }
        text += items[i];
    }
    return text;
}
}  // namespace

std::string mapGuidelines(const maps::MapRequirements& requirements) {
    std::vector<std::string> counts;
    for (const maps::AreaRequirement& area: requirements.areas) {
        counts.push_back(countOf(area));
    }
    std::string text = "Un mapa tiene " + listOf(counts) + ".";

    for (const maps::AreaRequirement& area: requirements.areas) {
        std::vector<std::string> needed;
        for (const maps::ElementRule& rule: requirements.elements) {
            if (rule.areaType == area.type && rule.required) {
                needed.push_back(elementName(rule.type));
            }
        }
        if (!needed.empty()) {
            auto words = AREA_WORDS.find(area.type);
            const std::string owner =
                    words != AREA_WORDS.end() ? words->second.owner : "Cada " + areaName(area.type);
            text += "\n" + owner + " necesita " + listOf(needed) + ".";
        }
    }
    return text;
}
