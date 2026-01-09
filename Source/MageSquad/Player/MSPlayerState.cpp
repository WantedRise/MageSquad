// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerState.h"

#include "MSPlayerCharacter.h"
#include "Player/MSPlayerController.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "GameStates/MSGameState.h"
#include "Abilities/GameplayAbility.h"
#include "Net/UnrealNetwork.h"
#include <System/MSCharacterDataSubsystem.h>
#include "MageSquad.h"
#include "MSGameplayTags.h"

AMSPlayerState::AMSPlayerState()
{
	// PlayerState의 기본 복제 지연이 낮으므로, 네트워크 업데이트 빈도를 늘림
	SetNetUpdateFrequency(60.f);

	AbilitySystemComponent = CreateDefaultSubobject<UMSPlayerAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Mixed 모드 설정으로 클라이언트에 일부는 최소 복제, 일부는 전체 복제되도록 설정
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<UMSPlayerAttributeSet>(TEXT("AttributeSet"));

	bUIReady = false;
	bIsAlive = true;
}
void AMSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSPlayerState, bUIReady);
	DOREPLIFETIME(AMSPlayerState, bIsAlive);
	DOREPLIFETIME(AMSPlayerState, SelectedCharacterID);
}

void AMSPlayerState::SetUIReady(bool bReady)
{
	if (!HasAuthority())
	{
		return;
	}
	bUIReady = bReady;
}

UAbilitySystemComponent* AMSPlayerState::GetAbilitySystemComponent() const
{
	check(AbilitySystemComponent);
	return AbilitySystemComponent;
}

UMSPlayerAttributeSet* AMSPlayerState::GetAttributeSet() const
{
	check(AttributeSet);
	return AttributeSet;
}

int32 AMSPlayerState::FindOwnedSkillIndexByTag(const TArray<FMSSkillList>& OwnedSkills, const FGameplayTag& SkillTag)
{
	for (int32 i = 0; i < OwnedSkills.Num(); ++i)
	{
		if (OwnedSkills[i].SkillEventTag == SkillTag)
		{
			return i;
		}
	}
	return INDEX_NONE;
}

const FMSSkillList* AMSPlayerState::GetOwnedSkillByID(int32 SkillID) const
{
	for (const FMSSkillList& Skill : OwnedSkills)
	{
		if (Skill.SkillID == SkillID)
		{
			return &Skill;
		}
	}
	return nullptr;
}

const FMSSkillList* AMSPlayerState::FindSkillRowByTag(UDataTable* SkillListDataTable, const FGameplayTag& SkillTag)
{
	if (!SkillListDataTable || !SkillTag.IsValid())
		return nullptr;

	static const FString Ctx(TEXT("FindSkillRowByTag"));
	TArray<FMSSkillList*> AllRows;
	SkillListDataTable->GetAllRows(Ctx, AllRows);

	for (const FMSSkillList* Row : AllRows)
	{
		if (Row && Row->SkillEventTag == SkillTag)
		{
			return Row;
		}
	}
	return nullptr;
}

void AMSPlayerState::FindSkillRowBySkillIDAndAdd(const int32 SkillID)
{
	if (!HasAuthority())
	{
		return;
	}

	if (!SkillListDataTable || SkillID <= 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FindSkillRowBySkillIDAndAdd] Invalid DataTable or SkillID=%d"), SkillID);
		return;
	}

	// 이미 보유 중이면 무시(중복 방지)
	for (const FMSSkillList& Owned : OwnedSkills)
	{
		if (Owned.SkillID == SkillID)
		{
			return;
		}
	}

	// DataTable에서 Row 찾기 (선형 탐색)
	static const FString Ctx(TEXT("FindSkillRowBySkillIDAndAdd"));
	TArray<FMSSkillList*> AllRows;
	SkillListDataTable->GetAllRows(Ctx, AllRows);

	const FMSSkillList* FoundRow = nullptr;
	for (const FMSSkillList* Row : AllRows)
	{
		if (Row && Row->SkillID == SkillID)
		{
			FoundRow = Row;
			break;
		}
	}

	if (!FoundRow)
	{
		UE_LOG(LogTemp, Warning, TEXT("[FindSkillRowBySkillIDAndAdd] Row not found. SkillID=%d"), SkillID);
		return;
	}

	// 값 복사해서 추가
	FMSSkillList NewSkill = *FoundRow;
	OwnedSkills.Add(NewSkill);

	if (NewSkill.SkillType == 1) SkillNum++;
}

void AMSPlayerState::ApplyUpgradeTagToSkill(FMSSkillList& Skill, const FGameplayTag& UpgradeTag)
{
	if (!UpgradeTag.IsValid())
		return;

	FMSUpgradeInfo* UpgradeInfoPtr = nullptr;
	for (FMSUpgradeInfo& Info : Skill.UpgradeInfos)
	{
		if (Info.Tag == UpgradeTag)
		{
			UpgradeInfoPtr = &Info;
			break;
		}
	}

	if (!UpgradeInfoPtr)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ApplyUpgradeTagToSkill] Upgrade tag not found. Tag=%s"), *UpgradeTag.ToString());
		return;
	}

	if (UpgradeInfoPtr->Max > 0 && UpgradeInfoPtr->Current >= UpgradeInfoPtr->Max)
	{
		UE_LOG(LogTemp, Warning, TEXT("[ApplyUpgradeTagToSkill] Upgrade tag reached max. Tag=%s"), *UpgradeTag.ToString());
		return;
	}

	UpgradeInfoPtr->Current += 1;

	const FString TagStr = UpgradeTag.ToString();

	// Skill.CoolTime, Skill.SkillDamage, Skill.ProjectileNumber, Skill.Range
	if (TagStr.Contains(TEXT("Upgrade.CoolTime")))
	{
		const float BaseCoolTime = (Skill.BaseCoolTime > 0.f) ? Skill.BaseCoolTime : Skill.CoolTime;
		const float NewCoolTime = BaseCoolTime * (1.f - 0.1f * UpgradeInfoPtr->Current);
		Skill.CoolTime = FMath::Max(0.05f, NewCoolTime);
	}
	else if (TagStr.Contains(TEXT("Upgrade.Damage")))
	{
		const float BaseDamage = (Skill.BaseSkillDamage > 0.f) ? Skill.BaseSkillDamage : Skill.SkillDamage;
		Skill.SkillDamage = BaseDamage * (1.f + 0.3f * UpgradeInfoPtr->Current); // 30% damage
	}
	else if (TagStr.Contains(TEXT("Upgrade.Projectile")))
	{
		Skill.ProjectileNumber += 1;
	}
	else if (TagStr.Contains(TEXT("Upgrade.Range")))
	{
		const float BaseRange = (Skill.BaseRange > 0.f) ? Skill.BaseRange : Skill.Range;
		Skill.Range = BaseRange * (1.f + 0.2f * UpgradeInfoPtr->Current);
	}
	else if (TagStr.Contains(TEXT("Upgrade.Duration")))
	{
		const float BaseDuration = (Skill.BaseDuration > 0.f) ? Skill.BaseDuration : Skill.Duration;
		Skill.Duration = BaseDuration * (1.f + 0.2f * UpgradeInfoPtr->Current);
	}
	else if (TagStr.Contains(TEXT("Upgrade.Penetration")))
	{
		Skill.Penetration += 1;
	}
}

void AMSPlayerState::GiveAbilityForSkillRow_Server(const FMSSkillList& Skill)
{
	if (!HasAuthority() || !AbilitySystemComponent)
		return;
	
	if (Skill.SkillAbility.IsNull())
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GiveAbility] SkillAbility is NULL. Skill=%s"),
			*Skill.SkillEventTag.ToString());
		return;
	}

	// 소프트 로딩 (이미 로드돼 있으면 바로 반환)
	TSubclassOf<UGameplayAbility> AbilityClass =
		Skill.SkillAbility.LoadSynchronous();

	if (!AbilityClass)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[GiveAbility] Failed to load GA. Skill=%s"),
			*Skill.SkillEventTag.ToString());
		return;
	}

	FGameplayAbilitySpec Spec(AbilityClass, Skill.SkillLevel);
	AbilitySystemComponent->GiveAbility(Spec);

	UE_LOG(LogTemp, Log,
		TEXT("[GiveAbility] GA granted. Skill=%s Level=%d"),
		*Skill.SkillEventTag.ToString(),
		Skill.SkillLevel);
}

void AMSPlayerState::BeginSkillLevelUp(int32 SessionId, bool bIsSpellEnhancement)
{
	if (!HasAuthority())
		return;

	CurrentLevelUpSessionId = SessionId;
	bSkillLevelUpCompleted = false;
	CurrentSkillChoices.Reset();

	if (!SkillListDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[BeginSkillLevelUp] SkillListDataTable is NULL"));
		return;
	}

	static const FString Context(TEXT("BeginSkillLevelUp"));
	TArray<FMSSkillList*> AllRows;
	SkillListDataTable->GetAllRows(Context, AllRows);

	const bool bHasFreeSlot = (SkillNum < MaxSkillCount);

	// 후보 풀 분리
	TArray<FMSLevelUpChoicePair> AcquireCandidates; // 새 스킬 습득(UpgradeTag invalid)
	TArray<FMSLevelUpChoicePair> UpgradeCandidates; // 기존 스킬 업그레이드

	AcquireCandidates.Reserve(AllRows.Num());
	UpgradeCandidates.Reserve(AllRows.Num() * 4);

	for (const FMSSkillList* Row : AllRows)
	{
		if (!Row) continue;
		if (Row->SkillType != 1) continue;

		const FGameplayTag SkillTag = Row->SkillEventTag;
		if (!SkillTag.IsValid()) continue;

		// 보유 여부/레벨 조회
		bool bOwned = false;
		int32 CurrentLevel = 0;
		FMSSkillList* OwnedSkillPtr = nullptr;
		for (FMSSkillList& Owned : OwnedSkills)
		{
			if (Owned.SkillEventTag == SkillTag)
			{
				bOwned = true;
				CurrentLevel = Owned.SkillLevel;
				OwnedSkillPtr = &Owned;
				break;
			}
		}

		// -----------------------
		// 미보유 스킬 -> 습득 후보
		// -----------------------
		if (!bOwned)
		{
			if (bIsSpellEnhancement)
				continue;

			// 슬롯이 없으면 습득 후보 자체를 만들지 않음
			if (!bHasFreeSlot)
				continue;

			FMSLevelUpChoicePair Acquire;
			Acquire.SkillTag = SkillTag;
			Acquire.UpgradeTag = FGameplayTag(); // ✅ Invalid = Acquire로 해석
			Acquire.SkillLevelUpIcon = Row->SkillLevelUpIcon;
			Acquire.SkillName = FText::FromString(Row->SkillName);
			Acquire.SkillDescription = Row->SkillDescription;
			AcquireCandidates.Add(Acquire);
			continue; // 미보유는 업그레이드 후보로 내려가지 않음
		}

		// -----------------------
		// 2) 보유 스킬 -> 업그레이드 후보
		// -----------------------
		if (CurrentLevel >= MaxSkillLevel)
			continue;

		// 업그레이드 태그가 없으면 업그레이드 후보 제외 (원하면 "레벨업만" 후보로 넣을 수도 있음)
		if (!OwnedSkillPtr || OwnedSkillPtr->UpgradeInfos.Num() <= 0)
			continue;

		for (const FMSUpgradeInfo& UpgradeInfo : OwnedSkillPtr->UpgradeInfos)
		{
			if (!UpgradeInfo.Tag.IsValid())
				continue;

			if (UpgradeInfo.Max > 0 && UpgradeInfo.Current >= UpgradeInfo.Max)
				continue;

			const bool bIsEnhanceTag = (UpgradeInfo.Tag == MSGameplayTags::Upgrade_Enhance);
			if (bIsSpellEnhancement && !bIsEnhanceTag)
				continue;
			if (!bIsSpellEnhancement && bIsEnhanceTag)
				continue;

			FMSLevelUpChoicePair Pair;
			Pair.SkillTag = SkillTag;
			Pair.UpgradeTag = UpgradeInfo.Tag;
			Pair.SkillLevelUpIcon = Row->SkillLevelUpIcon;
			Pair.SkillName = FText::FromString(Row->SkillName);
			Pair.SkillDescription = Row->SkillDescription;
			UpgradeCandidates.Add(Pair);
		}
	}

	// 후보가 아예 없으면 종료
	if (AcquireCandidates.Num() == 0 && UpgradeCandidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BeginSkillLevelUp] No candidates"));
		return;
	}

	// 셔플
	const int32 PlayerSeed = HashCombine(CurrentLevelUpSessionId, GetPlayerId());
	FRandomStream RandStream(PlayerSeed);

	auto Shuffle = [&](TArray<FMSLevelUpChoicePair>& Arr)
		{
			for (int32 i = Arr.Num() - 1; i > 0; --i)
			{
				const int32 Index = RandStream.RandRange(0, i);
				Arr.Swap(i, Index);
			}
		};

	Shuffle(AcquireCandidates);
	Shuffle(UpgradeCandidates);

	// 최종 3개 구성 규칙
	// - 슬롯 여유 있으면 Acquire 1개 우선(가능할 때)
	// - 나머지는 Upgrade로 채우되 부족하면 Acquire로 채움
	auto PickOne = [&](TArray<FMSLevelUpChoicePair>& Pool)
		{
			if (Pool.Num() <= 0) return false;
			CurrentSkillChoices.Add(Pool.Pop(EAllowShrinking::No)); // 뒤에서 하나
			return true;
		};

	// 1) Acquire 1개(가능하면)
	if (bHasFreeSlot && AcquireCandidates.Num() > 0 && CurrentSkillChoices.Num() < 3)
	{
		PickOne(AcquireCandidates);
	}

	// 2) Upgrade로 채우기
	while (CurrentSkillChoices.Num() < 3 && UpgradeCandidates.Num() > 0)
	{
		PickOne(UpgradeCandidates);
	}

	// 3) 남으면 Acquire로 채우기
	while (CurrentSkillChoices.Num() < 3 && AcquireCandidates.Num() > 0)
	{
		PickOne(AcquireCandidates);
	}

	if (CurrentSkillChoices.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BeginSkillLevelUp] No choices; auto-complete"));
		bSkillLevelUpCompleted = true;
		if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
		{
			GS->NotifySkillLevelUpCompleted(this);
		}
		return;
	}

	AMSPlayerController* PC = Cast<AMSPlayerController>(GetOwner());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BeginSkillLevelUp] PlayerController not found"));
		return;
	}

	float RemainingSeconds = 30.f;
	if (AMSGameState* GS = GetWorld()->GetGameState<AMSGameState>())
	{
		RemainingSeconds = GS->GetSkillLevelUpRemainingSeconds_Server();
	}

	PC->Client_ShowSkillLevelUpChoices(SessionId, CurrentSkillChoices, RemainingSeconds);

	UE_LOG(LogTemp, Log,
		TEXT("[BeginSkillLevelUp] Session=%d | Acquire=%d | Upgrade=%d | Picked=%d"),
		SessionId,
		AcquireCandidates.Num(),
		UpgradeCandidates.Num(),
		CurrentSkillChoices.Num()
	);
}

void AMSPlayerState::ApplySkillLevelUpChoice_Server(int32 SessionId, const FMSLevelUpChoicePair& Picked)
{
	if (!HasAuthority())
	{
		return;
	}

	// 세션 검증
	if (SessionId != CurrentLevelUpSessionId)
	{
		UE_LOG(LogTemp, Warning,
			TEXT("[LevelUp] Invalid Session. PS=%s Given=%d Current=%d"),
			*GetName(), SessionId, CurrentLevelUpSessionId);
		return;
	}

	// 중복 방지
	if (bSkillLevelUpCompleted)
	{
		return;
	}

	const FGameplayTag SkillTag = Picked.SkillTag;
	const FGameplayTag UpgradeTag = Picked.UpgradeTag;

	if (!SkillTag.IsValid())
		return;

	const bool bIsAcquire = !UpgradeTag.IsValid();

	if (bIsAcquire)
	{
		// 방어: 이미 가지고 있는 스킬이면 Acquire가 오면 안 되지만, 혹시 모르니 Upgrade로 처리
		const int32 OwnedIdx = FindOwnedSkillIndexByTag(OwnedSkills, SkillTag);
		if (OwnedIdx != INDEX_NONE)
		{
			// 이미 보유 → 그냥 레벨업 1회로 처리(혹은 return)
			FMSSkillList& Skill = OwnedSkills[OwnedIdx];
			if (Skill.SkillLevel < MaxSkillLevel)
			{
				Skill.SkillLevel += 1;
			}
			bSkillLevelUpCompleted = true;
			return;
		}

		// DataTable에서 해당 스킬 구조체 가져오기
		const FMSSkillList* Row = FindSkillRowByTag(SkillListDataTable, SkillTag);
		if (!Row)
		{
			UE_LOG(LogTemp, Warning, TEXT("[LevelUp][Acquire] Skill row not found. Tag=%s"), *SkillTag.ToString());
			return;
		}

		FMSSkillList NewSkill = *Row;
		NewSkill.SkillLevel = 1;

		OwnedSkills.Add(NewSkill);
		SkillNum++;

		// GA 부여 
		GiveAbilityForSkillRow_Server(NewSkill);

		if (APawn* Pawn = GetPawn())
		{
			if (AMSPlayerCharacter* PC = Cast<AMSPlayerCharacter>(Pawn))
			{
				PC->AcquireSkill(NewSkill.SkillID);
			}
			else
			{
				UE_LOG(LogTemp, Warning, TEXT("[LevelUp][Acquire] Pawn is not AMSPlayerCharacter. Pawn=%s"), *GetNameSafe(Pawn));
			}
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[LevelUp][Acquire] GetPawn() is null (not possessed yet)."));
		}

		bSkillLevelUpCompleted = true;

		UE_LOG(LogTemp, Log, TEXT("[LevelUp][Acquire] Skill=%s Level=1"), *SkillTag.ToString());
		return;
	}

	// ---------------------------------------
	// 2) Upgrade: 기존 스킬 업그레이드
	// ---------------------------------------
	const int32 OwnedIdx = FindOwnedSkillIndexByTag(OwnedSkills, SkillTag);
	if (OwnedIdx == INDEX_NONE)
	{
		// 방어: 업그레이드인데 미보유면 이상한 상태 → 무시하거나 Acquire로 처리
		UE_LOG(LogTemp, Warning, TEXT("[LevelUp][Upgrade] Not owned. Tag=%s"), *SkillTag.ToString());
		return;
	}

	FMSSkillList& Skill = OwnedSkills[OwnedIdx];

	// 레벨 +1
	if (Skill.SkillLevel < MaxSkillLevel)
	{
		Skill.SkillLevel += 1;
	}

	// 업그레이드 태그 효과 적용
	ApplyUpgradeTagToSkill(Skill, UpgradeTag);

	// 완료 처리
	bSkillLevelUpCompleted = true;

	UE_LOG(LogTemp, Log,
		TEXT("[LevelUp] Choice applied. PS=%s Session=%d"),
		*GetName(), SessionId
	);
	
	if (APawn* Pawn = GetPawn())
	{
		if (AMSPlayerCharacter* PC = Cast<AMSPlayerCharacter>(Pawn))
		{
			PC->AcquireSkill(Skill.SkillID);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[LevelUp][Acquire] Pawn is not AMSPlayerCharacter. Pawn=%s"), *GetNameSafe(Pawn));
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("[LevelUp][Acquire] GetPawn() is null (not possessed yet)."));
	}
}

void AMSPlayerState::ApplyRandomSkillLevelUpChoice_Server()
{
	if (!HasAuthority())
		return;

	if (bSkillLevelUpCompleted)
		return;

	if (CurrentSkillChoices.Num() == 0)
		return;

	const int32 Index = FMath::RandRange(0, CurrentSkillChoices.Num() - 1);
	const FMSLevelUpChoicePair& Picked = CurrentSkillChoices[Index];

	ApplySkillLevelUpChoice_Server(CurrentLevelUpSessionId, Picked);
}

void AMSPlayerState::SetAliveState_Server(bool bInAlive)
{
	if (!HasAuthority()) return;
	if (bIsAlive == bInAlive) return;

	// 살아있는 상태 초기화
	bIsAlive = bInAlive;

	// 리슨 서버(호스트)의 로컬 관전 컨트롤러는 서버에 존재하므로 OnRep가 호출되지 않음
	// 따라서 서버에서도 델리게이트를 브로드캐스트 함
	OnAliveStateChanged.Broadcast(bIsAlive);

	ForceNetUpdate();
}

void AMSPlayerState::OnRep_IsAlive()
{
	OnAliveStateChanged.Broadcast(bIsAlive);
}

void AMSPlayerState::SetSelectedCharacterID(FName InCharacterID)
{
	SelectedCharacterID = InCharacterID;
}
