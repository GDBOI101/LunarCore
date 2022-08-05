#pragma once
#include "pch.h"
#include "Unreal.h"

struct UField_New : UObject
{
	UField_New* Next;
	void* UKD_0;
	void* UKD_1;
};

struct FField {
	void* VTable;
	void* Class;
	void* Owner;
	void* UKD_00;
	FField* Next;
	FName Name;
	int32_t FlagsPrivate;
};

struct FProperty : FField
{
	int32_t ArrayDim;
	int32_t ElementSize;
	int32_t PropertyFlags;
	uint16_t RepIndex;
	void* BlueprintReplicationCondition;
	int32_t Offset_Internal;
	FName RepNotifyFunc;
	FProperty* PropertyLinkNext;
	FProperty* NextRef;
	FProperty* DestructorLinkNext;
	FProperty* PostConstructLinkNext;
};


struct UStruct_New2 : UField_New
{
	UStruct_New2* Super;
	UField_New* Children;
	FField* ChildProperties;
	int32_t Size;
	int32_t MinAlignment;
	TArray<uint8_t> Script;
	FProperty* PropertyLink;
	FProperty* RefLink;
	FProperty* DestructorLink;
	FProperty* PostConstructLink;
};

namespace Finder {
	UObject* FindChild(UObject* InObject, std::string PropName) {
		FField* Prop = reinterpret_cast<UStruct_New2*>(InObject)->ChildProperties;
		while (Prop)
		{
			if (Prop->Name.GetName() == PropName) {
				return (UObject*)Prop;
			}
			else {
				Prop = Prop->Next;
			}
		}
		return nullptr;
	}

	int GetOffset(void* TargetProp) {
		return *reinterpret_cast<int*>(__int64(TargetProp) + 0x4C);
	}

	template<class T>
	T GetValuePointer(UObject* Object, void* Prop) {
		return reinterpret_cast<T>(Object->GetAddress() + GetOffset(Prop));
	}

	int GetStructOffset(UObject* TargetStruct, std::string TargetChildName) {
		UObject* Prop = nullptr;
		UStruct_New2* Class = (UStruct_New2*)TargetStruct;
		if (Class->ChildProperties) {
			Prop = FindChild(Class, TargetChildName);
		}
		if (Prop == nullptr) {
			UStruct_New2* Struct = reinterpret_cast<UStruct_New2*>(Class)->Super;
			while (Struct)
			{
				if (Struct->ChildProperties) {
					Prop = FindChild(Struct, TargetChildName);
					if (Prop != nullptr) {
						break;
					}
				}
				Struct = Struct->Super;
			}
		}
		return GetOffset(Prop);
	}

	template<class T = UObject*>
	T* Find(UObject* TargetObject, std::string TargetChildName) {
		UObject* Prop = nullptr;
		UStruct_New2* Class = (UStruct_New2*)TargetObject->Class;
		if (Class->ChildProperties) {
			Prop = FindChild(Class, TargetChildName);
		}
		if (Prop == nullptr) {
			UStruct_New2* Struct = reinterpret_cast<UStruct_New2*>(TargetObject->Class)->Super;
			while (Struct)
			{
				if (Struct->ChildProperties) {
					Prop = FindChild(Struct, TargetChildName);
					if (Prop != nullptr) {
						break;
					}
				}
				Struct = Struct->Super;
			}
		}
		if (Prop != nullptr) {
			return GetValuePointer<T*>(TargetObject, Prop);
		}
		else {
			return nullptr;
		}
	}

	UObject* FindActor(UObject* Class, int Index = 0) {
		struct {
			UObject* World;
			UObject* Class;
			TArray<UObject*> Actors;
		} Params;
		Params.World = *Game::GWorld;
		Params.Class = Class;
		FindObject(L"/Script/Engine.Default__GameplayStatics")->ProcessEvent(FindObject(L"/Script/Engine.GameplayStatics.GetAllActorsOfClass"), &Params);
		auto Actors = Params.Actors;
		return Actors.Data[Index];
	}
}