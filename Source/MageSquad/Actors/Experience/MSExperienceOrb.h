// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSExperienceOrb.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/21
 *
 * 경험치 오브 액터
 * 누군가 획득하면 서버가 Multicast로 연출(떠오름 -> 빨려들어감)
 */
UCLASS()
class MAGESQUAD_API AMSExperienceOrb : public AActor
{
	GENERATED_BODY()

public:
	AMSExperienceOrb();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	/*
	* 경험치 획득 처리 함수 (서버 전용)
	* CollectorActor : 개인 보정치(획득량 등) 계산 기준
	*/
	void Collect_Server(AActor* CollectorActor);

protected:
	// 획득 연출 시작 함수 (Multicast를 통해 모든 클라이언트 동기화)
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBeginAttract(AActor* InTargetActor);

	// 지연 Destroy 함수 (서버 전용)
	void DeferredDestroy_Server();

	// 중복 획득 방지용 플래그 변경 OnRep 함수
	UFUNCTION()
	void OnRep_Collected();

public:
	// 경험치 오브가 제공하는 기본 경험치(보정 전)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Experience")
	float ExperienceValue = 10.f;

protected:
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Components")
	TObjectPtr<USceneComponent> RootComp;

	// 경험치 외형 스태틱 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Components")
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Components")
	TObjectPtr<class UBoxComponent> CollisionComp;



	/*****************************************************
	* Attraction Section
	*****************************************************/
public:
	// 떠오르는 연출 높이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float FloatUpHeight = 60.f;

	// 떠오르는 연출 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float FloatUpTime = 0.12f;

	// 플레이어로 빨려들어가는 속도 (클수록 빠름)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float AttractInterpSpeed = 12.f;

	// 경험치 오브 획득 후 서버에서 Destroy까지 지연 (연출 재생 시간 확보)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float DestroyDelay = 0.35f;

private:
	// 중복 획득 방지용 플래그 (서버에서만 true로 설정)
	UPROPERTY(ReplicatedUsing = OnRep_Collected)
	bool bCollected = false;

	// 연출 진행 여부
	bool bAttracting = false;

	// 연출 시작 후 누적 시간
	float AttractElapsed = 0.f;

	// 연출 시작 지점 위치
	FVector AttractStartLocation = FVector::ZeroVector;

	// 떠오름 목표 위치
	FVector FloatUpTargetLocation = FVector::ZeroVector;

	// 클라이언트에게만 보여줄 연출용 액터
	TWeakObjectPtr<AActor> AttractionActor;
};
