// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataAssetManager.h"
#if WITH_EDITOR 
#include "LevelEditor.h"
#endif
#include "../SlateWidgets/DataAssetManagerWidget.h"

#define LOCTEXT_NAMESPACE "FDataAssetManagerModule"

void FDataAssetManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
#if WITH_EDITOR
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuBarExtension(
		"Help",  // Help 메뉴 뒤에 추가
		EExtensionHook::After,
		nullptr,
		FMenuBarExtensionDelegate::CreateRaw(this, &FDataAssetManagerModule::CreateMenu)
	);
	
		FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("DataAssetManager"),
    		FOnSpawnTab::CreateRaw(this, &FDataAssetManagerModule::OnSpawnDataAssetManagerTab))
    		// .SetDisplayName(FText::FromString(TEXT("DataAssetManager")))
			.SetMenuType(ETabSpawnerMenuType::Hidden);

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
#endif // WITH_EDITOR
}

void FDataAssetManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FDataAssetManagerModule::CreateMenu(FMenuBarBuilder& MenuBarBuilder)
{
#if WITH_EDITOR
	MenuBarBuilder.AddPullDownMenu(
	FText::FromString("DataAssetManager"),
	FText::FromString("Open DataAsset Manager"),
	FNewMenuDelegate::CreateRaw(this, &FDataAssetManagerModule::OpenWindow),
	"Data"
	);
#endif // WITH_EDITOR
}

void FDataAssetManagerModule::OpenWindow(FMenuBuilder& MenuBarBuilder)
{	
#if WITH_EDITOR
	FGlobalTabmanager::Get()->TryInvokeTab(FName("DataAssetManager"));
#endif // WITH_EDITOR
}

TSharedRef<SDockTab> FDataAssetManagerModule::OnSpawnDataAssetManagerTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab).TabRole(NomadTab)
	[ 
		SNew(SDataAssetManagerTab).TestString(TEXT("I am passing data"))
	];
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDataAssetManagerModule, DataAssetManager)