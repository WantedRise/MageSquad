// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_DamageFloater_Burst.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

#include "Widgets/HUD/MSDamageFloaterWidget.h"

UMSGC_DamageFloater_Burst::UMSGC_DamageFloater_Burst()
{
	// GameplayCue Tag 바인딩
	const UGameplayTagsManager& TagsManager = UGameplayTagsManager::Get();
	GameplayCueTag = TagsManager.RequestGameplayTag(FName("GameplayCue.UI.DamageFloater"), false);

	CriticalTag = FGameplayTag();
}

bool UMSGC_DamageFloater_Burst::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	if (!Target || !DamageFloaterWidget) return false;

	// 피해 수치 가져오기 (받은 피해량 or 회복량)
	const float DeltaHealth = Parameters.RawMagnitude;

	const float Amount = FMath::Abs(DeltaHealth);

	// 회복 여부
	const bool bIsHeal = (DeltaHealth > 0.f);

	// 치명타 여부
	const bool bIsCritical = CriticalTag.IsValid() ? Parameters.AggregatedSourceTags.HasTag(CriticalTag) : false;

	UWorld* World = Target->GetWorld();
	if (!World) return false;

	// 표시 위치 설정. 액터 머리 위 느낌을 위해 월드 Z 오프셋 적용
	const FVector WorldLocation = FVector(Parameters.Location) + FVector(0.f, 0.f, WorldZOffset);

	// 각 클라이언트는 자기 로컬 컨트롤러에만 UI를 띄움
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue;

		// 월드 좌표를 스크린 좌표로 변환
		FVector2D ScreenPos;
		if (!UGameplayStatics::ProjectWorldToScreen(PC, WorldLocation, ScreenPos, true))
		{
			continue;
		}

		// 대미지 플로터 위젯 풀링 or 생성
		UMSDamageFloaterWidget* Widget = UMSDamageFloaterWidget::Acquire(PC, DamageFloaterWidget);
		if (!Widget)
		{
			continue;
		}

		// 위젯 표시 시작
		Widget->Start(Amount, bIsCritical, bIsHeal, ScreenPos + ScreenOffset);
	}

	return true;
}
