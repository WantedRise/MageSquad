#include "Widgets/LevelUp/MSLevelUpChoice.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UMSLevelUpChoice::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	if (Button_Choice)
	{
		Button_Choice->OnClicked.AddDynamic(this, &UMSLevelUpChoice::HandleClicked);
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("[UMSLevelUpChoice] Button_Choice is NULL (BindWidget 실패?)"));
	}
}

void UMSLevelUpChoice::InitChoice(int32 InSessionId, const FMSLevelUpChoicePair& InChoice)
{
	SessionId = InSessionId;
	Choice = InChoice;

	// 기본 표시(최소 구현): 태그 문자열 그대로 보여주기
	if (Text_SkillName)
	{
		Text_SkillName->SetText(FText::FromString(Choice.SkillTag.ToString()));
	}
	if (Text_UpgradeName)
	{
		Text_UpgradeName->SetText(FText::FromString(Choice.UpgradeTag.ToString()));
	}
}

void UMSLevelUpChoice::HandleClicked()
{
	// 클릭 결과를 패널로 올려보냄
	OnChoiceClicked.Broadcast(Choice);
}
