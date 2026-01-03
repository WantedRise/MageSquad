// Copyright (c) 2025 MageSquad

#include "Components/MSDirectionIndicatorComponent.h"
#include "Components/Player/MSHUDDataComponent.h"

#include "Player/MSPlayerCharacter.h"

#include "System/MSDirectionIndicatorSubsystem.h"

UMSDirectionIndicatorComponent::UMSDirectionIndicatorComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
	bRegistered = false;
}

void UMSDirectionIndicatorComponent::BeginPlay()
{
	Super::BeginPlay();

	// HUDDataComponent 캐시 시도
	if (!CachedHUDData)
	{
		if (AActor* OwnerActor = GetOwner())
		{
			// 소유자에게 HUDDataComponent를 찾아 저장 시도
			CachedHUDData = OwnerActor->FindComponentByClass<UMSHUDDataComponent>();
		}
	}

	// 뱡향 표시 인디케이터 서브시스템에 등록
	if (GetWorld())
	{
		if (UMSDirectionIndicatorSubsystem* Subsystem = GetWorld()->GetSubsystem<UMSDirectionIndicatorSubsystem>())
		{
			// 서브시스템에 이 컴포넌트 등록
			Subsystem->RegisterSource(this);
			bRegistered = true;
		}
	}
}

void UMSDirectionIndicatorComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 방향 표시 인디케이터 서브시스템에서 이 컴포넌트 등록 해제
	if (bRegistered && GetWorld())
	{
		if (UMSDirectionIndicatorSubsystem* Subsystem = GetWorld()->GetSubsystem<UMSDirectionIndicatorSubsystem>())
		{
			// 서브시스템에 이 컴포넌트 등록 해제
			Subsystem->UnregisterSource(this);
		}
	}

	bRegistered = false;
	Super::EndPlay(EndPlayReason);
}

FVector UMSDirectionIndicatorComponent::GetIndicatorWorldLocation() const
{
	// 소유자의 위치 반환
	if (const AActor* OwnerActor = GetOwner())
	{
		return OwnerActor->GetActorLocation();
	}

	return FVector::ZeroVector;
}

UTexture2D* UMSDirectionIndicatorComponent::GetIndicatorIcon() const
{
	// 인디케이터 표시 아이콘이 있다면 이를 반환
	if (OverrideIcon)
	{
		return OverrideIcon;
	}

	// HUDData의 초상화 아이콘 반환
	if (CachedHUDData)
	{
		return CachedHUDData->GetPortraitIcon();
	}

	return nullptr;
}

bool UMSDirectionIndicatorComponent::ShouldShowIndicatorForPlayer(APlayerController* LocalPlayerController) const
{
	if (!LocalPlayerController) return false;

	// 자기 자신은 표시하지 않음
	if (APawn* LocalPawn = LocalPlayerController->GetPawn())
	{
		if (LocalPawn == GetOwner())
		{
			return false;
		}
	}

	// 비가시화를 원하는 경우 표시하지 않음
	if (!bRequiresActivation)
	{
		return false;
	}

	return true;
}
