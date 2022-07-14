#ifndef SOUND_HPP
#define SOUND_HPP
#pragma once

#include <algorithm>
#include<random>
#include<mmsystem.h>
#include<filesystem>

namespace fs = std::filesystem;

void shuffle_vector(std::vector<std::string>& vector) {
	std::random_device rd;
	std::shuffle(vector.begin(), vector.end(), rd);
}

void sonar_crosshair_sound(std::string filename) {
	PlaySound(((std::string(".\\sounds\\sonar\\crosshair_beep") + filename).c_str()), NULL, SND_ASYNC);
}

void sonar_crosshair_sound(std::vector<std::string>& filenames) {
	shuffle_vector(filenames);
	PlaySound(((std::string(".\\sounds\\sonar\\crosshair_beep") + filenames.front()).c_str()), NULL, SND_ASYNC);
}

void sonar_range_scan_sound(std::string filename) {
	PlaySound(((std::string(".\\sounds\\sonar\\range_scan_beep\\") + filename).c_str()), NULL, SND_ASYNC);
}

void sonar_range_scan_sound(std::vector<std::string>& filenames) {
	shuffle_vector(filenames);
	PlaySound(((std::string(".\\sounds\\sonar\\range_scan_beep\\") + filenames.front()).c_str()), NULL, SND_ASYNC);
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
	try {
		for (const auto& entry : fs::directory_iterator(path)) {
			static std::string suffix = std::string(".wav");

			std::string filename = entry.path().filename().string();

			bool has_suffix = filename.find(suffix, filename.size() - suffix.size()) != std::string::npos;
			if (has_suffix) {
				sound_files_container.push_back(filename);
			}
		}
	}
	catch (const std::exception& ex) {

	}
}

void init_sounds_files_of_hit_and_kill(
	std::vector<std::string>& hit_sound_files,
	std::vector<std::string>& kill_sound_files,
	std::vector<std::string>& headshotkill_sound_files,
	//TODO headshot_hit_sound_files
	std::vector<std::string>& headshot_hit_sound_files,
	std::vector<std::string>& flash_sound_files) {
	get_all_wav_files("./sounds/hit", hit_sound_files);
	get_all_wav_files("./sounds/kill", kill_sound_files);
	get_all_wav_files("./sounds/headshot_kill", headshotkill_sound_files);
	get_all_wav_files("./sounds/headshot_hit", headshot_hit_sound_files);
	get_all_wav_files("./sounds/flash", flash_sound_files);
}

void init_sounds_files_of_sonar(
	std::vector<std::string>& sonar_crosshair_beep_files,
	std::vector<std::string>& sonar_range_scan_beep_files) {
	get_all_wav_files("./sounds/sonar/crosshair_beep", sonar_crosshair_beep_files);
	get_all_wav_files("./sounds/sonar/range_scan_beep", sonar_range_scan_beep_files);
}

#endif