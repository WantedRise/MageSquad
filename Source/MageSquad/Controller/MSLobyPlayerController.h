// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MSLobyPlayerController.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/07
* 로비에서의 플레이어 컨트롤러
*/
UCLASS()
class MAGESQUAD_API AMSLobyPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	AMSLobyPlayerController();

protected:
	void BeginPlay() override;

};
