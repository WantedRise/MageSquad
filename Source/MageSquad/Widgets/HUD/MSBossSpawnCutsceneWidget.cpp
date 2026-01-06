// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/MSBossSpawnCutsceneWidget.h"

#include "GameStates/MSGameState.h"

void UMSBossSpawnCutsceneWidget::NativeConstruct()
{
	Super::NativeConstruct();
    
	// 바인딩 시도 함수 호출
	TryBindToGameState();
}

void UMSBossSpawnCutsceneWidget::TryBindToGameState()
{
	if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
	{
		// 성공적으로 찾았을 때 바인딩
		GS->OnBossSpawnCutsceneStateChanged.AddDynamic(this, &ThisClass::HandleBossSpawnEvent);
		//UE_LOG(LogTemp, Warning, TEXT("HUD Construct: Binding Success!"));
	}
	else
	{
		// 아직 GameState가 NULL이면 다음 프레임에 다시 시도 (성공할 때까지)
		GetWorld()->GetTimerManager().SetTimerForNextTick(this, &UMSBossSpawnCutsceneWidget::TryBindToGameState);
	}
}

void UMSBossSpawnCutsceneWidget::HandleBossSpawnEvent(bool bStart)
{
	if (!CutSceneAnim)
	{
		return;
	}
	
	if (bStart)
	{
		PlayAnimation(CutSceneAnim);
	}
	else
	{
		// 종료 시 역재생
		PlayAnimation(CutSceneAnim, 0.f, 1, EUMGSequencePlayMode::Reverse);
	}
}
