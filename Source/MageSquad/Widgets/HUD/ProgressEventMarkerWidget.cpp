// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/ProgressEventMarkerWidget.h"
#include "Components/Image.h"
#include "Engine/AssetManager.h"
#include <System/MSMissionDataSubsystem.h>

void UProgressEventMarkerWidget::SetMissionID(int32 InMissionID)
{
    MissionID = InMissionID;
    LoadMissionIcon();
}

void UProgressEventMarkerWidget::LoadMissionIcon()
{
    if (!Image_Icon || MissionID == INDEX_NONE)
        return;

    UGameInstance* GI = GetWorld()->GetGameInstance();
    if (!GI)
        return;

    UMSMissionDataSubsystem* MissionData = GI->GetSubsystem<UMSMissionDataSubsystem>();
    if (!MissionData)
        return;

    const FMSMissionRow* Row = MissionData->Find(MissionID);
    if (!Row)
        return;

    const TSoftObjectPtr<UTexture2D>& IconPtr = Row->MissionIcon;

    if (IconPtr.IsNull())
    {
        Image_Icon->SetBrushFromTexture(nullptr);
        return;
    }

    if (IconPtr.IsValid())
    {
        Image_Icon->SetBrushFromTexture(IconPtr.Get());
        SetVisibility(ESlateVisibility::SelfHitTestInvisible);
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
                if (Image_Icon && IconPtr.IsValid())
                {
                    Image_Icon->SetBrushFromTexture(IconPtr.Get());
                    SetVisibility(ESlateVisibility::SelfHitTestInvisible);
                }
            }
        )
    );
}

void UProgressEventMarkerWidget::NativeDestruct()
{
    MissionID = INDEX_NONE;
    SetVisibility(ESlateVisibility::Collapsed);
    Super::NativeDestruct();
}
