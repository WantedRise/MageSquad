#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "MSLevelUpChoice.generated.h"

class UDataTable;

UCLASS()
class MAGESQUAD_API UMSLevelUpChoice : public UUserWidget
{
	GENERATED_BODY()

public:
	

protected:
	// (현재 코드에선 사용 안 함) 스킬 레벨별 데이터테이블(레벨별 수치 등)
	UPROPERTY(EditDefaultsOnly, Category = "SkillData")
	TObjectPtr<UDataTable> SkillDataTable;
};
