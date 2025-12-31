// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/GameProgress/MSMissionEventMarker.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include <System/MSMissionDataSubsystem.h>
#include "Animation/WidgetAnimation.h"

void UMSMissionEventMarker::SetMissionID(int32 InMissionID)
{
    MissionID = InMissionID;
    LoadMissionIcon();

    
    PlayAnimation(Anim_Move,0.0f,0);
}

float UMSMissionEventMarker::GetImageSizeX()
{
    if (Image_Marker)
    {
        // 위젯의 최신 지오메트리 정보를 가져옴
        FGeometry MarkerGeometry = Image_Marker->GetCachedGeometry();

        // 로컬 크기를 가져옴 (이 값이 위젯의 실제 너비와 높이임)
        FVector2D MarkerSize = MarkerGeometry.GetLocalSize();

        return MarkerSize.X;  // 가로 길이
    }

    return 0.0f;
}

void UMSMissionEventMarker::LoadMissionIcon()
{
    UE_LOG(LogTemp, Error, TEXT("LoadMissionIcon"));
    if (!Image_Marker || MissionID == INDEX_NONE)
        return;

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI)
        return;

    UMSMissionDataSubsystem* MissionData = GI->GetSubsystem<UMSMissionDataSubsystem>();
    if (!MissionData)
        return;
    UE_LOG(LogTemp, Error, TEXT("MissionData"));
    const FMSMissionRow* Row = MissionData->Find(MissionID);
    if (!Row)
        return;
    UE_LOG(LogTemp, Error, TEXT("MissionData Find"));
    const TSoftObjectPtr<UTexture2D>& IconPtr = Row->MissionIcon;

    if (IconPtr.IsNull())
    {
        Image_Marker->SetBrushFromTexture(nullptr);
        return;
    }

    if (IconPtr.IsValid())
    {
        Image_Marker->SetBrushFromTexture(IconPtr.Get());
        //SetVisibility(ESlateVisibility::SelfHitTestInvisible);
        UE_LOG(LogTemp, Error, TEXT("IconPtr.IsValid()"));
        return;
    }

    FStreamableManager& Streamable =
        UAssetManager::GetStreamableManager();

    Streamable.RequestAsyncLoad(
        IconPtr.ToSoftObjectPath(),
        FStreamableDelegate::CreateWeakLambda(
            this,
            [this, IconPtr]()
            {
                if (Image_Marker && IconPtr.IsValid())
                {
                    Image_Marker->SetBrushFromTexture(IconPtr.Get());
                    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
                    UE_LOG(LogTemp, Error, TEXT("Image_Marker success"));
                }
            }
        )
    );
}

void UMSMissionEventMarker::NativeDestruct()
{
    MissionID = INDEX_NONE;
    StopAnimation(Anim_Move);
    //SetVisibility(ESlateVisibility::Collapsed);
    Super::NativeDestruct();
}

