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

	//OutViewRotation = s_tmpRot;

	if (s_bUseoffAxis && tmp)
	{
		UpdateProjectionMatrix_Internal(tmp, GenerateOffAxisMatrix(s_Width, s_Height, s_EyePosition, StereoPass), StereoPass);
	}

	double end = FPlatformTime::Seconds();


	if (s_ShowDebugMessages)
		GEngine->AddOnScreenDebugMessage(0, 1, FColor::Red, FString::Printf(TEXT("TimeToOffAxis in: %f s."), end - start));


	return tmp;
}

FMatrix UOffAxisLocalPlayer::FrustumMatrix(float left, float right, float bottom, float top, float nearVal, float farVal)
{
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

	switch (_Pass)
	{
	case eSSP_FULL:
		break;
	case eSSP_LEFT_EYE:
		stereoProjectionMatrix = FTranslationMatrix(FVector(s_ProjectionPlaneOffset, 0.f, 0.f)) * OffAxisMatrix;
		break;
	case eSSP_RIGHT_EYE:
		stereoProjectionMatrix = FTranslationMatrix(FVector(-s_ProjectionPlaneOffset, 0.f, 0.f)) * OffAxisMatrix;
		break;
	case eSSP_MONOSCOPIC_EYE:
		break;
	default:
		break;
	}

	//stereoProjectionMatrix = FRotationMatrix(FRotator(0.f, 0.f, s_tmp.X)) * OffAxisMatrix;

	FMatrix axisChanger; //rotates everything to UE4 coordinate system.
	axisChanger.SetIdentity();
	axisChanger.M[0][0] = 0.0f;
	axisChanger.M[1][1] = 0.0f;
	axisChanger.M[2][2] = 0.0f;
	axisChanger.M[0][2] = 1.0f;
	axisChanger.M[1][0] = 1.0f;
	axisChanger.M[2][1] = 1.0f;


	//View->ViewRotation = s_tmpRot;

	View->UpdateViewMatrix();

	FMatrix tmpMat = axisChanger * stereoProjectionMatrix;
	
	View->ProjectionMatrixUnadjustedForRHI = View->ViewMatrices.GetViewMatrix().Inverse() * tmpMat;

	View->UpdateProjectionMatrix(View->ViewMatrices.GetViewMatrix().Inverse() * tmpMat);


	s_ProjectionMatrix = View->ViewMatrices.GetProjectionMatrix();

	
	
}

void UOffAxisLocalPlayer::InitOffAxisProjection_Fast(float _screenWidth, float _screenHeight)
{
	s_TopLeftCorner = FVector(-_screenWidth / 2.f, -_screenHeight / 2.f, GNearClippingPlane);
	s_BottomRightCorner = FVector(_screenWidth / 2.f, _screenHeight / 2.f, GNearClippingPlane);


	FMatrix Frustum;
	Frustum.SetIdentity();
	Frustum.M[2][2] = -(GFarClippingPlane + GNearClippingPlane) / (GFarClippingPlane - GNearClippingPlane);
	Frustum.M[2][3] = -1.0f;
	Frustum.M[3][2] = -(2.f*GFarClippingPlane * GNearClippingPlane) / (GFarClippingPlane - GNearClippingPlane);
	Frustum.M[3][3] = 0.0f;


}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix_Internal_Slow(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon)
{
	FMatrix result;

	float width = _screenWidth;
	float height = _screenHeight;
	FVector eyePosition = _eyeRelativePositon;

	float l, r, b, t, n, f, nd;

	n = GNearClippingPlane;
	f = GFarClippingPlane;

	//this is analog to: http://csc.lsu.edu/~kooima/articles/genperspective/

	//Careful: coordinate system! left-handed, y-up

	//lower left, lower right, upper left, eye pos
	const FVector pa(-width / 2.0f, -height / 2.0f, n);
	const FVector pb(width / 2.0f, -height / 2.0f, n);
	const FVector pc(-width / 2.0f, height / 2.0f, n);
	const FVector pe(eyePosition.X, eyePosition.Y, eyePosition.Z);

	// Compute the screen corner vectors.
	FVector va, vb, vc;
	va = pa - pe;
	vb = pb - pe;
	vc = pc - pe;

	// Compute an orthonormal basis for the screen.
	FVector vr, vu, vn;
	vr = pb - pa;
	vr /= vr.Normalize();
	vu = pc - pa;
	vu /= vu.Normalize();
	vn = FVector::CrossProduct(vr, vu);
	vn /= vn.Normalize();

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

	// Rotate the projection to be non-perpendicular. 
	// This is currently unused until the screen is used.
	FMatrix M;
	M.SetIdentity();
	M.M[0][0] = vr.X; M.M[0][1] = vr.Y; M.M[0][2] = vr.Z;
	M.M[1][0] = vu.X; M.M[1][1] = vu.Y; M.M[1][2] = vu.Z;
	M.M[2][0] = vn.X; M.M[2][1] = vn.Y; M.M[2][2] = vn.Z;
	M.M[3][3] = 1.0f;
	result = result * M;

	if (s_ShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(10, 2, FColor::Red, FString::Printf(TEXT("pa: %s"), *pa.ToString()));
		GEngine->AddOnScreenDebugMessage(20, 2, FColor::Red, FString::Printf(TEXT("pb: %s"), *pb.ToString()));
		GEngine->AddOnScreenDebugMessage(30, 2, FColor::Red, FString::Printf(TEXT("pc: %s"), *pc.ToString()));
		GEngine->AddOnScreenDebugMessage(40, 2, FColor::Red, FString::Printf(TEXT("pe: %s"), *pe.ToString()));
		GEngine->AddOnScreenDebugMessage(50, 2, FColor::Red, FString::Printf(TEXT("vr: %s"), *vu.ToString()));
		GEngine->AddOnScreenDebugMessage(60, 2, FColor::Red, FString::Printf(TEXT("vu: %s"), *vr.ToString()));
		GEngine->AddOnScreenDebugMessage(70, 2, FColor::Red, FString::Printf(TEXT("vn: %s"), *vn.ToString()));
		GEngine->AddOnScreenDebugMessage(80, 4, FColor::Red, FString::Printf(TEXT("Frustum: %f \t %f \t %f \t %f \t %f \t %f \t "), l, r, b, t, n, f));
		GEngine->AddOnScreenDebugMessage(90, 2, FColor::Red, FString::Printf(TEXT("Eye-Screen-Distance: %f"), d));
		GEngine->AddOnScreenDebugMessage(100, 4, FColor::Red, FString::Printf(TEXT("nd: %f"), nd));
	}

	// Move the apex of the frustum to the origin.
	result = FTranslationMatrix(-eyePosition) * result;

	//scales matrix for UE4 and RHI
	result *= 1.0f / result.M[0][0];

	result.M[2][2] = 0.f; //?
	result.M[3][2] = n; //?

	return result;
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix_Internal_Fast(FVector _eyeRelativePositon)
{
	FVector eyeToTopLeft = s_TopLeftCorner - _eyeRelativePositon;
	FVector eyeToTopLeftNear = GNearClippingPlane / eyeToTopLeft.Z * eyeToTopLeft;

	FVector eyeToBottomRight = s_BottomRightCorner - _eyeRelativePositon;
	FVector eyeToBottomRightNear = eyeToBottomRight / eyeToBottomRight.Z * GNearClippingPlane;

	//Frustum: l, r, b, t, near, far
	FMatrix result = FrustumMatrix(-eyeToTopLeftNear.X, -eyeToBottomRightNear.X, -eyeToTopLeftNear.Y, -eyeToBottomRightNear.Y, GNearClippingPlane, GFarClippingPlane);

	// Move the apex of the frustum to the origin.
	result = FTranslationMatrix(-_eyeRelativePositon) * result;

	//scales matrix for UE4 and RHI
	result *= 1.0f / result.M[0][0];

	result.M[2][2] = 0.f; //?
	result.M[3][2] = GNearClippingPlane; //?

	return result;
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix_Internal_Test(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon)
{
	//GEngine->AddOnScreenDebugMessage(45, 2, FColor::Emerald, FString::Printf(TEXT("pe (before): %s"), *pe.ToString()));
	//_eyeRelativePositon = s_tmpRot.RotateVector(_eyeRelativePositon); //was just for debug puproses

	FMatrix result;

	float width = _screenWidth;
	float height = _screenHeight;
	FVector eyePosition = _eyeRelativePositon;

	float l, r, b, t, n, f, nd;

	n = GNearClippingPlane;
	f = GFarClippingPlane;

	//this is analog to: http://csc.lsu.edu/~kooima/articles/genperspective/

	//Careful: coordinate system! y-up, x-right

	//pa = lower left, pb = lower right, pc = upper left, eye pos

	/*
	 *pa=(-180,	-110,	0.1)
	 *pb=(180,	-110,	0.1)
	 *pc=(-180,	89,		0.1)
	 */

	FVector2D ProjectionScreenSize = FVector2D(2*58.f, 2*34.f);

	float hw = ProjectionScreenSize.X / 2.f * 100; //from pScreen
	float hh = ProjectionScreenSize.Y / 2.f * 100; //from pScreen

	FVector ProjectionScreenLoc = FVector(0.f, 0.f, 0.f);
	FRotator ProjectionScreenRot = FRotator(0.f, 0.f, 0.f);

	switch (s_test1)
	{
	case 0: 
		GEngine->AddOnScreenDebugMessage(200, 2, FColor::Emerald, FString::Printf(TEXT("Method: s_tmpVec @ everything eye")));
		pa = FVector(-width / 2.0f + s_tmpVec.X, -height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pb = FVector(width / 2.0f + s_tmpVec.X, -height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pc = FVector(-width / 2.0f + s_tmpVec.X, height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pe = FVector(eyePosition.X + s_tmpVec.X, eyePosition.Y + s_tmpVec.Y, eyePosition.Z + s_tmpVec.Z);
		break;
	case 1: // s_tmpVec @ pa, pb, pc
		GEngine->AddOnScreenDebugMessage(200, 2, FColor::Emerald, FString::Printf(TEXT("Method: s_tmpVec @ pa, pb, pc")));
		pa = FVector(-width / 2.0f + s_tmpVec.X, -height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pb = FVector(width / 2.0f + s_tmpVec.X, -height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pc = FVector(-width / 2.0f + s_tmpVec.X, height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	case 2: // s_tmpVec only @ pc
		GEngine->AddOnScreenDebugMessage(200, 2, FColor::Emerald, FString::Printf(TEXT("Method: s_tmpVec only @ pc")));
		pa = FVector(-width / 2.0f, -height / 2.0f, n);
		pb = FVector(width / 2.0f, -height / 2.0f, n);
		pc = FVector(-width / 2.0f + s_tmpVec.X, height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
	case 3:
		GEngine->AddOnScreenDebugMessage(200, 2, FColor::Emerald, FString::Printf(TEXT("Method: tilt z")));
		pa = FVector(-width / 2.0f, -height / 2.0f, n - s_tmpVec.Z);
		pb = FVector(width / 2.0f, -height / 2.0f, n - s_tmpVec.Z);
		pc = FVector(-width / 2.0f, height / 2.0f, n + s_tmpVec.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	case 4: 
		GEngine->AddOnScreenDebugMessage(45, 2, FColor::Emerald, FString::Printf(TEXT("Method: tilt z + move x/y")));
		pa = FVector(-width / 2.0f + s_tmpVec.X, -height / 2.0f + s_tmpVec.Y, n - s_tmpVec.Z);
		pb = FVector(width / 2.0f + s_tmpVec.X, -height / 2.0f + s_tmpVec.Y, n - s_tmpVec.Z);
		pc = FVector(-width / 2.0f + s_tmpVec.X, height / 2.0f + s_tmpVec.Y, n + s_tmpVec.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	case 5: 
		GEngine->AddOnScreenDebugMessage(200, 2, FColor::Emerald, FString::Printf(TEXT("Method: nothing; default calculation")));
		pa = FVector(-width / 2.0f, -height / 2.0f, n);
		pb = FVector(width / 2.0f, -height / 2.0f, n);
		pc = FVector(-width / 2.0f, height / 2.0f, n);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	case 6:
		GEngine->AddOnScreenDebugMessage(200, 2, FColor::Emerald, FString::Printf(TEXT("Method: nothing; cubes as values")));
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	default: //nothing
		GEngine->AddOnScreenDebugMessage(200, 2, FColor::Emerald, FString::Printf(TEXT("Method: nothing")));
		
		pa = FVector(-width / 2.0f, -height / 2.0f, n);
		pb = FVector(width / 2.0f, -height / 2.0f, n);
		pc = FVector(-width / 2.0f, height / 2.0f, n);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	}
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

	// Rotate the projection to be non-perpendicular. 
	// This is currently unused until the screen is used.
	FMatrix M;
	M.SetIdentity();
	M.M[0][0] = vr.X; M.M[0][1] = vr.Y; M.M[0][2] = vr.Z;
	M.M[1][0] = vu.X; M.M[1][1] = vu.Y; M.M[1][2] = vu.Z;
	M.M[2][0] = vn.X; M.M[2][1] = vn.Y; M.M[2][2] = vn.Z;
	M.M[3][3] = 1.0f;

	if (s_ShowDebugMessages)
	{
		GEngine->AddOnScreenDebugMessage(10, 2, FColor::Orange, FString::Printf(TEXT("pa: %s"), *pa.ToString()));
		GEngine->AddOnScreenDebugMessage(20, 2, FColor::Orange, FString::Printf(TEXT("pb: %s"), *pb.ToString()));
		GEngine->AddOnScreenDebugMessage(31, 2, FColor::Orange, FString::Printf(TEXT("pc: %s"), *pc.ToString()));
		GEngine->AddOnScreenDebugMessage(40, 2, FColor::Orange, FString::Printf(TEXT("pe: %s"), *pe.ToString()));
// 		GEngine->AddOnScreenDebugMessage(50, 2, FColor::Black, FString::Printf(TEXT("vr: %s"), *vu.ToString()));
// 		GEngine->AddOnScreenDebugMessage(60, 2, FColor::Black, FString::Printf(TEXT("vu: %s"), *vr.ToString()));
// 		GEngine->AddOnScreenDebugMessage(70, 2, FColor::Black, FString::Printf(TEXT("vn: %s"), *vn.ToString()));
//		GEngine->AddOnScreenDebugMessage(80, 4, FColor::Red, FString::Printf(TEXT("Frustum: %f \t %f \t %f \t %f \t %f \t %f \t "), l, r, b, t, n, f));
		GEngine->AddOnScreenDebugMessage(90, 2, FColor::Orange, FString::Printf(TEXT("Eye-Screen-Distance: %f"), d));
		GEngine->AddOnScreenDebugMessage(100, 4, FColor::Orange, FString::Printf(TEXT("nd: %f"), nd));
	}

	//Move the apex of the frustum to the origin.
	//result = FTranslationMatrix(-eyePosition) *  result;// * M.GetTransposed();#

	result = FTranslationMatrix(-eyePosition) *  result;
	
	//scales matrix for UE4 and RHI
	result *= 1.0f / result.M[0][0];

	result.M[2][2] = 0.f; //?
	result.M[3][2] = n; //?

	return result;
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon, EStereoscopicPass _PassType)
{
	FVector adjustedEyePositionForS3D = _eyeRelativePositon;
	switch (_PassType)
	{
	case eSSP_FULL:
		break;
	case eSSP_LEFT_EYE:
		adjustedEyePositionForS3D += FVector(s_EyeOffsetVal, 0.f, 0.f);
		break;
	case eSSP_RIGHT_EYE:
		adjustedEyePositionForS3D -= FVector(s_EyeOffsetVal, 0.f, 0.f);
		break;
	case eSSP_MONOSCOPIC_EYE:
		break;
	default:
		break;
	}

	switch (s_OffAxisMethod)
	{
	case EOffAxisMethod::Fast:
		return GenerateOffAxisMatrix_Internal_Fast(_eyeRelativePositon);
	case EOffAxisMethod::Slow:
		return GenerateOffAxisMatrix_Internal_Slow(_screenWidth, _screenHeight, _eyeRelativePositon);
	case EOffAxisMethod::Test:
		return GenerateOffAxisMatrix_Internal_Test(_screenWidth, _screenHeight, _eyeRelativePositon);
	default:
		return GenerateOffAxisMatrix_Internal_Fast(_eyeRelativePositon);
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

float UOffAxisLocalPlayer::SetWidth(float _width)
{
	s_Width = _width;
	return s_Width;
}

float UOffAxisLocalPlayer::SetHeight(float _height)
{
	s_Height = _height;
	return s_Height;
}

EOffAxisMethod UOffAxisLocalPlayer::ToggleOffAxisMethod()
{

	switch (s_OffAxisMethod)
	{
	case EOffAxisMethod::Fast:
		s_OffAxisMethod = EOffAxisMethod::Slow;
		break;
	case EOffAxisMethod::Slow:
		s_OffAxisMethod = EOffAxisMethod::Test;
		break;
	case EOffAxisMethod::Test:
		s_OffAxisMethod = EOffAxisMethod::Fast;
		break;
	default:
		s_OffAxisMethod = EOffAxisMethod::Fast;
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
	s_tmpRot = _newVal;
	return s_tmpRot;
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
		GEngine->AddOnScreenDebugMessage(40, 2, FColor::Cyan, FString::Printf(TEXT("EyeDistance: %f"), 2 * s_EyeOffsetVal));
	return s_EyeOffsetVal;
}

float UOffAxisLocalPlayer::UpdateProjectionPlaneOffsetForStereo(float _newVal)
{
	s_ProjectionPlaneOffset += _newVal;
	if (s_ShowDebugMessages)
		GEngine->AddOnScreenDebugMessage(50, 2, FColor::Cyan, FString::Printf(TEXT("ProjectionPlaneOffset: %f"), s_ProjectionPlaneOffset));
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

int UOffAxisLocalPlayer::SetTest1(int _newVal)
{
	s_test1 = _newVal;
	//GEngine->AddOnScreenDebugMessage(155, 2, FColor::Red, FString::Printf(TEXT("New : %i"), s_test1));
	return s_test1;
}

void UOffAxisLocalPlayer::SetPx(bool _setpa, FVector _pa, bool _setpb, FVector _pb, bool _setpc, FVector _pc)
{
	GEngine->AddOnScreenDebugMessage(10, 2, FColor::Blue, FString::Printf(TEXT("_pa: %s"), *_pa.ToString()));

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

	GEngine->AddOnScreenDebugMessage(100, 2, FColor::Blue, FString::Printf(TEXT("pa: %s"), *pa.ToString()));

}

/************************************************************************/
/* PICKING                                                              */
/************************************************************************/

bool UOffAxisLocalPlayer::OffAxisDeprojectScreenToWorld(APlayerController const* Player, const FVector2D& ScreenPosition, FVector& WorldPosition, FVector& WorldDirection)
{
	ULocalPlayer* const LP = Player ? Player->GetLocalPlayer() : nullptr;
	
	if (LP && LP->ViewportClient)
	{
		// get the projection data
		FSceneViewProjectionData ProjectionData;
		
		if (LP->GetProjectionData(LP->ViewportClient->Viewport, eSSP_FULL, /*out*/ ProjectionData))
		{
			ProjectionData.ProjectionMatrix = s_ProjectionMatrix;
			s_ProjectionMatrix = FTranslationMatrix(-ProjectionData.ViewOrigin) * ProjectionData.ViewRotationMatrix * s_ProjectionMatrix;
			FMatrix const InvViewProjMatrix = s_ProjectionMatrix.InverseFast();

			FSceneView::DeprojectScreenToWorld(ScreenPosition, ProjectionData.GetConstrainedViewRect(), InvViewProjMatrix, /*out*/ WorldPosition, /*out*/ WorldDirection);
			return true;
		}
	}

	// something went wrong, zero things and return false
	WorldPosition = FVector::ZeroVector;
	WorldDirection = FVector::ZeroVector;
	return false;
}

bool UOffAxisLocalPlayer::OffAxisDeprojectScreenToWorld(APlayerController const* Player, FVector& WorldPosition, FVector& WorldDirection)
{
	float x, y;
	Player->GetMousePosition(x, y);
	return OffAxisDeprojectScreenToWorld(Player, FVector2D(x,y), WorldPosition, WorldDirection);
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
	//transform eyeRelativePosition to UE4 coordinates
	FVector _eyeRelativePositioninUE4Coord = FVector(_eyeRelativePosition.Z, _eyeRelativePosition.X, _eyeRelativePosition.Y);

	//get end position for ray trace
	FVector WorldPosition, WorldDirection;
	OffAxisDeprojectScreenToWorld(UGameplayStatics::GetPlayerController(WorldContextObject, 0), WorldPosition, WorldDirection);
	FVector end = WorldPosition + _LengthOfRay * WorldDirection;

	if (bDrawDebugLine)
	{
		DrawDebugLine(WorldContextObject->GetWorld(), _eyeRelativePositioninUE4Coord, end, _color, bPersistentLines, _lifeTime, _depthPriority, _thickness);

		UE_LOG(OffAxisLog, Log, TEXT("Start: %s"), *_eyeRelativePosition.ToString());
		UE_LOG(OffAxisLog, Log, TEXT("End  : %s"), *end.ToString());
		
		GEngine->AddOnScreenDebugMessage(300, 10, FColor::Cyan, FString::Printf(TEXT("Start: %s"), *_eyeRelativePosition.ToString()));
		GEngine->AddOnScreenDebugMessage(310, 10, FColor::Cyan, FString::Printf(TEXT("End: %s"), *end.ToString()));
	}
	
	//do raytrace
	return WorldContextObject->GetWorld()->LineTraceSingleByChannel(OutHit, _eyeRelativePositioninUE4Coord, end, ECollisionChannel::ECC_Visibility);

}

