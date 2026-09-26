#include "i18n/i18n.hpp"

namespace tonus::i18n {
namespace {
Lang g_lang = Lang::en;
}

void setLang(Lang l) {
    if (static_cast<int>(l) < static_cast<int>(Lang::Count)) g_lang = l;
}

Lang lang() { return g_lang; }

const char* tr(Str id) {
    const int i = static_cast<int>(id);
    if (i < 0 || i >= static_cast<int>(Str::Count)) return "";
    return kTable[static_cast<int>(g_lang)][i];
}

std::string format(Str id, std::initializer_list<std::pair<std::string_view, std::string>> args) {
    std::string out = tr(id);
    for (const auto& [name, value] : args) {
        const std::string token = "{" + std::string(name) + "}";
        for (size_t pos = out.find(token); pos != std::string::npos; pos = out.find(token, pos + value.size()))
            out.replace(pos, token.size(), value);
    }
    return out;
}

}  // namespace tonus::i18n
