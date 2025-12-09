// Copyright Epic Games, Inc. All Rights Reserved.

#include "DataAssetManager.h"
#include "LevelEditor.h"

#define LOCTEXT_NAMESPACE "FDataAssetManagerModule"

void FDataAssetManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");

	MenuExtender = MakeShareable(new FExtender);
	MenuExtender->AddMenuBarExtension(
		"Help",  // Help 메뉴 뒤에 추가
		EExtensionHook::After,
		nullptr,
		FMenuBarExtensionDelegate::CreateRaw(this, &FDataAssetManagerModule::CreateMenu)
	);

	LevelEditorModule.GetMenuExtensibilityManager()->AddExtender(MenuExtender);
}

void FDataAssetManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

void FDataAssetManagerModule::CreateMenu(FMenuBarBuilder& MenuBarBuilder)
{
	MenuBarBuilder.AddPullDownMenu(
	FText::FromString("DataAssetManager"),
	FText::FromString("Open DataAsset Manager"),
	FNewMenuDelegate::CreateRaw(this, &FDataAssetManagerModule::OpenWindow),
	"Data"
	);
	
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("DataAssetManager"),
		FOnSpawnTab::CreateRaw(this, &FDataAssetManagerModule::OnSpawnDataAssetManagerTab)).SetDisplayName(FText::FromString(TEXT("DataAsset Manager")));
	
}

void FDataAssetManagerModule::OpenWindow(FMenuBuilder& MenuBarBuilder)
{
	// FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("DataAssetManager"),
	// 	FOnSpawnTab::CreateRaw(this, &FDataAssetManagerModule::OnSpawnDataAssetManagerTab)).SetDisplayName(FText::FromString(TEXT("DataAsset Manager")));
	
	FGlobalTabmanager::Get()->TryInvokeTab(FName("DataAssetManager"));
}

TSharedRef<SDockTab> FDataAssetManagerModule::OnSpawnDataAssetManagerTab(const FSpawnTabArgs&)
{
	return SNew(SDockTab).TabRole(NomadTab);
}


#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FDataAssetManagerModule, DataAssetManager)