#ifndef HACKS_HPP
#define HACKS_HPP
#pragma once

#include<mmsystem.h>
#include "client.hpp"
#include "player_entity.hpp"
#include <iostream>

using namespace hazedumper;

struct GlowStruct {
	BYTE base[8];
	float red;
	float green;
	float blue;
	float alpha;
	BYTE buffer[16];
	bool redner_occluded;
	bool render_unoccluded;
	bool fullBloom;
	BYTE buffer1[5];
	int glow_style;
};

class Hacks {
private:
	Memory* memory;
	Client* client;
	PlayerEntity local_player;

public:
	bool bsp_setted;

	void init() {
		local_player = client->get_local_player();
	}

	bool hit(bool enable_when_spectate = false) {
		static int oldDamage = 0;
		static int last_spect_id = local_player.get_observer_target();

		PlayerEntity* player;
		if (!enable_when_spectate) {
			player = &local_player;
		}
		else {
			//判断local_player是否死亡
			if (local_player.get_health() == 0) {
				//查看local_player当前旁观的玩家id
				int spect_id = local_player.get_observer_target();

				//刚死亡还没旁观
				if (spect_id > 64) {
					spect_id = last_spect_id;
				}

				//获取被旁观的玩家
				player = &PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + signatures::dwEntityList + (short)0x10 * (spect_id - 1)));

				//切换旁观人物
				if (spect_id != last_spect_id) {
					last_spect_id = spect_id;
					//初始化数据
					oldDamage = player->get_round_deald_damage();
					return false;
				}
			}
			else {
				player = &local_player;
				last_spect_id = 0;
			}
		}

		if (!player->valid_player()) {
			return false;
		}

		//std::cout << "player health:" << player->get_health() <<std::endl;
		//std::cout << "old damage:" << oldDamage << std::endl;
		//std::cout << "damage:" << player->get_round_deald_damage() << std::endl;

		int damage = player ->get_round_deald_damage();

		//round start or restart reset the killnum
		if (oldDamage > damage) {
			oldDamage = 0;
			std::cout << std::endl<<"====================="<<std::endl;
		}
		else {
			if (damage > oldDamage && (damage - oldDamage < 500))
			{
				std::cout << "damage:" << oldDamage <<" + " <<damage -oldDamage<<" = " <<damage<< std::endl;
				oldDamage = damage;
				return true;
			}
		}
		return false;
	}

	bool kill(bool enable_when_spectate = false) {
		static int oldKillNum = 0;
		static int last_spect_id = local_player.get_observer_target();

		PlayerEntity* player;
		if (!enable_when_spectate) {
			player = &local_player;
		}
		else {
			//判断local_player是否死亡
			if (local_player.get_health() == 0) {
				//查看local_player当前旁观的玩家id
				int spect_id = local_player.get_observer_target();

				//刚死亡还没旁观
				if (spect_id < 64) {
					last_spect_id = spect_id;
				}

				//获取被旁观的玩家
				player = &PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + signatures::dwEntityList + (short)0x10 * (spect_id - 1)));
				
				//切换旁观人物
				if (spect_id != last_spect_id) {
					last_spect_id = spect_id;
					//初始化数据
					oldKillNum = player->get_round_kill_num();
					return false;
				}
			}
			else {
				player = &local_player;
				last_spect_id = 0;
			}
		}

		if (!player->valid_player()) {
			return false;
		}

		int killNum = player->get_round_kill_num();

		//round start or restart reset the killnum
		if (oldKillNum > killNum || killNum == 0) {
			oldKillNum = 0;
		}

		else {
			if (killNum > oldKillNum && ((killNum - oldKillNum < 500)))
			{
				oldKillNum = killNum;				
				std::cout << "kills:" << killNum << std::endl;
				return true;
			}
		}

		return false;
	}

	//this may not be functioning 
	bool flash(bool enable_when_spectate = false) {
		static int oldFlashNum = 0;
		static int last_spect_id = local_player.get_observer_target();

		PlayerEntity* player;
		if (!enable_when_spectate) {
			player = &local_player;
		}
		else {
			//判断local_player是否死亡
			if (local_player.get_health() == 0) {
				//查看local_player当前旁观的玩家id
				int spect_id = local_player.get_observer_target();

				//刚死亡还没旁观
				if (spect_id > 64) {
					spect_id = last_spect_id;
				}

				//获取被旁观的玩家
				player = &PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + signatures::dwEntityList + (short)0x10 * (spect_id - 1)));

				//切换旁观人物
				if (spect_id != last_spect_id) {
					last_spect_id = spect_id;
					//初始化数据
					oldFlashNum = player->get_flashed_enemies_num();
					return false;
				}
			}
			else {
				player = &local_player;
				last_spect_id = 0;
			}
		}

		if (!player->valid_player()) {
			return false;
		}

		int flashedNums = player->get_flashed_enemies_num();

		//round start or restart reset the killnum
		if (oldFlashNum > flashedNums || flashedNums == 0) {
			oldFlashNum = 0;
		}

		else {
			if (flashedNums > oldFlashNum && ( flashedNums - oldFlashNum < 500))
			{
				oldFlashNum = flashedNums;
				std::cout << "flashed enemies' num:" << flashedNums << std::endl;
				return true;
			}
		}

		return false;
	}

	void hitsound() {
		PlaySound(("hit.wav"), NULL, SND_ASYNC);
	}

	void killsound() {
		PlaySound(("kill.wav"), NULL, SND_ASYNC);
	}

	void flashsound() {
		PlaySound(("flash.wav"), NULL, SND_ASYNC);
	}
	
	//enable_when_spectate 旁观时是否启用
	void thread_hitmaker(hacks_state* state, bool enable_when_spectate) {
		while (true) {
			if (state->game) {
				bool hitted = false;
				bool killed = false;
				bool flashedEnemy = false;
				hitted = hit(enable_when_spectate);
				killed = kill(enable_when_spectate);
				flashedEnemy = flash(enable_when_spectate);
				if (killed) {
					killsound();
					Sleep(10);
					continue;
				}
				if (hitted) {
					hitsound();
					Sleep(10);
					continue;
				}
				if (flashedEnemy) {
					flashsound();
					Sleep(10);
					continue;
				}

				//std::cout << local_player.get_observer_target() << std::endl;
			}
			else Sleep(1000);

			Sleep(2);
		}
	}

	Hacks(Memory* memory, Client* client) {
		this->memory = memory;
		this->client = client;
		bsp_setted = false;
		init();
	}

	

};
#endif
