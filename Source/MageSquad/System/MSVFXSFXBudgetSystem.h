// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MSVFXSFXBudgetSystem.generated.h"

/**
 * VFX&SFX를 생성하고 풀링하는 게임 인스턴스 하위 시스템
 * VFX&SFX의 풀을 유지 관리하여 구성 요소의 반복적인 할당 및 소멸을 방지
 * 요청에 따라 효과를 계속 생성하고, 풀링된 구성 요소가 있는 경우 재사용
 */
UCLASS()
class MAGESQUAD_API UMSVFXSFXBudgetSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMSVFXSFXBudgetSystem();

	// 서브시스템 초기화 함수 (풀과 예산 초기화)
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;


	/*
	* VFX 스폰 함수
	* 지정된 Transform에서 VFX를 스폰한다.
	* System	: 스폰할 VFX 객체. 공통 VFX 기본 클래스를 통해 Cascade/Niagara 모두 지원한다.
	* Transform	: 스폰할 FTransform
	* @return	: 공통 VFX 기본 컴포넌트 클래스를 반환. 용도에 따라 캐스팅해서 사용
	*/
	class UFXSystemComponent* SpawnVFX(class UFXSystemAsset* System, const FTransform& Transform);

	/*
	* SFX 재생 함수
	* 지정된 위치에서 SFX를 재생한다.
	* Sound		: 재생할 SFX 객체
	* Location	: 재생할 FVector
	* @return	: 공통 SFX 기본 컴포넌트 클래스를 반환. 용도에 따라 캐스팅해서 사용
	*/
	class UAudioComponent* PlaySFX(class USoundBase* Sound, const FVector& Location);

protected:
	// Niagara 시스템이 끝났을 때 풀로 되돌리는 콜백 함수
	UFUNCTION()
	void OnNiagaraSystemPooled(UNiagaraComponent* FinishedComponent);

	// Particle 시스템이 끝났을 때 풀로 되돌리는 콜백 함수
	UFUNCTION()
	void OnParticleSystemPooled(UParticleSystemComponent* PSC);

protected:
	// VFX 풀링 배열 (Cascade or Niagara)
	TArray<class UFXSystemComponent*> VFXPool;

	// SFX 풀링 배열
	TArray<class UAudioComponent*> SFXPool;
};
