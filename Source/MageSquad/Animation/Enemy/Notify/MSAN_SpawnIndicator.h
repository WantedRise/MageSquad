// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "Types/MSIndicatorTypes.h"
#include "MSAN_SpawnIndicator.generated.h"

/**
 * 작성자 : 임희섭
 * 작성일 : 2025/12/31
 * Indicator를 스폰할 때 GameplayCue에 Param을 전달하기 위한 노티파이
 */
UCLASS(DisplayName = "Spawn Attack Indicator")
class MAGESQUAD_API UMSAN_SpawnIndicator : public UAnimNotify
{
	GENERATED_BODY()
	
public:
	UMSAN_SpawnIndicator();

	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
	virtual FString GetNotifyName_Implementation() const override;

#if WITH_EDITOR
	virtual bool CanBePlaced(UAnimSequenceBase* Animation) const override { return true; }
#endif

protected:
	/** 인디케이터 파라미터 - 디테일 패널에서 설정 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FAttackIndicatorParams IndicatorParams;

	/** 스폰 위치 오프셋 (캐릭터 로컬 좌표) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FVector LocationOffset = FVector::ZeroVector;

	/** 회전 오프셋 (캐릭터 기준) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator")
	FRotator RotationOffset = FRotator::ZeroRotator;

	/** 소켓 사용 여부 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator|Socket")
	bool bUseSocket = false;

	/** 스폰 위치로 사용할 소켓 이름 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator|Socket", meta = (EditCondition = "bUseSocket"))
	FName SocketName;

	/** GameplayCue 태그 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Indicator|Advanced", meta = (Categories = "GameplayCue.Enemy"))
	FGameplayTag IndicatorCueTag;
	
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category = "Indicator|Preview")
	TSubclassOf<class AMSIndicatorActor> PreviewIndicatorClass;
#endif
	
};
