// Fill out your copyright notice in the Description page of Project Settings.


#include "Animation/Player/MSPlayerAnimInstance.h"
#include "Player/MSPlayerCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "KismetAnimationLibrary.h"

#include "MSFunctionLibrary.h"
#include "MSGameplayTags.h"

void UMSPlayerAnimInstance::NativeInitializeAnimation()
{
	OwningPlayer = Cast<AMSPlayerCharacter>(TryGetPawnOwner());
	if (OwningPlayer)
	{
		MovementComponent = OwningPlayer->GetCharacterMovement();
	}
}

void UMSPlayerAnimInstance::NativeThreadSafeUpdateAnimation(float DeltaSeconds)
{
	if (OwningPlayer && MovementComponent)
	{
		// 이동 속도 구하기
		Velocity = MovementComponent->Velocity;
		GroundSpeed = Velocity.Size2D();

		// 이동 방향 구하기
		Direction = UKismetAnimationLibrary::CalculateDirection(Velocity, OwningPlayer->GetActorRotation());

		// 이동 중 여부 구하기
		bShoudMove = (MovementComponent->GetCurrentAcceleration() != FVector::ZeroVector) && (GroundSpeed > 3.f);
	}
}

bool UMSPlayerAnimInstance::DoesOwnerHaveTag(const FGameplayTag TagToCheck) const
{
	if (APawn* OwningPawn = TryGetPawnOwner())
	{
		return UMSFunctionLibrary::NativeDoesActorHaveTag(OwningPawn, TagToCheck);
	}
	return false;
}
