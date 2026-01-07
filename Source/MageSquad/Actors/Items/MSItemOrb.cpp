// Fill out your copyright notice in the Description page of Project Settings.

#include "Actors/Items/MSItemOrb.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Net/UnrealNetwork.h"

AMSItemOrb::AMSItemOrb()
{
	// 기본 Tick OFF (획득 연출 때만 ON)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	// 네트워크 설정
	bReplicates = true;
	bAlwaysRelevant = true;
	SetReplicateMovement(false);
	SetNetUpdateFrequency(30.f); // 기본값보다 낮춰서 대역폭 절약
	SetMinNetUpdateFrequency(5.f);

	// 컴포넌트 구성
	RootComp = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	SetRootComponent(RootComp);

	MeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ItemOrbMesh"));
	MeshComp->SetupAttachment(RootComp);
	MeshComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	MeshComp->SetCollisionProfileName(TEXT("NoCollision"));
	MeshComp->SetGenerateOverlapEvents(false);
	MeshComp->bReceivesDecals = false;

	CollisionComp = CreateDefaultSubobject<UBoxComponent>(TEXT("Collision"));
	CollisionComp->SetupAttachment(RootComp);
	CollisionComp->InitBoxExtent(FVector(32.f));
	// 프로파일 이름은 MSItemOrb로 통일
	CollisionComp->SetCollisionProfileName(TEXT("MSItemOrb"));
	CollisionComp->SetGenerateOverlapEvents(true);
}

void AMSItemOrb::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSItemOrb, bCollected);
}

void AMSItemOrb::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	/*
	* 공통 오브 획득 연출 수행
	*/
	{
		// 연출이 진행중이지 않으면 종료
		if (!bAttracting) return;

		// 연출 시간 누적
		AttractElapsed += DeltaSeconds;

		// #1: 위로 살짝 떠오르기
		// 누적 시간이 연출 시간을 넘을 때 까지 수행
		if (AttractElapsed < FloatUpTime)
		{
			const float Alpha = (FloatUpTime > 0.f) ? FMath::Clamp(AttractElapsed / FloatUpTime, 0.f, 1.f) : 1.f;
			const FVector NewLoc = FMath::Lerp(AttractStartLocation, FloatUpTargetLocation, Alpha);
			SetActorLocation(NewLoc);
			return;
		}

		// #2: 타겟(오브를 획득한 액터)에게 빨려 들어가기
		AActor* Target = AttractionActor.Get();
		if (!IsValid(Target))
		{
			// 타겟이 사라졌다면 연출 중단 후 숨김 처리
			SetActorTickEnabled(false);
			SetActorHiddenInGame(true);
			bAttracting = false;
			return;
		}

		// 이동 보간
		const FVector TargetLoc = Target->GetActorLocation();
		const FVector CurrentLoc = GetActorLocation();
		const FVector NewLoc = FMath::VInterpTo(CurrentLoc, TargetLoc, DeltaSeconds, AttractInterpSpeed);
		SetActorLocation(NewLoc);

		// 충분히 가까워지면 클라이언트에서는 바로 숨김(서버 Destroy는 이미 예약됨)
		if (FVector::DistSquared(NewLoc, TargetLoc) < FMath::Square(40.f))
		{
			SetActorTickEnabled(false);
			SetActorHiddenInGame(true);
			bAttracting = false;
		}
	}
}

void AMSItemOrb::Collect_Server(AActor* CollectorActor)
{
	// 서버에서만 실행
	if (!HasAuthority()) return;

	// 이미 획득된 경우 중복 처리 방지
	if (bCollected) return;

	// 중복 획득 방지 플래그 설정
	bCollected = true;

	// 서버에서 충돌/오버랩 즉시 비활성화 (추가 트리거 방지)
	if (CollisionComp)
	{
		CollisionComp->SetGenerateOverlapEvents(false);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}

	// 모든 클라이언트에서 연출 시작
	MulticastRPCBeginAttract(CollectorActor);

	// 연출 재생 시간 확보 후 Destroy (서버 Destroy가 모든 클라이언트에 반영됨)
	DeferredDestroy_Server();
}

void AMSItemOrb::MulticastRPCBeginAttract_Implementation(AActor* InTargetActor)
{
	// 이미 획득하여 연출 진행중인 경우 종료
	if (bAttracting) return;

	AttractionActor = InTargetActor;
	AttractStartLocation = GetActorLocation();
	FloatUpTargetLocation = AttractStartLocation + FVector(0.f, 0.f, FloatUpHeight);
	AttractElapsed = 0.f;
	bAttracting = true;

	// 연출 동안은 Tick을 켬
	SetActorTickEnabled(true);
}

void AMSItemOrb::DeferredDestroy_Server()
{
	if (!HasAuthority()) return;

	// 연출이 끝날 시간만큼 딜레이 후 Destroy
	FTimerHandle TmpHandle;
	GetWorldTimerManager().SetTimer(
		TmpHandle,
		[this]()
		{
			Destroy();
		},
		DestroyDelay,
		false
	);
}

void AMSItemOrb::OnRep_Collected()
{
	// 서버에서 중복 획득 설정 시 충돌 비활성화를 클라이언트에게도 적용
	if (CollisionComp)
	{
		CollisionComp->SetGenerateOverlapEvents(false);
		CollisionComp->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
}
