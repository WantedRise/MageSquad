// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "AbilitySystemInterface.h"
#include "SkillData/MSSkillList.h"
#include "Types/MageSquadTypes.h"
#include "MSPlayerState.generated.h"

// 생존 상태 변경 델리게이트
DECLARE_MULTICAST_DELEGATE_OneParam(FOnAliveStateChangedNative, bool /*bIsAlive*/);

/**
 * 작성자: 김준형
 * 작성일: 25/12/08
 *
 * 플레이어의 ASC, AttributeSet을 관리하는 PlayerState
 * 폰의 사망 및 리스폰 후에도 유지되며 모든 클라이언트에 복제된다.
 *
 * 수정자: 박세찬
 * 수정일: 25/12/22
 *
 * 플레이어 스킬 레벨업 로직 추가
 */
UCLASS()
class MAGESQUAD_API AMSPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	AMSPlayerState();
	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	// 모든 플레이어의 UI 체크
	void SetUIReady(bool bReady);

	bool IsUIReady() { return bUIReady; }

	// PlayerState가 소유한 ASC를 반환하는 함수
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	// PlayerState가 소유한 AttributeSet을 반환하는 함수
	class UMSPlayerAttributeSet* GetAttributeSet() const;

protected:
	// 플레이어의 ASC
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Custom | GAS", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<class UMSPlayerAbilitySystemComponent> AbilitySystemComponent;

	// 플레이어의 능력치(스탯)
	UPROPERTY(Transient)
	TObjectPtr< class UMSPlayerAttributeSet> AttributeSet;

	// 현재 보유한 자동 발동 스킬 개수 (최대 4)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	int32 SkillNum = 0;

	// 플레이어가 보유 중인 스킬 목록
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Skill")
	TArray<FMSSkillList> OwnedSkills;

	// 스킬 업그레이드 적용
	void ApplyUpgradeTagToSkill(FMSSkillList& Skill, const FGameplayTag& UpgradeTag);

	// 스킬 GA 부여
	void GiveAbilityForSkillRow_Server(const FMSSkillList& Skill);

public:
	// 태그로 가지고 있는 스킬 검색
	static int32 FindOwnedSkillIndexByTag(const TArray<FMSSkillList>& OwnedSkills, const FGameplayTag& SkillTag);

	// SkillID로 가지고 있는 스킬 검색
	const FMSSkillList* GetOwnedSkillByID(int32 SkillID) const;

	// DataTable에서 스킬 Row 찾기 (SkillEventTag로 매칭) ---
	static const FMSSkillList* FindSkillRowByTag(UDataTable* SkillListDataTable, const FGameplayTag& SkillTag);

	// DataTable에서 스킬 Row 찾고 가지고 있는 스킬 리스트에 추가
	void FindSkillRowBySkillIDAndAdd(const int32 SkillID);
private:
	//게임 시작 체크를 위해
	UPROPERTY(Replicated)
	uint8 bUIReady : 1;

public:
	// 스킬 레벨업 시작
	void BeginSkillLevelUp(int32 SessionId);

	// 스킬 레벨업 적용
	void ApplySkillLevelUpChoice_Server(int32 SessionId, const FMSLevelUpChoicePair& Picked);

	// 랜덤 스킬 레벨업 적용
	void ApplyRandomSkillLevelUpChoice_Server();

public:
	const TArray<FMSLevelUpChoicePair>& GetCurrentSkillChoices() const { return CurrentSkillChoices; }
	bool IsSkillLevelUpCompleted() const { return bSkillLevelUpCompleted; }
	int32 GetCurrentLevelUpSessionId() const { return CurrentLevelUpSessionId; }

protected:
	// 현재 진행 중인 레벨업 세션 ID
	int32 CurrentLevelUpSessionId = INDEX_NONE;

	// 이번 레벨업 세션에서 선택 여부
	bool bSkillLevelUpCompleted = false;

	// 이번 레벨업 세션의 후보(서버 확정 선택지)
	TArray<FMSLevelUpChoicePair> CurrentSkillChoices;

	// 스킬 데이터 테이블
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	UDataTable* SkillListDataTable;

	// 플레이어가 가질 수 있는 최대 자동 발동 스킬 개수
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	int32 MaxSkillCount = 4;

	// 스킬 최대 레벨
	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	int32 MaxSkillLevel = 10;



	/*****************************************************
	* Death & Respawn Section
	*****************************************************/
public:
	// 서버: 살아있음 상태 변경 콜백 함수
	void SetAliveState_Server(bool bInAlive);

	// 현재 생존 여부 Getter
	bool IsAlive() const { return bIsAlive; }

	// 캐릭터 ID 선택 저장
	void SetSelectedCharacterID(FName InCharacterID);

	// 캐릭터 ID
	FName GetSelectedCharacterID() const { return SelectedCharacterID; }

	//UFUNCTION()
	//void OnRep_SelectedCharacterID();
private:
	// 생존 상태 변경 OnRep 함수
	UFUNCTION()
	void OnRep_IsAlive();


public:
	// 생존 상태 변경 이벤트 델리게이트
	FOnAliveStateChangedNative OnAliveStateChanged;

private:
	// 살아있는 상태 (관전 대상 자동 전환/필터링 용)
	UPROPERTY(ReplicatedUsing = OnRep_IsAlive)
	bool bIsAlive = true;

	//
	UPROPERTY(Replicated)
	FName SelectedCharacterID = NAME_None;
};
