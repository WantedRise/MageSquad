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

    
    PlayAnimation(
        Anim_Move
    );
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
    //SetVisibility(ESlateVisibility::Collapsed);
    Super::NativeDestruct();
}

