#ifndef CLIENT_HPP
#define CLIENT_HPP
#pragma once

#include "memory.hpp"
#include "./signature/csgo.h"
#include "player_entity.hpp"


using namespace offsets;

class Client {
private:
	Memory* memory;
	DWORD _survivalDecisionTypes;


	DWORD _getClientState() {
		return memory->read_mem<DWORD>(memory->engineBaseAddr + dwClientState);
	}
public:
	Client(Memory* memory) {
		this->memory = memory;
		_survivalDecisionTypes = 0;
	}

	// Client states: LOBBY = 1, LOADING = 2, CONNECTING = 3, CONNECTED = 5, INGAME = 6
	bool in_game() {
		if (memory->tProcess != NULL) {
			if (memory->tPID != Memory::get_porcId_by_name(TARGET)) {
				memory->~Memory();
				return false;
			}
			return memory->read_mem<DWORD>(_getClientState() + dwClientState_State) == 6;
		}
		return false;
	}

	PlayerEntity get_local_player() {
		DWORD player_id = memory->read_mem<DWORD>(_getClientState() + dwClientState_GetLocalPlayer);
		return PlayerEntity(memory,memory->read_mem<DWORD>(memory->clientBaseAddr + dwEntityList + player_id * 0x10));
	}

	bool is_dangerzone() {
		return _survivalDecisionTypes != 0;
	}

	void update_gamemode() {
		DWORD gameRulesProxy = memory->read_mem<DWORD>(memory->clientBaseAddr + dwGameRulesProxy);
		_survivalDecisionTypes = memory->read_mem<DWORD>(gameRulesProxy + m_SurvivalGameRuleDecisionTypes);
	}



	void set_sensitivity(float val) {
		uint32_t sens_pt = memory->read_mem<uint32_t>(memory->clientBaseAddr + dwSensitivityPtr);
		uint32_t sest_fn = *reinterpret_cast<uint32_t*>(&val) ^ sens_pt;

		memory->write_mem<uint32_t>(memory->clientBaseAddr + dwSensitivity, sest_fn);
	}

	float get_sensitivity() {
		uint32_t sens_pt = memory->read_mem<uint32_t>(memory->clientBaseAddr + dwSensitivityPtr);
		uint32_t sens_fn = memory->read_mem<uint32_t>(memory->clientBaseAddr + dwSensitivity);

		sens_fn ^= sens_pt;

		return *reinterpret_cast<float*>(&sens_fn);
	}

	//index from 0, the first round 's index is 0
	int get_round_index() {
		return memory->read_mem<int>(memory->read_mem<int>(memory->clientBaseAddr + dwGameRulesProxy) + m_totalRoundsPlayed);
	}
};

#endif