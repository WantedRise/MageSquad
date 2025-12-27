// Copyright Notice

#include "MSTeamReviveActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Player/MSPlayerCharacter.h"
#include "GameFramework/PlayerState.h"
#include "GameFramework/GameStateBase.h"

#include "GameStates/MSGameState.h"

#include "Net/UnrealNetwork.h"

AMSTeamReviveActor::AMSTeamReviveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickInterval(0.01f);
	bReplicates = true;
	SetReplicateMovement(false);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	// 오버랩 영역 (서버 판정용)
	AreaComp = CreateDefaultSubobject<USphereComponent>(TEXT("ReviveAreaComponent"));
	AreaComp->SetupAttachment(Root);
	AreaComp->InitSphereRadius(1000.f);
	// 플레이어와만 콜리전 오버랩 설정
	AreaComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	AreaComp->SetGenerateOverlapEvents(true);

	// 시각화: 마커(상승)
	MarkerComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MarkerMesh"));
	MarkerComp->SetupAttachment(Root);
	MarkerComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerComp->SetGenerateOverlapEvents(false);
	MarkerComp->SetIsReplicated(false);

	// 시각화: 링(축소)
	RingComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingMesh"));
	RingComp->SetupAttachment(Root);
	RingComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RingComp->SetGenerateOverlapEvents(false);
	RingComp->SetIsReplicated(false);

	RingComp2 = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingComp2"));
	RingComp2->SetupAttachment(Root);
	RingComp2->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RingComp2->SetGenerateOverlapEvents(false);
	RingComp2->SetIsReplicated(false);

	// 초기 RepState
	RepState.Progress = 0.f;
	RepState.bIncreasing = false;
	RepState.Duration = 2.5f;
	RepState.ServerTimeStamp = 0.f;
	RepState.Reviver = nullptr;
}

void AMSTeamReviveActor::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 오버랩 설정
	if (HasAuthority() && AreaComp)
	{
		AreaComp->OnComponentBeginOverlap.AddDynamic(this, &AMSTeamReviveActor::OnAreaBeginOverlap);
		AreaComp->OnComponentEndOverlap.AddDynamic(this, &AMSTeamReviveActor::OnAreaEndOverlap);
	}

	// 시각화 캐시/머티리얼 설정
	if (GetNetMode() != NM_DedicatedServer)
	{
		// 위치/크기 초기화
		MarkerBaseZ = MarkerComp ? MarkerComp->GetRelativeLocation().Z : 0.f;
		RingBaseScale = RingComp ? RingComp->GetRelativeScale3D() : FVector(1.f);

		// 시각화 시작 플래그 설정
		bVisualInitialized = true;
	}
}

void AMSTeamReviveActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		// 서버에서 부활 진행/완료 처리 시작
		TickUpdate_Server(DeltaSeconds);
	}

	// 로컬 클라이언트 시각화 (Tick 로컬 연출, 복제 X)
	if (GetNetMode() != NM_DedicatedServer)
	{
		// 부활 진행/완료 시간 가져오기
		const float NowServer = GetServerTimeSecondsForVisual();

		// 서버 시간 기준 부활 진행률 계산
		const float Pct = ComputeProgressAtServerTime(NowServer);

		// 시각화 갱신
		UpdateVisuals(Pct);
	}
}

void AMSTeamReviveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSTeamReviveActor, DownedCharacter);
	DOREPLIFETIME(AMSTeamReviveActor, RepState);
	DOREPLIFETIME(AMSTeamReviveActor, ReviveDuration);
}

void AMSTeamReviveActor::Initialize(AMSPlayerCharacter* InDownedCharacter, float InReviveDuration)
{
	if (!HasAuthority()) return;

	// 사망(다운)한 캐릭터와 부활 진행 시간 초기화
	DownedCharacter = InDownedCharacter;
	ReviveDuration = FMath::Max(0.01f, InReviveDuration);

	// 부활 진행 상태 스냅샷 초기화
	const float Now = GetWorld() ? GetWorld()->GetTimeSeconds() : 0.f;
	RepState.ServerTimeStamp = Now;
	RepState.Progress = 0.f;
	RepState.Duration = ReviveDuration;
	RepState.bIncreasing = false;
	RepState.Reviver = nullptr;
	ForceNetUpdate();
}

void AMSTeamReviveActor::OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	// 이미 부활 진행자가 있으면 무시
	if (RepState.Reviver != nullptr) return;

	// 부활 진행 시작
	SetState_Server(true, Pawn);
}

void AMSTeamReviveActor::OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	// 오버랩 종료된 폰이 부활 진행자인 경우
	if (Pawn == RepState.Reviver)
	{
		// 부활 진행을 승계할 Area 안의 다른 Pawn 탐색
		if (APawn* Candidate = FindFirstOverlappingPawn_Server(Pawn))
		{
			// Area 안에 부활 진행을 승계할 다른 Pawn이 있는 경우
			// 진행 지속(승계)
			SetState_Server(true, Candidate);
		}
		else
		{
			// Area 안에 부활 진행을 승계할 다른 Pawn이 없는 경우
			// 진행자 없음 -> 부활 진행률 감소 시작
			SetState_Server(false, nullptr);
		}
	}
}

void AMSTeamReviveActor::OnRep_ReviveState()
{
	if (GetNetMode() == NM_DedicatedServer) return;

	// 로컬 클라이언트: RepState가 갱신되는 순간, 즉시 시각화 반영(이후 Tick에서 지속 보간)
	// 부활 진행/완료 시간 가져오기
	const float NowServer = GetServerTimeSecondsForVisual();

	// 서버 시간 기준 부활 진행률 계산
	const float Pct = ComputeProgressAtServerTime(NowServer);

	// 시각화 갱신
	UpdateVisuals(Pct);
}

void AMSTeamReviveActor::TickUpdate_Server(float DeltaSeconds)
{
	if (!DownedCharacter.IsValid())
	{
		Destroy();
		return;
	}

	// 서버 시간을 가져와 서버 시간 기준 부활 진행률 계산
	const float Now = GetWorld()->GetTimeSeconds();
	const float Pct = ComputeProgressAtServerTime(Now);

	// 완료 조건
	if (Pct >= 1.f)
	{
		// 최종 스냅샷 한 번만 갱신
		RepState.Progress = 1.f;
		RepState.ServerTimeStamp = Now;
		RepState.bIncreasing = false;
		RepState.Reviver = nullptr;
		ForceNetUpdate();

		// 사망(다운)한 캐릭터의 부활 진행
		if (DownedCharacter.IsValid())
		{
			DownedCharacter->ResetCharacterOnRespawn();
		}

		Destroy();
		return;
	}
}

float AMSTeamReviveActor::ComputeProgressAtServerTime(float ServerTimeSeconds) const
{
	// 총 부활 진행 시간 가져오기
	const float Duration = FMath::Max(0.01f, RepState.Duration);

	// 지난 업데이트로부터 현재 업데이트 시간을 통해 보간값 계산
	const float Delta = (ServerTimeSeconds - RepState.ServerTimeStamp) / Duration;

	// 부활 진행률 증가/감소
	UWorld* World = GetWorld();
	AMSGameState* GS = World ? World->GetGameState<AMSGameState>() : nullptr;
	float Pct = RepState.Progress;
	if (RepState.bIncreasing && (GS && GS->GetSharedLives() > 0))
	{
		Pct += Delta;
	}
	else
	{
		Pct -= Delta;
	}

	return FMath::Clamp(Pct, 0.f, 1.f);
}

float AMSTeamReviveActor::GetServerTimeSecondsForVisual() const
{
	// 현재 서버 시간을 가져와 반환
	if (UWorld* World = GetWorld())
	{
		if (AGameStateBase* GS = World->GetGameState())
		{
			return GS->GetServerWorldTimeSeconds();
		}
		return World->GetTimeSeconds();
	}
	return 0.f;
}

void AMSTeamReviveActor::UpdateVisuals(float InProgress)
{
	if (!bVisualInitialized) return;

	// 부활 로직이 진행중인지 확인
	const bool bActive = (RepState.bIncreasing && RepState.Reviver != nullptr);

	// 부활용 액터 Z값 업데이트
	if (MarkerComp)
	{
		FVector Loc = MarkerComp->GetRelativeLocation();
		Loc.Z = MarkerBaseZ + (InProgress * MarkerRiseHeight);
		MarkerComp->SetRelativeLocation(Loc);
	}

	// 장판 Area의 스케일 업데이트
	if (RingComp)
	{
		// 선형 보간으로 부드럽게 업데이트
		const float ScaleAlpha = FMath::Lerp(AreaMaxScale, AreaMinScale, InProgress);
		RingComp->SetRelativeScale3D(RingBaseScale * ScaleAlpha);
	}
}

void AMSTeamReviveActor::SetState_Server(bool bInIncreasing, APawn* NewReviver)
{
	check(HasAuthority());

	// 서버 시간을 가져와 서버 시간 기준 부활 진행률 계산
	const float Now = GetWorld()->GetTimeSeconds();
	const float CurrentPct = ComputeProgressAtServerTime(Now);

	// 부활 상태 스냅샷 업데이트 
	RepState.Progress = CurrentPct;
	RepState.ServerTimeStamp = Now;
	RepState.Duration = ReviveDuration;
	RepState.bIncreasing = bInIncreasing;
	RepState.Reviver = NewReviver;

	ForceNetUpdate();
}

APawn* AMSTeamReviveActor::FindFirstOverlappingPawn_Server(APawn* Excluded) const
{
	if (!AreaComp) return nullptr;

	// 현재 오버랩된 액터 배열을 가져옴
	TArray<AActor*> Overlapping;
	AreaComp->GetOverlappingActors(Overlapping, APawn::StaticClass());

	// 새로 부활 진행을 계승 가능한 Pawn을 찾아 반환
	for (AActor* A : Overlapping)
	{
		APawn* P = Cast<APawn>(A);
		if (!P || P == Excluded || P == DownedCharacter) continue;
		return P;
	}

	return nullptr;
}
