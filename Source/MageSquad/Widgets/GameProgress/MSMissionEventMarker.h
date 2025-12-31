// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSMissionEventMarker.generated.h"

/**
 * 작성자: 이상준
 * 작성일: 25/12/30
 *
 * 진행도 바 위에 표시되는 개별 미션 이벤트 마커 위젯
 *
 * - 미션 ID에 따라 아이콘을 동적으로 로드
 * - 등장 시 간단한 이동 애니메이션 재생
 */
UCLASS()
class MAGESQUAD_API UMSMissionEventMarker : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMissionID(int32 InMissionID);
	float GetImageSizeX();
protected:
	virtual void NativeDestruct() override;
private:
	void LoadMissionIcon();
private:
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Point;
	UPROPERTY(meta = (BindWidget))
	class UImage* Image_Marker;
	UPROPERTY(Transient, meta = (BindWidgetAnim))
	class UWidgetAnimation* Anim_Move;
	

	int32 MissionID = INDEX_NONE;
};
