#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>

#include "util/io.hpp"

namespace net {
namespace packet {

using packet_flag = struct { };

template <class T>
concept packet = requires(T x) { x.packet_id; };

template <class T>
concept compressed_packet = packet<T> && requires(T x) { x.compressed; };

template <class T>
concept netmodule = requires(T x) { x.module_id; };

struct nstring {
	enum ns_type : uint8_t {
		Literal,
		Formattable,
		LocalizationKey,
	} type;

	std::string text;
	std::vector<std::string> substitutions;

	template <class T>
	ssize_t read(io::serialized_io<T>& io) {
		uint8_t t;
		auto offset = io.read(t);
		type = (ns_type(t));

		offset += io.read(text);
		
		uint8_t subs_len;
		offset += io.read(subs_len);
		if (subs_len > 0) {
			substitutions.resize(subs_len);
			io.read(substitutions);
		}
		
		return offset;
	}
	
	template <class T>
	ssize_t write(io::serialized_io<T>& io) {
		auto offset = io.write(uint8_t(type));
		offset += io.write(text);
		offset += io.write(uint8_t(substitutions.size()));
		offset += io.write(substitutions);
		return offset;
	}
};

struct rgb {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

template<class T>
struct vec2 {
	T x;
	T y;
};

// NetMessages

struct connect {
	static constexpr uint8_t packet_id = 1;
	
	std::string version;
};

struct disconnect {
	static constexpr uint8_t packet_id = 2;

	nstring reason;
};

struct accept {
	uint8_t slot;
	uint8_t server_flags = 0;
};

inline nstring operator""_ns(const char* str, std::size_t)
{
	return nstring{nstring::Literal, str};
}

}
}

using net::packet::operator""_ns;
