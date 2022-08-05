#pragma once
#include "pch.h"
#include "Memory.h"
#include <string>
#include <locale>

struct BitField
{
	unsigned char A : 1;
	unsigned char B : 1;
	unsigned char C : 1;
	unsigned char D : 1;
	unsigned char E : 1;
	unsigned char F : 1;
	unsigned char G : 1;
	unsigned char H : 1;
};

template<class T>
struct TArray
{
	friend class FString;

public:
	inline TArray()
	{
		Data = nullptr;
		Count = Max = 0;
	};

	inline int Num() const
	{
		return Count;
	};

	inline void Add(T InputData)
	{
		Data = (T*)realloc(Data, sizeof(T) * (Count + 1));
		Data[Count++] = InputData;
		Max = Count;
	};

	inline void Remove(int32_t Index)
	{
		TArray<T> NewArray;
		for (int i = 0; i < this->Count; i++)
		{
			if (i == Index)
				continue;

			NewArray.Add(this->Data[i]);
		}
		this->Data = (T*)realloc(NewArray.Data, sizeof(T) * (NewArray.Count));
		this->Count = NewArray.Count;
		this->Max = NewArray.Count;
	}

	T* Data;
	int Count;
	int Max;
};

struct FString : private TArray<wchar_t>
{
	FString()
	{
	};

	FString(const wchar_t* other)
	{
		Max = Count = *other ? std::wcslen(other) + 1 : 0;

		if (Count)
		{
			Data = const_cast<wchar_t*>(other);
		}
	}

	bool IsValid() const
	{
		return Data != nullptr;
	}

	const wchar_t* c_str() const
	{
		return Data;
	}

	std::string ToString() const
	{
		if (IsValid()) {
			auto length = std::wcslen(Data);

			std::string str(length, '\0');

			std::use_facet<std::ctype<wchar_t>>(std::locale()).narrow(Data, Data + length, '?', &str[0]);

			return str;
		}
		else {
			return "INVALID";
		}
	}
};

struct UObject;
struct FName;
//New
FString(*GetObjectPath)(UObject* In);

//Old
void(*FNameToString)(FName* NameIn, FString& Out);
void(*FMFree)(__int64);

struct FName
{
	uint32_t ComparisonIndex;
	uint32_t DisplayIndex;

	FName() = default;

	std::string GetName() {
		FString temp;
		FNameToString(this, temp);
		std::string ret(temp.ToString());
		FMFree(__int64(temp.c_str()));

		return ret;
	}
};

void* (*ProcessEventOG)(void* Object, void* Function, void* Params);

struct UObject
{
	void** VTable;
	int32_t ObjectFlags;
	int32_t InternalIndex;
	UObject* Class;
	FName Name;
	UObject* Outer;

	uintptr_t GetAddress() {
		return __int64(this);
	}

	void* ProcessEvent(UObject* Function, void* Params = nullptr) {
		return ProcessEventOG(this, Function, Params);
	}

	std::string GetPath() {
		return GetObjectPath(this).ToString();
	}

	std::string GetNameOld() {
		return Name.GetName();
	}

	//Use GetPath Instead!
	std::string GetFullName() {
		std::string temp;

		for (auto outer = Outer; outer; outer = outer->Outer)
		{
			temp = outer->Name.GetName() + "." + temp;
		}

		temp = reinterpret_cast<UObject*>(Class)->Name.GetName() + " " + temp + this->Name.GetName();

		return temp;
	}
};

struct UObjectItem
{
	UObject* Object;
	DWORD Flags;
	DWORD ClusterIndex;
	DWORD SerialNumber;
};

class NewUObjectArray {
public:
	UObjectItem* Objects[9];
};

struct GObjects
{
	NewUObjectArray* Objects;
	BYTE _padding_0[0xC];
	uint32_t NumElements;

	inline void NumChunks(int* start, int* end) const
	{
		int cStart = 0, cEnd = 0;

		if (!cEnd)
		{
			while (1)
			{
				if (Objects->Objects[cStart] == 0)
				{
					cStart++;
				}
				else
				{
					break;
				}
			}

			cEnd = cStart;
			while (1)
			{
				if (Objects->Objects[cEnd] == nullptr)
				{
					break;
				}
				else
				{
					cEnd++;
				}
			}
		}

		*start = cStart;
		*end = cEnd;
	}

	inline int32_t Num() const
	{
		return NumElements;
	}

	inline UObject* GetByIndex(int32_t index) const
	{
		int cStart = 0, cEnd = 0;
		int chunkIndex = 0, chunkSize = 0xFFFF, chunkPos;
		UObjectItem* Object;

		NumChunks(&cStart, &cEnd);

		chunkIndex = index / chunkSize;
		if (chunkSize * chunkIndex != 0 &&
			chunkSize * chunkIndex == index)
		{
			chunkIndex--;
		}

		chunkPos = cStart + chunkIndex;
		if (chunkPos < cEnd)
		{
			Object = Objects->Objects[chunkPos] + (index - chunkSize * chunkIndex);
			return Object->Object;
		}

		return nullptr;
	}
};

struct FVector {
	float X;
	float Y;
	float Z;

	FVector() {
		X = Y = Z = 0;
	}

	FVector(float NX, float NY, float NZ) {
		X = NX;
		Y = NY;
		Z = NZ;
	}
};

namespace Game {
	//Vars
	bool InGame = false;

	//Main
	GObjects* GObjs;
	UObject** GWorld;
	UObject** GEngine;

	//In Game
	UObject* GPC;
	UObject* GPawn;
	UObject* CM;
}

UObject* (*StaticFindObject)(struct UClass*, struct UObject*, const wchar_t*, bool);

UObject* FindObject(std::wstring TargetName, bool Equals = false) {
	return StaticFindObject(nullptr, nullptr, TargetName.c_str(), Equals);
}

#include <fstream>
void DumpObjects() {
	std::ofstream Objs("Objects.txt");
	for (int i = 0; i < Game::GObjs->Num(); i++) {
		UObject* Object = Game::GObjs->GetByIndex(i);
		std::string ObjName = Object->GetPath();
		Objs << Object->Class->GetNameOld() << " " << ObjName << "\n";
	}
	std::cout << "Done Dumping!\n";
	Objs.close();
}

struct FText
{
	char data[0x18];
};

struct FWeakObjectPtr
{
public:
	inline bool SerialNumbersMatch(UObjectItem* ObjectItem) const
	{
		return ObjectItem->SerialNumber == ObjectSerialNumber;
	}

	int32_t ObjectIndex;
	int32_t ObjectSerialNumber;
};

template<class T, class TWeakObjectPtrBase = FWeakObjectPtr>
struct TWeakObjectPtr : public FWeakObjectPtr
{
public:

	inline T* Get() {
		return Game::GObjs->GetByIndex<T>(ObjectIndex);
	}
};

template<typename TObjectID>
class TPersistentObjectPtr
{
public:
	FWeakObjectPtr WeakPtr;
	int32_t TagAtLastTest;
	TObjectID ObjectID;
};

struct FSoftObjectPath
{
	FName AssetPathName;
	FString SubPathString;
};

class FSoftObjectPtr : public TPersistentObjectPtr<FSoftObjectPath>
{
	inline UObject* Get() {
		return Game::GObjs->GetByIndex(WeakPtr.ObjectIndex);
	}
};

class TSoftObjectPtr : FSoftObjectPtr
{

};
class TSoftClassPtr : FSoftObjectPtr
{

};

UObject* (*Conv_SoftClassReferenceToClass)(TSoftClassPtr const&);
FText(*Conv_StringToText)(class FString const&);
FName(*Conv_StringToName)(class FString const&);
FString(*Conv_TextToString)(class FText const&);
void (*SetText)(UObject* TB, FText*);

UObject* (*GetFirstPlayerController)(UObject* World);

enum class EGameMode {
	Parent = 0,
	Panda = 1,
	Podium = 2,
	Training = 3,
	Tutorial = 4,
	Lobby = 5,
	MAX = 6
};