#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GameplayTagContainer.h"
#include "Types/MageSquadTypes.h"
#include "MSLevelUpChoice.generated.h"

class UButton;
class UTextBlock;
class UImage;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnLevelUpChoiceClicked, const FMSLevelUpChoicePair&, Choice);

UCLASS()
class MAGESQUAD_API UMSLevelUpChoice : public UUserWidget
{
	GENERATED_BODY()

public:
	/** 패널에서 생성 후 데이터 주입 */
	UFUNCTION(BlueprintCallable)
	void InitChoice(int32 InSessionId, const FMSLevelUpChoicePair& InChoice);

	/** 패널이 바인딩해서 클릭을 받음 */
	UPROPERTY(BlueprintAssignable)
	FOnLevelUpChoiceClicked OnChoiceClicked;

	UFUNCTION(BlueprintPure)
	FMSLevelUpChoicePair GetChoice() const { return Choice; }
	
	UFUNCTION(BlueprintPure)
	int32 GetSessionId() const { return SessionId; }
	
protected:
	virtual void NativeOnInitialized() override;

	UFUNCTION()
	void HandleClicked();

	
protected:
	/** 전체 클릭 영역(필수) */
	UPROPERTY(meta=(BindWidget))
	UButton* Button_Choice = nullptr;

	/** 아래는 없어도 동작하게 Optional */
	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_SkillName = nullptr;

	UPROPERTY(meta=(BindWidgetOptional))
	UTextBlock* Text_UpgradeName = nullptr;


private:
	UPROPERTY()
	int32 SessionId = 0;

	UPROPERTY()
	FMSLevelUpChoicePair Choice;
};
