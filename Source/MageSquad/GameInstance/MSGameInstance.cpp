// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/MSGameInstance.h"
#include "AbilitysystemGlobals.h"

void UMSGameInstance::Init()
{
	Super::Init();

	// 어빌리티 시스템의 글로벌 설정과 데이터 관리
	UAbilitySystemGlobals::Get().InitGlobalData();
}
