#pragma once

#include <string>
#include <string_view>

enum class Base {
	Base16,
	Base32,
	Base64,
};

std::string encodeBaseN(std::string_view data, Base base = Base::Base64);
std::string decodeBaseN(std::string_view encoded, Base base = Base::Base64);
