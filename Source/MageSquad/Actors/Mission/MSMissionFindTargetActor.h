// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSMissionFindTargetActor.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnTargetCollected);

UCLASS()
class MAGESQUAD_API AMSMissionFindTargetActor : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSMissionFindTargetActor();

	/** 수집 시 호출되는 델리게이트 */
	FOnTargetCollected OnTargetCollected;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
protected:
	UPROPERTY(VisibleAnywhere)
	class USphereComponent* Collision;

	UFUNCTION()
	void OnOverlapBegin(
		UPrimitiveComponent* OverlappedComp,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult
	);

protected:
	// 서버가 호출하면 모든 클라이언트에서 실행되는 함수
	UFUNCTION(NetMulticast, Reliable)
	void MulticastPlaySound();
protected:
	// 방향 표시 인디케이터 컴포넌트 (팀원에게 위치 및 초상화 제공)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Indicator")
	TObjectPtr<class UMSDirectionIndicatorComponent> DirectionIndicatorComponent;

	UPROPERTY(EditAnywhere, Category = "Float")
	float FloatHeight = 25.f;

	UPROPERTY(EditAnywhere, Category = "Float")
	float FloatSpeed = 1.5f;

	FVector BaseLocation;
	float RunningTime = 0.f;

	// 사운드
	UPROPERTY(EditDefaultsOnly, Category = "Custom | SFX")
	TObjectPtr<class USoundBase> StartSound;
};
