// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Wave/MSWaveBlock.h"

#include "Components/BoxComponent.h"
#include <Kismet/GameplayStatics.h>
#include "Components/AudioComponent.h"
// Sets default values
AMSWaveBlock::AMSWaveBlock()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	bAlwaysRelevant = true;
	
	BoxCollision = CreateDefaultSubobject<UBoxComponent>("Box");
	RootComponent = BoxCollision;

	Mesh1 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh1");
	Mesh1->SetupAttachment(RootComponent);
	Mesh2 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh2");
	Mesh2->SetupAttachment(RootComponent);
	Mesh3 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh3");
	Mesh3->SetupAttachment(RootComponent);
	Mesh4 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh4");
	Mesh4->SetupAttachment(RootComponent);
	Mesh5 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh5");
	Mesh5->SetupAttachment(RootComponent);
	Mesh6 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh6");
	Mesh6->SetupAttachment(RootComponent);
	Mesh7 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh7");
	Mesh7->SetupAttachment(RootComponent);
	Mesh8 = CreateDefaultSubobject<USkeletalMeshComponent>("Mesh8");
	Mesh8->SetupAttachment(RootComponent);
}

void AMSWaveBlock::ActivateBlock()
{
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);
}

void AMSWaveBlock::DeactivateBlock()
{
	SetActorEnableCollision(false);
	SetActorHiddenInGame(true);
}

// Called when the game starts or when spawned
void AMSWaveBlock::BeginPlay()
{
	Super::BeginPlay();

	// 이 액터에 붙은 모든 컴포넌트를 순회
	TArray<USkeletalMeshComponent*> SkeletalMeshComponents;
	GetComponents<USkeletalMeshComponent>(SkeletalMeshComponents);

	for (USkeletalMeshComponent* MeshComp : SkeletalMeshComponents)
	{
		if (MeshComp)
		{
			MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
		}
	}

	auto* SigManager = USignificanceManager::Get(GetWorld());
	if (SigManager)
	{
		// "Enemy"라는 태그로 자신을 등록하고, 위에서 만든 함수를 연결
		SigManager->RegisterObject(
			this,
			"WaveBlock",
			&AMSWaveBlock::CalculateSignificance, // 여기에 만든 함수 위치를 전달
			USignificanceManager::EPostSignificanceType::Sequential,
			[this](USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSig, float NewSig, bool bInView)
			{
				this->OnSignificanceChanged(ObjectInfo, OldSig, NewSig, bInView);
			}
		);
	}
}

void AMSWaveBlock::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		if (USignificanceManager* SigManager = USignificanceManager::Get(World))
		{
			SigManager->UnregisterObject(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

float AMSWaveBlock::CalculateSignificance(USignificanceManager::FManagedObjectInfo* ObjectInfo,
                                          const FTransform& Viewpoint)
{
	AActor* Owner = Cast<AActor>(ObjectInfo->GetObject());
	if (!Owner)
	{
		return 0.0f;
	}

	float DistanceSq = FVector::DistSquared(Owner->GetActorLocation(), Viewpoint.GetLocation());
	
	if (DistanceSq < FMath::Square(3000.0f))
	{
		return 1.0f;
	}
	if (DistanceSq < FMath::Square(5500.0f))
	{
		return 0.5f;
	}

	return 0.1f;
}

void AMSWaveBlock::OnSignificanceChanged(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSig,
                                         float NewSig, bool bInView)
{
	bool IsCulling = NewSig <= 0.1f;

	// Mesh들을 배열에 담아 처리
	UMeshComponent* Meshes[] = { Mesh1, Mesh2, Mesh3, Mesh4, Mesh5, Mesh6, Mesh7, Mesh8 };

	for (UMeshComponent* Mesh : Meshes)
	{
		if (IsValid(Mesh))
		{
			Mesh->SetComponentTickEnabled(!IsCulling);
			Mesh->SetVisibility(!IsCulling);
            
			if (USkeletalMeshComponent* SkelMesh = Cast<USkeletalMeshComponent>(Mesh))
			{
				SkelMesh->bPauseAnims = IsCulling;
			}
		}
	}
}