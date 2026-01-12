// Copyright Notice

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSTeamReviveActor.generated.h"

/**
 * 작성자: 김준형
 * 작성일: 25/12/25
 *
 * 팀 부활용 액터 클래스
 * 서버 권한으로만 진행/승계/완료 등의 진행을 판정/계산하며 최소 데이터만을 복제하여 클라이언트에서 보간 처리하게 한다.
 * 클라이언트는 OnRep 함수를 통해 진행률을 업데이트하며 비주얼 및 이펙트를 로컬에서 처리한다.
 */
UCLASS()
class MAGESQUAD_API AMSTeamReviveActor : public AActor
{
	GENERATED_BODY()

public:
	AMSTeamReviveActor();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 부활 액터 초기화 함수
	 * @param InDownedCharacter	: 사망한 캐릭터
	 * @param InReviveDuration	: 부활 진행 시간(초)
	 */
	void Initialize(class AMSPlayerCharacter* InDownedCharacter, float InReviveDuration);

protected:
	// Area 오버랩 시작 함수
	UFUNCTION()
	void OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Area 오버랩 종료 함수
	UFUNCTION()
	void OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 부활 진행률 갱신 OnRep 함수
	UFUNCTION()
	void OnRep_ProgressByte();

private:
	// 서버: 부활 진행/감소 관련 Tick 로직 수행 함수
	void Tick_Server(float DeltaSeconds);

	// 서버: 부활 진행자 유효 검사 및 승계 처리 함수
	void ResolveReviver_Server();

	// 서버: 부활 진행 가능/불가능 여부 반환 함수
	bool CanIncrease_Server() const;

	// 서버: 부활 진행률 증가/감소 함수
	void UpdateProgress_Server(float DeltaSeconds, bool bIncrease);

	// 서버: 부활(완료) 시도 함수
	void TryComplete_Server();

	// 서버: 장판에 오버랩 된 다른 생존자를 찾는 함수
	APawn* FindFirstOverlappingPawn_Server(APawn* Excluded) const;

	// 애니메이션 시각화 업데이트 함수
	void UpdateVisuals(float InProgress);

protected:
	// Root
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> Root;

	// 부활 장판 콜리전
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> AreaComp;

	// 마커(상승/하강)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	TObjectPtr<class UStaticMeshComponent> MarkerComp;

	// 장판(축소/확대)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	TObjectPtr<class UStaticMeshComponent> RingComp;

	// 장판(기존 범위 확인용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	TObjectPtr<class UStaticMeshComponent> RingOriginComp;

	// 마커 상승 높이
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	float MarkerRiseHeight = 1000.f;

	// 장판 최대 스케일(Progress==0)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	float AreaMaxScale = 1.f;

	// 장판 최소 스케일(Progress==1)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	float AreaMinScale = 0.02f;

private:
	// 부활 대상
	UPROPERTY(Replicated)
	TObjectPtr<class AMSPlayerCharacter> DownedCharacter;

	// 현재 부활 진행자(디버그/관찰용, 로직은 서버에서만 사용)
	UPROPERTY(Replicated)
	TObjectPtr<APawn> CurrentReviver = nullptr;

	// 부활 진행 시간(초)
	UPROPERTY(Replicated)
	float ReviveDuration = 2.5f;

	// 부활 진행률(0~255). Progress만 복제해서 클라가 그대로 연출
	UPROPERTY(ReplicatedUsing = OnRep_ProgressByte)
	uint8 RepProgressByte = 0;

private:
	// 서버에서 사용하는 부활 진행률(0~1)
	float ServerProgress = 0.f;

	// 시각화 진행 여부
	bool bVisualInitialized = false;

	// 부활용 액터의 기본 Z값
	float MarkerBaseZ = 0.f;

	// 부활 Area의 기본 스케일 값
	FVector RingBaseScale = FVector(1.f);
};
