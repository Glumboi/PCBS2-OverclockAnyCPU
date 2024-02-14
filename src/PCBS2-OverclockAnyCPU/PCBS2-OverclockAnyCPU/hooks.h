#pragma once
#include <iostream>
#include "offsets.h"
#define MODNAME "[PCBS2-OverclockAnyCPU]"
#define JSONFILE "\\Plugins\\PCBS2-OverclockAnyCPU.offsetRequests.json"
#define QUICKDEBUG(msg) std::cout << MODNAME << ": " <<  msg << "\n"

//Define your hooks here, or somewhere else and include them here
//Example from my infinite money plugin:
/*
int32_t(__fastcall* CareerStatus__GetCash_o)(DWORD*, DWORD*);

int32_t __stdcall CareerStatus__GetCash_hook(DWORD* __tis, DWORD* method)
{
    return 1000000;
}
*/

bool(__fastcall* BiosConfig_AllowOverclock_o)(DWORD*, const DWORD*);

bool __stdcall BiosConfig_AllowOverclock_hook(DWORD* __this, const DWORD* method)
{
    return true;
}

inline void CreateAndLoadHooks()
{
    //Load addresses from json file
    QUICKDEBUG("Loading offsets from " << JSONFILE);
    //Append json path with dll location
    char buffer[MAX_PATH];
    GetCurrentDirectory(MAX_PATH, buffer);
    std::string dllJsonPath = std::string(buffer) + JSONFILE;

    //Load offsets
    functionOffsets = readJsonFromFile(dllJsonPath).offsetRequests;

    //Instantiate your hooks here using MH_CreateHook
    uintptr_t BiosConfig_AllowOverclockOffset = std::stoull(functionOffsets[0].value, nullptr, 16);
    MH_CreateHook(
        reinterpret_cast<LPVOID*>(gameAsm + BiosConfig_AllowOverclockOffset),
        &BiosConfig_AllowOverclock_hook,
        (LPVOID*)&BiosConfig_AllowOverclock_o);


    MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS); //Get the hook status, 0 = ALl good

    QUICKDEBUG("Hooks loaded with status: " << status);
}
