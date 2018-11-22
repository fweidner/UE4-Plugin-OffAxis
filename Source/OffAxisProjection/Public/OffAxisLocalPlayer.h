// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EOffAxisMethod.h"
#include "Runtime/Core/Public/Math/IntRect.h"
#include "Engine/LocalPlayer.h"
#include "OffAxisLocalPlayer.generated.h"




/**
 * 
 */
UCLASS(BlueprintType)
class OFFAXISPROJECTION_API UOffAxisLocalPlayer : public ULocalPlayer
{
	GENERATED_BODY()


	FSceneView* CalcSceneView(class FSceneViewFamily* ViewFamily,
		FVector& OutViewLocation,
		FRotator& OutViewRotation,
		FViewport* Viewport,
		class FViewElementDrawer* ViewDrawer = NULL,
		EStereoscopicPass StereoPass = eSSP_FULL) override;

public:

	void UpdateProjectionMatrix_Internal(FSceneView * View, FMatrix OffAxisMatrix, EStereoscopicPass _Pass);

	FMatrix GenerateOffAxisMatrix(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon, EStereoscopicPass _PassType);

	FMatrix GenerateOffAxisMatrix_Internal_Slow(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon);
	FMatrix GenerateOffAxisMatrix_Internal_Fast(FVector _eyeRelativePositon);
	FMatrix GenerateOffAxisMatrix_Internal_Test(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon);


	FMatrix FrustumMatrix(float left, float right, float bottom, float top, float nearVal, float farVal);




	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PrintCurrentOffAxisVersion", Keywords = "OffAxisProjection print"), Category = "OffAxisProjection")
		static FText GetOffAxisEnumValueAsString(EOffAxisMethod _val);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "InitOffAxisProjection", Keywords = "OffAxisProjection init"), Category = "OffAxisProjection")
		static void InitOffAxisProjection_Fast(float _screenWidth, float _screenHeight);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UpdateEyeRelativePosition", Keywords = "OffAxisProjection update relative eye position "), Category = "OffAxisProjection")
		static FVector UpdateEyeRelativePosition(FVector _eyeRelativePosition);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetWidth", Keywords = "OffAxisProjection width "), Category = "OffAxisProjection")
		static float SetWidth(float _width);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetHeight", Keywords = "OffAxisProjection height "), Category = "OffAxisProjection")
		static float SetHeight(float _height);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ToggleOffAxisMethod", Keywords = "OffAxisProjection toggle method "), Category = "OffAxisProjection")
		static EOffAxisMethod ToggleOffAxisMethod();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "PrintCurrentOffAxisVersion", Keywords = "OffAxisProjection print"), Category = "OffAxisProjection")
		static void PrintCurrentOffAxisVersion();

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetOffAxisMethod", Keywords = "OffAxisProjection set method "), Category = "OffAxisProjection")
		static EOffAxisMethod SetOffAxisMethod(EOffAxisMethod _newMethod, bool _bShouldPrintLogMessage);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UpdateTmpVector", Keywords = "OffAxisProjection tmp update"), Category = "OffAxisProjection")
		static FVector UpdateTmpVector(FVector _newVal);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UpdateShowDebugMessages", Keywords = "OffAxisProjection show debug "), Category = "OffAxisProjection")
		static bool UpdateShowDebugMessages(bool _newVal);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UseOffAxis", Keywords = "OffAxisProjection use "), Category = "OffAxisProjection")
		static bool UseOffAxis(bool _newVal);

	//////////////////////////////////////////////////////////////////////////
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UpdateEyeOffsetForStereo", Keywords = "OffAxisProjection eye offset eyeoffset set"), Category = "OffAxisProjection")
		static float UpdateEyeOffsetForStereo(float _newVal);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "UpdateProjectionPlaneOffsetForStereo", Keywords = "OffAxisProjection projection offset projectionplaneoffset plane "), Category = "OffAxisProjection")
		static float UpdateProjectionPlaneOffsetForStereo(float _newVal);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ResetProjectionPlaneOffsetForStereo", Keywords = "OffAxisProjection projection offset reset projectionplaneoffset plane"), Category = "OffAxisProjection")
		static float ResetProjectionPlaneOffsetForStereo(float _newVal);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "ResetEyeOffsetForStereo", Keywords = "OffAxisProjection eye offset eyeoffset reset "), Category = "OffAxisProjection")
		static float ResetEyeOffsetForStereo(float _newVal = 3.200001);

	//////////////////////////////////////////////////////////////////////////

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "SetI", Keywords = "OffAxisProjection SetI"), Category = "OffAxisProjection")
		static int SetI(int _newVal);

	UFUNCTION(BlueprintCallable, meta = (DisplayName = "OffAxisDeprojectScreenToWorld", Keywords = "OffAxis DeprojectScreenToWorld"), Category = "OffAxisProjection")
		static bool OffAxisDeprojectScreenToWorld(APlayerController const* Player, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection);

	UFUNCTION(BlueprintCallable, meta = (WorldContext = "WorldContextObject", DisplayName = "OffAxisLineTraceByChannel", AdvancedDisplay = "_color,bPersistentLines,_lifeTime,_depthPriority,_thickness,_LengthOfRay", Keywords = " OffAxis LineTraceByChannel"), Category = "OffAxisProjection")
		static bool OffAxisLineTraceByChannel(
			UObject* WorldContextObject, 
			/*out*/ struct FHitResult& OutHit, 
			FVector _eyeRelativePosition, 
			bool bDrawDebugLine, 
			FColor _color, 
			bool bPersistentLines = false, 
			float _lifeTime = 10.f, 
			uint8 _depthPriority = 0, 
			float _thickness = 1.f, 
			float _LengthOfRay = 1000.f);

	static bool OffAxisDeprojectScreenToWorld(APlayerController const* Player, FVector& WorldPosition, FVector& WorldDirection);

private: 
	FMatrix mOffAxisMatrix = FMatrix();
	float f = 10000.f;
	float n = .1f;
};

//////////////////////////////////////////////////////////////////////////
static float s_ProjectionPlaneOffset = 0.f;
static float s_EyeOffsetVal = 3.2f;

//////////////////////////////////////////////////////////////////////////
static bool s_ShowDebugMessages = false;
static FVector s_tmp = FVector();

//////////////////////////////////////////////////////////////////////////
static FVector s_EyePosition = FVector();
static bool s_bUseoffAxis = false;

static EOffAxisMethod s_OffAxisMethod = EOffAxisMethod::Fast;

static float s_Width = 0.f;
static float s_Height = 0.f;

static FVector s_TopLeftCorner = FVector();
static FVector s_BottomRightCorner = FVector();

static float GFarClippingPlane = 10000.f;

static FMatrix s_Frustum;

//////////////////////////////////////////////////////////////////////////
static int i = 0;

static FVector pa, pb, pc, pe;

static FMatrix s_InvProjectionMatrix = FMatrix();
static FMatrix s_ProjectionMatrix = FMatrix();

static FIntRect s_ConstrainedViewRect = FIntRect(); 
