// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "MSLevelManagerSubsystem.generated.h"

/*
* 작성자: 이상준
* 작성일: 25/12/07
* 레벨 이동과 레벨이동시 로딩창과 전달할 데이터를 관리하기 위한 매니저 클래스
*
*/
UCLASS()
class MAGESQUAD_API UMSLevelManagerSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()
	
public:
    UMSLevelManagerSubsystem();
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    void TravelToLoadingLevel();
    void HostGameAndTravelToLobby();

    void ShowLoadingWidget();
    void HideLoadingWidget();
    void OnMapLoaded(UWorld* World, const UWorld::InitializationValues IVS);
private:
    UPROPERTY()
    TSubclassOf<UUserWidget> LoadingWidgetClass;
    UPROPERTY()
    UUserWidget* CurrentLoadingWidget;

    UPROPERTY(VisibleAnywhere)
    FString LobbyLevelURL;
    UPROPERTY(VisibleAnywhere)
    FString LoadingLevelURL;
};