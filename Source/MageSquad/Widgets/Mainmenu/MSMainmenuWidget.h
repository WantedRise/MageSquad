// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSMainmenuWidget.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/07
* 메인메뉴 위젯이며 플레이 버튼으로 로비로 이동
* 버튼 클릭시 스팀세션을 생성하며 로비로 이동
*/
UCLASS()
class MAGESQUAD_API UMSMainmenuWidget : public UUserWidget
{
    GENERATED_BODY()
public:
    UMSMainmenuWidget(const FObjectInitializer& ObjectInitializer);
    void NativeConstruct() override;

    UFUNCTION()
    void CreateSessionAndTravalToLobby();
public:
    UPROPERTY(meta=(BindWidget))
    class UButton* Button_Play;
};
