#pragma once
#include "pch.h"
namespace Sigs {
	//Base
	const char* ProcessEvent = "40 55 56 57 41 54 41 55 41 56 41 57 48 81 EC ? ? ? ? 48 8D 6C 24 ? 48 89 9D ? ? ? ? 48 8B 05 ? ? ? ? 48 33 C5 48 89 85 ? ? ? ? 8B 41 0C 45 33 F6"; //void UObject::ProcessEvent(UObject* this, UFunction* Func, void* Params)
	const char* GetPathName = "40 53 48 83 EC 20 48 8B C2 48 8B D9 48 85 D2 75 43"; //FString UKismetSystemLibrary::GetPathName(UObject*)
	const char* FNTS = "48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 8B 01 48 8B F2"; //void FName::ToString(FName* InName, FString& Out)
	const char* Free = "48 85 C9 74 2E 53 48 83 EC 20 48 8B D9 48 8B 0D ? ? ? ? 48 85 C9"; // void FMemory::Free(void*)

	//Utils
	const char* SC2C = "48 89 5C 24 ? 48 89 6C 24 ? 48 89 74 24 ? 57 48 83 EC 20 48 8B F1 48 8B FA 48 8B CA E8 ? ? ? ? 33 ED";
	const char* String2Name = "40 53 48 83 EC 20 83 7A 08 00 48 8B D9 74 05 48 8B 12 EB 07 48 8D 15 ? ? ? ? 41 B8 ? ? ? ? E8 ? ? ? ?";
	const char* String2Text = "40 53 48 83 EC 20 48 8B D9 E8 ? ? ? ? 48 8B C3 48 83 C4 20 5B C3 CC CC CC CC CC CC CC CC CC 48 89 5C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56"; //FText UKismetTextLibrary::Conv_StringToText(FString*)
	const char* Text2String = "40 53 48 83 EC 20 48 8B D9 48 8B CA E8 ? ? ? ? 48 8B D0 48 8B CB E8 ? ? ? ? 48 8B C3";
	const char* GetFirstPlayerController = "83 B9 ? ? ? ? ? 7E 0C 48 8B 89 ? ? ? ? E9 ? ? ? ?"; // UObject* GetFirstPlayerController(UObject* InWorld)
	const char* FindObject = "48 89 5C 24 ? 48 89 74 24 ? 55 57 41 54 41 56 41 57 48 8B EC 48 83 EC 60 80 3D ? ? ? ? ? 45 0F B6 F1";
	//Hooks
	const char* SetText = "48 89 74 24 ? 48 89 7C 24 ? 41 56 48 83 EC 20 80 7A 18 00 4C 8D 72 20 48 8B F2"; //void STextBlock::SetText(STextBlock* This, FText*)
	const char* EngineExit = "40 53 48 83 EC 20 0F B6 D9 48 8D 0D ? ? ? ?"; //void FWindowsPlatformMisc::RequestExit(bool)

	//Objects
	const char* GObjects = "48 8D 0D ? ? ? ? E8 ? ? ? ? 89 43 04 48 83 C4 20"; //(3)
	const char* GEngine = "48 89 05 ? ? ? ? 48 85 C9 74 05 E8 ? ? ? ? 48 8D 4D F0"; //(3)
	const char* GWorld = "48 8B 1D ? ? ? ? 48 85 DB 74 3B 41 B0 01"; //(3)

	//Curl
	const char* CurlEasySet = "89 54 24 10 4C 89 44 24 18 4C 89 4C 24 20 48 83 EC 28 48 85 C9";
	const char* CurlSet = "48 89 5C 24 08 48 89 6C 24 10 48 89 74 24 18 57 48 83 EC 30 33 ED 49 8B F0 48 8B D9";
}

namespace Memory {
	uintptr_t FindPattern(const char* signature, bool bRelative = false, uint32_t offset = 0) {
		uintptr_t base_address = reinterpret_cast<uintptr_t>(GetModuleHandle(NULL));
		static auto patternToByte = [](const char* pattern)
		{
			auto bytes = std::vector<int>{};
			const auto start = const_cast<char*>(pattern);
			const auto end = const_cast<char*>(pattern) + strlen(pattern);

			for (auto current = start; current < end; ++current)
			{
				if (*current == '?')
				{
					++current;
					if (*current == '?') ++current;
					bytes.push_back(-1);
				}
				else { bytes.push_back(strtoul(current, &current, 16)); }
			}
			return bytes;
		};

		const auto dosHeader = (PIMAGE_DOS_HEADER)base_address;
		const auto ntHeaders = (PIMAGE_NT_HEADERS)((std::uint8_t*)base_address + dosHeader->e_lfanew);

		const auto sizeOfImage = ntHeaders->OptionalHeader.SizeOfImage;
		auto patternBytes = patternToByte(signature);
		const auto scanBytes = reinterpret_cast<std::uint8_t*>(base_address);

		const auto s = patternBytes.size();
		const auto d = patternBytes.data();

		for (auto i = 0ul; i < sizeOfImage - s; ++i)
		{
			bool found = true;
			for (auto j = 0ul; j < s; ++j)
			{
				if (scanBytes[i + j] != d[j] && d[j] != -1)
				{
					found = false;
					break;
				}
			}
			if (found)
			{
				uintptr_t address = reinterpret_cast<uintptr_t>(&scanBytes[i]);
				if (bRelative)
				{
					address = ((address + offset + 4) + *(int32_t*)(address + offset));
					return address;
				}
				return address;
			}
		}
		return 0;
	}
}