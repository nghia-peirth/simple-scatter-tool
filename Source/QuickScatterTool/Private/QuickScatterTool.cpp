// Copyright Epic Games, Inc. All Rights Reserved.

#include "QuickScatterTool.h"
#include "SQuickScatterWindow.h"
#include "QuickScatterConfig.h"
#include "LevelEditor.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/Commands/Commands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "ToolMenus.h"

#define LOCTEXT_NAMESPACE "FQuickScatterToolModule"

const FName QuickScatterTabName("QuickScatterTool");

class FQuickScatterCommands : public TCommands<FQuickScatterCommands>
{
public:
	FQuickScatterCommands()
		: TCommands<FQuickScatterCommands>(
			TEXT("QuickScatterTool"),
			NSLOCTEXT("Contexts", "QuickScatterTool", "Quick Scatter Tool Plugin"),
			NAME_None,
			FAppStyle::GetAppStyleSetName()
		)
	{}

	virtual void RegisterCommands() override
	{
		UI_COMMAND(OpenPluginWindow, "Quick Scatter", "Opens the Quick Scatter Tool Window", EUserInterfaceActionType::Button, FInputChord());
	}

	TSharedPtr<FUICommandInfo> OpenPluginWindow;
};

void FQuickScatterToolModule::StartupModule()
{
	FQuickScatterCommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FQuickScatterCommands::Get().OpenPluginWindow,
		FExecuteAction::CreateRaw(this, &FQuickScatterToolModule::OnOpenScatterTool),
		FCanExecuteAction()
	);

	ConfigObject = NewObject<UQuickScatterConfig>();
	ConfigObject->AddToRoot();

	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
		QuickScatterTabName,
		FOnSpawnTab::CreateRaw(this, &FQuickScatterToolModule::OnSpawnPluginTab))
		.SetDisplayName(LOCTEXT("QuickScatterTabTitle", "Quick Scatter Tool"))
		.SetMenuType(ETabSpawnerMenuType::Hidden);

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FQuickScatterToolModule::RegisterMenus));
}

void FQuickScatterToolModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(QuickScatterTabName);
	FQuickScatterCommands::Unregister();

	if (ConfigObject)
	{
		if (!GExitPurge)
		{
			ConfigObject->RemoveFromRoot();
		}
		ConfigObject = nullptr;
	}
}

void FQuickScatterToolModule::RegisterMenus()
{
	FToolMenuOwnerScoped OwnerScoped(this);
	{
		UToolMenu* ToolBar = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		if (ToolBar)
		{
			FToolMenuSection& Section = ToolBar->AddSection("QuickScatterSection");
			Section.InsertPosition = FToolMenuInsert("Play", EToolMenuInsertType::After);

			FToolMenuEntry Entry = FToolMenuEntry::InitToolBarButton(
				FQuickScatterCommands::Get().OpenPluginWindow,
				LOCTEXT("QuickScatterBtn", "Quick Scatter"),
				LOCTEXT("QuickScatterTooltip", "Open the Quick Scatter Tool"),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.Layout")
			);
			Entry.SetCommandList(PluginCommands);

			Section.AddEntry(Entry);
		}
	}
}

void FQuickScatterToolModule::OnOpenScatterTool()
{
	FGlobalTabmanager::Get()->TryInvokeTab(QuickScatterTabName);
}

TSharedRef<SDockTab> FQuickScatterToolModule::OnSpawnPluginTab(const FSpawnTabArgs& SpawnTabArgs)
{
	return SNew(SDockTab)
		.TabRole(ETabRole::NomadTab)
		[
			SNew(SQuickScatterWindow)
			.InConfigObject(ConfigObject)
		];
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FQuickScatterToolModule, QuickScatterTool)
