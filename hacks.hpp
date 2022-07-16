#ifndef HACKS_HPP
#define HACKS_HPP
#pragma once

#include"sound.hpp"
#include "client.hpp"
#include "player_entity.hpp"
#include <iostream>
#include<chrono>

using namespace offsets;
using namespace std::chrono;

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

	struct view_matrix_t {
		float matrix[16];
	};

	coords_vector WorldToScreen(const coords_vector pos, struct view_matrix_t matrix, int screen_width, int screen_height) {
		struct coords_vector out;
		float _x = matrix.matrix[0] * pos.x + matrix.matrix[1] * pos.y + matrix.matrix[2] * pos.z + matrix.matrix[3];
		float _y = matrix.matrix[4] * pos.x + matrix.matrix[5] * pos.y + matrix.matrix[6] * pos.z + matrix.matrix[7];
		out.z = matrix.matrix[12] * pos.x + matrix.matrix[13] * pos.y + matrix.matrix[14] * pos.z + matrix.matrix[15];

		_x *= 1.f / out.z;
		_y *= 1.f / out.z;

		out.x = screen_width * .5f;
		out.y = screen_height * .5f;

		out.x += 0.5f * _x * screen_width + 0.5f;
		out.y -= 0.5f * _y * screen_height + 0.5f;

		return out;
	}

	float get_min_distance_of_player_to_crosshair(PlayerEntity player, view_matrix_t view_matrix, int width, int height) {
		float min_distance = 999999999.f;

		//get bone ids 8 7 6 5 4 3 0
		coords_vector pos_origin = player.get_origin();

		std::vector<coords_vector> pos_of_bones;
		pos_of_bones.push_back(pos_origin);
		for (int i = 0; i <= 8; i++) {
			pos_of_bones.push_back(player.get_bone_position(i));
		}

		//iterate all players bone position on screen
		for (coords_vector pos : pos_of_bones) {
			coords_vector screen_pos = WorldToScreen(pos, view_matrix, width, height);
			float distance_to_crosshair = sqrt(pow(screen_pos.x - width / 2, 2) + pow(screen_pos.y - height / 2, 2));
			if (distance_to_crosshair < min_distance) {
				min_distance = distance_to_crosshair;
			}
		}

		return min_distance;
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
	void sonar_crosshair(hacks_state* state, std::vector<std::string>  sonar_crosshair_beep_files) {
		//100ms at least, between two played sounds
		//最低100毫秒时间间隔
		uint64_t min_interval = 100;
		uint64_t max_interval = 400;
		//enemies within distance to crosshair of 200 will trigger sonar
		//距离准心在200内才会有
		int max_threshold_distance_to_crosshair = 200;
		int min_threshold_distance_to_crosshair = 20;
		static uint64_t last_played_sound_time_millis = 0;

		int localteam = memory->read_mem<int>(memory->read_mem<DWORD>(memory->clientBaseAddr + dwEntityList) + m_iTeamNum);
		view_matrix_t vm = memory->read_mem<view_matrix_t>(memory->clientBaseAddr + dwViewMatrix);
		//screen resolution: 2160 1440
		int screen_width = 2160;
		int screen_height = 1440;

		uint64_t current_time = timeSinceEpochMillisec();
		uint64_t time_delta;




		for (int i = 1; i < 64; i++) {
			if (last_played_sound_time_millis >= current_time) {
				time_delta = 0;
			}
			else {
				time_delta = current_time - last_played_sound_time_millis;
			}

			//interval too short
			if (time_delta <= min_interval) {
				//std::cout << "interval too short" << std::endl;
				continue;
			}

			PlayerEntity pEnt = PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr+ dwEntityList + (i * 0x10)));
			int team = pEnt.get_team();

			if (team != localteam) {
				int health = pEnt.get_health();

				coords_vector pos = pEnt.get_origin();

				coords_vector screenpos = WorldToScreen(pos, vm, screen_width, screen_height);

				if (screenpos.z >= 0.01f && health > 0 && health < 101 && !pEnt.get_dormant()) {
					//get min distance to playerEntity
					float min_distance = get_min_distance_of_player_to_crosshair(pEnt, vm, screen_width, screen_height);

					if (min_distance < max_threshold_distance_to_crosshair) {
						sonar_crosshair_sound(sonar_crosshair_beep_files);
						state->sonar_crosshair_last_beep_time = current_time;
						last_played_sound_time_millis = current_time;

						//delay the next sound play time
						uint64_t delay = (max_interval - min_interval)
							* (min_distance - min_threshold_distance_to_crosshair)
							/ (max_threshold_distance_to_crosshair - min_threshold_distance_to_crosshair);
						if (delay < min_interval) {
							delay = min_interval;
						}
						last_played_sound_time_millis = last_played_sound_time_millis + delay;
						//std::cout << "delay secs: " << delay << std::endl;
						//std::cout << "last_played_sound_time :" << last_played_sound_time_millis << std::endl;
					}
				}
			}
		}
	}

	uint64_t timeSinceEpochMillisec() {
		using namespace std::chrono;
		return duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
	}

	//get closest enemy
	void sonar_range(hacks_state* state, std::vector<std::string> sonar_rangescan_beep_files) {
		//400ms at least, between two played sounds
		//最低400毫秒时间间隔
		uint64_t min_interval = 400;
		uint64_t max_interval = 2 * 1000;
		//enemies within distance of 300 will trigger sonar
		//距离在300内才会有声呐
		int max_threshold_distance = 600;
		int min_threshold_distance = 20;

		static uint64_t last_played_sound_time_millis = 0;
		PlayerEntity enemy;
		float distance_to_closest_enemy = local_player.get_closest_enemy(enemy);

		if (distance_to_closest_enemy > max_threshold_distance) {
			return;
		}

		uint64_t current_time = timeSinceEpochMillisec();
		uint64_t time_delta;
		if (last_played_sound_time_millis >= current_time) {
			time_delta = 0;
		}
		else {
			time_delta = current_time - last_played_sound_time_millis;
		}

		//interval too short
		if (time_delta <= min_interval || (current_time - state->sonar_crosshair_last_beep_time) < 1000) {
			//std::cout << "interval too short" << std::endl;
			return;
		}

		std::cout << "time_delta: " << time_delta << std::endl;

		sonar_range_scan_sound(sonar_rangescan_beep_files);
		last_played_sound_time_millis = current_time;
		std::cout << "Enemy nearby, distance: " << distance_to_closest_enemy <<" time_now: "<< current_time << std::endl;
		if (distance_to_closest_enemy - min_threshold_distance > 0) {
			//delay the next sound play time
			uint64_t delay = (max_interval - min_interval)
				* (distance_to_closest_enemy - min_threshold_distance)
				/ (max_threshold_distance - min_threshold_distance);
			if (delay < min_interval) {
				delay = min_interval;
			}
			last_played_sound_time_millis = last_played_sound_time_millis + delay;
						std::cout << "delay secs: " << delay << std::endl;
						std::cout << "last_played_sound_time :"<< last_played_sound_time_millis << std::endl;
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

		init_sounds_files_of_sonar(sonar_crosshair_beep_files, sonar_rangescan_beep_files);

		while (true) {
			if (state->game) {
				//sonar_crosshair(state, sonar_crosshair_beep_files);
				sonar_range(state, sonar_rangescan_beep_files);
			}
			else Sleep(1000);

			Sleep(2);
		}
	}


};
#endif
