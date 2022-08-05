#pragma once
#include "pch.h"
#include <regex>
#include "Finder.h"
#pragma comment(lib, "minhook.lib")
#include "MinHook.h"
bool PELog = true;
bool bInGame = false;
void InputThread() {
	while (true) {
		if (GetAsyncKeyState(VK_F1) & 0x1) {
			FString LobbyMap(L"Map_M009_small");
			GetFirstPlayerController(*Game::GWorld)->ProcessEvent(FindObject(L"/Script/Engine.PlayerController.SwitchLevel"), &LobbyMap);
			bInGame = true;
			Sleep(1000);
		}
		Sleep(1000 / 60);
	}
}

namespace Functions {
	UObject* SpawnObject(UObject* Class, UObject* Outer) {
		struct {
			UObject* Class;
			UObject* Outer;
			UObject* Ret;
		} params{Class, Outer};
		FindObject(L"/Script/Engine.Default__GameplayStatics")->ProcessEvent(FindObject(L"/Script/Engine.GameplayStatics.SpawnObject"), &params);
		return params.Ret;
	}
}

std::vector<std::string> KnownFuncs;
std::ofstream PEL("PE.txt");
bool IsKnown(std::string Name) {
	for (int i = 0; i < KnownFuncs.size(); i++) {
		if (KnownFuncs[i] == Name) {
			return true;
		}
	}
	return false;
}

namespace Hooks {
	//PE
	void* ProcessEvent_Hk(UObject* Obj, UObject* Func, void* params) {
		std::string FuncName = Func->GetPath();
		if (PELog && !IsKnown(FuncName)) {
			KnownFuncs.push_back(FuncName);
			PEL << FuncName << " " << Obj->GetPath() << std::endl;
		}

		if (FuncName == ("/Script/Engine.GameMode:ReadyToStartMatch")) {
			auto GS = Finder::Find(Obj, "GameState");
			if (GS && *GS) {
				auto RTR = Finder::Find<float>(*GS, "RoundTimeRemaining");
				if (RTR) {
					//Doesnt work
					*RTR = 99999.0f;
					//MessageBoxA(0, "Set RoundTime", "Test", MB_OK);
				}
			}
			auto PS = *Finder::Find(GetFirstPlayerController(*Game::GWorld),"PlayerState");
			auto GameMode = Finder::Find<EGameMode>(PS, "GameMode");
			if (GameMode) {
				//Doesnt work
				*GameMode = EGameMode::Training;
				MessageBoxA(0, "Set GameMode", "Test", MB_OK);
			}
		}

		if (FuncName == "/Script/Engine.CheatManager:CheatScript") {
			FString* F_Cmd = reinterpret_cast<FString*>(params);
			if (F_Cmd->IsValid()) {
				std::string Cmd = F_Cmd->ToString();
				//Doesnt work
				if (Cmd == "fixinput") {
					UObject* GI = *Finder::Find(*Game::GEngine, "GameInstance");
					UObject* PM = *Finder::Find(GI, "PreferencesManager");
					PM->ProcessEvent(FindObject(L"/Game/Panda_Main/Blueprints/Utilities/PreferencesManager.PreferencesManager_C.SetAllToDefaults"));
					PM->ProcessEvent(FindObject(L"/Game/Panda_Main/Blueprints/Utilities/PreferencesManager.PreferencesManager_C.SavePreferences"));
					struct {
						bool success;
						FString error;
					} out;
					PM->ProcessEvent(FindObject(L"/Game/Panda_Main/Blueprints/Utilities/PreferencesManager.PreferencesManager_C.ReloadPreferences"), &out);
					PM->ProcessEvent(FindObject(L"/Game/Panda_Main/Blueprints/Utilities/PreferencesManager.PreferencesManager_C.ApplyInputPrefs"));
					if (!out.success) {
						MessageBoxA(0, out.error.ToString().c_str(), "Test", MB_OK);
					}
					//GetFirstPlayerController(*Game::GWorld)->ProcessEvent(FindObject(L"/Game/Panda_Main/Blueprints/PFGPlayerController.PFGPlayerController_C.OnSyncedPreferencesLoaded"));
					MessageBoxA(0, "Input Fixed!", "Lunar", MB_OK);
				}
				//Scooby Doo Map
				if (Cmd == "play_sd") {
					FString LobbyMap(L"Map_ScoobyDoo");
					GetFirstPlayerController(*Game::GWorld)->ProcessEvent(FindObject(L"/Script/Engine.PlayerController.SwitchLevel"), &LobbyMap);
					bInGame = true;
				}
				//The Rick&Morty map
				if (Cmd == "play_gs") {
					FString LobbyMap(L"Map_M009_small");
					GetFirstPlayerController(*Game::GWorld)->ProcessEvent(FindObject(L"/Script/Engine.PlayerController.SwitchLevel"), &LobbyMap);
					bInGame = true;
				}
				//Batcave Map
				if (Cmd == "play_bc") {
					FString LobbyMap(L"Map_Batcave");
					GetFirstPlayerController(*Game::GWorld)->ProcessEvent(FindObject(L"/Script/Engine.PlayerController.SwitchLevel"), &LobbyMap);
					bInGame = true;
				}
			}
		}
		return ProcessEventOG(Obj, Func, params);
	}
	//CURL
	typedef enum {
		CURLOPT_URL = 10002
	} CURLoption;

	typedef enum {
		CURLE_OK = 0,
		CURLE_BAD_FUNCTION_ARGUMENT = 43,
	} CURLcode;

	static CURLcode(*CSO)(void*, CURLoption, va_list);
	//CURLcode(*CESO_OG)(void* curl, CURLoption option, ...);
	inline CURLcode CurlSetOpt_(void* data, CURLoption option, ...)
	{
		va_list arg;
		va_start(arg, option);

		const auto result = CSO(data, option, arg);

		va_end(arg);
		return result;
	}

	//Doesnt work. (afaik, havent tested much)
	CURLcode CESO_Hk(void* curl, CURLoption option, ...) {
		if (!curl) {
			return CURLcode::CURLE_BAD_FUNCTION_ARGUMENT;
		}
		va_list arg;
		va_start(arg, option);
		if (option == CURLoption::CURLOPT_URL) {
			std::regex Replace("(.*).wbagora.com");
			std::string url = va_arg(arg, char*);
			if (std::string(url).find(".wbagora.com") != std::string::npos) {
				url = const_cast<char*>(std::regex_replace(url, Replace, CHost).c_str());
				std::cout << "Redirected URL: " << url << std::endl;
			}
			return CurlSetOpt_(curl, option, url);
		}
		return CSO(curl, option, arg);
	}

	//SetText
	void SetText_Hk(UObject* STextBox, FText* InText) {
		if (STextBox != nullptr && InText != nullptr) {
			std::string InTextS = Conv_TextToString(*InText).ToString();
			//std::cout << "SetText Called with Text: " << InTextS << std::endl;
			if (InTextS == "ENTER THE MULTIVERSE") {
				auto NewText = Conv_StringToText(L"Enter Lunar");
				SetText(STextBox, &NewText);
			}
			//In the future this text will change lol (TODO): Maybe find a way to get the text by some sort of Id or something?
			else if (InTextS == "Welcome to our Open Beta!") {
				auto NewText = Conv_StringToText(L"Welcome to Lunar V1.0.0-alpha");
				SetText(STextBox, &NewText);
			}
			//In the future this text will change lol (TODO): Maybe find a way to get the text by some sort of Id or something?
			else if (InTextS == "We are still early in this process and wanted to thank you for joining us at the start of the journey. While in Beta some features may not work as expected and have stability issues. Official support is available through support@wbgames.com or through our official Discord server. The Player First Games team is committed to serving players and value all feedback so that we can make the game better, together. Thank you!") {
				auto NewText = Conv_StringToText(L"Lunar V1.0.0-alpha is now 20% done. Join our Discord at https://dsc.gg/mv-modding for more Info and Updates!");
				SetText(STextBox, &NewText);
			}
			else {
				SetText(STextBox, InText);
			}
		}
	}

	//EngineExit
	void RequestEngineExit_Hk(bool) {
		//Fixes Game Closing after a while when you press The Enter Lunar button.
		std::cout << "Bypassed Exit!" << std::endl;
		return;
	}
}

namespace Core {
	void Init() {
		MH_Initialize();
		//Show Console
		AllocConsole();
		FILE* pFile;
		freopen_s(&pFile, ("CONOUT$"), "w", stdout);

		//CURL
		uintptr_t CESO_Addr = Memory::FindPattern(Sigs::CurlEasySet);
		uintptr_t CSO_Addr = Memory::FindPattern(Sigs::CurlSet);
		Hooks::CSO = decltype(Hooks::CSO)(CSO_Addr);
		//DetoursEasy(CESO_Addr, Hooks::CESO_Hk);
		MH_CreateHook((void*)CESO_Addr, Hooks::CESO_Hk, nullptr);
		MH_EnableHook((void*)CESO_Addr);

		//Funny Promo Text (SetText)
		uintptr_t CS2T_Addr = Memory::FindPattern(Sigs::String2Text);
		uintptr_t CT2S_Addr = Memory::FindPattern(Sigs::Text2String);
		uintptr_t CS2N_Addr = Memory::FindPattern(Sigs::String2Name);
		uintptr_t CSC2C_Addr = Memory::FindPattern(Sigs::SC2C);
		uintptr_t SetTextAddr = Memory::FindPattern(Sigs::SetText);
		Conv_StringToName = decltype(Conv_StringToName)(CS2N_Addr);
		Conv_StringToText = decltype(Conv_StringToText)(CS2T_Addr);
		Conv_TextToString = decltype(Conv_TextToString)(CT2S_Addr);
		MH_CreateHook((void*)SetTextAddr, Hooks::SetText_Hk, (void**)&SetText);
		MH_EnableHook((void*)SetTextAddr);
		//EngienExit
		uintptr_t ExitAddr = Memory::FindPattern(Sigs::EngineExit);
		MH_CreateHook((void*)ExitAddr, Hooks::RequestEngineExit_Hk, nullptr);
		MH_EnableHook((void*)ExitAddr);
		//InGame
		uintptr_t ProcessEventAddr = Memory::FindPattern(Sigs::ProcessEvent);
		if (ProcessEventAddr != 0) {
			std::cout << "\nProcessEvent Address: 0x" << std::hex << ProcessEventAddr;
		}

		//GObjects struct is prob wrong (was literally fron NPP Lmao)
		uintptr_t GObjectsAddr = Memory::FindPattern(Sigs::GObjects, true, 3);
		if (GObjectsAddr != 0) {
			std::cout << "\nGObjects Address: 0x" << std::hex << GObjectsAddr;
		}

		uintptr_t GetPathAddr = Memory::FindPattern(Sigs::GetPathName);
		if (GetPathAddr != 0) {
			std::cout << "\nGetPathName Address: 0x" << std::hex << GetPathAddr;
		}

		uintptr_t FNTSAddr = Memory::FindPattern(Sigs::FNTS);
		if (FNTSAddr != 0) {
			std::cout << "\nFName::ToString Address: 0x" << std::hex << FNTSAddr;
		}

		uintptr_t FreeAddr = Memory::FindPattern(Sigs::Free);
		if (FreeAddr != 0) {
			std::cout << "\nFree Address: 0x" << std::hex << FreeAddr;
		}

		uintptr_t FOAddr = Memory::FindPattern(Sigs::FindObject);
		if (FOAddr != 0) {
			std::cout << "\nFO Address: 0x" << std::hex << FOAddr;
		}

		uintptr_t GFPCAddr = Memory::FindPattern(Sigs::GetFirstPlayerController);
		if (GFPCAddr != 0) {
			std::cout << "\nGFPC Address: 0x" << std::hex << GFPCAddr;
		}

		uintptr_t GEngineAddr = Memory::FindPattern(Sigs::GEngine, true, 3);
		if (GEngineAddr != 0) {
			std::cout << "\nGEngine Address: 0x" << std::hex << GEngineAddr;
		}

		uintptr_t GWorldAddr = Memory::FindPattern(Sigs::GWorld, true, 3);
		if (GWorldAddr != 0) {
			std::cout << "\nGWorld Address: 0x" << std::hex << GWorldAddr;
		}

		std::cout << std::endl;

		GetFirstPlayerController = decltype(GetFirstPlayerController)(GFPCAddr);
		StaticFindObject = decltype(StaticFindObject)(FOAddr);
		GetObjectPath = decltype(GetObjectPath)(GetPathAddr);
		FNameToString = decltype(FNameToString)(FNTSAddr);
		FMFree = decltype(FMFree)(FreeAddr);
		Game::GObjs = reinterpret_cast<GObjects*>(GObjectsAddr);
		Game::GEngine = reinterpret_cast<UObject**>(GEngineAddr);
		Game::GWorld = reinterpret_cast<UObject**>(GWorldAddr);
		UObject* PC = GetFirstPlayerController(*Game::GWorld);
		UObject* GVP = *Finder::Find(*Game::GEngine, "GameViewport");
		//Create Console and Cheat Manager
		MH_CreateHook((void*)ProcessEventAddr, Hooks::ProcessEvent_Hk, (void**)&ProcessEventOG);
		MH_EnableHook((void*)ProcessEventAddr);
		//"the console is literally uuu" - Shady https://media.discordapp.net/attachments/954527554935586886/1005224275189448767/unknown.png
		*Finder::Find(PC, "CheatManager") = Functions::SpawnObject(FindObject(L"/Script/Engine.CheatManager"), PC);
		*Finder::Find(GVP,"ViewportConsole") = Functions::SpawnObject(FindObject(L"/Script/Engine.Console"), GVP);
		CreateThread(0, 0, (LPTHREAD_START_ROUTINE)InputThread, 0, 0, 0);
		//Doesnt work bc of GObjs not working.
		DumpObjects();
	}
}