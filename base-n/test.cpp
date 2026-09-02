#include "base_n.h"

#include "../.for_tests/catch.hpp"

#include <array>
#include <stdexcept>
#include <string>
#include <string_view>

namespace {

struct TestVector {
	std::string_view decoded;
	std::string_view encoded;
};

void checkVectors(Base base, auto const & vectors) {
	for(auto const & [decoded, encoded] : vectors) {
		REQUIRE(encodeBaseN(decoded, base) == encoded);
		REQUIRE(decodeBaseN(encoded, base) == decoded);
	}
}

void checkInvalid(Base base, auto const & values) {
	for(std::string_view value : values) {
		REQUIRE_THROWS_AS(decodeBaseN(value, base), std::invalid_argument);
	}
}

} // namespace

TEST_CASE("RFC 4648 test vectors") {
	std::array const base16{
	        TestVector{"", ""},
	        TestVector{"f", "66"},
	        TestVector{"fo", "666F"},
	        TestVector{"foo", "666F6F"},
	        TestVector{"foob", "666F6F62"},
	        TestVector{"fooba", "666F6F6261"},
	        TestVector{"foobar", "666F6F626172"},
	};
	checkVectors(Base::Base16, base16);

	std::array const base32{
	        TestVector{"", ""},
	        TestVector{"f", "MY======"},
	        TestVector{"fo", "MZXQ===="},
	        TestVector{"foo", "MZXW6==="},
	        TestVector{"foob", "MZXW6YQ="},
	        TestVector{"fooba", "MZXW6YTB"},
	        TestVector{"foobar", "MZXW6YTBOI======"},
	};
	checkVectors(Base::Base32, base32);

	std::array const base64{
	        TestVector{"", ""},
	        TestVector{"f", "Zg=="},
	        TestVector{"fo", "Zm8="},
	        TestVector{"foo", "Zm9v"},
	        TestVector{"foob", "Zm9vYg=="},
	        TestVector{"fooba", "Zm9vYmE="},
	        TestVector{"foobar", "Zm9vYmFy"},
	};
	checkVectors(Base::Base64, base64);
}

TEST_CASE("Default base is Base64") {
	REQUIRE(encodeBaseN("foobar") == "Zm9vYmFy");
	REQUIRE(decodeBaseN("Zm9vYmFy") == "foobar");
}

TEST_CASE("Alphabets") {
	std::string const base16_data{"\x01\x23\x45\x67\x89\xAB\xCD\xEF", 8};
	REQUIRE(encodeBaseN(base16_data, Base::Base16) == "0123456789ABCDEF");

	std::string const base32_data{
	        "\x00\x44\x32\x14\xC7\x42\x54\xB6\x35\xCF\x84\x65\x3A\x56\xD7\xC6"
	        "\x75\xBE\x77\xDF",
	        20};
	REQUIRE(encodeBaseN(base32_data, Base::Base32) == "ABCDEFGHIJKLMNOPQRSTUVWXYZ234567");

	std::string const base64_data{
	        "\x00\x10\x83\x10\x51\x87\x20\x92\x8B\x30\xD3\x8F\x41\x14\x93\x51"
	        "\x55\x97\x61\x96\x9B\x71\xD7\x9F\x82\x18\xA3\x92\x59\xA7\xA2\x9A"
	        "\xAB\xB2\xDB\xAF\xC3\x1C\xB3\xD3\x5D\xB7\xE3\x9E\xBB\xF3\xDF\xBF",
	        48};
	REQUIRE(encodeBaseN(base64_data, Base::Base64) ==
	        "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/");
}

TEST_CASE("Binary data") {
	std::string data;
	for(int value = 0; value < 256; ++value) {
		data.push_back(static_cast<char>(value));
	}

	for(Base base : {Base::Base16, Base::Base32, Base::Base64}) {
		for(size_t size = 0; size <= data.size(); ++size) {
			std::string_view const prefix{data.data(), size};
			REQUIRE(decodeBaseN(encodeBaseN(prefix, base), base) == prefix);
		}
	}
}

TEST_CASE("Padding") {
	REQUIRE(encodeBaseN("sure.", Base::Base32) == "ON2XEZJO");
	REQUIRE(encodeBaseN("sure", Base::Base32) == "ON2XEZI=");
	REQUIRE(encodeBaseN("sur", Base::Base32) == "ON2XE===");
	REQUIRE(encodeBaseN("su", Base::Base32) == "ON2Q====");
	REQUIRE(encodeBaseN("s", Base::Base32) == "OM======");

	REQUIRE(encodeBaseN("any carnal pleasure.", Base::Base64) ==
	        "YW55IGNhcm5hbCBwbGVhc3VyZS4=");
	REQUIRE(encodeBaseN("any carnal pleasure", Base::Base64) == "YW55IGNhcm5hbCBwbGVhc3VyZQ==");
	REQUIRE(encodeBaseN("any carnal pleasur", Base::Base64) == "YW55IGNhcm5hbCBwbGVhc3Vy");
}

TEST_CASE("Invalid Base16") {
	std::array const invalid{
	        "0", "000", "FG", "ab", "00=0", "00 00", "00\n00",
	};
	checkInvalid(Base::Base16, invalid);
}

TEST_CASE("Invalid Base32") {
	std::array const invalid{
	        "M=======", "MZX=====", "MZXW6Y==",         "MZXW6YQ",  "MZXW6YQ==",
	        "========", "=ZXW6YQ=", "MZXW=YQ=",         "MZXW6YQ1", "mzxw6ytb",
	        "MZXW6Y T", "MZ======", "MY======MY======",
	};
	checkInvalid(Base::Base32, invalid);
}

TEST_CASE("Invalid Base64") {
	std::array const invalid{
	        "A",    "AA",       "AAA",   "Zg=",   "Zg===",  "====", "=m9v",
	        "Zm=v", "Zg==Zg==", "Zm9v-", "Zm 9v", "Zm9v\n", "Zh==", "Zm9=",
	};
	checkInvalid(Base::Base64, invalid);
}
