// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MSSkillList.generated.h"

/**
 * 
 */
USTRUCT(BlueprintType)
struct FMSSkillList : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 스킬 타입 ( 1: AutoActive, 2: Left Click, 3: Right Click )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SkillType = 0;

	// 고유 스킬 ID
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SkillID = 0;

	// 스킬 이름
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FString SkillName;

	// 스킬 이벤트 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SkillEventTag;

	// 스킬 설명 (UI 표시용)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText SkillDescription;

	// 스킬 아이콘 (소프트 레퍼런스)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UTexture2D> SkillIcon;

	// 스킬 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SkillLevel = 1;

	// 스킬 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SkillDamage = 0.f;

	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CoolTime = 0.f;

	// 발사체 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ProjectileNumber = 0;

	// 관통 횟수
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Penetration = 0;

	// 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration = 0.f;

	// 범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range = 0.f;

};
