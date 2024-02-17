#pragma once

//Can be used to alter unity behavior

#define MODNAME "[PCBS2-OverclockAnyCPU]"
#define JSONFILE "\\Plugins\\PCBS2-OverclockAnyCPU.offsetRequests.json"
#define QUICKDEBUG(msg) std::cout << MODNAME << ": " <<  msg << "\n"

//Define your hooks here, or somewhere else and include them here
void (__fastcall*PartDescCPU_GetHWInfoProps_o)(DWORD*, DWORD*, const DWORD*);

void __stdcall PartDescCPU_GetHWInfoProps_hook(DWORD* __this, DWORD* props, const DWORD* method)
{
    //init standard values
    PartDescCPU_GetHWInfoProps_o(__this, props, method);

    Unity::CComponent* caller = (Unity::CComponent*)__this;

    std::vector<Unity::il2cppFieldInfo*> fields;

    caller->SetMemberValue<bool>("m_canOverclock", true);
    //caller->SetMemberValue<bool>("m_canDelid", true);
    caller->SetMemberValue<int>("m_maxFreqMhz", 20000);

    float currentStepMultipler = caller->GetMemberValue<float>("m_multiplierStep");
    if (currentStepMultipler == 0)
    {
        caller->SetMemberValue<float>("m_multiplierStep", .25f);
    }

    return;
}

bool (__fastcall*BiosConfig_AllowOverclock_o)(DWORD*, const DWORD*);

bool __stdcall BiosConfig_AllowOverclock_hook(DWORD* __this, const DWORD* method)
{
    bool returnResult = BiosConfig_AllowOverclock_o(__this, method);
    if (returnResult)
        return returnResult;
    return true;
}


bool (__fastcall*BiosConfig_ChangeCPUMultiplier_o)(DWORD*, int32_t, const DWORD*);

bool __stdcall BiosConfig_ChangeCPUMultiplier_hook(DWORD* __this, int32_t dir, const DWORD* method)
{
    Unity::CComponent* caller = (Unity::CComponent*)__this;
    float cpuMultiplier = caller->GetMemberValue<float>("m_cpuMultiplier");
    QUICKDEBUG("cpu multiplier: " << cpuMultiplier << " dir: " << dir);

    if (dir == 1) // Incresase
    {
        cpuMultiplier += 0.25f;
    }
    else if (dir == -1) // Decrease
    {
        cpuMultiplier -= 0.25f;
    }

    caller->SetMemberValue<float>("m_cpuMultiplier", cpuMultiplier);

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

    uintptr_t PartDescCPU_GetHWInfoPropsOffset = std::stoull(functionOffsets[0].value, nullptr, 16);
    MH_CreateHook(
        reinterpret_cast<LPVOID*>(gameAsm + PartDescCPU_GetHWInfoPropsOffset),
        &PartDescCPU_GetHWInfoProps_hook,
        (LPVOID*)&PartDescCPU_GetHWInfoProps_o);


    uintptr_t BiosConfig_AllowOverclockOffset = std::stoull(functionOffsets[1].value, nullptr, 16);
    MH_CreateHook(
        reinterpret_cast<LPVOID*>(gameAsm + BiosConfig_AllowOverclockOffset),
        &BiosConfig_AllowOverclock_hook,
        (LPVOID*)&BiosConfig_AllowOverclock_o);

    uintptr_t BiosConfig_ChangeCPUMultiplier_hookOffset = std::stoull(functionOffsets[2].value, nullptr, 16);
    MH_CreateHook(
        reinterpret_cast<LPVOID*>(gameAsm + BiosConfig_ChangeCPUMultiplier_hookOffset),
        &BiosConfig_ChangeCPUMultiplier_hook,
        (LPVOID*)&BiosConfig_ChangeCPUMultiplier_o);


    MH_STATUS status = MH_EnableHook(MH_ALL_HOOKS); //Get the hook status, 0 = ALl good

    QUICKDEBUG("Hooks loaded with status: " << status);
}
