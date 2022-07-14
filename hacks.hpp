#ifndef HACKS_HPP
#define HACKS_HPP
#pragma once

#include"sound.hpp"
#include "client.hpp"
#include "player_entity.hpp"
#include <iostream>

using namespace offsets;

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
	hacks_state* state;
	PlayerEntity local_player;

public:
	bool bsp_setted;

	Hacks(Memory* memory, Client* client, hacks_state* state) {
		this->memory = memory;
		this->client = client;
		this->state = state;
		bsp_setted = false;
		init();
	}

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
					return false;
				}

				//获取被旁观的玩家
				player = &PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + dwEntityList + (short)0x10 * (spect_id - 1)));

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

	/*
	return:
		0 not killed
		1 normal kill
		2 headshot kill 
	*/
	int kill(bool enable_when_spectate = false) {
		static int oldKillNum = 0;
		static int oldHeadShotKillNum = 0;
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
					return false;
				}

				//获取被旁观的玩家
				player = &PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + 
					dwEntityList + (short)0x10 * (spect_id - 1)));
				
				//切换旁观人物
				if (spect_id != last_spect_id) {
					last_spect_id = spect_id;
					//初始化数据
					oldKillNum = player->get_round_kill_num();
					oldHeadShotKillNum = player->get_round_headshot_kill_num(state->round_index);
					return 0;
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
			oldHeadShotKillNum = 0;
		}

		else {
			if (killNum > oldKillNum && ((killNum - oldKillNum < 500)))
			{
				oldKillNum = killNum;

				int round_index = state->round_index;

				//回合刚结束
				if (time(NULL) - state->last_time_round_index_changed < 1) {
					round_index--;
				}

				//wait for headshot kill data been updated
				Sleep(20);
				int headshot_killNum = player->get_round_headshot_kill_num(round_index);
				if (headshot_killNum > oldHeadShotKillNum) {
					oldHeadShotKillNum = headshot_killNum;
					std::cout << "kills:" << killNum << " (that was a headshot kill)" << std::endl;
					std::cout << "oldHeadShotKillNum:" << oldHeadShotKillNum << " " << std::endl;
					return 2;
				}
				std::cout << "headshot_kill" << headshot_killNum << std::endl;
				std::cout << "kills:" << killNum << "oldHeadShotKillNum:" << oldHeadShotKillNum << " " << std::endl;

				return 1;
			}
		}

		return 0;
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
					return false;
				}

				//获取被旁观的玩家
				player = &PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + dwEntityList + (short)0x10 * (spect_id - 1)));

				//切换旁观人物
				if (spect_id != last_spect_id) {
					last_spect_id = spect_id;
					//初始化数据
					oldFlashNum = player->get_flashed_enemies_num(state->round_index);
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

		int flashedNums = player->get_flashed_enemies_num(state->round_index);

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

	//get enemy position(vector3) and transform to screen pos(vector2) using world_to_screen function
	void sonar_crosshair(std::vector<std::string>  sonar_crosshair_beep_files) {
		//TODO
	}

	//get closest enemy
	void sonar_range(std::vector<std::string> sonar_rangescan_beep_files) {
		//TODO
		//1 secs at least, between two played sounds
		//最低1秒时间间隔
		int min_interval = 1;
		int max_interval = 2;
		//enemies within distance of 300 will trigger sonar
		//距离在300内才会有声呐
		int max_threshold_distance = 300;
		int min_threshold_distance = 10;

		static time_t last_played_sound_time = time(NULL);
		PlayerEntity enemy;
		float distance_to_closest_enemy = local_player.get_closest_enemy(enemy);

		if (distance_to_closest_enemy > max_threshold_distance) {
			return;
		}

		time_t current_time = time(NULL);
		time_t time_delta = current_time - last_played_sound_time;

		//interval too short
		if (time_delta < min_interval) {
			return;
		}

		sonar_range_scan_sound(sonar_rangescan_beep_files);
		if (distance_to_closest_enemy - min_threshold_distance > 0) {
			//delay the next sound play time
			last_played_sound_time += (max_interval - min_interval)
				* (distance_to_closest_enemy - min_threshold_distance)
				/ (max_threshold_distance - min_threshold_distance);
		}
	}
	
	//enable_when_spectate 旁观时是否启用
	void thread_hitmaker(hacks_state* state, bool enable_when_spectate) {
		static std::vector<std::string> hit_sound_files;
		static std::vector<std::string> kill_sound_files;
		static std::vector<std::string> headshotkill_sound_files;
		static std::vector<std::string> headshot_hit_sound_files;
		static std::vector<std::string> flash_sound_files;

		init_sounds_files_of_hit_and_kill(hit_sound_files,
							kill_sound_files,
							headshotkill_sound_files,
							headshot_hit_sound_files,
							flash_sound_files);

		while (true) {
			if (state->game) {
				bool hitted = false;
				int killed = false;
				bool flashedEnemy = false;
				hitted = hit(enable_when_spectate);
				killed = kill(enable_when_spectate);
				flashedEnemy = flash(enable_when_spectate);
				if (killed!=0) {
					if (killed == 1) {

						kill_sound(kill_sound_files);
					}
					else if (killed == 2) {
						headshotkill_sound(headshotkill_sound_files);
					}

					Sleep(10);
					continue;
				}
				if (hitted) {
					hit_sound(hit_sound_files);
					Sleep(10);
					continue;
				}
				if (flashedEnemy) {
					flash_sound(flash_sound_files);
					Sleep(10);
					continue;
				}

				//std::cout << local_player.get_observer_target() << std::endl;
			}
			else Sleep(1000);

			Sleep(2);
		}
	}

	void thread_sonar(hacks_state* state) {
		static std::vector<std::string> sonar_crosshair_beep_files;//beep when crosshair near enemy
		static std::vector<std::string> sonar_rangescan_beep_files;//beep when enemy nearby

		init_sounds_files_of_sonar(sonar_crosshair_beep_files,sonar_rangescan_beep_files);

		while (true) {
			if (state->game) {				
				sonar_crosshair(sonar_crosshair_beep_files);
				sonar_range(sonar_rangescan_beep_files);
			}
			else Sleep(1000);

			Sleep(2);
		}
	}


};
#endif
