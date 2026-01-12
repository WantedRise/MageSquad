// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFlow/Mission/MSMissionScript.h"
#include "GameplayEffectTypes.h"
#include "MSMissionKillTargetScript.generated.h"

struct FMSMissionProgressUIData;

/** 
 * 작성자: 이상준	
 * 작성일: 25/12/30
 * 
 * 특정 타겟(엘리트 몬스터)을 처치하는 것을 목표로 하는 미션 스크립트
 *
 * - 미션 시작 시 몬스터를 스폰
 * - GAS Attribute(HP) 변경을 감지하여 진행도(Progress)를 계산
 */
UCLASS()
class MAGESQUAD_API UMSMissionKillTargetScript : public UMSMissionScript
{
    GENERATED_BODY()

public:
    //미션 시작 시 호출
    virtual void Initialize(UWorld* World) override;
    virtual void Deinitialize() override;
    //미션 진행도 반환
    virtual void GetProgress(FMSMissionProgressUIData& OutData) const override;
    //미션 완료 여부 판단
    virtual bool IsCompleted() const override;
private:
    //몬스터를 월드에 스폰
    void SpawnElite(UWorld* World);
    //몬스터 HP 변경 시 호출되는 콜백
    void OnEliteHPChanged(const FOnAttributeChangeData& Data);

private:
    TWeakObjectPtr<class AMSBaseEnemy> EliteMonster;
    float Progress = 0.f;
    float TimeLimit = -1.f;
    float MaxHp;
    float CurrentHp;
};
