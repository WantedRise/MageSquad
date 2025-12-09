// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "MSFunctionLibrary.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 전역 헬퍼 함수 라이브러리
 * 블루프린트/C++에서 사용할 다양한 헬퍼 함수 라이브러리 (GAS 포함)
 */
UCLASS()
class MAGESQUAD_API UMSFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	// 액터의 플레이어 ASC를 가져오는 함수
	static class UMSPlayerAbilitySystemComponent* NativeGetPlayerAbilitySystemComponentFromActor(AActor* InActor);

	// 액터의 특정 태그 보유 여부 반환 함수
	static bool NativeDoesActorHaveTag(AActor* InActor, const FGameplayTag TagToCheck);
};
