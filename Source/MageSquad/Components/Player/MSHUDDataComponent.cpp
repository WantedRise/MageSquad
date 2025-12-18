// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Player/MSHUDDataComponent.h"

#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"

#include "Net/UnrealNetwork.h"

UMSHUDDataComponent::UMSHUDDataComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	SetIsReplicatedByDefault(true);
}

void UMSHUDDataComponent::BindToASC_Server(UMSPlayerAbilitySystemComponent* InASC)
{
	// 서버가 아니면 종료
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// ASC 초기화
	BoundASC = InASC;
	if (!BoundASC) return;

	// 최신 현재/최대 체력 값을 받아와 초기화
	RepHealth = BoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetHealthAttribute());
	RepMaxHealth = BoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetMaxHealthAttribute());

	// AttributeSet의 현재 체력 변경 델리게이트에 콜백 함수 바인딩 (로컬, 팀 모두)
	BoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UMSHUDDataComponent::HandleHealthChanged);

	// AttributeSet의 최대 체력 변경 델리게이트에 콜백 함수 바인딩 (로컬, 팀 모두)
	BoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UMSHUDDataComponent::HandleMaxHealthChanged);

	// 데이터 변경 브로드캐스트
	BroadcastChanged();
}

void UMSHUDDataComponent::SetDisplayName_Server(const FText& InName)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// 플레이어 이름 초기화
	RepDisplayName = InName;

	// 데이터 변경 브로드캐스트
	BroadcastChanged();
}

void UMSHUDDataComponent::SetPortraitIcon_Server(UTexture2D* InPortrait)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// 플레이어 아이콘 초기화
	RepPortraitIcon = InPortrait;

	// 데이터 변경 브로드캐스트
	BroadcastChanged();
}

void UMSHUDDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMSHUDDataComponent, RepDisplayName);
	DOREPLIFETIME(UMSHUDDataComponent, RepPortraitIcon);
	DOREPLIFETIME(UMSHUDDataComponent, RepHealth);
	DOREPLIFETIME(UMSHUDDataComponent, RepMaxHealth);
}

void UMSHUDDataComponent::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	// 현재 체력 갱신 후 데이터 변경 브로드캐스트
	RepHealth = Data.NewValue;
	BroadcastChanged();
}

void UMSHUDDataComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	// 최대 체력 갱신 후 데이터 변경 브로드캐스트
	RepMaxHealth = Data.NewValue;
	BroadcastChanged();
}

void UMSHUDDataComponent::BroadcastChanged()
{
	// 현재 체력 비율 계산 (ex. 0.1 = 10%)
	const float Pct = (RepMaxHealth > 0.f) ? FMath::Clamp(RepHealth / RepMaxHealth, 0.f, 1.f) : 0.f;

	// 현재 체력 변경 브로드캐스트
	OnPublicHealthChanged.Broadcast(RepHealth, RepMaxHealth, Pct);
}

void UMSHUDDataComponent::OnRep_Data()
{
	BroadcastChanged();
}
