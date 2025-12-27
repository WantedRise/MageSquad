// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSGameProgressWidget.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSGameProgressWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    /** Percent (0~1) 기준으로 이벤트 마커 생성 */
    void AddProgressEventMarker(int32 MissionID,float Percent);
protected:
    virtual void NativeConstruct() override;
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	UFUNCTION()
	void TryInitializGameProgress();
private:
    void OnProgressUpdated(float Normalized);
	void HandleMissionChanged(int32 MissionID);
private:
    /** 기준 SizeBox */
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* PB_GameProgress;

    UPROPERTY(meta = (BindWidget))
    class USizeBox* SizeBox_Track;
    
    /** 이벤트 마커 레이어 */
    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* MissionEventLayer;

    /** 이벤트 마커 위젯 클래스 */
    UPROPERTY(EditDefaultsOnly, Category = "Progress")
    TSubclassOf<UUserWidget> ProgressEventMarkerClass;

    float TargetPercent = 0.f;
    float CurrentPercent = 0.f;

    UPROPERTY(EditAnywhere, Category = "Progress")
    float InterpSpeed = 3.f;
};
