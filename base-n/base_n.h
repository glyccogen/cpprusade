#pragma once

#include <stdexcept>
#include <string>
#include <string_view>

enum class Base {
	Base16,
	Base32,
	Base64,
};

constexpr std::string_view kBase32Alphabet = "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567";
constexpr std::string_view kBase16Alphabet = "0123456789ABCDEF";
constexpr std::string_view kBase64Alphabet =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::pair<std::size_t, std::string_view> getAlphaAndBlock(Base base) {
	switch(base) {
	case Base::Base16:
		return {4, kBase16Alphabet};
	case Base::Base32:
		return {5, kBase32Alphabet};
	case Base::Base64:
		return {6, kBase64Alphabet};
	}
}

std::string encodeBaseN(std::string_view data, Base base = Base::Base64) {
	auto [block_sz, alpha] = getAlphaAndBlock(base);
	std::string res;
	for(size_t l = 0; l < data.size() * 8; l += block_sz) {
		unsigned val = 0;
		for(size_t i = 0; i < block_sz; ++i) {
			size_t pos = i + l;
			val <<= 1;
			if(pos < data.size() * 8) {
				size_t byte_pos = pos / 8;
				size_t bit_in_byte_pos = pos % 8;
				auto byte = static_cast<unsigned char>(data[byte_pos]);
				unsigned bit = (byte >> (7 - bit_in_byte_pos)) & 1;
				val |= bit;
			}
		}
		res += alpha[val];
	}

	if(base != Base::Base16) {
		size_t block_sz = 4;
		if(base == Base::Base32) {
			block_sz = 8;
		}

		while(res.size() % block_sz != 0) {
			res += '=';
		}
	}

	return res;
}

std::string decodeBaseN(std::string_view encoded, Base base = Base::Base64) {
	auto [block_sz, alpha] = getAlphaAndBlock(base);
	std::string res;

	unsigned byte{};
	size_t cnt{}; // how many bits already in byte

	for(char c : encoded) {
		if(c == '=') {
			continue;
		}
		size_t val = alpha.find(c);
		if(val == std::string_view::npos) {
			throw std::invalid_argument{""};
		}

		for(size_t i = 0; i < block_sz; ++i) {
			unsigned bit = (static_cast<unsigned>(val) >> (block_sz - 1 - i)) & 1;
			byte = (byte << 1) | bit;
			++cnt;
			if(cnt == 8) {
				res += byte;
				byte = 0;
				cnt = 0;
			}
		}
	}

	if(encodeBaseN(res, base) != encoded) {
		throw std::invalid_argument{""};
	}

	return res;
}
