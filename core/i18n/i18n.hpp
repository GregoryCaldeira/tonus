#pragma once
// Runtime string lookup. Keys come from assets/strings/*.json via tools/i18n/gen_strings.py.

#include <initializer_list>
#include <string>
#include <string_view>
#include <utility>

#include "tonus_strings_gen.hpp"

namespace tonus::i18n {

void setLang(Lang lang);
Lang lang();

const char* tr(Str id);

// Replaces {name} placeholders: format(Str::diag_chip, {{"value", "v1.3"}}).
std::string format(Str id, std::initializer_list<std::pair<std::string_view, std::string>> args);

}  // namespace tonus::i18n
