#pragma once

#include <cstdint>
#include <cstring>
#include <string>
#include <unistd.h>
#include <vector>

#include "util/io.hpp"
#include "util/bitset.hpp"

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
	static constexpr uint8_t packet_id = 3;

	uint8_t slot;
	uint8_t server_flags = 0;
};

struct player_info {
	static constexpr uint8_t packet_id = 4;

	uint8_t 	slot;
	uint8_t 	skin_variant;
	uint8_t 	voice_variant;
	float 		voice_offset;
	uint8_t 	hair;
	std::string name;
	uint8_t 	hair_dye;
	bitset<16>	hidden_accessories; // FIXME: this is rounded number of flags. Terraria may use less than 16
	bitset<8>	hide_misc;			// same here
	rgb 		hair_color;
	rgb 		skin_color;
	rgb 		eye_color;
	rgb 		shirt_color;
	rgb 		undershirt_color;
	rgb 		pants_color;
	rgb 		shoe_color;
	bitset<4>	player_difficulty;
	bitset<5>	permanent_buffs;
	bitset<7>	permanent_buffs_shimmer;
};

struct player_inventory_slot {
	static constexpr uint8_t packet_id = 5;

	enum class slot_flags : int {
		Favourite = 0,
		BlockTransfer = 1,
	};

	uint8_t 	slot;
	uint16_t 	item_slot_id;

	uint16_t 	stack;
	uint8_t 	prefix;
	uint16_t 	type;
	bitset<2> 	flags;
};

inline nstring operator""_ns(const char* str, std::size_t)
{
	return nstring{nstring::Literal, str};
}

}
}

using net::packet::operator""_ns;
