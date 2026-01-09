// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSLobbyReadyWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Player/MSLobbyPlayerController.h"
#include "MageSquad.h"
#include "GameStates/MSLobbyGameState.h"
#include "Sound/SoundWave.h"
#include "Kismet/GameplayStatics.h"

void UMSLobbyReadyWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if (Button_Ready)
    {
        Button_Ready->OnClicked.AddDynamic(this, &UMSLobbyReadyWidget::OnReadyButtonClicked);
    }

    // GameState의 RemainingReadySeconds 변경 → UI 카운트다운 반영
    if (AMSLobbyGameState* GS = GetWorld()->GetGameState<AMSLobbyGameState>())
    {
        GS->OnReadyTimeChanged.AddUObject(this,&UMSLobbyReadyWidget::ApplyReadyTimeText);
        GS->OnLobbyReadyPhaseChanged.AddUObject(this, &UMSLobbyReadyWidget::ApplyReadyStateUI);
    }
}

void UMSLobbyReadyWidget::ApplyReadyTimeText(int32 RemainingSeconds)
{
    if (Text_Ready_Second)
    {
        Text_Ready_Second->SetText(FText::AsNumber(RemainingSeconds));
    }
}

void UMSLobbyReadyWidget::ApplyReadyStateUI(ELobbyReadyPhase NewLobbyReadyPhase)
{
    UE_LOG(LogMSNetwork, Warning, TEXT("%d"), (int32)NewLobbyReadyPhase);
    // 로비에 최소 1명 이상이 준비 상태인지에 따라 Ready UI를 전환한다.
    // bIsAnyReady == true  : 누군가 준비함 (PartialReady 상태)
    // bIsAnyReady == false : 아무도 준비하지 않음 (NotReady 상태)
    if (NewLobbyReadyPhase!=ELobbyReadyPhase::NotReady)
    {
        // PartialReady 상태 UI
        // - 기본 Ready 텍스트 숨김
        // - Ready 및 카운트다운 텍스트 표시
        if (Text_Ready_Default)
        {
            Text_Ready_Default->SetVisibility(ESlateVisibility::Collapsed);
        }
        if (Text_Ready_Selected && Text_Ready_Second)
        {
            Text_Ready_Selected->SetVisibility(ESlateVisibility::Visible);
            Text_Ready_Second->SetVisibility(ESlateVisibility::Visible);
        }

        if (ReadyButtonSound)
        {
            UGameplayStatics::PlaySound2D(this, ReadyButtonSound);
        }
        
    }
    else
    {
        // NotReady 상태 UI
        // - 기본 Ready 텍스트 표시
        // - Ready 취소 및 카운트다운 텍스트 숨김
        if (Text_Ready_Selected && Text_Ready_Second)
        {
            Text_Ready_Selected->SetVisibility(ESlateVisibility::Collapsed);
            Text_Ready_Second->SetVisibility(ESlateVisibility::Collapsed);
        }
        if (Text_Ready_Default)
        {
            Text_Ready_Default->SetVisibility(ESlateVisibility::Visible);
        }

        if (CancelButtonSound)
        {
            UGameplayStatics::PlaySound2D(this, CancelButtonSound);
        }
    }
}

void UMSLobbyReadyWidget::OnReadyButtonClicked()
{
    AMSLobbyPlayerController* PC = Cast<AMSLobbyPlayerController>(GetWorld()->GetFirstPlayerController());
    //Button_Ready->SetStyle(EButton);
    if (PC)
    {
        UE_LOG(LogMSNetwork, Log, TEXT("OnReadyButtonClicked"));
        bReady = !bReady;
        PC->ServerRequestSetReady(bReady);
    }
}