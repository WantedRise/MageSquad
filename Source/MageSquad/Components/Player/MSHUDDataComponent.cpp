// Fill out your copyright notice in the Description page of Project Settings.


#include "Components/Player/MSHUDDataComponent.h"

#include "AbilitySystemComponent.h"
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
	// 서버에서만 바인딩/갱신
	if (!GetOwner() || !GetOwner()->HasAuthority() || !InASC) return;

	// ASC ActorInfo가 아직 준비되지 않았으면 바인딩을 보류하는게 안전
	// 하지만 PossessedBy 이후 호출되는 것을 전제로 여기서는 최소 체크만 수행
	if (!InASC->GetOwnerActor() || !InASC->GetAvatarActor()) return;

	// 기존 바인딩이 있다면 정리
	if (BoundASC)
	{
		if (HealthChangedHandle.IsValid())
		{
			BoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetHealthAttribute()).Remove(HealthChangedHandle);
			HealthChangedHandle.Reset();
		}
		if (MaxHealthChangedHandle.IsValid())
		{
			BoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetMaxHealthAttribute()).Remove(MaxHealthChangedHandle);
			MaxHealthChangedHandle.Reset();
		}
	}

	// ASC 초기화
	BoundASC = InASC;

	// 초기값 반영
	PullHealthFromASC();

	// 델리게이트 바인딩 (서버에서만)
	HealthChangedHandle =
		BoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetHealthAttribute())
		.AddUObject(this, &UMSHUDDataComponent::HandleHealthChanged);

	MaxHealthChangedHandle =
		BoundASC->GetGameplayAttributeValueChangeDelegate(UMSPlayerAttributeSet::GetMaxHealthAttribute())
		.AddUObject(this, &UMSHUDDataComponent::HandleMaxHealthChanged);

	// 서버에서도 즉시 브로드캐스트
	BroadcastHealth();
}

void UMSHUDDataComponent::BindDisplayName_Server(const FText& InName)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// 이름 초기화
	RepDisplayName = InName;

	// 서버에서는 직접 호출해 UI 갱신
	OnRep_PublicData();
}

void UMSHUDDataComponent::BindPortraitIcon_Server(UTexture2D* InPortrait)
{
	if (!GetOwner() || !GetOwner()->HasAuthority()) return;

	// 플레이어 아이콘 초기화
	RepPortraitIcon = InPortrait;

	// 서버에서는 직접 호출해 UI 갱신
	OnRep_PublicData();
}

void UMSHUDDataComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UMSHUDDataComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UMSHUDDataComponent, RepHealth);
	DOREPLIFETIME(UMSHUDDataComponent, RepMaxHealth);
	DOREPLIFETIME(UMSHUDDataComponent, RepDisplayName);
	DOREPLIFETIME(UMSHUDDataComponent, RepPortraitIcon);
}

void UMSHUDDataComponent::PullHealthFromASC()
{
	if (!BoundASC) return;

	// ASC에서 초기값을 읽어 Rep 변수에 반영
	RepHealth = BoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetHealthAttribute());
	RepMaxHealth = BoundASC->GetNumericAttribute(UMSPlayerAttributeSet::GetMaxHealthAttribute());
}

void UMSHUDDataComponent::BroadcastHealth()
{
	// 현재 체력 비율 계산 (ex. 0.1 = 10%)
	const float Pct = (RepMaxHealth > 0.f) ? FMath::Clamp(RepHealth / RepMaxHealth, 0.f, 1.f) : 0.f;

	// 현재 체력 변경 브로드캐스트
	OnPublicHealthChanged.Broadcast(RepHealth, RepMaxHealth, Pct);
}

void UMSHUDDataComponent::HandleHealthChanged(const FOnAttributeChangeData& Data)
{
	// 현재 체력 갱신 후 데이터 변경 브로드캐스트
	RepHealth = Data.NewValue;
	BroadcastHealth();
}

void UMSHUDDataComponent::HandleMaxHealthChanged(const FOnAttributeChangeData& Data)
{
	// 최대 체력 갱신 후 데이터 변경 브로드캐스트
	RepMaxHealth = Data.NewValue;
	BroadcastHealth();
}

void UMSHUDDataComponent::OnRep_PublicData()
{
	// UI 갱신 이벤트 호출
	BroadcastHealth();
}
