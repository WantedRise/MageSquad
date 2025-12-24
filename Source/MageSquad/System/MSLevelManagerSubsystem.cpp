// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSLevelManagerSubsystem.h"
#include <Kismet/GameplayStatics.h>
#include "Blueprint/UserWidget.h"

UMSLevelManagerSubsystem::UMSLevelManagerSubsystem()
{

	static ConstructorHelpers::FClassFinder<UUserWidget> LoadingWidgetClassFinder(TEXT("/Game/Blueprints/Widgets/Loading/WBP_Loading.WBP_Loading_C"));
	FWorldDelegates::OnPostWorldInitialization.AddUObject(this, &UMSLevelManagerSubsystem::OnMapLoaded);
	if (LoadingWidgetClassFinder.Succeeded())
	{
		LoadingWidgetClass = LoadingWidgetClassFinder.Class;
	}
}

void UMSLevelManagerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
    Super::Initialize(Collection);

    LobbyLevelURL = TEXT("LobbyLevel");
	GameLevelURL = TEXT("GameLevel?Listen");
    LoadingLevelURL = TEXT("LoadingLevel");
}

void UMSLevelManagerSubsystem::Deinitialize()
{

    Super::Deinitialize();
}

void UMSLevelManagerSubsystem::TravelToLoadingLevel()
{
    UGameplayStatics::OpenLevel(GetWorld(), FName(LoadingLevelURL));
}

void UMSLevelManagerSubsystem::TravelToGameLevel()
{
	UGameplayStatics::OpenLevel(GetWorld(), FName(GameLevelURL));
}

void UMSLevelManagerSubsystem::HostGameAndTravelToLobby()
{
	FString TravelURL = LobbyLevelURL;

	// 현재 NetMode 확인
	ENetMode NetMode = GetWorld()->GetNetMode();

	// 서버가 아니거나, 아직 Listen 서버가 아닌 경우
	// (Standalone 또는 Client에서 Host 개념으로 쓰는 경우)
	if (NetMode == NM_Standalone)
	{
		TravelURL += TEXT("?listen");
		UE_LOG(LogTemp, Log, TEXT("TravelToLobby: Standalone -> Listen Server"));
	}
	else
	{
		UE_LOG(LogTemp, Log, TEXT("TravelToLobby: Already Listen Server"));
	}

	GetWorld()->ServerTravel(TravelURL);
	

    //UGameplayStatics::OpenLevel(GetWorld(), FName(LobbyLevelURL));
}

void UMSLevelManagerSubsystem::ShowLoadingWidget()
{
	if (CurrentLoadingWidget || !LoadingWidgetClass) return;

	UGameInstance* GI = GetGameInstance();
	if (!GI) return;

	// 위젯 생성
	CurrentLoadingWidget = CreateWidget<UUserWidget>(GI, LoadingWidgetClass);

	if (CurrentLoadingWidget)
	{
		// Z-Order를 높게 설정하여 렌더링 최상단에 배치
		CurrentLoadingWidget->AddToViewport(9999);
	}
}

void UMSLevelManagerSubsystem::HideLoadingWidget()
{
	if (CurrentLoadingWidget)
	{
		CurrentLoadingWidget->RemoveFromParent();
		CurrentLoadingWidget = nullptr;
	}
}

void UMSLevelManagerSubsystem::OnMapLoaded(UWorld* World, const UWorld::InitializationValues IVS)
{
	//BeginPlay보다 훨씬 빠른 시점인 월드 초기화 직후에 
	//위젯을 다시 뷰포트에 추가하여 검은 화면 노출 시간을 최소화
	if (CurrentLoadingWidget && !CurrentLoadingWidget->IsInViewport())
	{
		CurrentLoadingWidget->AddToViewport(9999);
	}
}