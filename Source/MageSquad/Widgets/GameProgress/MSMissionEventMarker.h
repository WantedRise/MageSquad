// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "MSMissionEventMarker.generated.h"

/**
 * 
 */
UCLASS()
class MAGESQUAD_API UMSMissionEventMarker : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetMissionID(int32 InMissionID);
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
