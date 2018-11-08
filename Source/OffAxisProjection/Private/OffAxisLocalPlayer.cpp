// Fill out your copyright notice in the Description page of Project Settings.

#include "OffAxisLocalPlayer.h"

#include "Engine.h"


#include "Runtime/Core/Public/GenericPlatform/GenericPlatformTime.h" // for FPlatformTime


FSceneView * UOffAxisLocalPlayer::CalcSceneView(FSceneViewFamily * ViewFamily, FVector & OutViewLocation, FRotator & OutViewRotation, FViewport * Viewport, FViewElementDrawer * ViewDrawer, EStereoscopicPass StereoPass)
{
	double start = FPlatformTime::Seconds();

	FSceneView* tmp = ULocalPlayer::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer, StereoPass);

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
	Result.M[2][0] = (right + left) / (right - left);
	Result.M[2][1] = (top + bottom) / (top - bottom);
	Result.M[2][2] = -(farVal + nearVal) / (farVal - nearVal);
	Result.M[2][3] = -1.0f;
	Result.M[3][2] = -(2.0f * farVal * nearVal) / (farVal - nearVal);
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

	FMatrix axisChanger; //rotates everything to UE4 coordinate system.
	axisChanger.SetIdentity();
	axisChanger.M[0][0] = 0.0f;
	axisChanger.M[1][1] = 0.0f;
	axisChanger.M[2][2] = 0.0f;
	axisChanger.M[0][2] = 1.0f;
	axisChanger.M[1][0] = 1.0f;
	axisChanger.M[2][1] = 1.0f;

	View->ProjectionMatrixUnadjustedForRHI = View->ViewMatrices.GetViewMatrix().Inverse() * axisChanger * stereoProjectionMatrix;

	View->UpdateProjectionMatrix(View->ViewMatrices.GetViewMatrix().Inverse() * axisChanger * stereoProjectionMatrix);

	View->UpdateViewMatrix();
}

void UOffAxisLocalPlayer::InitOffAxisProjection_Fast(float _screenWidth, float _screenHeight)
{
	TopLeftCorner_ = FVector(-_screenWidth / 2.f, -_screenHeight / 2.f, GNearClippingPlane);
	BottomRightCorner_ = FVector(_screenWidth / 2.f, _screenHeight / 2.f, GNearClippingPlane);


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
	FVector eyeToTopLeft = TopLeftCorner_ - _eyeRelativePositon;
	FVector eyeToTopLeftNear = GNearClippingPlane / eyeToTopLeft.Z * eyeToTopLeft;

	FVector eyeToBottomRight = BottomRightCorner_ - _eyeRelativePositon;
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

	FVector pa, pb, pc, pe;
	/*
	 *pa=(-180,	-110,	0.1)
	 *pb=(180,	-110,	0.1)
	 *pc=(-180,	89,		0.1)
	 */

	switch (i)
	{
	case 0: // everything
		GEngine->AddOnScreenDebugMessage(150, 2, FColor::Red, FString::Printf(TEXT("Using I = 0")));
		pa = FVector(-width / 2.0f + s_tmp.X, -height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pb = FVector(width / 2.0f + s_tmp.X, -height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pc = FVector(-width / 2.0f + s_tmp.X, height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pe = FVector(eyePosition.X + s_tmp.X, eyePosition.Y + s_tmp.Y, eyePosition.Z + s_tmp.Z);
		break;
	case 1: // only z uni
		GEngine->AddOnScreenDebugMessage(150, 2, FColor::Red, FString::Printf(TEXT("Using I = 1")));
		pa = FVector(-width / 2.0f + s_tmp.X, -height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pb = FVector(width / 2.0f + s_tmp.X, -height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pc = FVector(-width / 2.0f + s_tmp.X, height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	case 2: //only pc
		GEngine->AddOnScreenDebugMessage(150, 2, FColor::Red, FString::Printf(TEXT("Using I = 2")));
		pa = FVector(-width / 2.0f, -height / 2.0f, n);
		pb = FVector(width / 2.0f, -height / 2.0f, n);
		pc = FVector(-width / 2.0f + s_tmp.X, height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
	case 3: //only z opp
		GEngine->AddOnScreenDebugMessage(150, 2, FColor::Red, FString::Printf(TEXT("Using I = 2")));
		pa = FVector(-width / 2.0f, -height / 2.0f, n - s_tmp.Z);
		pb = FVector(width / 2.0f, -height / 2.0f, n - s_tmp.Z);
		pc = FVector(-width / 2.0f, height / 2.0f, n + s_tmp.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	case 4: //only tilt z + move x/y
		GEngine->AddOnScreenDebugMessage(150, 2, FColor::Red, FString::Printf(TEXT("Using I = 2")));
		pa = FVector(-width / 2.0f + s_tmp.X, -height / 2.0f + s_tmp.Y, n - s_tmp.Z);
		pb = FVector(width / 2.0f + s_tmp.X, -height / 2.0f + s_tmp.Y, n - s_tmp.Z);
		pc = FVector(-width / 2.0f + s_tmp.X, height / 2.0f + s_tmp.Y, n + s_tmp.Z);
		pe = FVector(eyePosition.X, eyePosition.Y, eyePosition.Z);
		break;
	default: //nothing
		GEngine->AddOnScreenDebugMessage(150, 2, FColor::Red, FString::Printf(TEXT("Using I = ?")));
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
		GEngine->AddOnScreenDebugMessage(10, 2, FColor::Orange, FString::Printf(TEXT("pa: %s"), *pa.ToString()));
		GEngine->AddOnScreenDebugMessage(20, 2, FColor::Orange, FString::Printf(TEXT("pb: %s"), *pb.ToString()));
		GEngine->AddOnScreenDebugMessage(31, 2, FColor::Orange, FString::Printf(TEXT("pc: %s"), *pc.ToString()));
		GEngine->AddOnScreenDebugMessage(40, 2, FColor::Orange, FString::Printf(TEXT("pe: %s"), *pe.ToString()));
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
//	result.M[3][2] = n; //?

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
	s_tmp = _newVal;
	return s_tmp;
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

int UOffAxisLocalPlayer::SetI(int _newVal)
{
	i = _newVal;
	GEngine->AddOnScreenDebugMessage(150, 2, FColor::Red, FString::Printf(TEXT("New I: %i"), i));
	return i;
}
