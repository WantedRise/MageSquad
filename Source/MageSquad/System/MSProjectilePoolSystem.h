// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MSProjectilePoolSystem.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/11
 *
 * 발사체 풀링 서브시스템
 * 기본 발사체 클래스를 풀링 대상으로 사용
 * 발사체 수명이 끝나면 Destroy 대신 풀로 반환하여 재사용
 */
UCLASS()
class MAGESQUAD_API UMSProjectilePoolSystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	// 서브시스템 Getter
	UFUNCTION(BlueprintCallable, Category = "Custom | System", meta = (WorldContext = "WorldContextObject"))
	static UMSProjectilePoolSystem* GetProjectilePoolSystem(UObject* WorldContextObject);

	// 풀링에 사용할 발사체 클래스와 초기 풀 사이즈 설정 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | System")
	void Configure(TSubclassOf<class AMSBaseProjectile> InProjectileClass, int32 InInitialPoolSize = 32);

	// 발사체 스폰 함수 (서버에서만 호출 추천)
	UFUNCTION(BlueprintCallable, Category = "Custom | System")
	class AMSBaseProjectile* SpawnProjectile(const FTransform& SpawnTransform, AActor* Instigator);

	// 발사체를 풀에 수동으로 반환하는 함수
	UFUNCTION(BlueprintCallable, Category = "Custom | System")
	void ReturnProjectile(class AMSBaseProjectile* InProjectile);

private:
	// 내부 World 접근 함수
	UWorld* GetTypeWorld() const;

	// 초기 발사체 풀 채우기 함수
	void PrewarmPool();

	// 발사체 생명주기 종료 델리게이트 콜백 함수
	void HandleProjectileFinished(class AMSBaseProjectile* InProjectile);

protected:
	// 풀링에 사용할 기본 발사체 클래스
	UPROPERTY(EditDefaultsOnly, Category = "Custom | System")
	TSubclassOf<class AMSBaseProjectile> PoolProjectileClass;

	// 초기 풀 사이즈
	UPROPERTY(EditDefaultsOnly, Category = "Custom | System")
	int32 InitialPoolSize = 32;

	// 재사용 가능한 발사체 리스트
	TArray<TObjectPtr<class AMSBaseProjectile>> FreeProjectiles;
};
