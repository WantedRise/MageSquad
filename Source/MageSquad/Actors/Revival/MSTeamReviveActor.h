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
 * 서버 권한으로만 진행 상태를 계산하며 최소 데이터만을 복제하여 클라이언트에서 보간 처리하게 한다.
 * 클라이언트는 OnRep 함수를 통해 진행률을 업데이트하며 비주얼 및 이펙트를 로컬에서 처리한다.
 */
UCLASS()
class MAGESQUAD_API AMSTeamReviveActor : public AActor
{
	GENERATED_BODY()

public:
	AMSTeamReviveActor();

	virtual void Tick(float DeltaSeconds) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/**
	 * 부활 액터 초기화 함수
	 * @param InDownedCharacter	: 사망한 캐릭터 포인터
	 * @param InReviveDuration	: 부활 진행 시간
	 */
	void Initialize(class AMSPlayerCharacter* InDownedCharacter, float InReviveDuration);

protected:
	virtual void BeginPlay() override;

	// Area 오버랩 시작 함수
	UFUNCTION()
	void OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	// Area 오버랩 종료 함수
	UFUNCTION()
	void OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 부활 진행률 OnRep 함수. 클라이언트는 이 함수를 통해 로컬 보간을 갱신
	UFUNCTION()
	void OnRep_ReviveProgress();

	// 부활 진행률 업데이트 함수
	void UpdateRevive(float DeltaSeconds);

protected:
	// 다운된(죽은) 캐릭터
	UPROPERTY(Replicated)
	TWeakObjectPtr<class AMSPlayerCharacter> DownedCharacter;

	// 부활 진행 오버랩 콜리전
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<class USphereComponent> AreaComp;

	// 부활 진행률(0~1). 서버에서 계산하고 클라이언트에 복제
	UPROPERTY(ReplicatedUsing = OnRep_ReviveProgress)
	float ReviveProgress;

	// 부활 진행 시간
	UPROPERTY(Replicated)
	float ReviveDuration;

	// 서버에서 시작된 부활 진행 시간. 서버 전용
	float ServerStartTime;

	// 현재 부활 진행자. 서버에서만 유지
	UPROPERTY(Transient)
	TWeakObjectPtr<APawn> CurrentReviver;
};
