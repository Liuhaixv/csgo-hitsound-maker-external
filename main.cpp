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

DWORD pid;

struct Offset {
	DWORD dwLocalPlayer = 0xDBF4CC;//ok
	DWORD m_fFlags = 0x104;//ok
	//int m_iCrosshairId = 0x11838;//ok
	DWORD dwEntityList = 0x4DDB92C;//ok
	int TotalHitCount = 0x103f8;//ok
	DWORD m_iHealth = 0x100;//ok
	int RoundKillNum = 0x99a0;//ok
}O;
struct Values {
	DWORD localPlayer;
	DWORD gameModule;
	BYTE flag;
	int LocalTeam;
	int CrossHairID;
	int EnemyTeam;
	DWORD EnemyInCH;
}V;


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

//to avoid console pause
void disable_quick_edit() {
    //¹Ø±Õ¿ìËÙ±à¼­
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
	Hacks hacks(&mem, &client);
    int connect_count = 0;

    std::thread hitmaker_thread(&Hacks::thread_hitmaker, &hacks, &state, true);

    disable_quick_edit();

    while (true) {
        while (mem.tProcess != NULL && mem.clientBaseAddr != NULL && mem.engineBaseAddr != NULL) {
            if (!state.process) {
                state.process = true;
            }

            while (client.in_game()) {
                if (!state.game) {
                    state.game = true;
                    client.update_gamemode();
                    hacks.init();
                }

                // SendMessage(ol.hwnd, WM_PAINT, NULL, NULL);
                Sleep(500);
            }

            state.game = false;
            hacks.bsp_setted = false;
            Sleep(5000);
        }

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
            new(&hacks) Hacks(&mem, &client);
        }
    }

	return 0;
}
