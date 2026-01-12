// Fill out your copyright notice in the Description page of Project Settings.


#include "GameModes/MSLobbyGameMode.h"
#include "EngineUtils.h"
#include "Actors/MSLobbyPlayerSlot.h"
#include "System/MSSteamManagerSubsystem.h"
#include "GameFramework/GameState.h"
#include <Player/MSLobbyPlayerState.h>
#include "MageSquad.h"
#include "GameStates/MSLobbyGameState.h"
#include <System/MSLevelManagerSubsystem.h>
#include "Player/MSLobbyPlayerController.h"
#include <System/MSCharacterDataSubsystem.h>
#include "DataAssets/Player/DA_CharacterData.h"
#include "Player/MSLobbyCharacter.h"

AMSLobbyGameMode::AMSLobbyGameMode()
{
    bUseSeamlessTravel = true;
}
void AMSLobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
    Super::PostLogin(NewPlayer);
    MS_LOG(LogMSNetwork, Log,TEXT("%s"), TEXT("TEST"));
    if (AMSLobbyPlayerState* PS = NewPlayer->GetPlayerState<AMSLobbyPlayerState>())
    {
        if (GameState->PlayerArray.Num() == 1)
        {
            //호스트 표시
            PS->SetHost(true);
        }
    }
}

AActor* AMSLobbyGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
    //if (!Player)
    //    return Super::ChoosePlayerStart_Implementation(Player);

    //UWorld* World = GetWorld();
    //if (!World)
    //    return Super::ChoosePlayerStart_Implementation(Player);

    //// 슬롯 캐싱 (한 번만)
    //if (PlayerSlots.IsEmpty())
    //{
    //    CachePlayerSlotsIfNeeded();
    //}

    //for (const TWeakObjectPtr<AMSLobbyPlayerSlot>& SlotPtr : PlayerSlots)
    //{
    //    if (!SlotPtr.IsValid())
    //        continue;

    //    AMSLobbyPlayerSlot* Slot = SlotPtr.Get();
    //    // 안전
    //}
    //// 비어있는 슬롯 찾기
    //for (AMSLobbyPlayerSlot* Slot : PlayerSlots)
    //{
    //    if (!IsValid(Slot))
    //        continue;

    //    if (Slot->GetController() == nullptr)
    //    {
    //        Slot->SetController(Player);
    //        Slot->HiddenInviteWidgetComponent();
    //        return Slot;
    //    }
    //}
    //for (AMSLobbyPlayerSlot* Slot : TActorRange<AMSLobbyPlayerSlot>(World))
    //{
    //    if (IsValid(Slot))
    //    {
    //        // 슬롯은 "비어 있는 상태"로 시작
    //        Slot->SetController(nullptr);
    //        PlayerSlots.Add(Slot);
    //    }
    //}



    if (!Player)
    {
        return Super::ChoosePlayerStart_Implementation(Player);
    }

    // PlayerState & NetId 확인
    AMSLobbyPlayerState* PS = Player->GetPlayerState<AMSLobbyPlayerState>();
    if (!PS)
    {
        UE_LOG(LogTemp, Log, TEXT("ChoosePlayerStart_Implementation : PS is nullptr"));
        return Super::ChoosePlayerStart_Implementation(Player);
    }

    const FUniqueNetIdRepl NetId = PS->GetUniqueId();
    if (!NetId.IsValid())
    {
        UE_LOG(LogTemp, Log, TEXT("ChoosePlayerStart_Implementation : NetId is nullptr"));
        return Super::ChoosePlayerStart_Implementation(Player);
    }

    UMSCharacterDataSubsystem* CharacterDataSubsystem = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
    if (!CharacterDataSubsystem)
    {
        return Super::ChoosePlayerStart_Implementation(Player);
    }

    CachePlayerSlotsIfNeeded();
    // =====================================================
// 1️⃣ 이미 NetId에 매칭된 SlotIndex가 있는 경우
// =====================================================
    if (const int32* AssignedIndex = CharacterDataSubsystem->FindSlotIndex(NetId))
    {
        for (const TWeakObjectPtr<AMSLobbyPlayerSlot>& SlotPtr : PlayerSlots)
        {
            if (!SlotPtr.IsValid())
                continue;

            AMSLobbyPlayerSlot* Slot = SlotPtr.Get();
            if (Slot->SlotIndex == *AssignedIndex)
            {
                Slot->SetController(Player);
                Slot->HiddenInviteWidgetComponent();
                return Slot;
            }
        }

        // SlotIndex는 있는데 SlotActor가 없다면 (이론상 거의 없음)
        UE_LOG(LogTemp, Warning,
            TEXT("SlotIndex %d assigned but SlotActor not found"), *AssignedIndex);
    }

    // =====================================================
    // 2️⃣ 처음 접속한 플레이어 → 빈 슬롯 배정
    // =====================================================
    for (const TWeakObjectPtr<AMSLobbyPlayerSlot>& SlotPtr : PlayerSlots)
    {
        if (!SlotPtr.IsValid())
            continue;

        AMSLobbyPlayerSlot* Slot = SlotPtr.Get();
        if (Slot->GetController() == nullptr)
        {
            // NetId ↔ SlotIndex 매핑
            CharacterDataSubsystem->Assign(NetId, Slot->SlotIndex);

            Slot->SetController(Player);
            Slot->HiddenInviteWidgetComponent();
            return Slot;
        }
    }

    // 슬롯이 없으면 fallback
    return Super::ChoosePlayerStart_Implementation(Player);
}

void AMSLobbyGameMode::CachePlayerSlotsIfNeeded()
{
    if (!PlayerSlots.IsEmpty())
        return;

    UWorld* World = GetWorld();
    if (!World)
        return;

    for (AMSLobbyPlayerSlot* Slot : TActorRange<AMSLobbyPlayerSlot>(World))
    {
        if (IsValid(Slot))
        {
            Slot->SetController(nullptr); // 월드 기준 초기화
            PlayerSlots.Add(Slot);
        }
    }

    PlayerSlots.Sort([](const TWeakObjectPtr<AMSLobbyPlayerSlot>& A, const TWeakObjectPtr<AMSLobbyPlayerSlot>& B)
    {
        if (!A.IsValid()) return false;
        if (!B.IsValid()) return true;
        return A->SlotIndex < B->SlotIndex;
    });
}

void AMSLobbyGameMode::PostSeamlessTravel()
{
    Super::PostSeamlessTravel();
}

void AMSLobbyGameMode::SetHiddenPlayerSlots()
{
    for (const TWeakObjectPtr<AMSLobbyPlayerSlot>& SlotPtr : PlayerSlots)
    {
        if (!SlotPtr.IsValid())
            continue;

        AMSLobbyPlayerSlot* Slot = SlotPtr.Get();
        Slot->HiddenInviteWidgetComponent();
    }
}

void AMSLobbyGameMode::SetShowPlayerSlots()
{
    for (const TWeakObjectPtr<AMSLobbyPlayerSlot>& SlotPtr : PlayerSlots)
    {
        if (!SlotPtr.IsValid())
            continue;

        AMSLobbyPlayerSlot* Slot = SlotPtr.Get();
        if (nullptr == Slot->GetController())
        {
            Slot->ShowInviteWidgetComponent();
        }
    }
}

void AMSLobbyGameMode::SetShowTargetPlayerSlot(AController* Target) const
{
    for (const TWeakObjectPtr<AMSLobbyPlayerSlot>& SlotPtr : PlayerSlots)
    {
        if (!SlotPtr.IsValid())
            continue;

        AMSLobbyPlayerSlot* Slot = SlotPtr.Get();
        if (Target == Slot->GetController())
        {
            Slot->ShowInviteWidgetComponent();
            Slot->SetController(nullptr);
        }
    }
}

void AMSLobbyGameMode::Logout(AController* Exiting)
{
    if (AMSLobbyPlayerState* PS = Exiting->GetPlayerState<AMSLobbyPlayerState>())
    {
        if (UMSCharacterDataSubsystem* CharacterDataSubsystem = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>())
        {
            CharacterDataSubsystem->ReleaseByNetId(PS->GetUniqueId());
        }
    }

    Super::Logout(Exiting);

    SetShowTargetPlayerSlot(Exiting);
}

void AMSLobbyGameMode::HandleReadyCountdownFinished()
{
    UE_LOG(LogTemp, Warning, TEXT("Lobby -> GameLevel ServerTravel"));
    //클라이언트들에게 로딩창 띄우라고 명령
    if (AMSLobbyGameState* LobbyGS = Cast<AMSLobbyGameState>(GameState))
    {
        LobbyGS->Multicast_ShowLoadingScreen();
    }

    //호스트 로딩창 띄우기
    if (UMSLevelManagerSubsystem* LevelManager = GetGameInstance()->GetSubsystem<UMSLevelManagerSubsystem>())
    {
        LevelManager->ShowLoadingWidget();
        GetWorld()->ServerTravel(LevelManager->GetGameLevelURL());
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("UMSLevelManagerSubsystem is nullptr"));
    }
    
}


void AMSLobbyGameMode::HandlePlayerReadyStateChanged()
{
    AMSLobbyGameState* LobbyGS = Cast<AMSLobbyGameState>(GameState);
    if (nullptr == LobbyGS)
    {
        MS_LOG(LogMSNetwork, Warning, TEXT("%s"), TEXT("AMSLobbyGameState nullptr"));
        return;
    }

    int32 ReadyCount = 0;
    int32 TotalPlayers = 0;

    //모든 PlayerState를 조회하여 준비상태인지 확인
    for (APlayerState* PS : GameState->PlayerArray)
    {
        if (AMSLobbyPlayerState* LobbyPS = Cast<AMSLobbyPlayerState>(PS))
        {
            ++TotalPlayers;
            if (LobbyPS->IsReady())
            {
                ++ReadyCount;
            }
        }
    }

    ELobbyReadyPhase NewPhase =
        (ReadyCount == 0) ? ELobbyReadyPhase::NotReady :
        (ReadyCount == TotalPlayers) ? ELobbyReadyPhase::AllReady :
        ELobbyReadyPhase::PartialReady;

    // Phase 변경 시에만 처리
    if (LobbyGS->GetLobbyReadyPhase() != NewPhase)
    {
        LobbyGS->SetLobbyReadyPhase(NewPhase);
        LobbyGS->OnRep_LobbyReadyPhase(); // 리슨서버 즉시 반영용
        MS_LOG(LogMSNetwork, Warning, TEXT("%d"), (int32)NewPhase);
        switch (NewPhase)
        {
        case ELobbyReadyPhase::NotReady:
            LobbyGS->StopReadyCountdown();
            break;

        case ELobbyReadyPhase::PartialReady:
            LobbyGS->StartReadyCountdown(60);
            break;

        case ELobbyReadyPhase::AllReady:
            LobbyGS->StartReadyCountdown(3);
            break;
        }
    }
}

void AMSLobbyGameMode::RestartPlayer(AController* NewPlayer)
{
    if (!NewPlayer)
    {
        return;
    }
    AMSLobbyPlayerState* PS = NewPlayer->GetPlayerState<AMSLobbyPlayerState>();
    if (!PS)
    {
        Super::RestartPlayer(NewPlayer);
        return;
    }

    const FUniqueNetIdRepl NetId = PS->GetUniqueId();
    auto* CharacterDataManager = GetGameInstance()->GetSubsystem<UMSCharacterDataSubsystem>();
    if (!NetId.IsValid() || !CharacterDataManager)
    {
        Super::RestartPlayer(NewPlayer);
        return;
    }

    const FMSCharacterSelection* CharacterSelection = CharacterDataManager->FindSelectionByNetId(NetId);
    if (!CharacterSelection)
    {
        CharacterDataManager->CacheSelectedCharacter(NetId, CharacterDataManager->GetDefaultCharacterID());
        CharacterSelection = CharacterDataManager->FindSelectionByNetId(NetId);
    }
    if (!CharacterSelection->LobbyCharacterClass)
    {
        UE_LOG(LogTemp, Log, TEXT("RestartPlayer : FMSCharacterSelection is nullptr"));
        Super::RestartPlayer(NewPlayer);
        return;
    }

    FTransform SpawnTM = ChoosePlayerStart(NewPlayer)->GetTransform();

    APawn* NewPawn = GetWorld()->SpawnActor<APawn>(
        CharacterSelection->LobbyCharacterClass,
        SpawnTM
    );
    NewPlayer->Possess(NewPawn);
    MS_LOG(LogMSNetwork, Log, TEXT("%s"), *NewPawn->GetActorRotation().ToString());
}