#pragma once

#include <array>
#include <cstdint>
#include <cstdio>
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


template<class T>
struct plist {
	std::vector<T> v;

	template <class K>
	ssize_t read(io::serialized_io<K>& io) {
		uint8_t len;

		auto offset = io.read(len);
		v.resize(len);
		offset += io.read(v);

		return offset;
	}
	
	template <class K>
	ssize_t write(io::serialized_io<K>& io) {
		auto offset = io.write(uint8_t(v.size()));
		offset += io.write(v);
		return offset;
	}
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

struct request_world_info {
	static constexpr uint8_t packet_id = 6;
};

struct world_info {
	static constexpr uint8_t packet_id = 7;

	int32_t time;
	bitset<3> time_flags; // day? blood moon? eclipse?
	uint8_t moon_phase;
	vec2<uint16_t> tiles_limit;
	vec2<uint16_t> spawn_tile;
	uint16_t surface_level;
	uint16_t rock_level;
	int32_t worldid;
	std::string worldname;
	uint8_t gamemode;
	std::array<uint8_t, 16> uuid;
	uint64_t worldgen_version;
	uint8_t moon_type;
	std::array<uint8_t, 13> backgrounds;
	std::array<uint8_t, 3> bg_styles;

	float wind_speed_target;
	uint8_t num_clouds;

	std::array<int32_t, 3> trees;
	std::array<uint8_t, 4> trees_styles;
	std::array<int32_t, 3> cave_bg;
	std::array<uint8_t, 4> cave_bg_styles;

	std::array<uint8_t, 13> tree_tops;

	float max_raining;

	bitset<81> flags; // TODO: document them

	uint8_t sundial_cooldown;
	uint8_t moondial_cooldown;

	std::array<uint16_t, 7> world_ores;
	int8_t invasion_type;

	uint64_t lobby_id = 0;

	float sandstorm_severity;
	plist<vec2<uint16_t>> extra_spawn_points;
};

struct request_spawn {
	static constexpr uint8_t packet_id = 8;
	
	vec2<int32_t>  	tile;
	uint8_t    		team;
};

struct set_loading_message {
	static constexpr uint8_t packet_id = 9;
	
	int32_t sections;
	nstring message;
	uint8_t flags = 0;
};

inline nstring operator""_ns(const char* str, std::size_t)
{
	return nstring{nstring::Literal, str};
}

}
}

using net::packet::operator""_ns;
