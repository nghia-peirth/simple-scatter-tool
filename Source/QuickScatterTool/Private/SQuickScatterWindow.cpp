// Copyright Epic Games, Inc. All Rights Reserved.

#include "SQuickScatterWindow.h"
#include "AssetThumbnail.h"
#include "SAssetDropTarget.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SSpinBox.h"
#include "Widgets/Input/SCheckBox.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "Editor.h"
#include "Selection.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HierarchicalInstancedStaticMeshComponent.h"
#include "Engine/World.h"

#define LOCTEXT_NAMESPACE "QuickScatterTool"

void SQuickScatterWindow::Construct(const FArguments& InArgs)
{
	Config = InArgs._InConfigObject;
	ThumbnailPool = MakeShareable(new FAssetThumbnailPool(64));

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("Docking.Tab.ContentAreaBrush"))
		.Padding(FMargin(10.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 0.f, 0.f, 10.f))
			[
				CreateDropTargetArea()
			]

			+ SVerticalBox::Slot()
			.FillHeight(1.0f)
			.Padding(FMargin(0.f, 0.f, 0.f, 10.f))
			[
				SNew(SVerticalBox)
				+ SVerticalBox::Slot()
				.AutoHeight()
				.Padding(FMargin(0.f, 0.f, 0.f, 4.f))
				[
					SNew(STextBlock)
					.Text(LOCTEXT("AssetListTitle", "Static Meshes to Scatter:"))
					.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
				]
				+ SVerticalBox::Slot()
				.FillHeight(1.0f)
				[
					CreateListViewArea()
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 0.f, 0.f, 15.f))
			[
				CreateSettingsArea()
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				CreateActionArea()
			]
		]
	];
}

SQuickScatterWindow::~SQuickScatterWindow()
{
	if (ThumbnailPool.IsValid())
	{
		ThumbnailPool.Reset();
	}
}

TSharedRef<SWidget> SQuickScatterWindow::CreateDropTargetArea()
{
	return SNew(SAssetDropTarget)
		.OnAreAssetsAcceptableForDrop(this, &SQuickScatterWindow::OnAreAssetsValidForDrop)
		.OnAssetsDropped(this, &SQuickScatterWindow::OnAssetsDropped)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("Menu.Background"))
			.BorderBackgroundColor(FLinearColor(0.2f, 0.2f, 0.2f, 1.0f))
			.Padding(FMargin(15.0f))
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("DropTargetText", "Drag & Drop Static Meshes Here"))
				.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
				.ColorAndOpacity(FLinearColor(0.7f, 0.7f, 0.7f, 1.0f))
			]
		];
}

TSharedRef<SWidget> SQuickScatterWindow::CreateListViewArea()
{
	SAssignNew(AssetListView, SListView<TSharedPtr<FAssetData>>)
		.ListItemsSource(&SelectedAssets)
		.OnGenerateRow(this, &SQuickScatterWindow::OnGenerateAssetRow)
		.SelectionMode(ESelectionMode::None);

	return SNew(SBox)
		.HeightOverride(180.0f)
		[
			SNew(SBorder)
			.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
			.Padding(FMargin(2.0f))
			[
				AssetListView.ToSharedRef()
			]
		];
}

TSharedRef<SWidget> SQuickScatterWindow::CreateSettingsArea()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::Get().GetBrush("ToolPanel.GroupBorder"))
		.Padding(FMargin(8.0f))
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 4.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("ScaleLabel", "Random Scale (Min / Max):"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.35f)
				.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
				[
					SNew(SSpinBox<float>)
					.Value(this, &SQuickScatterWindow::GetMinScale)
					.OnValueChanged(this, &SQuickScatterWindow::OnMinScaleChanged)
					.MinValue(0.1f)
					.MaxValue(10.0f)
					.SliderExponent(1.0f)
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.35f)
				[
					SNew(SSpinBox<float>)
					.Value(this, &SQuickScatterWindow::GetMaxScale)
					.OnValueChanged(this, &SQuickScatterWindow::OnMaxScaleChanged)
					.MinValue(0.1f)
					.MaxValue(10.0f)
					.SliderExponent(1.0f)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 4.f))
			[
				SNew(SCheckBox)
				.IsChecked(this, &SQuickScatterWindow::GetRandomRotationState)
				.OnCheckStateChanged(this, &SQuickScatterWindow::OnRandomRotationStateChanged)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RotLabel", "Random Rotation (Yaw)"))
					.Font(FAppStyle::Get().GetFontStyle("NormalFont"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 4.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("RadiusLabel", "Scatter Radius:"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.7f)
				[
					SNew(SSpinBox<float>)
					.Value(this, &SQuickScatterWindow::GetScatterRadius)
					.OnValueChanged(this, &SQuickScatterWindow::OnScatterRadiusChanged)
					.MinValue(50.0f)
					.MaxValue(10000.0f)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 4.f))
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.FillWidth(0.3f)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("CountLabel", "Spawn Count:"))
				]
				+ SHorizontalBox::Slot()
				.FillWidth(0.7f)
				[
					SNew(SSpinBox<int32>)
					.Value(this, &SQuickScatterWindow::GetSpawnCount)
					.OnValueChanged(this, &SQuickScatterWindow::OnSpawnCountChanged)
					.MinValue(1)
					.MaxValue(200)
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 4.f))
			[
				SNew(SCheckBox)
				.IsChecked(this, &SQuickScatterWindow::GetUseSelectedAsOriginState)
				.OnCheckStateChanged(this, &SQuickScatterWindow::OnUseSelectedAsOriginStateChanged)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("TargetAsOriginLabel", "Use Selected Actor as Origin"))
					.Font(FAppStyle::Get().GetFontStyle("NormalFont"))
				]
			]

			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(0.f, 4.f))
			[
				SNew(SCheckBox)
				.IsChecked(this, &SQuickScatterWindow::GetEnablePhysicsState)
				.OnCheckStateChanged(this, &SQuickScatterWindow::OnEnablePhysicsStateChanged)
				[
					SNew(STextBlock)
					.Text(LOCTEXT("PhysicsLabel", "Enable Physics (Simulate)"))
					.Font(FAppStyle::Get().GetFontStyle("NormalFont"))
				]
			]
		];
}

TSharedRef<SWidget> SQuickScatterWindow::CreateActionArea()
{
	return SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		.FillWidth(0.5f)
		.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ContentPadding(FMargin(0, 10.0f))
			.ButtonStyle(FAppStyle::Get(), "PrimaryButton")
			.OnClicked(this, &SQuickScatterWindow::OnExecuteScatter)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ScatterBtnText", "SCATTER"))
				.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
				.ColorAndOpacity(FLinearColor::White)
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		.Padding(FMargin(0.f, 0.f, 4.f, 0.f))
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ContentPadding(FMargin(0, 10.0f))
			.ButtonStyle(FAppStyle::Get(), "Button")
			.OnClicked(this, &SQuickScatterWindow::OnClearLastScatter)
			.IsEnabled(this, &SQuickScatterWindow::IsClearButtonEnabled)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("ClearBtnText", "CLEAR LAST"))
				.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
				.ColorAndOpacity(FLinearColor::White)
			]
		]
		+ SHorizontalBox::Slot()
		.FillWidth(0.25f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ContentPadding(FMargin(0, 10.0f))
			.ButtonStyle(FAppStyle::Get(), "Button")
			.OnClicked(this, &SQuickScatterWindow::OnBakeToISM)
			.IsEnabled(this, &SQuickScatterWindow::IsBakeButtonEnabled)
			[
				SNew(STextBlock)
				.Text(LOCTEXT("BakeBtnText", "BAKE TO ISM"))
				.Font(FAppStyle::Get().GetFontStyle("NormalFontBold"))
				.ColorAndOpacity(FLinearColor::White)
			]
		];
}

TSharedRef<ITableRow> SQuickScatterWindow::OnGenerateAssetRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerTable)
{
	TSharedPtr<FAssetThumbnail> AssetThumbnail = MakeShareable(new FAssetThumbnail(*Item, 48, 48, ThumbnailPool));

	return SNew(STableRow<TSharedPtr<FAssetData>>, OwnerTable)
		.Padding(FMargin(2.0f, 4.0f))
		[
			SNew(SHorizontalBox)

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.Padding(FMargin(5.f, 0.f, 10.f, 0.f))
			[
				SNew(SBox)
				.WidthOverride(48)
				.HeightOverride(48)
				[
					AssetThumbnail->MakeThumbnailWidget()
				]
			]

			+ SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Text(FText::FromName(Item->AssetName))
				.Font(FAppStyle::Get().GetFontStyle("NormalFont"))
			]

			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(FMargin(0.f, 0.f, 5.f, 0.f))
			[
				SNew(SButton)
				.Text(FText::FromString("X"))
				.ButtonStyle(FAppStyle::Get(), "SimpleButton")
				.OnClicked_Lambda([this, Item]() -> FReply
				{
					OnRemoveAsset(Item);
					return FReply::Handled();
				})
			]
		];
}

bool SQuickScatterWindow::OnAreAssetsValidForDrop(TArrayView<FAssetData> DraggedAssets) const
{
	for (const FAssetData& AssetData : DraggedAssets)
	{
		if (AssetData.GetClass() == UStaticMesh::StaticClass())
		{
			continue;
		}
		return false;
	}
	return true;
}

void SQuickScatterWindow::OnAssetsDropped(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> DraggedAssets)
{
	for (const FAssetData& AssetData : DraggedAssets)
	{
		if (AssetData.GetClass() == UStaticMesh::StaticClass())
		{
			bool bAlreadyExists = SelectedAssets.ContainsByPredicate([&AssetData](const TSharedPtr<FAssetData>& Existing)
			{
				return Existing->GetSoftObjectPath() == AssetData.GetSoftObjectPath();
			});

			if (!bAlreadyExists)
			{
				SelectedAssets.Add(MakeShareable(new FAssetData(AssetData)));
			}
		}
	}

	if (AssetListView.IsValid())
	{
		AssetListView->RequestListRefresh();
	}
}

void SQuickScatterWindow::OnRemoveAsset(TSharedPtr<FAssetData> AssetToRemove)
{
	SelectedAssets.Remove(AssetToRemove);
	if (AssetListView.IsValid())
	{
		AssetListView->RequestListRefresh();
	}
}

FReply SQuickScatterWindow::OnExecuteScatter()
{
	if (!Config.IsValid())
	{
		return FReply::Handled();
	}

	if (SelectedAssets.Num() == 0)
	{
		FNotificationInfo Info(LOCTEXT("NoAssetsNotify", "Please add at least one Static Mesh first!"));
		Info.ExpireDuration = 3.0f;
		FSlateNotificationManager::Get().AddNotification(Info);
		return FReply::Handled();
	}

	FVector ScatterOrigin = FVector::ZeroVector;
	bool bOriginFound = false;

	if (Config->bUseSelectedAsOrigin && GEditor)
	{
		USelection* SelectedActors = GEditor->GetSelectedActors();
		if (SelectedActors && SelectedActors->Num() > 0)
		{
			AActor* TargetActor = Cast<AActor>(SelectedActors->GetSelectedObject(0));
			if (TargetActor)
			{
				ScatterOrigin = TargetActor->GetActorLocation();
				bOriginFound = true;
			}
		}
	}

	if (!bOriginFound && GCurrentLevelEditingViewportClient)
	{
		FVector ViewLocation = GCurrentLevelEditingViewportClient->GetViewLocation();
		FRotator ViewRotation = GCurrentLevelEditingViewportClient->GetViewRotation();
		FVector ViewDir = ViewRotation.Vector();

		UWorld* EditorWorld = GCurrentLevelEditingViewportClient->GetWorld();
		if (EditorWorld)
		{
			FHitResult HitResult;
			FVector TraceEnd = ViewLocation + (ViewDir * 5000.0f);
			FCollisionQueryParams TraceParams(SCENE_QUERY_STAT(ScatterTrace), true);

			if (EditorWorld->LineTraceSingleByChannel(HitResult, ViewLocation, TraceEnd, ECC_WorldStatic, TraceParams))
			{
				ScatterOrigin = HitResult.Location;
			}
			else
			{
				ScatterOrigin = ViewLocation + (ViewDir * 1000.0f);
			}
		}
	}

	UWorld* World = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;
	if (!World)
	{
		return FReply::Handled();
	}

	LastSpawnedActors.Empty();

	GEditor->BeginTransaction(LOCTEXT("ScatterTransaction", "Execute Quick Scatter"));

	int32 SuccessfullySpawned = 0;

	for (int32 i = 0; i < Config->SpawnCount; ++i)
	{
		int32 AssetIndex = FMath::RandRange(0, SelectedAssets.Num() - 1);
		TSharedPtr<FAssetData> SelectedAsset = SelectedAssets[AssetIndex];
		UStaticMesh* Mesh = Cast<UStaticMesh>(SelectedAsset->GetAsset());

		if (!Mesh)
		{
			continue;
		}

		float Angle = FMath::FRandRange(0.f, 2.f * PI);
		float Distance = FMath::FRandRange(0.f, Config->ScatterRadius);
		FVector RandomOffset(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.0f);
		FVector TargetLoc = ScatterOrigin + RandomOffset;

		FVector TraceStart = TargetLoc + FVector(0.f, 0.f, 2000.f);
		FVector TraceEnd = TargetLoc - FVector(0.f, 0.f, 2000.f);
		FHitResult SpawnHit;
		FCollisionQueryParams SpawnTraceParams(SCENE_QUERY_STAT(ScatterDropTrace), true);

		FVector FinalLocation = TargetLoc;
		if (World->LineTraceSingleByChannel(SpawnHit, TraceStart, TraceEnd, ECC_WorldStatic, SpawnTraceParams))
		{
			FinalLocation = SpawnHit.Location;
		}

		AActor* NewActor = GEditor->AddActor(World->GetCurrentLevel(), AStaticMeshActor::StaticClass(), FTransform::Identity);
		AStaticMeshActor* SMA = Cast<AStaticMeshActor>(NewActor);
		if (SMA)
		{
			SMA->GetStaticMeshComponent()->SetStaticMesh(Mesh);
			SMA->SetActorLabel(FString::Printf(TEXT("QST_%s"), *Mesh->GetName()));

			FRotator FinalRotation = FRotator::ZeroRotator;
			if (Config->bRandomRotationYaw)
			{
				FinalRotation.Yaw = FMath::FRandRange(0.f, 360.f);
			}

			float ScaleMultiplier = FMath::FRandRange(Config->MinScale, Config->MaxScale);
			FVector FinalScale(ScaleMultiplier);

			SMA->SetActorLocationAndRotation(FinalLocation, FinalRotation);
			SMA->SetActorScale3D(FinalScale);

			if (Config->bEnablePhysics)
			{
				SMA->GetStaticMeshComponent()->SetMobility(EComponentMobility::Movable);
				SMA->GetStaticMeshComponent()->SetSimulatePhysics(true);
				SMA->GetStaticMeshComponent()->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
			}
			else
			{
				SMA->GetStaticMeshComponent()->SetMobility(EComponentMobility::Static);
			}

			SMA->MarkPackageDirty();

			LastSpawnedActors.Add(SMA);
			SuccessfullySpawned++;
		}
	}

	GEditor->EndTransaction();

	FNotificationInfo Info(FText::Format(LOCTEXT("SuccessNotify", "Successfully scattered {0} objects!"), FText::AsNumber(SuccessfullySpawned)));
	Info.ExpireDuration = 4.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	return FReply::Handled();
}

FReply SQuickScatterWindow::OnClearLastScatter()
{
	UWorld* World = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;
	if (!World || LastSpawnedActors.Num() == 0)
	{
		return FReply::Handled();
	}

	GEditor->BeginTransaction(LOCTEXT("ClearScatterTransaction", "Clear Last Scatter"));

	int32 ClearedCount = 0;
	for (TWeakObjectPtr<AActor> ActorPtr : LastSpawnedActors)
	{
		if (ActorPtr.IsValid())
		{
			World->DestroyActor(ActorPtr.Get());
			ClearedCount++;
		}
	}

	GEditor->EndTransaction();

	LastSpawnedActors.Empty();

	FNotificationInfo Info(FText::Format(LOCTEXT("ClearNotify", "Successfully cleared {0} scattered objects!"), FText::AsNumber(ClearedCount)));
	Info.ExpireDuration = 3.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	return FReply::Handled();
}

bool SQuickScatterWindow::IsClearButtonEnabled() const
{
	for (const TWeakObjectPtr<AActor>& ActorPtr : LastSpawnedActors)
	{
		if (ActorPtr.IsValid())
		{
			return true;
		}
	}
	return false;
}

FReply SQuickScatterWindow::OnBakeToISM()
{
	UWorld* World = GCurrentLevelEditingViewportClient ? GCurrentLevelEditingViewportClient->GetWorld() : nullptr;
	if (!World || LastSpawnedActors.Num() == 0)
	{
		return FReply::Handled();
	}

	TArray<AActor*> ValidActors;
	for (TWeakObjectPtr<AActor> ActorPtr : LastSpawnedActors)
	{
		if (ActorPtr.IsValid())
		{
			ValidActors.Add(ActorPtr.Get());
		}
	}

	if (ValidActors.Num() == 0)
	{
		return FReply::Handled();
	}

	GEditor->BeginTransaction(LOCTEXT("BakeToISMTransaction", "Bake Scattered to ISM"));

	AActor* ISMContainerActor = GEditor->AddActor(World->GetCurrentLevel(), AActor::StaticClass(), FTransform::Identity);
	if (ISMContainerActor)
	{
		ISMContainerActor->SetActorLabel(TEXT("QuickScatter_BakedISM"));

		TMap<UStaticMesh*, TArray<FTransform>> MeshTransformsMap;
		for (AActor* Actor : ValidActors)
		{
			AStaticMeshActor* SMA = Cast<AStaticMeshActor>(Actor);
			if (SMA && SMA->GetStaticMeshComponent())
			{
				UStaticMesh* Mesh = SMA->GetStaticMeshComponent()->GetStaticMesh();
				if (Mesh)
				{
					MeshTransformsMap.FindOrAdd(Mesh).Add(SMA->GetActorTransform());
				}
			}
		}

		for (auto& Elem : MeshTransformsMap)
		{
			UStaticMesh* Mesh = Elem.Key;
			TArray<FTransform>& Transforms = Elem.Value;

			FName CompName = MakeUniqueObjectName(ISMContainerActor, UHierarchicalInstancedStaticMeshComponent::StaticClass(), FName(*Mesh->GetName()));
			UHierarchicalInstancedStaticMeshComponent* HISMComp = NewObject<UHierarchicalInstancedStaticMeshComponent>(ISMContainerActor, UHierarchicalInstancedStaticMeshComponent::StaticClass(), CompName);

			if (HISMComp)
			{
				HISMComp->SetupAttachment(ISMContainerActor->GetRootComponent());
				HISMComp->RegisterComponent();
				ISMContainerActor->AddInstanceComponent(HISMComp);

				HISMComp->SetStaticMesh(Mesh);
				HISMComp->SetMobility(EComponentMobility::Static);

				for (const FTransform& Trans : Transforms)
				{
					HISMComp->AddInstance(Trans, true);
				}
			}
		}

		for (AActor* Actor : ValidActors)
		{
			World->DestroyActor(Actor);
		}

		LastSpawnedActors.Empty();
		ISMContainerActor->MarkPackageDirty();
	}

	GEditor->EndTransaction();

	FNotificationInfo Info(LOCTEXT("BakeSuccessNotify", "Successfully baked objects to Hierarchical Instanced Static Mesh!"));
	Info.ExpireDuration = 4.0f;
	FSlateNotificationManager::Get().AddNotification(Info);

	return FReply::Handled();
}

bool SQuickScatterWindow::IsBakeButtonEnabled() const
{
	return IsClearButtonEnabled();
}

float SQuickScatterWindow::GetMinScale() const
{
	return Config.IsValid() ? Config->MinScale : 0.8f;
}

void SQuickScatterWindow::OnMinScaleChanged(float NewValue)
{
	if (Config.IsValid())
	{
		Config->MinScale = NewValue;
	}
}

float SQuickScatterWindow::GetMaxScale() const
{
	return Config.IsValid() ? Config->MaxScale : 1.2f;
}

void SQuickScatterWindow::OnMaxScaleChanged(float NewValue)
{
	if (Config.IsValid())
	{
		Config->MaxScale = NewValue;
	}
}

ECheckBoxState SQuickScatterWindow::GetRandomRotationState() const
{
	if (Config.IsValid())
	{
		return Config->bRandomRotationYaw ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Unchecked;
}

void SQuickScatterWindow::OnRandomRotationStateChanged(ECheckBoxState NewState)
{
	if (Config.IsValid())
	{
		Config->bRandomRotationYaw = (NewState == ECheckBoxState::Checked);
	}
}

float SQuickScatterWindow::GetScatterRadius() const
{
	return Config.IsValid() ? Config->ScatterRadius : 500.0f;
}

void SQuickScatterWindow::OnScatterRadiusChanged(float NewValue)
{
	if (Config.IsValid())
	{
		Config->ScatterRadius = NewValue;
	}
}

int32 SQuickScatterWindow::GetSpawnCount() const
{
	return Config.IsValid() ? Config->SpawnCount : 5;
}

void SQuickScatterWindow::OnSpawnCountChanged(int32 NewValue)
{
	if (Config.IsValid())
	{
		Config->SpawnCount = NewValue;
	}
}

ECheckBoxState SQuickScatterWindow::GetUseSelectedAsOriginState() const
{
	if (Config.IsValid())
	{
		return Config->bUseSelectedAsOrigin ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Unchecked;
}

void SQuickScatterWindow::OnUseSelectedAsOriginStateChanged(ECheckBoxState NewState)
{
	if (Config.IsValid())
	{
		Config->bUseSelectedAsOrigin = (NewState == ECheckBoxState::Checked);
	}
}

ECheckBoxState SQuickScatterWindow::GetEnablePhysicsState() const
{
	if (Config.IsValid())
	{
		return Config->bEnablePhysics ? ECheckBoxState::Checked : ECheckBoxState::Unchecked;
	}
	return ECheckBoxState::Unchecked;
}

void SQuickScatterWindow::OnEnablePhysicsStateChanged(ECheckBoxState NewState)
{
	if (Config.IsValid())
	{
		Config->bEnablePhysics = (NewState == ECheckBoxState::Checked);
	}
}

#undef LOCTEXT_NAMESPACE
