// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Lobby/MSLobbyMainWidget.h"
#include "Player/MSLobbyPlayerController.h"
#include "Components/Button.h"
#include "MSCharacterSelectWidget.h"
#include "MSLobbyReadyWidget.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"
#include "Components/Overlay.h"
#include "Components/CanvasPanel.h"

UMSLobbyMainWidget::UMSLobbyMainWidget(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    SetIsFocusable(true);
}

void UMSLobbyMainWidget::NativeConstruct()
{
    Super::NativeConstruct();
    if(MSCharacterSelect)
    {
        MSCharacterSelect->SetVisibility(ESlateVisibility::Collapsed);
    }
    
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

    ButtonState = EButtonState::SelectCharacter;
    //세팅 : 보일것 감출것
    check(MSCharacterSelect);
    MSCharacterSelect->SetVisibility(ESlateVisibility::Visible);
    check(WBP_MSLobbyReady);
    WBP_MSLobbyReady->SetVisibility(ESlateVisibility::Collapsed);
    check(Button_Exit);
    Button_Exit->SetVisibility(ESlateVisibility::Collapsed);
    check(CanvasPanel_OutClick);
    CanvasPanel_OutClick->SetVisibility(ESlateVisibility::Visible);

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
    UE_LOG(LogTemp, Error, TEXT("OnClickedLobbyButton"));
    AMSLobbyPlayerController* PC = GetOwningPlayer<AMSLobbyPlayerController>();
    if (!PC)
    {
        UE_LOG(LogTemp, Error, TEXT("AMSLobbyPlayerController is nullptr"));
        return;
    }
    
    if (ButtonState == EButtonState::Lobby)
    {
        return;
    }

    ButtonState = EButtonState::Lobby;

    check(MSCharacterSelect);
    MSCharacterSelect->SetVisibility(ESlateVisibility::Collapsed);
    check(WBP_MSLobbyReady);
    WBP_MSLobbyReady->SetVisibility(ESlateVisibility::Visible);
    check(Button_Exit);
    Button_Exit->SetVisibility(ESlateVisibility::Visible);
    check(CanvasPanel_OutClick);
    CanvasPanel_OutClick->SetVisibility(ESlateVisibility::SelfHitTestInvisible);

    if (Button_Select)
    {
        SetScaleButton(Button_Select, 1.0f);
    }
    UE_LOG(LogTemp, Error, TEXT("SwitchToLobbyCamera"));
    PC->SwitchToLobbyCamera();
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

FReply UMSLobbyMainWidget::NativeOnMouseButtonDown(
    const FGeometry& InGeometry,
    const FPointerEvent& InMouseEvent)
{
    // 마우스 위치
    const FVector2D ScreenPos = InMouseEvent.GetScreenSpacePosition();
    if (Overlay_CharacterSelect && ButtonState == EButtonState::SelectCharacter)
    {
        // Border의 스크린 영역 계산
        const FGeometry& BorderGeometry = Overlay_CharacterSelect->GetCachedGeometry();
        const FSlateRect BorderRect = BorderGeometry.GetLayoutBoundingRect();

        // Border 밖을 눌렀다면 → 닫기
        if (!BorderRect.ContainsPoint(ScreenPos))
        {
            OnHoveredLobbyButton();
            OnClickedLobbyButton();
            return FReply::Handled();
        }
    }

    // Border 안 클릭 → 그냥 소비해서 아래로 전달 안 함
    return FReply::Handled();
}