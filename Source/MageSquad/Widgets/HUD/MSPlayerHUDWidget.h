// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "GameplayEffectTypes.h"
#include "MSPlayerHUDWidget.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/17
 *
 * 플레이어 HUD 위젯
 * - 로컬 체력
 * - 로컬 스킬/쿨다운
 * - 팀원 상태(자신 제외)
 * - 공유 경험치/레벨
 */
UCLASS()
class MAGESQUAD_API UMSPlayerHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:

};
