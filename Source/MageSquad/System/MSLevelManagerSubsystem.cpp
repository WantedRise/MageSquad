// Fill out your copyright notice in the Description page of Project Settings.


#include "System/MSLevelManagerSubsystem.h"
#include <Kismet/GameplayStatics.h>
#include "Blueprint/UserWidget.h"
#include "MSSteamManagerSubsystem.h"
#include <GameModes/MSGameMode.h>

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

    LobbyLevelURL = TEXT("LobbyLevel?listen");
	GameLevelURL = TEXT("GameLevel");
    LoadingLevelURL = TEXT("LoadingLevel");
}

void UMSLevelManagerSubsystem::Deinitialize()
{

    Super::Deinitialize();
}

void UMSLevelManagerSubsystem::SaveSelectedCharacter(const FUniqueNetIdRepl& NetId, FName CharacterID)
{
	PendingSelectedCharacters.Add(NetId, CharacterID);
	
	UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem::SaveSelectedCharacter() %s"), *NetId.GetUniqueNetId().Get()->ToString());
}

bool UMSLevelManagerSubsystem::ConsumeSelectedCharacter(const FUniqueNetIdRepl& NetId, FName& OutCharacterID)
{
	if (FName* Found = PendingSelectedCharacters.Find(NetId))
	{
		OutCharacterID = *Found;
		PendingSelectedCharacters.Remove(NetId);

		UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem::ConsumeSelectedCharacter() %s"), *NetId.GetUniqueNetId().Get()->ToString());

		return true;
	}

	return false;
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
	// 현재 NetMode 확인
	ENetMode NetMode = GetWorld()->GetNetMode();
	UMSSteamManagerSubsystem* SteamManager = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>();
	if (SteamManager)
	{
		if (AMSGameMode* GM = Cast<AMSGameMode>(GetWorld()->GetAuthGameMode()))
		{
			GM->NotifyClientsShowLoadingWidget();
		}
		//로딩 위젯을 띄움
		ShowLoadingWidget();
		//세션 완료 델리게이트에 이동 함수 바인딩
		SteamManager->MSOnCreateSessionCompleteDelegate.AddDynamic(this, &UMSLevelManagerSubsystem::OnSessionCreatedDelayTravel);
		//세션 생성 시작
		SteamManager->CreateSteamSession(true, 4);
	}

    //UGameplayStatics::OpenLevel(GetWorld(), FName(LobbyLevelURL));
}

void UMSLevelManagerSubsystem::OnSessionCreatedDelayTravel(bool bWasSuccessful)
{
	UMSSteamManagerSubsystem* SteamManager = GetGameInstance()->GetSubsystem<UMSSteamManagerSubsystem>();
	if (SteamManager)
	{
		SteamManager->MSOnCreateSessionCompleteDelegate.RemoveDynamic(this, &UMSLevelManagerSubsystem::OnSessionCreatedDelayTravel);
	}

	GetWorld()->ServerTravel(LobbyLevelURL);
}

void UMSLevelManagerSubsystem::ShowLoadingWidget()
{
	UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem::Begin()"));
	if (CurrentLoadingWidget || !LoadingWidgetClass) return;
	UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem::if (CurrentLoadingWidget || !LoadingWidgetClass)()"));
	UGameInstance* GI = GetGameInstance();
	if (!GI) return;
	UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem::GetGameInstance()"));
	// 위젯 생성
	CurrentLoadingWidget = CreateWidget<UUserWidget>(GI, LoadingWidgetClass);

	if (CurrentLoadingWidget)
	{
		UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem::CreateWidget<UUserWidget>(GI, LoadingWidgetClass);()"));
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