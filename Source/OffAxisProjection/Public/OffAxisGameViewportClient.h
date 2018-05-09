// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/GameViewportClient.h"
#include "OffAxisGameViewportClient.generated.h"

/**
 * 
 */
UCLASS(BlueprintType)
class UOffAxisGameViewportClient : public UGameViewportClient
{
	GENERATED_BODY()

public:

	UFUNCTION(BlueprintCallable, Category = "OffAxis")
		static FMatrix GenerateOffAxisMatrix(float _screenWidth, float _screenHeight, const FVector& _eyeRelativePositon);

	UFUNCTION(BlueprintCallable, Category = "OffAxis")
		static void SetOffAxisMatrix(FMatrix OffAxisMatrix);

	UFUNCTION(BlueprintCallable, Category = "OffAxis")
		static void ToggleOffAxisMethod();

	UFUNCTION(BlueprintCallable, Category = "OffAxis")
		static void PrintCurrentOffAxisVersioN();
	
	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;

private:
	FName CurrentBufferVisualizationMode;
	FMatrix		mOffAxisMatrix;
	bool		mOffAxisMatrixSetted = false;
};

