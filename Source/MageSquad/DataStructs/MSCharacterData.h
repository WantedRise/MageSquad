#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "GameplayEffect.h"
#include "Types/MageSquadTypes.h"
#include "MSCharacterData.generated.h"

/*
 * 작성자 : 이상준
 * 작성일 : 2026/01/04
 * 캐릭터 데이터 정보
 */

USTRUCT(BlueprintType)
struct FMSCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FText CharacterInfo;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UMaterialInterface* OverrideMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UStaticMesh* StaffMesh;

    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    UTexture2D* Portrait;

    // 패시브 스킬
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FStartSkillData PassiveSkill;

    // 좌클릭
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FStartSkillData PrimarySkill;

    // 우클릭
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    FStartSkillData SecondarySkill;

    // 초기 스탯 GE
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TSubclassOf<UGameplayEffect> InitialStatEffect;

    // 초기 스탯 설명
    UPROPERTY(EditAnywhere, BlueprintReadOnly)
    TArray<FText> InitialStatInfo;
};
