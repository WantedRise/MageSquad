// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"
#include "MSSkillList.generated.h"

/**
 * 
 */

class UGameplayAbility;

USTRUCT(BlueprintType)
struct FMSUpgradeInfo
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag Tag;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Current = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Max = 0;
};

USTRUCT(BlueprintType)
struct FMSSkillList : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	// 스킬 타입 ( 1: AutoActive, 2: Left Click, 3: Right Click )
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SkillType = 1;

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

	// 스킬 아이콘 머티리얼 (소프트 레퍼런스)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftObjectPtr<UMaterialInterface> SkillIcon;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UMaterialInterface* SkillLevelUpIcon = nullptr;
	
	// 스킬 GA (소프트 레퍼런스)
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSoftClassPtr<UGameplayAbility> SkillAbility;

	// 스킬 레벨
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 SkillLevel = 1;

	// 스킬 데미지
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float SkillDamage = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseSkillDamage = 0.f;
	
	// 쿨타임
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float CoolTime = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseCoolTime = 10.f;

	// 발사체 개수
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 ProjectileNumber = 0;

	// 관통 횟수
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	int32 Penetration = 0;

	// 지속 시간
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Duration = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseDuration = 0.f;

	// 범위
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Range = 0.f;
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float BaseRange = 0.f;

	// 업그레이드 정보
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<FMSUpgradeInfo> UpgradeInfos;
	
	// 액티브 스킬 쿨타임 태그
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag SkillCooldownTag;
};
