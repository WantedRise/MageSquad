// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/HUD/GameProgressWidget.h"
#include "Components/ProgressBar.h"
#include "GameStates/MSGameState.h"

void UGameProgressWidget::NativeConstruct()
{
    Super::NativeConstruct();


    if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
    {
        GS->OnProgressUpdated.AddUObject(this,&UGameProgressWidget::OnProgressUpdated);

        OnProgressUpdated(GS->GetProgressNormalized());
    }
}
void UGameProgressWidget::OnProgressUpdated(float Normalized)
{
    if (PB_GameProgress)
    {
        PB_GameProgress->SetPercent(Normalized);
    }
}