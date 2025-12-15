// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/MageSquadTypes.h"
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

	// 발사체의 원본 데이터를 가져오는 함수
	UFUNCTION(BlueprintCallable)
	static const UProjectileStaticData* GetProjectileStaticData(TSubclassOf<UProjectileStaticData> ProjectileDataClass);

	// 발사체의 원본 데이터로부터 런타임 데이터를 생성하는 함수
	UFUNCTION(BlueprintCallable)
	static FProjectileRuntimeData MakeProjectileRuntimeData(TSubclassOf<UProjectileStaticData> ProjectileDataClass);

	// 발사체 발사 함수 (원본 데이터)
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static class AMSBaseProjectile* LaunchProjectileNative(
		UObject* WorldContextObject,	// 월드 객체
		TSubclassOf<UProjectileStaticData> ProjectileDataClass,	// 발사체 원본 데이터 클래스
		FTransform Transform,			// 위치 및 회전값
		AActor* Owner,					// 발사체 소유자
		APawn* Instigator				// 발사체를 발사한 객체
	);

	// 발사체 발사 함수 (재정의 데이터)
	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject"))
	static class AMSBaseProjectile* LaunchProjectile(
		UObject* WorldContextObject,	// 월드 객체
		TSubclassOf<UProjectileStaticData> ProjectileDataClass,	// 발사체 원본 데이터 클래스
		FProjectileRuntimeData RuntimeData,						// 발사체 재정의 데이터
		FTransform Transform,			// 위치 및 회전값
		AActor* Owner,					// 발사체 소유자
		APawn* Instigator				// 발사체를 발사한 객체
	);
};
