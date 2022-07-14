#ifndef PLAYER_ENTITY_HPP
#define PLAYER_ENTITY_HPP

#pragma once
#pragma warning(disable: 26451) // disabled a warning about arithmetic overflow (cast from float to double)

#define _USE_MATH_DEFINES // to access math consts like PI number
#include <math.h>
#include <time.h>
#include "memory.hpp"
#include "client.hpp"
#include "./signature/csgo.h"
#include "misc/config.hpp"
#include "bsp_parser/valve-bsp-parser/core/matrix.hpp"

using namespace offsets;
using namespace rn;

struct BoneMatrix {
	BYTE _junk1[0xC];
	float x;
	BYTE _junk2[0xC];
	float y;
	BYTE _junk3[0xC];
	float z;
};

const int MAX_EDICT_BITS = 11;
const int NUM_ENT_ENTRY_BITS = (MAX_EDICT_BITS + 1);
const int NUM_ENT_ENTRIES = (1 << NUM_ENT_ENTRY_BITS);
const int ENT_ENTRY_MASK = (NUM_ENT_ENTRIES - 1);

class PlayerEntity {
private:
	Memory* memory;
	DWORD playerBaseAddr;
	time_t last_time_fire;
	

public:
	PlayerEntity() {
		memory = nullptr;
		//client = nullptr;
		playerBaseAddr = NULL;
	}

	PlayerEntity(Memory* memory, DWORD playerBaseAddr) {
		this->memory = memory;
		this->playerBaseAddr = playerBaseAddr;
	}

	bool valid_player() {
		if (!memory || !playerBaseAddr) return false;

		DWORD health = this->get_health();
		return (health > 0 && health <= 100) && !get_dormant();
	}

	// 4 - don't attack
	// 5 - attack
	// 6 - attack once, next reset to 4
	inline void set_attack_state(DWORD val) {
		memory->write_mem<DWORD>(memory->clientBaseAddr + dwForceAttack, val);
	}

	// 4 - don't jump
	// 5 - jump
	// 6 - jump once, next reset to 4
	inline void set_jump_state(DWORD val) {
		memory->write_mem<DWORD>(memory->clientBaseAddr + dwForceJump, val);
	}

	inline void set_flash_duration(float val) {
		memory->write_mem<float>(playerBaseAddr + m_flFlashDuration, val);
	}

	inline void set_spotted(bool val) {
		memory->write_mem<bool>(playerBaseAddr + m_bSpotted, val);
	}

	inline bool is_spotted() {
		return memory->read_mem<bool>(playerBaseAddr + m_bSpotted);
	}

	// [0, 100]
	inline DWORD get_health() {
		return memory->read_mem<DWORD>(playerBaseAddr + m_iHealth);
	}

	inline int get_round_hit_times() {
		return memory->read_mem<int>(playerBaseAddr + currentRoundHitTimes);
	}

	inline int get_round_kill_num() {
		return memory->read_mem<int>(playerBaseAddr + m_iNumRoundKills);
	}

	inline int get_round_headshot_kill_num(int round_index) {
		return memory->read_mem<int>(playerBaseAddr + headshotKilledEnemiesAtFirstRound + round_index * 0x4);
	}

	inline int get_round_deald_damage() {
		return memory->read_mem<int>(playerBaseAddr + currentRoundDealdDamageTotal);
	}

	inline int get_flashed_enemies_num(int round_index) {
		return memory->read_mem<int>(playerBaseAddr + flashedEnemiesNumAtFirstRound + round_index * 0x4);
	}

	// player's x, y, z bone position
// bone_id - an ID in bone's vector, the most important ones are: 8 - head, 7 - neck, 6, 5, 4, 3, 0 - chest (from top)
	inline coords_vector get_bone_position(int bone_id) {
		DWORD bone_matrix = memory->read_mem<DWORD>(playerBaseAddr + offsets::m_dwBoneMatrix);
		BoneMatrix bone = memory->read_mem<BoneMatrix>(bone_matrix + bone_id * 0x30);
		return coords_vector({ bone.x, bone.y, bone.z });
	}

	// returns rn::vector3 bone position. needed for bsp parser
	inline vector3 get_bone_position_v3(int bone_id) {
		coords_vector cv = get_bone_position(bone_id);
		const std::array<float, 3U> vec = { cv.x, cv.y, cv.z };
		return vector3(vec);
	}

	inline bool get_dormant() {
		return memory->read_mem<bool>(playerBaseAddr + 
			
			m_bDormant);
	}

	inline DWORD get_crosshair_id() {
		return memory->read_mem<DWORD>(playerBaseAddr + m_iCrosshairId);
	}

	// 2 - terrorist
	// 3 - counter terrorist
	inline int get_team() {
		return memory->read_mem<int>(playerBaseAddr + m_iTeamNum);
	}

	//index from 0, the first round 's index is 0
	inline int get_round_index() {
		return memory->read_mem<int>(memory->read_mem<int>(memory->clientBaseAddr + dwGameRulesProxy) + m_totalRoundsPlayed);
	}

	// FL_ONGROUND   (1 << 0) on the ground
	// FL_DUCKING    (1 << 1) fully crouched
	// FL_WATERJUMP  (1 << 2) jumping out of water
	// FL_ONTRAIN    (1 << 3) _controlling_ a train
	// FL_INRAIN     (1 << 4) standing in rain
	// FL_FROZEN     (1 << 5) frozen
	// FL_ATCONTROLS (1 << 6) can't move
	// FL_CLIENT     (1 << 7) player
	// FL_FAKECLIENT (1 << 8) fake client
	// FL_INWATER    (1 << 9) in water
	inline BYTE get_flags() {
		return memory->read_mem<BYTE>(playerBaseAddr + m_fFlags);
	}

	inline DWORD get_base_addr() {
		return this->playerBaseAddr;
	}

	inline coords_vector get_velocity() {
		return memory->read_mem<coords_vector>(playerBaseAddr + m_vecVelocity);
	}

	inline bool is_moving() {
		coords_vector vec = get_velocity();
		float sum = vec.x + vec.y + vec.z;
		return sum < -0.003 || sum > 0; // 0.003 due to incorrect float precision
	}

	//return player_id
	inline int get_observer_target() {
		int lpobstarget = memory->read_mem<int>(playerBaseAddr + m_hObserverTarget)& ENT_ENTRY_MASK;
		return lpobstarget;
	}

	//get closet_enemy and distance
	float get_closest_enemy(PlayerEntity& enemy) {
		float closest_distance = 10000000; // initial closest distance between local player and an enemy
		int player_team = get_team();

		for (short i = 0; i < 32; ++i) {
			PlayerEntity target(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + offsets::dwEntityList + (short)0x10 * i));

			if (target.valid_player() && target.get_team() != player_team) {
				float distance = get_distance(target);
				if (distance < closest_distance) {
					closest_distance = distance;
					enemy = target;
				}
			}
		}

		return closest_distance;
	}

	// returns the distance between 2 player's entities (XYZ points)
// https://www.math.usm.edu/lambers/mat169/fall09/lecture17.pdf
	inline float get_distance(coords_vector target) {
		coords_vector local_player_vector3 = get_origin();
		return (float)sqrt(pow(target.x - local_player_vector3.x, 2)
			+ pow(target.y - local_player_vector3.y, 2)
			+ pow(target.z - local_player_vector3.z, 2));
	}

	inline float get_distance(PlayerEntity* target) {
		return get_distance(target->get_origin());
	}

	inline float get_distance(PlayerEntity target) {
		return get_distance(target.get_origin());
	}

	void update_last_time_fire() {
		this->last_time_fire = time(0);
	}

	coords_vector get_origin() {
		return get_origin(this);
	}

	inline coords_vector get_origin(PlayerEntity* player) {
		return memory->read_mem<coords_vector>(player->playerBaseAddr + offsets::m_vecOrigin);
	}
};
#endif