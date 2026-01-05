// Fill out your copyright notice in the Description page of Project Settings.


#include "Actors/Wave/MSWaveBlock.h"

// Sets default values
AMSWaveBlock::AMSWaveBlock()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
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
			// 1. 틱 시스템 자체는 허용 (애니메이션 업데이트를 위해)
			MeshComp->PrimaryComponentTick.bCanEverTick = true;
			MeshComp->PrimaryComponentTick.bStartWithTickEnabled = true;

			// 2. 핵심: 화면에 보일 때만 포즈를 계산하도록 설정
			// 이 설정이 되어 있으면 화면 밖에서는 틱이 돌아도 무거운 애니메이션 계산을 생략합니다.
			MeshComp->VisibilityBasedAnimTickOption = EVisibilityBasedAnimTickOption::OnlyTickPoseWhenRendered;
        
			// 3. (추가) 애니메이션 시퀀스가 루프 중인지 확인
			MeshComp->SetPlayRate(1.0f);
		}
	}
}

// Called every frame
void AMSWaveBlock::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

