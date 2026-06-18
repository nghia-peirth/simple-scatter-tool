// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "QuickScatterConfig.generated.h"

/**
 * Configuration settings for the Quick Scatter Tool.
 */
UCLASS()
class QUICKSCATTERTOOL_API UQuickScatterConfig : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Scatter Settings")
	float MinScale = 0.8f;

	UPROPERTY(EditAnywhere, Category = "Scatter Settings")
	float MaxScale = 1.2f;

	UPROPERTY(EditAnywhere, Category = "Scatter Settings")
	bool bRandomRotationYaw = true;

	UPROPERTY(EditAnywhere, Category = "Scatter Settings")
	float ScatterRadius = 500.0f;

	UPROPERTY(EditAnywhere, Category = "Scatter Settings")
	int32 SpawnCount = 5;

	UPROPERTY(EditAnywhere, Category = "Scatter Settings")
	bool bUseSelectedAsOrigin = false;

	UPROPERTY(EditAnywhere, Category = "Scatter Settings")
	bool bEnablePhysics = false;
};
