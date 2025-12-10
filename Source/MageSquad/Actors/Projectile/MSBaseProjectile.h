// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSBaseProjectile.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/10
 *
 * 마법 및 원거리 공격에 사용되는 기본 발사체 클래스
 * 연속적인 움직임을 재현하지 않고, 대신 서버에서 투사체를 생성하고 생성 위치와 방향을 포함하는 초기화를 멀티캐스트 함
 */
UCLASS()
class MAGESQUAD_API AMSBaseProjectile : public AActor
{
	GENERATED_BODY()

public:
	AMSBaseProjectile();

	/*
	* 발사체 초기화 함수
	* SpawnLocation	: 생성 위치
	* Direction		: 방향
	* Speed			: 발사 속도
	* LifeTime		: 생명주기
	*/
	// 발사체 초기화 함수
	virtual void InitProjectile(const FVector_NetQuantize& SpawnLocation, const FVector_NetQuantize& Direction, float Speed, float LifeTime);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnProjectileOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

protected:
	// 발사체 무브먼트 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TObjectPtr<class UProjectileMovementComponent> MovementComp;

	// 발사체 콜리전 컴포넌트
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	TObjectPtr<class UBoxComponent> CollisionComp;

	// 발사체 대미지 (이후에 스킬과 연동하면서 없어질 가능성 농후)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Projectile")
	float Damage;

	// 발사체 생명주기 타이머
	FTimerHandle LifeTimerHandle;

	// 발사체 생명주기
	float LifeDuration;
};
