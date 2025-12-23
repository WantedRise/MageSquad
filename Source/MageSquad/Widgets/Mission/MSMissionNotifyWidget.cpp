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

    /*FWidgetAnimationDynamicEvent EndEvent;
    EndEvent.BindDynamic(this, &UMSMissionNotifyWidget::OnNotifyFinished);
    BindToAnimationFinished(Anim_ShowNotify, EndEvent);*/

    PlayAnimation(
        Anim_ShowNotify,
        0.f,   // StartAtTime
        1,     // NumLoops (반드시 1)
        EUMGSequencePlayMode::Forward,
        1.f    // PlaybackSpeed
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

    // 애니메이션 종료 이벤트 바인딩
    /*FWidgetAnimationDynamicEvent EndEvent;
    EndEvent.BindDynamic(this, &UMSMissionNotifyWidget::OnResultFinished);
    BindToAnimationFinished(Anim_MissionResult, EndEvent);*/

    PlayAnimation(
        Anim_MissionResult,
        0.f,   // StartAtTime
        1,     // NumLoops (반드시 1)
        EUMGSequencePlayMode::Forward,
        1.f    // PlaybackSpeed
    );
}