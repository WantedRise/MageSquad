// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/MageSquadTypes.h"
#include "MSHUDDataComponent.generated.h"

// 현재 체력 변경 델리게이트
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPublicHealthChanged, float, float, float);

/**
 * 작성자: 김준형
 * 작성일: 25/12/16
 *
 * HUD에 필요한 최소 데이터를 서버 -> 모든 클라이언트로 복제하는 컴포넌트
 * 최소 데이터(체력, 이름, 아이콘, 스킬 슬롯 아이콘 등)
 */
UCLASS(ClassGroup = (UI), meta = (BlueprintSpawnableComponent))
class MAGESQUAD_API UMSHUDDataComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UMSHUDDataComponent();

	// ASC에 바인딩하는 함수 (서버 전용)
	void BindToASC_Server(class UMSPlayerAbilitySystemComponent* InASC);

	// 플레이어 이름 설정 함수 (서버 전용)
	void SetDisplayName_Server(const FText& InName);

	// 플레이어 아이콘 설정 함수 (서버 전용)
	void SetPortraitIcon_Server(UTexture2D* InPortrait);

public:
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	// 현재/최대 체력 바인딩 콜백 함수
	void HandleHealthChanged(const FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const FOnAttributeChangeData& Data);

	// 무언가 데이터가 변경된 이벤트를 받는 콜백 함수
	void BroadcastChanged();

	UFUNCTION()
	void OnRep_Data();

public:
	// 현재 체력 변경 델리게이트 객체
	FOnPublicHealthChanged OnPublicHealthChanged;

protected:
	// 플레이어 이름
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUDData", ReplicatedUsing = OnRep_Data)
	FText RepDisplayName;

	// 플레이어 아이콘
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUDData", ReplicatedUsing = OnRep_Data)
	TObjectPtr<UTexture2D> RepPortraitIcon = nullptr;

	// 현재 체력
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUDData", ReplicatedUsing = OnRep_Data)
	float RepHealth = 0.f;

	// 최대 체력
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUDData", ReplicatedUsing = OnRep_Data)
	float RepMaxHealth = 0.f;

	// ASC
	UPROPERTY(Transient)
	TObjectPtr<class UMSPlayerAbilitySystemComponent> BoundASC = nullptr;
};
