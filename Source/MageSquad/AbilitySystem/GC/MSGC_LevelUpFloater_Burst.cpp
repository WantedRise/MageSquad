// Fill out your copyright notice in the Description page of Project Settings.


#include "AbilitySystem/GC/MSGC_LevelUpFloater_Burst.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "GameplayTagsManager.h"

#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

#include "Widgets/HUD/MSLevelUpFloaterWidget.h"

UMSGC_LevelUpFloater_Burst::UMSGC_LevelUpFloater_Burst()
{
}

bool UMSGC_LevelUpFloater_Burst::OnExecute_Implementation(AActor* Target, const FGameplayCueParameters& Parameters) const
{
	if (!Target || !LevelUpFloaterWidget) return false;

	UWorld* World = Target->GetWorld();
	if (!World) return false;

	// 표시 위치 설정. 액터 머리 위 느낌을 위해 월드 Z 오프셋 적용
	const FVector WorldLocation = FVector(Parameters.Location) + FVector(0.f, 0.f, WorldZOffset);

	// 각 클라이언트는 자기 로컬 컨트롤러에만 UI를 띄움
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (!PC || !PC->IsLocalController()) continue;

		// 나이아가라 스폰
		if (LevelUpNiagara)
		{
			UNiagaraComponent* Niagara = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
				World,
				LevelUpNiagara,
				Parameters.Location,
				FRotator::ZeroRotator
			);
		}

		// 월드 좌표를 스크린 좌표로 변환
		FVector2D ScreenPos;
		if (!UGameplayStatics::ProjectWorldToScreen(PC, WorldLocation, ScreenPos, true))
		{
			continue;
		}

		// 레벨업 플로터 위젯 풀링 or 생성
		UMSLevelUpFloaterWidget* Widget = UMSLevelUpFloaterWidget::Acquire(PC, LevelUpFloaterWidget);
		if (!Widget)
		{
			continue;
		}

		// 위젯 표시 시작
		Widget->Start(ScreenPos);
	}

	return true;
}
