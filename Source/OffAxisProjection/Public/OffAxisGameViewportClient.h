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

	virtual void Draw(FViewport* Viewport, FCanvas* SceneCanvas) override;



private:
	FName CurrentBufferVisualizationMode;

	
	/** Delegate called when the engine finishes drawing a game viewport */
	FSimpleMulticastDelegate EndDrawDelegate;

	/** Delegate called when the engine starts drawing a game viewport */
	FSimpleMulticastDelegate BeginDrawDelegate;

	/** Delegate called when the game viewport is drawn, before drawing the console */
	FSimpleMulticastDelegate DrawnDelegate;
};

