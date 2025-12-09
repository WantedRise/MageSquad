// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"),GetLocalRole()))
#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"),GetRemoteRole()))

#define LOG_NETMODEINFO ((GetNetMode()==ENetMode::NM_Client)?*FString::Printf(TEXT("Client_%d"),static_cast<int32>(GPlayInEditorID)): ((GetNetMode()==ENetMode::NM_Standalone)? TEXT("Standalone"): TEXT("Server")))
//__LINE__ __FILE__
//__FUNCTION__ -> const char*(ANSI)
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__ )
#define MS_LOG(LogCat,Verbosity,Format, ...) \
 UE_LOG(LogCat,Verbosity,TEXT("[%s][%s/%s] %s %s"),LOG_NETMODEINFO,LOG_LOCALROLEINFO,LOG_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format,##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogMSNetwork, Log, All);
