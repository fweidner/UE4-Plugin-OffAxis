// Fill out your copyright notice in the Description page of Project Settings.

#include "OffAxisLocalPlayer.h"

#include "Engine.h"


#include "Runtime/Core/Public/GenericPlatform/GenericPlatformTime.h" // for FPlatformTime


FSceneView * UOffAxisLocalPlayer::CalcSceneView(FSceneViewFamily * ViewFamily, FVector & OutViewLocation, FRotator & OutViewRotation, FViewport * Viewport, FViewElementDrawer * ViewDrawer, EStereoscopicPass StereoPass)
{
	double start = FPlatformTime::Seconds();

	FSceneView* tmp = ULocalPlayer::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer, StereoPass);

	if (s_bUseoffAxis)
	{
		UpdateProjectionMatrix(tmp, GenerateOffAxisMatrix(s_Width, s_Height, s_EyePosition, StereoPass), StereoPass);
	}

	GEngine->AddOnScreenDebugMessage(200, 4, FColor::Red, FString::Printf(TEXT("CalcSceneViewLocalPlayer")));

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

void UOffAxisLocalPlayer::UpdateProjectionMatrix(FSceneView* View, FMatrix OffAxisMatrix, EStereoscopicPass _Pass)
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

}

FMatrix UOffAxisLocalPlayer::_AdjustProjectionMatrixForRHI(const FMatrix& InProjectionMatrix)
{
	const float GMinClipZ = GNearClippingPlane;
	const float GProjectionSignY = 1.0f;

	FScaleMatrix ClipSpaceFixScale(FVector(1.0f, GProjectionSignY, 1.0f - GMinClipZ));
	FTranslationMatrix ClipSpaceFixTranslate(FVector(0.0f, 0.0f, GMinClipZ));
	return InProjectionMatrix * ClipSpaceFixScale * ClipSpaceFixTranslate;
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
	
	
// 	FMatrix result = Frustum;
// 	Frustum.M[0][0] = (2.0f * GNearClippingPlane) / (-eyeToBottomRightNear.X - -eyeToTopLeftNear.X);
// 	Frustum.M[1][1] = (2.0f * GNearClippingPlane) / (-eyeToBottomRightNear.Y - -eyeToTopLeftNear.Y);
// 	Frustum.M[2][0] = (-eyeToBottomRightNear.X + -eyeToTopLeftNear.X) / (-eyeToBottomRightNear.X - -eyeToTopLeftNear.X);
// 	Frustum.M[2][1] = (-eyeToBottomRightNear.Y + -eyeToTopLeftNear.Y) / (-eyeToBottomRightNear.Y - -eyeToTopLeftNear.Y);

	FMatrix result = FrustumMatrix(-eyeToTopLeftNear.X, -eyeToBottomRightNear.X, -eyeToTopLeftNear.Y, -eyeToBottomRightNear.Y, GNearClippingPlane, GFarClippingPlane);
	
	// Move the apex of the frustum to the origin.
	result = FTranslationMatrix(-_eyeRelativePositon) * result;

	//scales matrix for UE4 and RHI
	result *= 1.0f / result.M[0][0];

	result.M[2][2] = 0.f; //?
	result.M[3][2] = GNearClippingPlane; //?

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


	if (s_OffAxisVersion == 0)
	{
		return GenerateOffAxisMatrix_Internal_Fast(adjustedEyePositionForS3D);
	}
	else
	{
		return GenerateOffAxisMatrix_Internal_Slow(_screenWidth, _screenHeight, adjustedEyePositionForS3D);
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

bool UOffAxisLocalPlayer::ToggleOffAxisMethod()
{
	if (s_OffAxisVersion == 0)
	{
		s_OffAxisVersion = 1;
	}
	else
	{
		s_OffAxisVersion = 0;
	}
	PrintCurrentOffAxisVersion();
	return s_OffAxisVersion;
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
	UE_LOG(LogConsoleResponse, Warning, TEXT("OffAxisVersion: %s"), (s_OffAxisVersion ? TEXT("Basic") : TEXT("Optimized"))); //if true (==1) -> basic, else opitmized
	GEngine->AddOnScreenDebugMessage(30, 4, FColor::Cyan, FString::Printf(TEXT("OffAxisVersion: %s"), (s_OffAxisVersion ? TEXT("Basic") : TEXT("Optimized"))));
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