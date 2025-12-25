// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "MSAN_SendGameplayEvent.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/23
 * 실행중인 몽타주에 이벤트를 전달할 노티파이
 */
UCLASS()
class MAGESQUAD_API UMSAN_SendGameplayEvent : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UMSAN_SendGameplayEvent();
	
public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	
private:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Tag", meta = (AllowPrivateAccess = "true", Categories = "Enemy.Event"))
	FGameplayTag EventTag;
	
	
	
};
