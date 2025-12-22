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
    Text_MissionMessage->SetText(InMessage);

    SetVisibility(ESlateVisibility::Visible);

    FWidgetAnimationDynamicEvent EndEvent;
    EndEvent.BindDynamic(this, &UMSMissionNotifyWidget::OnNotifyFinished);
    BindToAnimationFinished(Anim_ShowNotify, EndEvent);

    PlayAnimation(Anim_ShowNotify);
}

void UMSMissionNotifyWidget::PlayMissionResult(bool bSuccess)
{
    // 결과에 따른 텍스트 설정
    FText ResultText = bSuccess ?
        NSLOCTEXT("Mission", "Success", "미션 완료") :
        NSLOCTEXT("Mission", "Failed", "미션 실패");

    if (Text_MissionMessage)
    {
        Text_MissionMessage->SetText(ResultText);
    }

    SetVisibility(ESlateVisibility::Visible);

    // 애니메이션 종료 이벤트 바인딩
    FWidgetAnimationDynamicEvent EndEvent;
    EndEvent.BindDynamic(this, &UMSMissionNotifyWidget::OnResultFinished);
    BindToAnimationFinished(Anim_MissionResult, EndEvent);

    // 4. 애니메이션 실행
    PlayAnimation(Anim_MissionResult);
}

void UMSMissionNotifyWidget::OnNotifyFinished()
{
    // 애니메이션이 끝나면 위젯을 숨기거나 비활성화
    SetVisibility(ESlateVisibility::Collapsed); // 또는 Hidden

    // 필요 시 바인딩 해제
    UnbindAllFromAnimationFinished(Anim_ShowNotify);
}
void UMSMissionNotifyWidget::OnResultFinished()
{
    // 애니메이션이 끝나면 위젯을 숨기거나 비활성화
    SetVisibility(ESlateVisibility::Collapsed); // 또는 Hidden

    // 필요 시 바인딩 해제
    UnbindAllFromAnimationFinished(Anim_MissionResult);
}
