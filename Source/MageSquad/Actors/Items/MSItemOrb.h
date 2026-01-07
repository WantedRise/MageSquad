// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSItemOrb.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 26/01/07
 *
 * 아이템 오브 액터 베이스
 * 모든 오브가 상속받아 동일한 획득 연출과 파괴 처리를 공유
 * 자식 클래스에서 Collect_Server를 재정의하여 각자 효과를 구현
 * - 컴포넌트 구성 + 콜리전 설정
 * - Tick 기반 연출 (획득 연출 애니메이션 수행, 클라이언트 자체 수행)
 * - 서버 지연 Destroy 타이머
 * - 중복 획득/연출 재시작 방지
 */
UCLASS()
class MAGESQUAD_API AMSItemOrb : public AActor
{
	GENERATED_BODY()

public:
	AMSItemOrb();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void Tick(float DeltaSeconds) override;

	/*
	* 서버: 오브 획득 처리 함수
	* 반드시 서버에서만 호출되어야 함
	* 자식 클래스는 Super::Collect_Server를 호출한 후, 각자 효과를 구현
	* @param CollectorActor: 획득자
	*/
	virtual void Collect_Server(AActor* CollectorActor);

protected:
	// 획득 연출 시작 함수. (Multicast를 통해 모든 클라이언트 연출 동기화)
	UFUNCTION(NetMulticast, Reliable)
	void MulticastRPCBeginAttract(AActor* InTargetActor);

	// 서버: 지연 Destroy 함수 (연출을 위해 지연)
	void DeferredDestroy_Server();

	// bCollected 변경 시, 클라이언트 측에서 충돌 비활성화를 동기화하기 위한 OnRep 함수
	UFUNCTION()
	void OnRep_Collected();

protected:
	// 루트 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Components")
	TObjectPtr<USceneComponent> RootComp;

	// 오브 외형 스태틱 메시
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Components")
	TObjectPtr<class UStaticMeshComponent> MeshComp;

	// 충돌 컴포넌트
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | Components")
	TObjectPtr<class UBoxComponent> CollisionComp;


	/*****************************************************
	* Attraction Section
	*****************************************************/
protected:
	// 떠오르는 연출 높이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float FloatUpHeight = 200.f;

	// 떠오르는 연출 시간
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float FloatUpTime = 0.20f;

	// 플레이어로 빨려들어가는 속도 (클수록 빠름)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float AttractInterpSpeed = 8.f;

	// 오브 획득 후 Destroy까지 지연 시간 (연출 재생 시간 확보)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Animation")
	float DestroyDelay = 0.5f;

protected:
	// 중복 획득 방지 플래그 (서버에서만 true로 설정)
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

	// 타겟(획득자) 액터. (클라이언트에게만 보여줄 연출용 액터)
	TWeakObjectPtr<AActor> AttractionActor;
};
