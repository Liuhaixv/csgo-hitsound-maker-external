#include <iostream>
#include <Windows.h>
#include <TlHelp32.h>
#include <thread>
#include <mmsystem.h>
#include <thread>
#include "memory.hpp"
#include "client.hpp"
#include "hacks.hpp"
#include "antiAC.hpp"
#include<time.h>

void hacks_state_monitor(hacks_state* state, Client* client) { // Bunny hop - F2
	while (true) {
		while (client->in_game()) {
			if (!state->game) {
				state->game = true;
			}

			// SendMessage(ol.hwnd, WM_PAINT, NULL, NULL);
			Sleep(500);
		}
		state->game = false;
	}
}


//¹Ø±Õ¿ìËÙ±à¼­
//disable quick edit
//to avoid console pause
void disable_quick_edit() {
    HANDLE hInput;
    DWORD prev_mode;
    hInput = GetStdHandle(STD_INPUT_HANDLE);
    GetConsoleMode(hInput, &prev_mode);
    SetConsoleMode(hInput, prev_mode & ENABLE_EXTENDED_FLAGS);
}

int main()
{
    AntiAC ac;
	Memory mem;
	hacks_state state;
	Client client(&mem);
	Hacks hacks(&mem, &client, &state);
    int connect_count = 0;

    std::thread hitmaker_thread(&Hacks::thread_hitmaker, &hacks, &state, true);

    disable_quick_edit();

    while (true) {
        while (mem.tProcess != NULL && mem.clientBaseAddr != NULL && mem.engineBaseAddr != NULL) {
            if (!state.process) {

                std::cout << "Game found" << std::endl;
                state.process = true;
            }

            while (client.in_game()) {
                if (!state.game) {
                    state.game = true;
                    client.update_gamemode();
                    hacks.init();
                }


                int round_index = client.get_round_index();

                //update last_time_round_index_changed
                if (round_index != state.round_index) {
                    time_t current_time =time(NULL);                                 
                    state.last_time_round_index_changed = current_time;
                }
                //update round_index
                state.round_index = round_index;

                // SendMessage(ol.hwnd, WM_PAINT, NULL, NULL);
                Sleep(1000);
            }

            state.game = false;
            hacks.bsp_setted = false;
            Sleep(5000);
        }

        std::cout << "Game not found, sleeping for 5 secs" << std::endl;

        state.process = false;
        ++connect_count;
        Sleep(5000);
        if (connect_count >= 2) {
            connect_count = 0;
            mem.~Memory();
            client.~Client();
            hacks.~Hacks();

            new(&mem) Memory();
            new(&client) Client(&mem);
            new(&hacks) Hacks(&mem, &client, &state);
        }
    }

	return 0;
}
