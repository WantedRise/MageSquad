// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSGameProgressWidget.generated.h"

/*
* 작성자: 이상준
* 작성일: 2025-12-30
* 게임 전체 진행도를 표시하는 상단 Progress UI 위젯
*
* - GameState에서 전달되는 진행도를 시각적으로 표현
* - 진행도 바 위에 미션 이벤트 마커를 동적으로 배치
* - 진행도 변화는 보간으로 부드럽게 표현
*/
UCLASS()
class MAGESQUAD_API UMSGameProgressWidget : public UUserWidget
{
	GENERATED_BODY()
public:
    // 미션 발생 시 진행도 기준 위치에 이벤트 마커 추가
    void AddProgressEventMarker(int32 MissionID,float Percent);
protected:
    virtual void NativeConstruct() override;
    // 매 프레임 진행도 보간 처리
    virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
    // GameState가 아직 준비되지 않았을 경우 재시도
	UFUNCTION()
	void TryInitializGameProgress();
private:
    // GameState에서 전달되는 진행도 변경 이벤트 처리
    void OnProgressUpdated(float Normalized);
    // 미션 변경 이벤트 처리 (이벤트 마커 생성)
	void HandleMissionChanged(int32 MissionID);
private:
    // 실제 진행도를 표시
    UPROPERTY(meta = (BindWidget))
    class UProgressBar* PB_GameProgress;
    // ProgressBar의 실제 너비 계산 기준
    UPROPERTY(meta = (BindWidget))
    class USizeBox* SizeBox_Track;
    
    // 미션 이벤트 마커를 배치하는 레이어
    UPROPERTY(meta = (BindWidget))
    class UCanvasPanel* MissionEventLayer;

    // 미션 이벤트 마커 위젯 클래스
    UPROPERTY(EditDefaultsOnly, Category = "Progress")
    TSubclassOf<UUserWidget> ProgressEventMarkerClass;

    float TargetPercent = 0.f;
    float CurrentPercent = 0.f;
    // 진행도 보간 속도
    UPROPERTY(EditAnywhere, Category = "Progress")
    float InterpSpeed = 3.f;
};
