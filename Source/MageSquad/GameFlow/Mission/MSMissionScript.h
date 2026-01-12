// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "MSMissionScript.generated.h"

struct FMSMissionProgressUIData;

/**
 * 작성자: 이상준
 * 작성일: 25/12/30
 *
 * 모든 미션 스크립트의 베이스 클래스
 *
 * - 개별 미션 로직은 이 클래스를 상속하여 구현
 * - 미션 진행도 및 완료 여부 판단의 기본 인터페이스 제공
 */
UCLASS()
class MAGESQUAD_API UMSMissionScript : public UObject
{
	GENERATED_BODY()
	
public:
    // 미션에 필요한 액터 스폰, 델리게이트 바인딩 등 수행
    virtual void Initialize(class UWorld* World);
    // 스폰한 액터 제거, 델리게이트 해제 등
    virtual void Deinitialize();
    // 현재 미션 진행도 반환
    virtual void  GetProgress(FMSMissionProgressUIData& OutData) const;
    // 미션 완료 여부 판단
    virtual bool IsCompleted() const { return true; }
    // 미션을 소유하는 MissionComponent 설정
    void SetOwnerMissionComponent(class UMSMissionComponent* InOwner);
protected:
    TWeakObjectPtr<class UMSMissionComponent> OwnerMissionComponent;
};
