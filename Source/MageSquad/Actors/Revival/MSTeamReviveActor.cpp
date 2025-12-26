// Copyright Notice

#include "MSTeamReviveActor.h"
#include "Player/MSPlayerCharacter.h"
#include "Components/SphereComponent.h"
#include "GameFramework/PlayerState.h"

#include "Net/UnrealNetwork.h"

AMSTeamReviveActor::AMSTeamReviveActor()
{
	PrimaryActorTick.bCanEverTick = true;
	SetActorTickInterval(0.01f);
	bReplicates = true;
	SetReplicateMovement(false);

	AreaComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	AreaComp->InitSphereRadius(200.f);
	RootComponent = AreaComp;
	// 플레이어와만 콜리전 오버랩 설정
	AreaComp->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	AreaComp->SetCollisionResponseToAllChannels(ECR_Ignore);
	AreaComp->SetCollisionResponseToChannel(ECC_EngineTraceChannel1, ECR_Overlap);
	AreaComp->SetGenerateOverlapEvents(true);
}

void AMSTeamReviveActor::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// 부활 진행률 업데이트는 서버에서만 진행. 클라이언트는 OnRep이나 보간 처리
	if (!HasAuthority()) return;

	// 부활 진행률 업데이트
	UpdateRevive(DeltaSeconds);
}

void AMSTeamReviveActor::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSTeamReviveActor, DownedCharacter);
	DOREPLIFETIME(AMSTeamReviveActor, ReviveProgress);
	DOREPLIFETIME(AMSTeamReviveActor, ReviveDuration);
}

void AMSTeamReviveActor::Initialize(AMSPlayerCharacter* InDownedCharacter, float InReviveDuration)
{
	// 부활 진행을 위한 초기화 설정
	DownedCharacter = InDownedCharacter;
	ReviveDuration = FMath::Max(0.01f, InReviveDuration);
	ReviveProgress = 0.f;
	ServerStartTime = 0.f;
	CurrentReviver = nullptr;
}

void AMSTeamReviveActor::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority() && AreaComp)
	{
		// Area 콜리전 오버랩 바인딩
		AreaComp->OnComponentBeginOverlap.AddDynamic(this, &AMSTeamReviveActor::OnAreaBeginOverlap);
		AreaComp->OnComponentEndOverlap.AddDynamic(this, &AMSTeamReviveActor::OnAreaEndOverlap);
	}
}

void AMSTeamReviveActor::OnAreaBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (!HasAuthority()) return;

	// 첫 번째 겹친 Pawn을 진행자로 지정. 이미 진행자가 있다면 아무 동작 안 함
	APawn* Pawn = Cast<APawn>(OtherActor);
	if (Pawn && !CurrentReviver.IsValid())
	{
		CurrentReviver = Pawn;
		ServerStartTime = GetWorld()->GetTimeSeconds() - ReviveProgress * ReviveDuration;
	}
}

void AMSTeamReviveActor::OnAreaEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (!HasAuthority()) return;

	APawn* Pawn = Cast<APawn>(OtherActor);
	if (!Pawn || Pawn != CurrentReviver.Get()) return;

	// 현재 진행자가 나갔으므로 초기화
	CurrentReviver = nullptr;

	// 현재 Area에 오버랩된 플레이어 탐색
	TArray<AActor*> Overlapping;
	AreaComp->GetOverlappingActors(Overlapping, APawn::StaticClass());

	for (AActor* A : Overlapping)
	{
		APawn* Candidate = Cast<APawn>(A);
		if (!Candidate) continue;
		if (Candidate == Pawn) continue;
		if (!IsValid(Candidate)) continue;

		// 현재 진행자 계승
		CurrentReviver = Candidate;

		// 진행률 유지하면서 이어받기
		ServerStartTime = GetWorld()->GetTimeSeconds() - ReviveProgress * ReviveDuration;
		break;
	}
}

void AMSTeamReviveActor::OnRep_ReviveProgress()
{
	// 클라이언트에서 진행률이 갱신될 때 호출. 실제 비주얼 보간은 위젯이나 FX에서 처리한다.
}

void AMSTeamReviveActor::UpdateRevive(float DeltaSeconds)
{
	if (!DownedCharacter.IsValid())
	{
		Destroy();
		return;
	}

	// 부활 완료되었거나 진행 중이지 않은 경우 아무런 동작 안 함
	if (ReviveProgress >= 1.f) return;

	// 현재 진행자 유지. 진행자가 없으면 진행률 감소
	const bool bHasReviver = CurrentReviver.IsValid();

	if (bHasReviver)
	{
		// 진행 시작 시간 초기화
		if (ServerStartTime <= 0.f)
		{
			ServerStartTime = GetWorld()->GetTimeSeconds();
		}

		// 부활 진행 경과 시간 추적
		const float Elapsed = GetWorld()->GetTimeSeconds() - ServerStartTime;
		ReviveProgress = FMath::Clamp(Elapsed / ReviveDuration, 0.f, 1.f);
	}
	else
	{
		// 진행자가 없으면 진행률 감소: 감소 속도 = 동일 시간으로 감소
		if (ReviveProgress > 0.f)
		{
			const float DecayRate = 1.f / ReviveDuration;
			ReviveProgress = FMath::Clamp(ReviveProgress - DeltaSeconds * DecayRate, 0.f, 1.f);

			// 진행률이 0이 되면 다시 타이머 초기화
			if (ReviveProgress <= 0.f)
			{
				ServerStartTime = 0.f;
			}
		}
	}

	// 진행 완료 시 캐릭터 부활
	if (ReviveProgress >= 1.f)
	{
		if (DownedCharacter.IsValid())
		{
			DownedCharacter->ServerFinishRevive();
		}
		Destroy();
	}
}
