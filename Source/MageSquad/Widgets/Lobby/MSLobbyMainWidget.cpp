// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSLobbyMainWidget.h"
#include "Player/MSLobbyPlayerController.h"
#include "Components/Button.h"
#include "MSCharacterSelectWidget.h"
#include "MSLobbyReadyWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"

UMSLobbyMainWidget::UMSLobbyMainWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

void UMSLobbyMainWidget::NativeConstruct()
{
    check(MSCharacterSelect);
    MSCharacterSelect->SetVisibility(ESlateVisibility::Collapsed);
    Super::NativeConstruct();

    if (Button_Select)
    {
        // ⭐ 버튼 클릭 바인드
        Button_Select->OnClicked.AddDynamic(this, &UMSLobbyMainWidget::OnClickedSelectCharacterButton);
        Button_Select->OnHovered.AddDynamic(this, &UMSLobbyMainWidget::OnHoveredSelectCharacterButton);
        Button_Select->OnUnhovered.AddDynamic(this, &UMSLobbyMainWidget::OnUnHoveredSelectCharacterButton);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Button_Select is nullptr"));
    }

    if (Button_Lobby)
    {
        // ⭐ 버튼 클릭 바인드
        Button_Lobby->OnClicked.AddDynamic(this, &UMSLobbyMainWidget::OnClickedLobbyButton);
        Button_Lobby->OnHovered.AddDynamic(this, &UMSLobbyMainWidget::OnHoveredLobbyButton);
        Button_Lobby->OnUnhovered.AddDynamic(this, &UMSLobbyMainWidget::OnUnHoveredLobbyButton);
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Button_Lobby is nullptr"));
    }
    if (Button_Exit)
    {
        Button_Exit->OnClicked.AddDynamic(this, &UMSLobbyMainWidget::OnExitClicked);
    }
    ButtonState = EButtonState::Lobby;
    SetScaleButton(Button_Lobby, 1.2f);

    StartBackgroundMusic();
}

//Outline 검은색으로, 크기 1.2f
void UMSLobbyMainWidget::OnClickedSelectCharacterButton()
{
    if (ButtonState == EButtonState::SelectCharacter)
    {
        return;
    }
    if (ClickButtonSound)
    {
        UGameplayStatics::PlaySound2D(this, ClickButtonSound);
    }

    ButtonState = EButtonState::SelectCharacter;
    //세팅 : 보일것 감출것
    check(MSCharacterSelect);
    MSCharacterSelect->SetVisibility(ESlateVisibility::Visible);

    check(WBP_MSLobbyReady);
    WBP_MSLobbyReady->SetVisibility(ESlateVisibility::Collapsed);

    if (Button_Lobby)
    {
        SetScaleButton(Button_Lobby, 1.0f);
    }

    if (AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>())
    {
        UE_LOG(LogTemp, Log, TEXT("SwitchToCharacterCamera"));
        PC->SwitchToCharacterCamera();
    }
}

void UMSLobbyMainWidget::OnHoveredSelectCharacterButton()
{
    if (ButtonState == EButtonState::SelectCharacter)
    {
        return;
    }

    if (Button_Select)
    {
        SetScaleButton(Button_Select, 1.2f);
    }
}

void UMSLobbyMainWidget::OnUnHoveredSelectCharacterButton()
{
    if (ButtonState == EButtonState::SelectCharacter)
    {
        return;
    }

    if (Button_Select)
    {
        SetScaleButton(Button_Select, 1.0f);
    }
}

void UMSLobbyMainWidget::OnClickedLobbyButton()
{
    if (ButtonState == EButtonState::Lobby)
    {
        return;
    }
    if (ClickButtonSound)
    {
        UGameplayStatics::PlaySound2D(this, ClickButtonSound);
    }

    ButtonState = EButtonState::Lobby;

    check(MSCharacterSelect);
    MSCharacterSelect->SetVisibility(ESlateVisibility::Collapsed);
    check(WBP_MSLobbyReady);
    WBP_MSLobbyReady->SetVisibility(ESlateVisibility::Visible);

    if (Button_Select)
    {
        SetScaleButton(Button_Select, 1.0f);
    }

    if (AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>())
    {
        PC->SwitchToLobbyCamera();
    }
}

void UMSLobbyMainWidget::OnHoveredLobbyButton()
{
    if (ButtonState == EButtonState::Lobby)
    {
        return;
    }

    if (Button_Lobby)
    {
        SetScaleButton(Button_Lobby, 1.2f);
    }
}

void UMSLobbyMainWidget::OnUnHoveredLobbyButton()
{
    if (ButtonState == EButtonState::Lobby)
    {
        return;
    }

    if (Button_Lobby)
    {
        SetScaleButton(Button_Lobby, 1.0f);
    }
}

void UMSLobbyMainWidget::SetScaleButton(UButton* InButton,float SetSize)
{
    if (SetSize > 1.0f)
    {
        InButton->SetRenderScale(FVector2D(SetSize, SetSize));
        FButtonStyle Style = InButton->GetStyle();
        Style.Normal.OutlineSettings.Color = FSlateColor(FLinearColor::White);
        InButton->SetStyle(Style);
    }
    else
    {
        InButton->SetRenderScale(FVector2D(SetSize, SetSize));
        FButtonStyle Style = InButton->GetStyle();
        Style.Normal.OutlineSettings.Color = FSlateColor(FLinearColor::Black);
        InButton->SetStyle(Style);
    }
    //Button_MainMenu
}

void UMSLobbyMainWidget::OnExitClicked()
{
    if (ClickButtonSound)
    {
        UGameplayStatics::PlaySound2D(this, ClickButtonSound);
    }

    APlayerController* PC = GetOwningPlayer();
    if (!PC) return;

    AMSLobbyPlayerController* LobbyPC = Cast<AMSLobbyPlayerController>(PC);
    if (!LobbyPC) return;

    LobbyPC->RequestExitLobby();
}

// 배경음 시작
void UMSLobbyMainWidget::StartBackgroundMusic()
{
    if (BGMAsset && !BGMComponent)
    {
        // 사운드를 재생하고 그 참조를 BGMComponent에 저장합니다.
        // 마지막 인자 true는 자동 소멸 방지 등을 위해 설정할 수 있습니다.
        BGMComponent = UGameplayStatics::SpawnSound2D(this, BGMAsset);
        if (BGMComponent)
        {
            BGMComponent->FadeIn(1.5f, 1.0f); // 1.5초 동안 볼륨 1.0까지 상승
        }
    }
}