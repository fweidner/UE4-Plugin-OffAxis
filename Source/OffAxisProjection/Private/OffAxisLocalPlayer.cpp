// Fill out your copyright notice in the Description page of Project Settings.

#include "OffAxisLocalPlayer.h"

#include "Engine.h"

#include "OffAxisProjection.h" //used for log
#include "Runtime/Core/Public/GenericPlatform/GenericPlatformTime.h" // for FPlatformTime
#include "Runtime/Engine/Classes/Engine/GameViewportClient.h" //for ViewportClient

FSceneView * UOffAxisLocalPlayer::CalcSceneView(FSceneViewFamily * ViewFamily, FVector & OutViewLocation, FRotator & OutViewRotation, FViewport * Viewport, FViewElementDrawer * ViewDrawer, EStereoscopicPass StereoPass)
{
	double start = FPlatformTime::Seconds();

	FSceneView* tmp = ULocalPlayer::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer, StereoPass);

	if (s_bUseoffAxis && tmp)
	{
		UpdateProjectionMatrix_Internal(tmp, GenerateOffAxisMatrix(s_EyePosition, StereoPass), StereoPass);
	}

	double end = FPlatformTime::Seconds();

	//if (s_ShowDebugMessages)
	//	GEngine->AddOnScreenDebugMessage(0, 1, FColor::Red, FString::Printf(TEXT("TimeToOffAxis in: %f s."), end - start));

	return tmp;
}

FMatrix UOffAxisLocalPlayer::FrustumMatrix(float left, float right, float bottom, float top, float nearVal, float farVal)
{
	//column-major order
	FMatrix Result;
	Result.SetIdentity();
	Result.M[0][0] = (2.0f * nearVal) / (right - left);
	Result.M[1][1] = (2.0f * nearVal) / (top - bottom);
	Result.M[2][0] = -(right + left) / (right - left);
	Result.M[2][1] = -(top + bottom) / (top - bottom);
	Result.M[2][2] = (farVal) / (farVal - nearVal);
	Result.M[2][3] = 1.0f;
	Result.M[3][2] = -(farVal * nearVal) / (farVal - nearVal);
	Result.M[3][3] = 0.0f;

	return Result;
}

void UOffAxisLocalPlayer::UpdateProjectionMatrix_Internal(FSceneView* View, FMatrix OffAxisMatrix, EStereoscopicPass _Pass)
{
	FMatrix stereoProjectionMatrix = OffAxisMatrix;
	s_CurrentPassType = _Pass;

	switch (_Pass)
	{
	case eSSP_LEFT_EYE:
		stereoProjectionMatrix = FTranslationMatrix(FVector(s_ProjectionPlaneOffset, 0.f, 0.f)) * OffAxisMatrix;
		
		break;
	case eSSP_RIGHT_EYE:
		stereoProjectionMatrix = FTranslationMatrix(FVector(-s_ProjectionPlaneOffset, 0.f, 0.f)) * OffAxisMatrix;
		
		break;
	default:
		break;
	}

	FMatrix axisChanger; //rotates everything to UE4 coordinate system.
	axisChanger.SetIdentity();
	axisChanger.M[0][0] = 0.0f;
	axisChanger.M[1][1] = 0.0f;
	axisChanger.M[2][2] = 0.0f;
	axisChanger.M[0][2] = 1.0f;
	axisChanger.M[1][0] = 1.0f;
	axisChanger.M[2][1] = 1.0f;

	View->ViewRotation = s_ViewRotation;
	
	FMatrix tmpMat = axisChanger * stereoProjectionMatrix;
	View->UpdateViewMatrix();
	
//	View->ProjectionMatrixUnadjustedForRHI = View->ViewMatrices.GetViewMatrix().Inverse() * tmpMat;
	View->UpdateProjectionMatrix(View->ViewMatrices.GetViewMatrix().Inverse() * tmpMat);
	
	switch (_Pass)
	{
	case eSSP_LEFT_EYE:
		if (s_ShowDebugMessages)
		{
			GEngine->AddOnScreenDebugMessage(72, 2, FColor::Black, FString::Printf(TEXT("l\n: %s\n"), *View->ViewMatrices.GetProjectionMatrix().ToString()));
			GEngine->AddOnScreenDebugMessage(73, 2, FColor::Black, FString::Printf(TEXT("l\n: %s\n"), *View->ViewLocation.ToString()));
			GEngine->AddOnScreenDebugMessage(74, 2, FColor::Black, FString::Printf(TEXT("l\n: %s\n"), *View->ViewRotation.ToString()));
		}
		s_ProjectionMatrix_left = View->ViewMatrices.GetProjectionMatrix();
		break;
	case eSSP_RIGHT_EYE:
		s_ProjectionMatrix_right = View->ViewMatrices.GetProjectionMatrix();
		if (s_ShowDebugMessages)
		{
			GEngine->AddOnScreenDebugMessage(75, 2, FColor::Black, FString::Printf(TEXT("r\n: %s \n"), *View->ViewMatrices.GetProjectionMatrix().ToString()));
			GEngine->AddOnScreenDebugMessage(76, 2, FColor::Black, FString::Printf(TEXT("l\n: %s\n"), *View->ViewLocation.ToString()));
			GEngine->AddOnScreenDebugMessage(77, 2, FColor::Black, FString::Printf(TEXT("l\n: %s\n"), *View->ViewRotation.ToString()));
		}
		break;
	default:
		s_ProjectionMatrix = View->ViewMatrices.GetProjectionMatrix();
		break;
	}
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix_Internal(FVector _eyeRelativePosition)
{
	FMatrix result;

	FVector eyePosition = _eyeRelativePosition;

	float l, r, b, t, n, f, nd;

	n = GNearClippingPlane;
	f = GFarClippingPlane;

	//this is analog to: http://csc.lsu.edu/~kooima/articles/genperspective/

	//Careful: coordinate system! y-up, x-right (UE4 uses inverted LHS)

	//pa = lower left, pb = lower right, pc = upper left, eye pos

	pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);

	// Compute the screen corner vectors.
	FVector va, vb, vc;
	va = pa - pe;
	vb = pb - pe;
	vc = pc - pe;

	// Compute an orthonormal basis for the screen.
	FVector vr, vu, vn;
	vr = pb - pa;
	vr.Normalize();
	vu = pc - pa;
	vu.Normalize();
	vn = -FVector::CrossProduct(vr, vu);
	vn.Normalize();

	// Find the distance from the eye to screen plane.
	float d = -FVector::DotProduct(va, vn);

	nd = n / d;

	// Find the extent of the perpendicular projection.
	l = FVector::DotProduct(vr, va) * nd;
	r = FVector::DotProduct(vr, vb) * nd;
	b = FVector::DotProduct(vu, va) * nd;
	t = FVector::DotProduct(vu, vc) * nd;

	// Load the perpendicular projection.
	result = FrustumMatrix(l, r, b, t, n, f);
	//GEngine->AddOnScreenDebugMessage(40, 2, FColor::Red, FString::Printf(TEXT("FrustumMatrix_ORIG: %s"), *result.ToString()));

	if (s_ShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(1, 2, FColor::Orange, FString::Printf(TEXT("pa: %s"), *pa.ToString()));
		GEngine->AddOnScreenDebugMessage(2, 2, FColor::Orange, FString::Printf(TEXT("pb: %s"), *pb.ToString()));
		GEngine->AddOnScreenDebugMessage(3, 2, FColor::Orange, FString::Printf(TEXT("pc: %s"), *pc.ToString()));
		GEngine->AddOnScreenDebugMessage(4, 2, FColor::Orange, FString::Printf(TEXT("pe: %s"), *pe.ToString()));
		// 		GEngine->AddOnScreenDebugMessage(5, 2, FColor::Black, FString::Printf(TEXT("vr: %s"), *vu.ToString()));
		// 		GEngine->AddOnScreenDebugMessage(6, 2, FColor::Black, FString::Printf(TEXT("vu: %s"), *vr.ToString()));
		// 		GEngine->AddOnScreenDebugMessage(7, 2, FColor::Black, FString::Printf(TEXT("vn: %s"), *vn.ToString()));
		//		GEngine->AddOnScreenDebugMessage(8, 4, FColor::Red, FString::Printf(TEXT("Frustum: %f \t %f \t %f \t %f \t %f \t %f \t "), l, r, b, t, n, f));
		GEngine->AddOnScreenDebugMessage(9, 2, FColor::Orange, FString::Printf(TEXT("Eye-Screen-Distance: %f"), d));
		GEngine->AddOnScreenDebugMessage(10, 4, FColor::Orange, FString::Printf(TEXT("nd: %f"), nd));
	}

	//Move the apex of the frustum to the origin.
	result = FTranslationMatrix(-eyePosition) * result;

	//scales matrix for UE4 and RHI
	result *= 1.0f / result.M[0][0];

	result.M[2][2] = 0.f; //?
	result.M[3][2] = n; //?

	return result;

}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix_Internal_Test(FVector _eyeRelativePositon)
{
	return GenerateOffAxisMatrix_Internal(_eyeRelativePositon);
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix(FVector _eyeRelativePosition, EStereoscopicPass _PassType)
{
	EyeOffsetVector = FVector::ZeroVector;
	EyeOffsetVector.X = s_EyeOffsetVal;	//no
	//EyeOffsetVector.Y = s_EyeOffsetVal;		//yes
	//EyeOffsetVector.Z = s_EyeOffsetVal;
	
	s_currentAdjustedViewLocation = _eyeRelativePosition;
	if (Is3DEnabled())
	{
		if (s_ShowDebugMessages)
		{
			GEngine->AddOnScreenDebugMessage(459, 0, FColor::Cyan, FString::Printf(TEXT("3D")));
		}
		if (_PassType == eSSP_LEFT_EYE)
		{
			s_currentAdjustedViewLocation -= EyeOffsetVector;
			//GEngine->AddOnScreenDebugMessage(455, 10, FColor::Cyan, FString::Printf(TEXT("adjustedEyePositionForS3D l: %s"), *s_currentAdjustedViewLocation.ToString()));

		}
		else if (_PassType == eSSP_RIGHT_EYE)
		{
			s_currentAdjustedViewLocation += EyeOffsetVector;
			//GEngine->AddOnScreenDebugMessage(456, 10, FColor::Cyan, FString::Printf(TEXT("adjustedEyePositionForS3D r: %s"), *s_currentAdjustedViewLocation.ToString()));
		}
		else 
		{
			GEngine->AddOnScreenDebugMessage(699, 10, FColor::Red, FString::Printf(TEXT("Error: This should not happen! (Cyclooops?)")));
		}
	}


	switch (s_OffAxisMethod)
	{
	case EOffAxisMethod::Slow:
		return GenerateOffAxisMatrix_Internal(s_currentAdjustedViewLocation);
	case EOffAxisMethod::Test:
		return GenerateOffAxisMatrix_Internal_Test(s_currentAdjustedViewLocation);
	default:
		return GenerateOffAxisMatrix_Internal(s_currentAdjustedViewLocation);
	}
}

//////////////////////////////////
// Getter and Setter			//
//////////////////////////////////

FVector UOffAxisLocalPlayer::UpdateEyeRelativePosition(FVector _eyeRelativePosition)
{
	s_EyePosition = _eyeRelativePosition;
	return s_EyePosition;
}

EOffAxisMethod UOffAxisLocalPlayer::ToggleOffAxisMethod()
{

	switch (s_OffAxisMethod)
	{
	case EOffAxisMethod::Test:
		s_OffAxisMethod = EOffAxisMethod::Slow;
		break;
	case EOffAxisMethod::Slow:
		s_OffAxisMethod = EOffAxisMethod::Test;
		break;
	default:
		s_OffAxisMethod = EOffAxisMethod::Slow;
		break;
	};

	PrintCurrentOffAxisVersion();

	return s_OffAxisMethod;
}

FVector UOffAxisLocalPlayer::UpdateTmpVector(FVector _newVal)
{
	s_tmpVec = _newVal;
	return s_tmpVec;
}

FRotator UOffAxisLocalPlayer::UpdateTmpRotator(FRotator _newVal)
{
	s_ViewRotation = _newVal;
	return s_ViewRotation;
}

FRotator UOffAxisLocalPlayer::AddTmpRotaterOffset(FRotator _offset)
{
	s_ViewRotation += _offset;
	return s_ViewRotation;
}

bool UOffAxisLocalPlayer::UpdateShowDebugMessages(bool _newVal)
{
	s_ShowDebugMessages = _newVal;
	return s_ShowDebugMessages;
}

bool UOffAxisLocalPlayer::UseOffAxis(bool _newVal)
{
	s_bUseoffAxis = _newVal;
	return s_bUseoffAxis;
}

void UOffAxisLocalPlayer::PrintCurrentOffAxisVersion()
{
	UE_LOG(LogConsoleResponse, Warning, TEXT("OffAxisVersion: %s"), *GetOffAxisEnumValueAsString(s_OffAxisMethod).ToString());
	GEngine->AddOnScreenDebugMessage(30, 4, FColor::Cyan, FString::Printf(TEXT("OffAxisVersion: %s"), *GetOffAxisEnumValueAsString(s_OffAxisMethod).ToString()));
}

EOffAxisMethod UOffAxisLocalPlayer::SetOffAxisMethod(EOffAxisMethod _newMethod, bool _bShouldPrintLogMessage)
{
	s_OffAxisMethod = _newMethod;
	if (_bShouldPrintLogMessage)
		PrintCurrentOffAxisVersion();
	return s_OffAxisMethod;
}

//////////////////////////////////////////////////////////////////////////
float UOffAxisLocalPlayer::UpdateEyeOffsetForStereo(float _newVal)
{
	s_EyeOffsetVal += _newVal;

	if (s_ShowDebugMessages)
		GEngine->AddOnScreenDebugMessage(45, 2, FColor::Cyan, FString::Printf(TEXT("ss_EyeOffsetVal: %f"), s_EyeOffsetVal));
	return s_EyeOffsetVal;
}

float UOffAxisLocalPlayer::UpdateProjectionPlaneOffsetForStereo(float _newVal)
{
	s_ProjectionPlaneOffset += _newVal;
	if (s_ShowDebugMessages)
		GEngine->AddOnScreenDebugMessage(55, 2, FColor::Cyan, FString::Printf(TEXT("ProjectionPlaneOffset: %f"), s_ProjectionPlaneOffset));
	return s_ProjectionPlaneOffset;
}

float UOffAxisLocalPlayer::ResetProjectionPlaneOffsetForStereo(float _newVal /*= 0.f*/)
{
	s_ProjectionPlaneOffset = _newVal;
	return s_ProjectionPlaneOffset;
}

float UOffAxisLocalPlayer::ResetEyeOffsetForStereo(float _newVal)
{
	s_EyeOffsetVal = _newVal;
	return s_EyeOffsetVal;
}

FText UOffAxisLocalPlayer::GetOffAxisEnumValueAsString(EOffAxisMethod _val)
{
	const UEnum* EnumPtr = FindObject<UEnum>(ANY_PACKAGE, TEXT("EOffAxisMethod"), true);
	if (!EnumPtr) return NSLOCTEXT("Invalid", "Invalid", "Invalid");

	return EnumPtr->GetDisplayNameTextByIndex(_val);
}

void UOffAxisLocalPlayer::SetPx(bool _setpa, FVector _pa, bool _setpb, FVector _pb, bool _setpc, FVector _pc)
{
	if (_setpa)
	{
		pa = _pa;
	}
	if (_setpb)
	{
		pb = _pb;
	}
	if (_setpc)
	{
		pc = _pc;
	}
}

/************************************************************************/
/* PICKING                                                              */
/************************************************************************/

bool UOffAxisLocalPlayer::OffAxisDeprojectScreenToWorld_Internal(
	APlayerController const* Player,
	FVector2D& _screenPosition,
	FVector2D& _viewportsize,
	FVector& WorldPosition,
	FVector& WorldDirection)
{
	//////////////////////////////////////////////////////////////////////////

	ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;

	if (LP && LP->ViewportClient)
	{
		// get the projection data and projection matrix
		FMatrix eyeProjectionMatrix;
		FSceneViewProjectionData ProjectionData;
	
		if (Is3DEnabled())
		{
			if (_screenPosition.X < _viewportsize.X/2)
			{
				LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_LEFT_EYE, /*out*/ ProjectionData);
				eyeProjectionMatrix = s_ProjectionMatrix_left;
			}
			else
			{
				LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_RIGHT_EYE, /*out*/ ProjectionData);
				eyeProjectionMatrix = s_ProjectionMatrix_right;
			}
		}
		else
		{
			LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ ProjectionData);
			eyeProjectionMatrix = s_ProjectionMatrix;
		}
		//GEngine->AddOnScreenDebugMessage(312, 10, FColor::Cyan, FString::Printf(TEXT("ViewOrigin: %s"), *(ProjectionData.ViewOrigin).ToString()));

		//calculate deprojection matrix
		FMatrix t = FTranslationMatrix(-ProjectionData.ViewOrigin) * ProjectionData.ViewRotationMatrix * eyeProjectionMatrix;
		FMatrix const InvViewProjMatrix = t.Inverse();

		//Use unreals deproject function.
		FSceneView::DeprojectScreenToWorld(_screenPosition, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ WorldPosition, /*out*/ WorldDirection);
		return true;
	}

	// something went wrong, zero things and return false
	WorldPosition = FVector::ZeroVector;
	WorldDirection = FVector::ZeroVector;
	GEngine->AddOnScreenDebugMessage(312, 10, FColor::Red, FString::Printf(TEXT("Error: Deprojection failed for picking!")));
	return false;
}

bool UOffAxisLocalPlayer::OffAxisLineTraceByChannel(
	UObject* WorldContextObject,
	/*out*/ struct FHitResult& OutHit,
	FVector _eyeRelativePosition,
	bool bDrawDebugLine,
	FColor _color,
	bool bPersistentLines /*= false*/,
	float _lifeTime /*= 10.f*/,
	uint8 _depthPriority /*= 0*/,
	float _thickness /*= 1.f*/,
	float _LengthOfRay /*= 1000.f*/)
{

	//////////////////////////////////////////////////////////////////////////

	//get mouse position
	APlayerController* player = UGameplayStatics::GetPlayerController(WorldContextObject, 0);
	float x, y;
	player->GetMousePosition(x, y);
	FVector2D screenPosition = FVector2D(x, y);

	//get resolution
	FVector2D viewportsize;
	GEngine->GameViewport->GetViewportSize(/*out */ viewportsize);

	//Correct eye offset
	FVector tmpEyeRelativePosition = _eyeRelativePosition;
	if (Is3DEnabled())
	{
		if (x <= viewportsize.X/2)
		{
			tmpEyeRelativePosition -= EyeOffsetVector;
		}
		else
		{
			tmpEyeRelativePosition += EyeOffsetVector;
		}
	}

	//transform eyeRelativePosition to UE4 coordinates
	FVector _eyeRelativePositioninUE4Coord = FVector(tmpEyeRelativePosition.Z, tmpEyeRelativePosition.X, tmpEyeRelativePosition.Y);

	//get end position for ray trace
	FVector WorldPosition, WorldDirection;
	OffAxisDeprojectScreenToWorld_Internal(player, screenPosition, viewportsize, WorldPosition, WorldDirection);
	FVector end = WorldPosition + _LengthOfRay * WorldDirection;

	//draw debug line
	if (bDrawDebugLine)
	{
		DrawDebugLine(WorldContextObject->GetWorld(), _eyeRelativePositioninUE4Coord, end, _color, bPersistentLines, _lifeTime, _depthPriority, _thickness);

		//UE_LOG(OffAxisLog, Log, TEXT("Start: %s"), *_eyeRelativePosition.ToString());
		//UE_LOG(OffAxisLog, Log, TEXT("End  : %s"), *end.ToString());

		GEngine->AddOnScreenDebugMessage(301, 10, FColor::Cyan, FString::Printf(TEXT("Start: %s"), *_eyeRelativePositioninUE4Coord.ToString()));
		GEngine->AddOnScreenDebugMessage(302, 10, FColor::Cyan, FString::Printf(TEXT("End  : %s"), *end.ToString()));
	}

	//do raytrace
	return WorldContextObject->GetWorld()->LineTraceSingleByChannel(OutHit, _eyeRelativePositioninUE4Coord, end, ECollisionChannel::ECC_Visibility);

}

bool UOffAxisLocalPlayer::Is3DEnabled()
{

	switch (s_CurrentPassType)
	{
	case eSSP_LEFT_EYE:
		s_Is3D = true;
		break;
	case eSSP_RIGHT_EYE:
		s_Is3D = true;
		break;
	default:
		s_Is3D = false;
		break;
	}
	return s_Is3D;
}


/************************************************************************/
/* HELPER UE4 <-> OFFAXIS                                               */
/************************************************************************/

FVector UOffAxisLocalPlayer::OffAxisToUE(FVector _OffAxisVector)
{
	FVector uEVector= _OffAxisVector;
	
	uEVector.X = _OffAxisVector.Z;
	uEVector.Y = _OffAxisVector.X;
	uEVector.Z = _OffAxisVector.Y;

	return uEVector;
}

