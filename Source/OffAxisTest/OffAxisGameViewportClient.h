// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameViewportClient.h"
#include "OffAxisGameViewportClient.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class OFFAXISTEST_API UOffAxisGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "OffAxis")
	static FMatrix GenerateOffAxisMatrix(float ScreenWidth, float ScreenHeight, FVector eyePosition);

	UFUNCTION(BlueprintCallable, Category = "OffAxis")
	static void SetOffAxisMatrix(FMatrix OffAxisMatrix);
	
	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;

private:

	FName CurrentBufferVisualizationMode;

	FMatrix		mOffAxisMatrix;
	bool		mOffAxisMatrixSetted = false;

};

