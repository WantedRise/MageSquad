// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MSLobyPlayerSlot.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/07
* PlayerStart를 대신하여 AMSLobyGameMode에서 플레이어의 위치 지정, 초대 표시를 담당하는 클래스
*/
UCLASS()
class MAGESQUAD_API AMSLobyPlayerSlot : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AMSLobyPlayerSlot();
	AController* GetController() const { return PlayerController; }
	void SetController(AController* NewPlayer) { PlayerController = NewPlayer; }
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
protected:
	//UPROPERTY(EditAnywhere,meta=(AllowPrivateAccess = "true"))
	//class UWidgetComponent* PlaySlotWidgetComponent;
	//UPROPERTY(EditAnywhere, meta = (AllowPrivateAccess = "true"))
	//TSubclassOf<class UUserWidget> PlaySlotWidgetClass;
	//UPROPERTY()
	//class UPlayerSlotWidget* UPlayerSlotWidget;
	class AController* PlayerController;
};
