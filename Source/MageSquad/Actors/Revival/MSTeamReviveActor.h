// Copyright Notice

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSTeamReviveActor.generated.h"

/**
 * 부활 상태 스냅샷 구조체
 * 서버는 "상태 변화" 시점(진행 시작/정지/승계/완료)에만 RepState를 갱신
 * 클라이언트는 GameState의 ServerWorldTimeSeconds를 이용해 로컬에서 진행률을 예측/보간
 */
USTRUCT(BlueprintType)
struct FMSReviveRepState
{
	GENERATED_BODY()

	// 서버 시간
	UPROPERTY()
	float ServerTimeStamp = 0.f;

	// 서버 시작 시점의 부활 진행률(0~1)
	UPROPERTY()
	float Progress = 0.f;

	// 진행도 증가/감소 여부 플래그 (true면 증가, false면 감소)
	UPROPERTY()
	bool bIncreasing = false;

	// 총 부활 진행 시간(초)
	UPROPERTY()
	float Duration = 2.5f;

	// 현재 부활 진행자
	UPROPERTY()
	TObjectPtr<APawn> Reviver = nullptr;
};

/**
 * 작성자: 김준형
 * 작성일: 25/12/25
 *
 * 팀 부활용 액터 클래스
 * 서버 권한으로만 진행 상태를 계산하며 최소 데이터만을 복제하여 클라이언트에서 보간 처리하게 한다.
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

	// 부활 상태 변경 OnRep 함수. 클라에서 시각화 갱신 트리거
	UFUNCTION()
	void OnRep_ReviveState();

	// 서버: 부활 진행/완료 처리 함수
	void TickUpdate_Server(float DeltaSeconds);

	// 서버 시간 기준 진행률 계산 함수
	float ComputeProgressAtServerTime(float ServerTimeSeconds) const;

	// 서버 시간 획득 함수
	float GetServerTimeSecondsForVisual() const;

	// 시각화 갱신 함수
	void UpdateVisuals(float InProgress);

	// 서버에서 상태 변경 스냅샷 갱신 함수 (승계/시작/정지)
	void SetState_Server(bool bInIncreasing, APawn* NewReviver);

	// 서버: 현재 장판 안에 오버랩 중인 Pawn 중 첫 번째 유효 후보를 반환하는 함수
	APawn* FindFirstOverlappingPawn_Server(APawn* Excluded) const;


	/*****************************************************
	* Visualization Section
	*****************************************************/
protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USceneComponent> Root;

	// 장판 Area 콜리전
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> AreaComp;

	// 부활용 비석
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	TObjectPtr<class UStaticMeshComponent> MarkerComp;

	// 장판 Area 메시
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	TObjectPtr<class UStaticMeshComponent> RingComp;

	// 장판 Area 메시2 (범위 확인용)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	TObjectPtr<class UStaticMeshComponent> RingComp2;

	// 마커 상승 높이(월드/로컬 Z)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	float MarkerRiseHeight = 1000.f;

	// Area 스케일(Progress=1일 때)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	float AreaMinScale = 0.02f;

	// Area 스케일(Progress=0일 때)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Custom | Revive")
	float AreaMaxScale = 1.f;


	/* ======================== Replication ======================== */
	// 다운된 캐릭터(부활 대상)
	UPROPERTY(Replicated)
	TWeakObjectPtr<class AMSPlayerCharacter> DownedCharacter;

	// 부활 상태 스냅샷. (Tick 복제 대신 상태 변화에만 갱신)
	UPROPERTY(ReplicatedUsing = OnRep_ReviveState)
	FMSReviveRepState RepState;

	// 부활 진행 시간(초). Initialize에서 설정되며 RepState.Duration에도 기록
	UPROPERTY(Replicated)
	float ReviveDuration = 2.5f;
	/* ======================== Replication ======================== */

private:
	// 시각화 진행 여부
	bool bVisualInitialized = false;

	// 부활용 액터의 기본 Z값
	float MarkerBaseZ = 0.f;

	// 부활 Area의 기본 스케일 값
	FVector RingBaseScale = FVector(1.f);
};
