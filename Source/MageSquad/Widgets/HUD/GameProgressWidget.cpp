// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/GameProgressWidget.h"
#include "Components/ProgressBar.h"
#include "GameStates/MSGameState.h"
#include "Components/MSGameProgressComponent.h"

void UGameProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();


    if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
    {
        if (UMSGameProgressComponent* Progress = GS->FindComponentByClass<UMSGameProgressComponent>())
        {
            Progress->OnProgressChanged.AddUObject(this,&UGameProgressWidget::OnProgressUpdated);
            OnProgressUpdated(Progress->GetNormalizedProgress());
        }
    }
}
void UGameProgressWidget::OnProgressUpdated(float Normalized)
{
    if (PB_GameProgress)
    {
        PB_GameProgress->SetPercent(Normalized);
    }
}