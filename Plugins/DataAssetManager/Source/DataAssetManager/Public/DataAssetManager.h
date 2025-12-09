// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FDataAssetManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
	
private:
	void CreateMenu(FMenuBarBuilder& MenuBarBuilder);
	void OpenWindow(FMenuBuilder& MenuBarBuilder);
	TSharedRef<SDockTab> OnSpawnDataAssetManagerTab(const FSpawnTabArgs&);
	
private:
	TSharedPtr<FExtender> MenuExtender;
};
