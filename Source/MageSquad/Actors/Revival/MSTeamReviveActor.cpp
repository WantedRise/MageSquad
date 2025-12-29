// Copyright Notice

#include "MSTeamReviveActor.h"

#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Player/MSPlayerCharacter.h"

#include "GameStates/MSGameState.h"

#include "Net/UnrealNetwork.h"

AMSTeamReviveActor::AMSTeamReviveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickInterval(0.05f);

	bReplicates = true;
	SetReplicateMovement(false);

	// 부활 진행도만 복제하면 되므로 네트워크 주기를 아낌
	SetNetUpdateFrequency(30.f);
	SetMinNetUpdateFrequency(2.f);

	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(Root);

	AreaComp = CreateDefaultSubobject<USphereComponent>(TEXT("ReviveAreaComponent"));
	AreaComp->SetupAttachment(Root);
	AreaComp->InitSphereRadius(1000.f);
	// 플레이어와만 콜리전 오버랩 설정
	AreaComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaComp->SetCollisionResponseToChannel(ECC_GameTraceChannel1, ECR_Overlap);
	AreaComp->SetGenerateOverlapEvents(true);

	MarkerComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Marker"));
	MarkerComp->SetupAttachment(Root);
	MarkerComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MarkerComp->SetGenerateOverlapEvents(false);
	MarkerComp->SetIsReplicated(false);

	RingComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingAnimation"));
	RingComp->SetupAttachment(Root);
	RingComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RingComp->SetGenerateOverlapEvents(false);
	RingComp->SetIsReplicated(false);

	RingOriginComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RingOrigin"));
	RingOriginComp->SetupAttachment(Root);
	RingOriginComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RingOriginComp->SetGenerateOverlapEvents(false);
	RingOriginComp->SetIsReplicated(false);

	// 초기값
	ServerProgress = 0.f;
	RepProgressByte = 0;
	CurrentReviver = nullptr;
	ReviveDuration = 2.5f;
}

void AMSTeamReviveActor::BeginPlay()
{
	Super::BeginPlay();

	// 서버에서만 오버랩 이벤트로 Reviver 후보를 받음
	if (HasAuthority() && AreaComp)
	{
		AreaComp->OnComponentBeginOverlap.AddDynamic(this, &AMSTeamReviveActor::OnAreaBeginOverlap);
		AreaComp->OnComponentEndOverlap.AddDynamic(this, &AMSTeamReviveActor::OnAreaEndOverlap);
	}

	// 로컬에서 시각화용 기본값 캐시
	if (GetNetMode() != NM_DedicatedServer)
	{
		// 위치/크기 초기화
		MarkerBaseZ = MarkerComp ? MarkerComp->GetRelativeLocation().Z : 0.f;
		RingBaseScale = RingComp ? RingComp->GetRelativeScale3D() : FVector(1.f);
		bVisualInitialized = true;

		// 시각화 시작 플래그 설정
		bVisualInitialized = true;
	}
}

void AMSTeamReviveActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (HasAuthority())
	{
		// 서버에서 부활 진행/승계/완료 로직 수행
		Tick_Server(DeltaSeconds);
	}

	if (GetNetMode() != NM_DedicatedServer)
	{
		// 클라는 복제된 Progress로만 연출
		UpdateVisuals(static_cast<float>(RepProgressByte) / 255.f);
	}
}

void AMSTeamReviveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSTeamReviveActor, DownedCharacter);
	DOREPLIFETIME(AMSTeamReviveActor, CurrentReviver);
	DOREPLIFETIME(AMSTeamReviveActor, ReviveDuration);
	DOREPLIFETIME(AMSTeamReviveActor, RepProgressByte);
}

void AMSTeamReviveActor::Initialize(AMSPlayerCharacter* InDownedCharacter, float InReviveDuration)
{
	if (!HasAuthority()) return;

	// 사망(다운)한 캐릭터와 부활 진행 시간 초기화
	DownedCharacter = InDownedCharacter;
	ReviveDuration = FMath::Max(0.01f, InReviveDuration);

	// 부활 진행 상태 초기화
	ServerProgress = 0.f;
	RepProgressByte = 0;
	CurrentReviver = nullptr;

	ForceNetUpdate();
}

void AMSTeamReviveActor::OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;
	if (Pawn == DownedCharacter) return;

	// 이미 진행자가 있으면(첫 1명만 인정) 무시
	if (CurrentReviver) return;

	// 공유 목숨이 0이면 진행 자체가 불가하므로 진행자도 세팅하지 않음
	UWorld* World = GetWorld();
	AMSGameState* GS = World ? World->GetGameState<AMSGameState>() : nullptr;
	if (!GS || GS->GetSharedLives() <= 0) return;

	// 부활 진행자 초기화
	CurrentReviver = Pawn;
	ForceNetUpdate();
}

void AMSTeamReviveActor::OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn) return;

	// 현재 진행자가 나갔을 때는 진행자를 비워둠
	// 부활 진행 승계는 Tick에서 일관되게 처리
	if (Pawn == CurrentReviver)
	{
		CurrentReviver = nullptr;
		ForceNetUpdate();
	}
}

void AMSTeamReviveActor::OnRep_ProgressByte()
{
	if (GetNetMode() == NM_DedicatedServer) return;

	// 복제된 진행률을 즉시 반영 (이후 Tick에서도 동일 값으로 계속 갱신)
	UpdateVisuals(static_cast<float>(RepProgressByte) / 255.f);
}

void AMSTeamReviveActor::Tick_Server(float DeltaSeconds)
{
	// 부활 대상이 없어지면 자동 소멸
	if (!DownedCharacter)
	{
		Destroy();
		return;
	}

	// 부활 진행자 유효 검사 및 승계 처리
	ResolveReviver_Server();

	// 부활 진행률을 증가시켜야 하는지 감소시켜야 하는지 검사
	const bool bIncrease = CanIncrease_Server();

	// 부활 진행률 증가/감소 처리
	UpdateProgress_Server(DeltaSeconds, bIncrease);

	// 부활 진행률 업데이트 (클라이언트 처리용)
	const uint8 NewByte = static_cast<uint8>(FMath::RoundToInt(ServerProgress * 255.f));
	if (NewByte != RepProgressByte)
	{
		RepProgressByte = NewByte;
	}

	// 부활(완료) 시도
	TryComplete_Server();
}

void AMSTeamReviveActor::ResolveReviver_Server()
{
	if (!AreaComp) return;

	// 현재 부활 진행자가 유효하고 아직 영역 안에 있으면 유지
	if (CurrentReviver)
	{
		// 부활 진행자가 영역 안에 있는지 검사
		if (!AreaComp->IsOverlappingActor(CurrentReviver) || CurrentReviver == DownedCharacter)
		{
			CurrentReviver = nullptr;
			ForceNetUpdate();
		}
	}

	// 현재 부활 진행자가 없으면, 공유 목숨이 0인 경우에만 영역 내 후보를 찾아 즉시 승계
	if (!CurrentReviver)
	{
		UWorld* World = GetWorld();
		AMSGameState* GS = World ? World->GetGameState<AMSGameState>() : nullptr;
		if (GS && GS->GetSharedLives() > 0)
		{
			// 장판에 오버랩 된 다른 생존자를 찾아 부활 진행 승계
			if (APawn* Candidate = FindFirstOverlappingPawn_Server(nullptr))
			{
				CurrentReviver = Candidate;
				ForceNetUpdate();
			}
		}
	}
}

bool AMSTeamReviveActor::CanIncrease_Server() const
{
	if (!DownedCharacter || !CurrentReviver) return false;

	UWorld* World = GetWorld();
	AMSGameState* GS = World ? World->GetGameState<AMSGameState>() : nullptr;
	if (!GS) return false;

	// 공유 목숨이 1개 이상인지 확인
	return GS->GetSharedLives() > 0;
}

void AMSTeamReviveActor::UpdateProgress_Server(float DeltaSeconds, bool bIncrease)
{
	// 현재 부활 진행률 계산
	const float Duration = FMath::Max(0.01f, ReviveDuration);
	const float Step = (DeltaSeconds / Duration);

	// 부활 진행률 증가/감소 처리
	ServerProgress = bIncrease ? (ServerProgress + Step) : (ServerProgress - Step);
	ServerProgress = FMath::Clamp(ServerProgress, 0.f, 1.f);
}

void AMSTeamReviveActor::TryComplete_Server()
{
	// 부활 진행이 덜 됐으면 종료
	if (ServerProgress < 1.f) return;

	// 완료 판정은 증가 가능한 상태에서만 수행
	if (!CanIncrease_Server()) return;

	UWorld* World = GetWorld();
	AMSGameState* GS = World ? World->GetGameState<AMSGameState>() : nullptr;
	if (!GS) return;

	// 목숨이 이미 0이 됨 -> 다음 Tick부터 감소로 전환
	if (GS->GetSharedLives() <= 0) return;

	// 캐릭터 부활 진행
	if (DownedCharacter)
	{
		DownedCharacter->ResetCharacterOnRespawn();
	}

	Destroy();
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
		if (AMSPlayerCharacter* Player = Cast<AMSPlayerCharacter>(P))
		{
			if (Player->GetIsDead()) continue;
		}
		return P;
	}

	return nullptr;
}

void AMSTeamReviveActor::UpdateVisuals(float InProgress)
{
	if (!bVisualInitialized) return;

	// 마커: 진행률 기반 Z 이동 (증가/감소 모두 동일 수식)
	if (MarkerComp)
	{
		FVector Loc = MarkerComp->GetRelativeLocation();
		Loc.Z = MarkerBaseZ + (InProgress * MarkerRiseHeight);
		MarkerComp->SetRelativeLocation(Loc);
	}

	// 장판: 진행률 기반 스케일 축소/확대
	if (RingComp)
	{
		const float ScaleAlpha = FMath::Lerp(AreaMaxScale, AreaMinScale, InProgress);
		RingComp->SetRelativeScale3D(RingBaseScale * ScaleAlpha);
	}
}
