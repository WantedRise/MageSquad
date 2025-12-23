// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Mission/MSMissionNotifyWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UMSMissionNotifyWidget::SetMissionMessage(FText InMessage)
{
	Text_MissionMessage->SetText(InMessage);
}

void UMSMissionNotifyWidget::PlayNotify(FText InMessage)
{
    StopAllAnimations();

    Text_MissionMessage->SetText(InMessage);

    PlayAnimation(
        Anim_ShowNotify,
        0.f,   
        1,     
        EUMGSequencePlayMode::Forward,
        1.f   
    );
}

void UMSMissionNotifyWidget::PlayMissionResult(bool bSuccess)
{
    StopAllAnimations();

    // 결과에 따른 텍스트 설정
    FText ResultText = bSuccess ?
        NSLOCTEXT("Mission", "Success", "미션 완료") :
        NSLOCTEXT("Mission", "Failed", "미션 실패");

    Text_MissionMessage->SetText(ResultText);

    PlayAnimation(
        Anim_MissionResult,
        0.f, 
        1,    
        EUMGSequencePlayMode::Forward,
        1.f    
    );
}