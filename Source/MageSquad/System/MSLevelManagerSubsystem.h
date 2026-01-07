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
    //레벨 URL 초기화
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;
    virtual void Deinitialize() override;

public:
    //클라 로딩 전용 레벨로 이동
    void TravelToLoadingLevel();
    //클라 실제 게임 레벨로 이동
    void TravelToGameLevel();
    void TravelToMainLevel();
    //서버에서 로비를 호스트하고 모든 클라이언트를 로비로 이동
    void HostGameAndTravelToLobby();
    
    FString GetMainmenuLevelURL() { return MainmenuLevelURL; }
    FString GetGameLevelURL() { return GameLevelURL; }
    //로딩 위젯 표시
    void ShowLoadingWidget();
    //로딩 위젯 제거
    void HideLoadingWidget();
    //월드 초기화 직 로딩 위젯을 Viewport에 추가
    void OnMapLoaded(UWorld* World, const UWorld::InitializationValues IVS);
private:
    UFUNCTION()
    void OnSessionCreatedDelayTravel(bool bWasSuccessful);
private:
    //로딩 화면으로 사용할 위젯 클래스
    UPROPERTY()
    TSubclassOf<UUserWidget> LoadingWidgetClass;
    //현재 표시 중인 로딩 위젯 인스턴스
    UPROPERTY()
    UUserWidget* CurrentLoadingWidget;
    //서버가 Listen 상태로 이동할 로비 레벨 URL
    UPROPERTY(VisibleAnywhere)
    FString LobbyLevelURL;
    ////로딩 전용 레벨 URL
    UPROPERTY(VisibleAnywhere)
    FString LoadingLevelURL;
    //실제 게임 플레이 레벨 URL
    UPROPERTY(VisibleAnywhere)
    FString GameLevelURL;
    UPROPERTY(VisibleAnywhere)
    FString MainmenuLevelURL;
};