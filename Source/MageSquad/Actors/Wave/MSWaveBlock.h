// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SignificanceManager.h"
#include "GameFramework/Actor.h"
#include "MSWaveBlock.generated.h"

UCLASS()
class MAGESQUAD_API AMSWaveBlock : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSWaveBlock();

	UFUNCTION(BlueprintCallable)
	void ActivateBlock();

	UFUNCTION(BlueprintCallable)
	void DeactivateBlock();
	
	// 시그니컨스 매니저가 호출할 계산 함수
	static float CalculateSignificance(USignificanceManager::FManagedObjectInfo* ObjectInfo, const FTransform& Viewpoint);
	
	// 중요도가 변경되었을 때 호출될 커스텀 함수
	// 매개변수: 정보객체, 이전 중요도, 새로운 중요도, 화면에 보이는지 여부
	void OnSignificanceChanged(USignificanceManager::FManagedObjectInfo* ObjectInfo, float OldSig, float NewSig, bool bInView);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Collision", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UBoxComponent> BoxCollision;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh1;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh2;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh3;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh4;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh5;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh6;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh7;
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Mesh", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class USkeletalMeshComponent> Mesh8;
};
