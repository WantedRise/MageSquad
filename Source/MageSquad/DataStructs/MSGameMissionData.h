// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/GameMissionTypes.h"
#include "GameFlow/Mission/MSMissionScript.h"
#include "MSGameMissionData.generated.h"

/*
 * 작성자 : 이상준
 * 작성일 : 2025/12/20
 * 미션 데이터 정보
 */

USTRUCT(BlueprintType)
struct FMSMissionRow : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere)
    int32 MissionID;

    UPROPERTY(EditAnywhere)
    EMissionType MissionType;

    UPROPERTY(EditAnywhere)
    float TimeLimit;

    // UI용 아이콘 (Soft Reference)
    UPROPERTY(EditAnywhere)
    TSoftObjectPtr<class UTexture2D> MissionIcon;
    
    //미션용 스크립트
    UPROPERTY(EditAnywhere)
    TSubclassOf<class UMSMissionScript> ScriptClass;

    UPROPERTY(EditAnywhere)
    FText Title;

    UPROPERTY(EditAnywhere)
    FText Description;
};