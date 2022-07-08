#ifndef HACKS_HPP
#define HACKS_HPP
#pragma once

#include <algorithm>
#include<random>
#include<mmsystem.h>
#include "client.hpp"
#include "player_entity.hpp"
#include <iostream>
#include<filesystem>

using namespace hazedumper;
namespace fs = std::filesystem;

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
					last_spect_id = spect_id;
				}

				//获取被旁观的玩家
				player = &PlayerEntity(memory, memory->read_mem<DWORD>(memory->clientBaseAddr + signatures::dwEntityList + (short)0x10 * (spect_id - 1)));
				
				//切换旁观人物
				if (spect_id != last_spect_id) {
					last_spect_id = spect_id;
					//初始化数据
					oldKillNum = player->get_round_kill_num();
					oldHeadShotKillNum = player->get_round_headshot_kill_num();
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

				//wait for headshot kill data been updated
				Sleep(20);
				int headshot_killNum = player->get_round_headshot_kill_num();
				if (headshot_killNum > oldHeadShotKillNum) {
					oldHeadShotKillNum = headshot_killNum;
					std::cout << "kills:" << killNum <<" (that was a headshot kill)" << std::endl;
					return 2;
				}
				std::cout << "headshot_kill" << headshot_killNum << std::endl;
				std::cout << "kills:" << killNum << std::endl;
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

	void shuffle_vector(std::vector<std::string>& vector) {
		std::random_device rd;
		std::shuffle(vector.begin(), vector.end(), rd);
	}

	void hit_sound(std::string filename) {
		PlaySound(((std::string(".\\sounds\\hit\\") + filename).c_str()), NULL, SND_ASYNC);
	}

	void hit_sound(std::vector<std::string>& filenames) {
		shuffle_vector(filenames);
		PlaySound(((std::string(".\\sounds\\hit\\") + filenames.front()).c_str()), NULL, SND_ASYNC);
	}

	void kill_sound(std::string filename) {
		PlaySound(((std::string(".\\sounds\\kill\\") + filename).c_str()), NULL, SND_ASYNC);
	}	
	
	void kill_sound(std::vector<std::string>& filenames) {
		shuffle_vector(filenames);
		PlaySound(((std::string(".\\sounds\\kill\\") + filenames.front()).c_str()), NULL, SND_ASYNC);
	}

	void headshotkill_sound(std::string filename) {
		PlaySound(((std::string(".\\sounds\\headhshot_kill\\") + filename).c_str()), NULL, SND_ASYNC);
	}

	void headshotkill_sound(std::vector<std::string>& filenames) {
		shuffle_vector(filenames);
		PlaySound(((std::string(".\\sounds\\headshot_kill\\") + filenames.front()).c_str()), NULL, SND_ASYNC);
	}

	void headshot_hit_sound(std::string filename) {
		PlaySound(((std::string(".\\sounds\\headshot_hit\\") + filename).c_str()), NULL, SND_ASYNC);
	}	
	
	void headshot_hit_sound(std::vector<std::string>& filenames) {
		shuffle_vector(filenames);
		PlaySound(((std::string(".\\sounds\\headshot_hit\\") + filenames.front()).c_str()), NULL, SND_ASYNC);
	}

	void flash_sound(std::string filename) {
		PlaySound(((std::string(".\\sounds\\flash\\") + filename).c_str()), NULL, SND_ASYNC);
	}
	
	void flash_sound(std::vector <std::string>& filenames) {
		shuffle_vector(filenames);
		PlaySound(((std::string(".\\sounds\\flash\\") + filenames.front()).c_str()), NULL, SND_ASYNC);
	}

	void get_all_wav_files(std::string path, std::vector<std::string>& sound_files_container) {
		try{
			for (const auto& entry : fs::directory_iterator(path)) {
				static std::string suffix = std::string(".wav");

				std::string filename = entry.path().filename().string();

				bool has_suffix = filename.find(suffix, filename.size() - suffix.size()) != std::string::npos;
				if (has_suffix) {
					sound_files_container.push_back(filename);
				}
			}
		}
		catch (const std::exception& ex){

		}
	}

	void init_sounds_files(
		std::vector<std::string>& hit_sound_files,
		std::vector<std::string>& kill_sound_files,
		std::vector<std::string>& headshotkill_sound_files,
		//TODO headshot_hit_sound_files
		std::vector<std::string>& headshot_hit_sound_files,
		std::vector<std::string>& flash_sound_files) {
		get_all_wav_files("./sounds/hit", hit_sound_files);
		get_all_wav_files("./sounds/kill",kill_sound_files);
		get_all_wav_files("./sounds/headshot_kill",headshotkill_sound_files);
		get_all_wav_files("./sounds/headshot_hit",headshot_hit_sound_files);
		get_all_wav_files("./sounds/flash",flash_sound_files);
	}
	
	//enable_when_spectate 旁观时是否启用
	void thread_hitmaker(hacks_state* state, bool enable_when_spectate) {
		static std::vector<std::string> hit_sound_files;
		static std::vector<std::string> kill_sound_files;
		static std::vector<std::string> headshotkill_sound_files;
		static std::vector<std::string> headshot_hit_sound_files;
		static std::vector<std::string> flash_sound_files;

		init_sounds_files(hit_sound_files,
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

	Hacks(Memory* memory, Client* client) {
		this->memory = memory;
		this->client = client;
		bsp_setted = false;
		init();
	}

	

};
#endif
