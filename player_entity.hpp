#ifndef PLAYER_ENTITY_HPP
#define PLAYER_ENTITY_HPP

#pragma once
#pragma warning(disable: 26451) // disabled a warning about arithmetic overflow (cast from float to double)

#define _USE_MATH_DEFINES // to access math consts like PI number
#include <math.h>
#include <time.h>
#include "memory.hpp"
#include "client.hpp"
#include "signatures.hpp"
#include "misc/config.hpp"

using namespace hazedumper;

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
		memory->write_mem<DWORD>(memory->clientBaseAddr + signatures::dwForceAttack, val);
	}

	// 4 - don't jump
	// 5 - jump
	// 6 - jump once, next reset to 4
	inline void set_jump_state(DWORD val) {
		memory->write_mem<DWORD>(memory->clientBaseAddr + signatures::dwForceJump, val);
	}

	inline void set_flash_duration(float val) {
		memory->write_mem<float>(playerBaseAddr + netvars::m_flFlashDuration, val);
	}

	inline void set_spotted(bool val) {
		memory->write_mem<bool>(playerBaseAddr + netvars::m_bSpotted, val);
	}

	inline bool is_spotted() {
		return memory->read_mem<bool>(playerBaseAddr + netvars::m_bSpotted);
	}

	// [0, 100]
	inline DWORD get_health() {
		return memory->read_mem<DWORD>(playerBaseAddr + netvars::m_iHealth);
	}

	inline int get_round_hit_times() {
		return memory->read_mem<int>(playerBaseAddr + netvars::currentRoundHitTimes);
	}

	inline int get_round_kill_num() {
		return memory->read_mem<int>(playerBaseAddr + netvars::m_iNumRoundKills);
	}

	inline int get_round_headshot_kill_num() {
		int current_round_index = get_round_index();
		return memory->read_mem<int>(playerBaseAddr + netvars::headshotKilledEnemiesAtFirstRound + current_round_index * 0x4);
	}

	inline int get_round_deald_damage() {
		return memory->read_mem<int>(playerBaseAddr + netvars::currentRoundDealdDamageTotal);
	}

	inline int get_flashed_enemies_num(int round_index) {
		return memory->read_mem<int>(playerBaseAddr + netvars::flashedEnemiesNumAtFirstRound + round_index * 0x4);
	}

	inline bool get_dormant() {
		return memory->read_mem<bool>(playerBaseAddr + signatures::m_bDormant);
	}

	inline DWORD get_crosshair_id() {
		return memory->read_mem<DWORD>(playerBaseAddr + netvars::m_iCrosshairId);
	}

	// 2 - terrorist
	// 3 - counter terrorist
	inline int get_team() {
		return memory->read_mem<int>(playerBaseAddr + netvars::m_iTeamNum);
	}

	//index from 0, the first round 's index is 0
	inline int get_round_index() {
		return memory->read_mem<int>(memory->read_mem<int>(memory->clientBaseAddr + signatures::dwGameRulesProxy) + signatures::m_totalRoundsPlayed);
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
		return memory->read_mem<BYTE>(playerBaseAddr + netvars::m_fFlags);
	}

	inline DWORD get_base_addr() {
		return this->playerBaseAddr;
	}

	inline coords_vector get_velocity() {
		return memory->read_mem<coords_vector>(playerBaseAddr + netvars::m_vecVelocity);
	}

	inline bool is_moving() {
		coords_vector vec = get_velocity();
		float sum = vec.x + vec.y + vec.z;
		return sum < -0.003 || sum > 0; // 0.003 due to incorrect float precision
	}

	//return player_id
	inline int get_observer_target() {
		int lpobstarget = memory->read_mem<int>(playerBaseAddr + netvars::m_hObserverTarget)& ENT_ENTRY_MASK;
		return lpobstarget;
	}

	void update_last_time_fire() {
		this->last_time_fire = time(0);
	}
};
#endif