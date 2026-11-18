#include "server/game/commands/chat_command.h"

#include <optional>
#include <string>

#include "server/consts.h"
#include "server/game/cheats/cheat.h"
#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

namespace {

// Lo que sobra a los costados de un mensaje.
constexpr const char* BLANKS = " \t\r\n";

std::string trim(const std::string& text) {
    std::size_t start = text.find_first_not_of(BLANKS);
    if (start == std::string::npos) {
        return "";
    }
    std::size_t end = text.find_last_not_of(BLANKS);
    return text.substr(start, end - start + 1);
}

// Corta en MAX_CHAT_LENGTH bytes sin partir una letra de UTF-8 a la mitad: si el corte cae en
// un byte de continuacion (10xxxxxx), retrocede hasta el comienzo de esa letra.
std::string limit_length(const std::string& text) {
    if (text.size() <= MAX_CHAT_LENGTH) {
        return text;
    }
    std::size_t cut = MAX_CHAT_LENGTH;
    while (cut > 0 && (static_cast<unsigned char>(text[cut]) & 0xC0) == 0x80) {
        --cut;
    }
    return text.substr(0, cut);
}

}  // namespace

ChatCommand::ChatCommand(uint32_t player_id, const ChatRequest& request):
        GameCommand(player_id), text(request.text) {}

void ChatCommand::execute(GameContext& context) {
    Player* author = context.characters().find_player(player_id);
    std::string message = trim(limit_length(text));
    if (author == nullptr || message.empty()) {
        return;
    }
    if (message.front() == ServerConstants::MERCHANT_COMMAND_PREFIX) {
        TradeReply reply = context.commerce().trade(*author, message);
        context.sessions().send_to(player_id, reply.message);
        if (reply.inventory_changed) {
            context.sessions().send_to(player_id, author->get_inventory_state());
        }
        return;
    }
    if (message.front() == ServerConstants::CHEAT_PREFIX) {
        run_cheat(context, *author, message.substr(1));
        return;
    }
    if (message.front() == ServerConstants::PRIVATE_CHAT_PREFIX) {
        send_private(context, author->get_name(), message.substr(1));
        return;
    }
    context.sessions().broadcast(ChatMessage{ChatKind::Public, author->get_name(), message});
}

void ChatCommand::send_private(GameContext& context, const std::string& author,
                               const std::string& message) const {
    // El nombre del destinatario llega hasta el primer espacio; lo demas es el texto
    std::size_t space = message.find(' ');
    std::string target_name = message.substr(0, space);
    std::string body = space == std::string::npos ? "" : trim(message.substr(space + 1));
    std::optional<uint32_t> target = context.sessions().find_player_by_name(target_name);
    if (!target) {
        context.sessions().send_to(
                player_id,
                ChatMessage{ChatKind::System, "", target_name + ServerConstants::NOT_CONNECTED});
        return;
    }
    if (body.empty()) {
        return;
    }
    ChatMessage private_message{ChatKind::Private, author, body};
    context.sessions().send_to(*target, private_message);
    // Una copia para el que lo mando, asi ve lo que escribio (salvo que se lo mande a si mismo)
    if (*target != player_id) {
        context.sessions().send_to(player_id, private_message);
    }
}

void ChatCommand::run_cheat(GameContext& context, Player& author, const std::string& message) const {
    // El nombre llega hasta el primer espacio; lo demás es el payload
    std::size_t space = message.find(' ');
    std::string name = message.substr(0, space);
    std::string payload = space == std::string::npos ? "" : trim(message.substr(space + 1));
    const Cheat* cheat = cheat_for(name);
    if (cheat == nullptr) {
        context.sessions().send_to(
                player_id, ChatMessage{ChatKind::System, "", ServerConstants::UNKNOWN_CHEAT + name});
        return;
    }
    cheat->apply(author, context, payload);
}
