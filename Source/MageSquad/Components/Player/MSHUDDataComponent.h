// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Types/MageSquadTypes.h"
#include "MSHUDDataComponent.generated.h"

// 현재 체력 변경 델리게이트 (현재 체력 / 최대 체력 / 체력 비율)
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnPublicHealthChanged, float, float, float);

/**
 * 작성자: 김준형
 * 작성일: 25/12/18
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

	// ASC 바인딩 함수 (서버 전용)
	void BindToASC_Server(class UMSPlayerAbilitySystemComponent* InASC);

	// 팀원 이름 설정 함수 (서버 전용)
	void BindDisplayName_Server(const FText& InName);

	// 팀원 아이콘 설정 함수 (서버 전용)
	void BindPortraitIcon_Server(UTexture2D* InPortrait);


	// 팀원 UI에서 읽기 위한 Getter 함수
	float GetHealth() const { return RepHealth; }
	float GetMaxHealth() const { return RepMaxHealth; }
	FText GetDisplayName() const { return RepDisplayName; }
	UTexture2D* GetPortraitIcon() const { return RepPortraitIcon; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

protected:
	virtual void BeginPlay() override;

	// 모든 공개 데이터 변경 시 호출
	UFUNCTION()
	void OnRep_PublicData();

private:
	// ASC에서 초기값을 읽어 Rep 변수에 반영하는 함수
	void PullHealthFromASC();

	// 체력 변경 브로드캐스트 함수
	void BroadcastHealth();

	// AttributeSet 갱신 콜백 함수
	void HandleHealthChanged(const struct FOnAttributeChangeData& Data);
	void HandleMaxHealthChanged(const struct FOnAttributeChangeData& Data);

public:
	// 복제된 값이 변경되면 호출되는 이벤트 (C++ 전용)
	FOnPublicHealthChanged OnPublicHealthChanged;

protected:
	// 현재 체력
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUD", ReplicatedUsing = OnRep_PublicData)
	float RepHealth = 0.f;

	// 최대 체력
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUD", ReplicatedUsing = OnRep_PublicData)
	float RepMaxHealth = 0.f;

	// 이름
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUD", ReplicatedUsing = OnRep_PublicData)
	FText RepDisplayName;

	// 플레이어 아이콘
	UPROPERTY(BlueprintReadOnly, Category = "Custom | HUD", ReplicatedUsing = OnRep_PublicData)
	TObjectPtr<UTexture2D> RepPortraitIcon;

	// 바인딩 델리게이트 핸들(중복 바인딩/정확한 해제 목적)
	FDelegateHandle HealthChangedHandle;
	FDelegateHandle MaxHealthChangedHandle;

	// 서버 전용 ASC 캐시
	UPROPERTY(Transient)
	TObjectPtr<UMSPlayerAbilitySystemComponent> BoundASC = nullptr;
};
