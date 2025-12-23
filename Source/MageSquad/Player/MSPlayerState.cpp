// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/MSPlayerState.h"

#include "Player/MSPlayerController.h"
#include "AbilitySystem/ASC/MSPlayerAbilitySystemComponent.h"
#include "AbilitySystem/AttributeSets/MSPlayerAttributeSet.h"
#include "Net/UnrealNetwork.h"

AMSPlayerState::AMSPlayerState()
{
	// PlayerState의 기본 복제 지연이 낮으므로, 네트워크 업데이트 빈도를 늘림
	SetNetUpdateFrequency(60.f);

	AbilitySystemComponent = CreateDefaultSubobject<UMSPlayerAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);

	// Minimal 모드는 클라이언트에는 필요한 정보만 복제하여 네트워크 부하를 줄임
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	AttributeSet = CreateDefaultSubobject<UMSPlayerAttributeSet>(TEXT("AttributeSet"));

	bUIReady = false;
}
void AMSPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AMSPlayerState, bUIReady);
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

void AMSPlayerState::BeginSkillLevelUp(int32 SessionId)
{
	// 서버 전용
	if (!HasAuthority())
	{
		return;
	}

	// 세션 초기화
	CurrentLevelUpSessionId = SessionId;
	bSkillLevelUpCompleted = false;
	CurrentSkillChoices.Reset();

	if (!SkillListDataTable)
	{
		UE_LOG(LogTemp, Error, TEXT("[BeginSkillLevelUp] SkillListDataTable is NULL"));
		return;
	}

	// 스킬 전체 Row 가져오기
	static const FString Context(TEXT("BeginSkillLevelUp"));
	TArray<FMSSkillList*> AllRows;
	SkillListDataTable->GetAllRows(Context, AllRows);

	const bool bHasFreeSlot = (SkillNum < MaxSkillCount);

	// (SkillTag, UpgradeTag) 후보 생성
	TArray<FMSLevelUpChoicePair> PairCandidates;
	PairCandidates.Reserve(AllRows.Num() * 4);

	for (const FMSSkillList* Row : AllRows)
	{
		if (!Row)
		{
			continue;
		}

		const FGameplayTag SkillTag = Row->SkillEventTag;
		if (!SkillTag.IsValid())
		{
			continue;
		}

		// 이 스킬이 제공하는 업그레이드가 하나도 없으면 후보 제외
		if (Row->AvailableUpgradeTags.Num() <= 0)
		{
			continue;
		}

		// 보유 여부 및 현재 레벨 조회
		bool bOwned = false;
		int32 CurrentLevel = 0;

		for (const FMSSkillList& Owned : OwnedSkills)
		{
			if (Owned.SkillEventTag == SkillTag)
			{
				bOwned = true;
				CurrentLevel = Owned.SkillLevel;
				break;
			}
		}

		// ---- 스킬 후보 규칙 ----
		// 슬롯 여유 O  : 보유/미보유 상관없이 레벨 < Max
		// 슬롯 여유 X  : 보유 중인 스킬만, 레벨 < Max
		const bool bSkillAllowed =
			(bHasFreeSlot && (CurrentLevel < MaxSkillLevel)) ||
			(!bHasFreeSlot && bOwned && (CurrentLevel < MaxSkillLevel));

		if (!bSkillAllowed)
		{
			continue;
		}

		// 업그레이드 태그 펼치기
		TArray<FGameplayTag> UpgradeTags;
		Row->AvailableUpgradeTags.GetGameplayTagArray(UpgradeTags);

		for (const FGameplayTag& UpgradeTag : UpgradeTags)
		{
			if (!UpgradeTag.IsValid())
			{
				continue;
			}

			FMSLevelUpChoicePair Pair;
			Pair.SkillTag = SkillTag;
			Pair.UpgradeTag = UpgradeTag;
			PairCandidates.Add(Pair);
		}
	}

	if (PairCandidates.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BeginSkillLevelUp] No valid (Skill,Upgrade) candidates"));
		return;
	}

	// 4) 랜덤 셔플
	FRandomStream RandStream(CurrentLevelUpSessionId);

	for (int32 i = PairCandidates.Num() - 1; i > 0; --i)
	{
		const int32 Index = RandStream.RandRange(0, i);
		PairCandidates.Swap(i, Index);
	}

	// 5) 최대 3개 선택
	const int32 PickCount = FMath::Min(3, PairCandidates.Num());
	for (int32 i = 0; i < PickCount; ++i)
	{
		CurrentSkillChoices.Add(PairCandidates[i]);
	}

	// 6) PlayerController에 UI 표시 요청
	AMSPlayerController* PC = Cast<AMSPlayerController>(GetOwner());
	if (!PC)
	{
		UE_LOG(LogTemp, Warning, TEXT("[BeginSkillLevelUp] PlayerController not found"));
		return;
	}

	PC->Client_ShowSkillLevelUpChoices(SessionId, CurrentSkillChoices);

	UE_LOG(LogTemp, Log,
		TEXT("[BeginSkillLevelUp] Session=%d | PairCandidates=%d | Picked=%d"),
		SessionId,
		PairCandidates.Num(),
		CurrentSkillChoices.Num()
	);
}
