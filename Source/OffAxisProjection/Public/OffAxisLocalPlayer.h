// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/LocalPlayer.h"
#include "OffAxisLocalPlayer.generated.h"

/**
 * 
 */
UCLASS()
class OFFAXISPROJECTION_API UOffAxisLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()


		FSceneView* CalcSceneView(class FSceneViewFamily* ViewFamily,
			FVector& OutViewLocation,
			FRotator& OutViewRotation,
			FViewport* Viewport,
			class FViewElementDrawer* ViewDrawer = NULL,
			EStereoscopicPass StereoPass = eSSP_FULL) override;

	void SetOffAxisMatrix(FMatrix OffAxisMatrix);

	void UpdateProjectionMatrix(FSceneView * View, FMatrix OffAxisMatrix, EStereoscopicPass _Pass);
	
	FMatrix GenerateOffAxisMatrix(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon);

	FMatrix GenerateOffAxisMatrix(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon, EStereoscopicPass _PassType);

	FMatrix GenerateOffAxisMatrix_Internal(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon);

	FMatrix FrustumMatrix(float left, float right, float bottom, float top, float nearVal, float farVal);

	FMatrix _AdjustProjectionMatrixForRHI(const FMatrix& InProjectionMatrix);

	void UpdateEyeRelativePosition(FVector _eyeRelativePosition);

	void SetWidth(float _width);

	void SetHeight(float _height);

	void ToggleOffAxisMethod();

	void PrintCurrentOffAxisVersioN();

	void UpdateEyeOffsetForStereo(float _newVal);

	void UpdateProjectionPlaneOffsetForStereo(float _newVal);

	void ResetProjectionPlaneOffsetForStereo(float _newVal);

	void ResetEyeOffsetForStereo(float _newVal);

	void UpdateTmpVector(FVector _newVal);

	void UpdateShowDebugMessages(bool _newVal);

	void UseOffAxis(bool _newVal);



	float s_ProjectionPlaneOffset = 0.f;
	bool s_bUseoffAxis = false;
	bool mOffAxisMatrixSetted = false;
	float s_Width = 0.f;
	float s_Height = 0.f;
	FVector s_EyePosition = FVector();
	FMatrix mOffAxisMatrix = FMatrix();

	bool s_OffAxisVersion = 1;
	bool s_ShowDebugMessages = false;
	float s_EyeOffsetVal = 3.2f;
	FVector s_tmp = FVector();
};

