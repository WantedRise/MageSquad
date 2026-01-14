// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Enemy/Notify/MSAN_SpawnIndicator.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameplayEffectTypes.h"
#include "Actors/Indicator/MSIndicatorActor.h"
#include "Components/DecalComponent.h"
#include "Types/MageSquadTypes.h"

UMSAN_SpawnIndicator::UMSAN_SpawnIndicator()
{
	// 기본 파라미터
	IndicatorParams.Shape = EIndicatorShape::Cone;
	IndicatorParams.Radius = 500.f;
	IndicatorParams.Angle = 90.f;
	IndicatorParams.Duration = 1.5f;
}

void UMSAN_SpawnIndicator::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                  const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);


	IndicatorCueTag = FGameplayTag::RequestGameplayTag(FName("GameplayCue.Enemy.Indicator"));
    
	// UE_LOG(LogTemp, Error, TEXT("[SpawnIndicator] Tag Valid: %s, Tag Name: %s"),
	// 	IndicatorCueTag.IsValid() ? TEXT("Yes") : TEXT("No"),
	// 	*IndicatorCueTag.ToString());
    
	if (!IndicatorCueTag.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("[SpawnIndicator] GameplayCue tag is INVALID!"));
		return;
	}

	if (!MeshComp)
	{
		return;
	}

	AActor* Owner = MeshComp->GetOwner();
	if (!Owner)
	{
		return;
	}

	// UE_LOG(LogTemp, Error, TEXT("[%s] - %s"),
	//        Owner->HasAuthority() ? TEXT("Server") : TEXT("Client"),
	//        	*GetName()
	//       );

	// 스폰 위치 계산
	FVector SpawnLocation;
	FRotator SpawnRotation;

	if (bUseSocket && MeshComp->DoesSocketExist(SocketName))
	{
		SpawnLocation = MeshComp->GetSocketLocation(SocketName);
		SpawnRotation = MeshComp->GetSocketRotation(SocketName);
	}
	else
	{
		SpawnLocation = Owner->GetActorLocation();
		SpawnRotation = Owner->GetActorRotation();
	}

	// 오프셋 적용
	SpawnLocation += SpawnRotation.RotateVector(LocationOffset);
	SpawnRotation += RotationOffset;

#if WITH_EDITOR
	// 에디터 프리뷰: ASC 없이 직접 스폰
	if (!Owner->GetWorld()->IsGameWorld())
	{
		if (PreviewIndicatorClass)
		{
			// 프리뷰용 회전 보정 (정면을 향하도록 -90도)
			FRotator PreviewRotation = SpawnRotation;
			PreviewRotation.Yaw += 90.f;

			AMSIndicatorActor* Indicator = Owner->GetWorld()->SpawnActor<AMSIndicatorActor>(
				PreviewIndicatorClass,
				SpawnLocation,
				PreviewRotation
			);
			if (Indicator)
			{
				Indicator->Initialize(IndicatorParams);

				// 스켈레탈 메시에 Decal이 안 그려지도록 설정
				if (UDecalComponent* Decal = Indicator->FindComponentByClass<UDecalComponent>())
				{
					FVector CurrentSize = Decal->DecalSize;
					Decal->DecalSize = FVector(30.f, CurrentSize.Y, CurrentSize.Z); // 깊이만 얕게
				}
			}
		}
		return;
	}
#endif

	// // 서버에서만 실행 (GameplayCue가 클라이언트에 자동 동기화)
	// if (!Owner->HasAuthority())
	// {
	// 	return;
	// }

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);
	if (!ASC)
	{
		UE_LOG(LogTemp, Warning, TEXT("AnimNotify_SpawnIndicator: Owner has no AbilitySystemComponent!"));
		return;
	}

	// Custom EffectContext 생성
	FMSGameplayEffectContext* EffectContext = new FMSGameplayEffectContext();
	EffectContext->SetIndicatorParams(IndicatorParams);
	EffectContext->AddInstigator(Owner, Owner);

	// GameplayCue 파라미터 설정
	FGameplayCueParameters CueParams;
	CueParams.Location = SpawnLocation;
	CueParams.Normal = SpawnRotation.Vector();
	CueParams.EffectContext = FGameplayEffectContextHandle(EffectContext);

	// GameplayCue 실행
	ASC->ExecuteGameplayCue(IndicatorCueTag, CueParams);
}

FString UMSAN_SpawnIndicator::GetNotifyName_Implementation() const
{
	// 에디터에서 표시될 이름
	FString ShapeName;
	switch (IndicatorParams.Shape)
	{
	case EIndicatorShape::Circle:
		ShapeName = FString::Printf(TEXT("Circle R:%.0f"), IndicatorParams.Radius);
		break;
	case EIndicatorShape::Cone:
		ShapeName = FString::Printf(TEXT("Cone R:%.0f A:%.0f"), IndicatorParams.Radius, IndicatorParams.Angle);
		break;
	case EIndicatorShape::Rectangle:
		ShapeName = FString::Printf(TEXT("Rect W:%.0f L:%.0f"), IndicatorParams.Width, IndicatorParams.Length);
		break;
	}

	return FString::Printf(TEXT("Indicator (%s)"), *ShapeName);
}
