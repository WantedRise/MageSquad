// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

// Local Role.
#define LOG_SUBLOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetLocalRole()))

// Remote Role.
#define LOG_SUBREMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetOwner()->GetRemoteRole()))

// Local Role.
#define LOG_LOCALROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetLocalRole()))

// Remote Role.
#define LOG_REMOTEROLEINFO *(UEnum::GetValueAsString(TEXT("Engine.ENetRole"), GetRemoteRole()))

//NetMode
#define LOG_NETMODEINFO ((GetNetMode() == ENetMode::NM_Client) ? \
*FString::Printf(TEXT("Client_%d"), UE::GetPlayInEditorID()) : \
(GetNetMode() == ENetMode::NM_Standalone) ? \
TEXT("StandAlone") : TEXT("Server"))

// __FUNCTION__ -> const char* (ANSI).
#define LOG_CALLINFO ANSI_TO_TCHAR(__FUNCTION__)

#define MS_LOG(LogCat, Verbosity, Format, ...) \
UE_LOG(LogCat, Verbosity, TEXT("[%s] [%s/%s] %s %s"), LOG_NETMODEINFO, LOG_LOCALROLEINFO, LOG_REMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))


#define MS_SUBLOG(LogCat, Verbosity, Format, ...) \
UE_LOG(LogCat, Verbosity, TEXT("[%s] [%s/%s] %s %s"), LOG_NETMODEINFO, LOG_SUBLOCALROLEINFO, LOG_SUBREMOTEROLEINFO, LOG_CALLINFO, *FString::Printf(Format, ##__VA_ARGS__))

DECLARE_LOG_CATEGORY_EXTERN(LogMSNetwork, Log, All);


