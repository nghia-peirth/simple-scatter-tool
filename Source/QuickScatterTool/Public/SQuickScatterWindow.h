// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "AssetRegistry/AssetData.h"
#include "QuickScatterConfig.h"

class FAssetThumbnailPool;
template <typename ItemType> class SListView;
class ITableRow;
class STableViewBase;

/**
 * Main Slate UI widget for the Quick Scatter Tool.
 */
class QUICKSCATTERTOOL_API SQuickScatterWindow : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SQuickScatterWindow) {}
		SLATE_ARGUMENT(TWeakObjectPtr<UQuickScatterConfig>, InConfigObject)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs);

	virtual ~SQuickScatterWindow() override;

private:
	TSharedRef<SWidget> CreateDropTargetArea();
	TSharedRef<SWidget> CreateListViewArea();
	TSharedRef<SWidget> CreateSettingsArea();
	TSharedRef<SWidget> CreateActionArea();

	TSharedRef<ITableRow> OnGenerateAssetRow(TSharedPtr<FAssetData> Item, const TSharedRef<STableViewBase>& OwnerTable);

	bool OnAreAssetsValidForDrop(TArrayView<FAssetData> DraggedAssets) const;
	void OnAssetsDropped(const FDragDropEvent& DragDropEvent, TArrayView<FAssetData> DraggedAssets);

	FReply OnExecuteScatter();
	FReply OnClearLastScatter();
	bool IsClearButtonEnabled() const;
	FReply OnBakeToISM();
	bool IsBakeButtonEnabled() const;
	void OnRemoveAsset(TSharedPtr<FAssetData> AssetToRemove);

	float GetMinScale() const;
	void OnMinScaleChanged(float NewValue);

	float GetMaxScale() const;
	void OnMaxScaleChanged(float NewValue);

	ECheckBoxState GetRandomRotationState() const;
	void OnRandomRotationStateChanged(ECheckBoxState NewState);

	float GetScatterRadius() const;
	void OnScatterRadiusChanged(float NewValue);

	int32 GetSpawnCount() const;
	void OnSpawnCountChanged(int32 NewValue);

	ECheckBoxState GetUseSelectedAsOriginState() const;
	void OnUseSelectedAsOriginStateChanged(ECheckBoxState NewState);

	ECheckBoxState GetEnablePhysicsState() const;
	void OnEnablePhysicsStateChanged(ECheckBoxState NewState);

private:
	TWeakObjectPtr<UQuickScatterConfig> Config;
	TArray<TSharedPtr<FAssetData>> SelectedAssets;
	TSharedPtr<SListView<TSharedPtr<FAssetData>>> AssetListView;
	TSharedPtr<FAssetThumbnailPool> ThumbnailPool;
	TArray<TWeakObjectPtr<AActor>> LastSpawnedActors;
};
