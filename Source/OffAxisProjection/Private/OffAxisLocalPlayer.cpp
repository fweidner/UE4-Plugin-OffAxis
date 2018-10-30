// Fill out your copyright notice in the Description page of Project Settings.

#include "OffAxisLocalPlayer.h"

#include "Engine.h"



FSceneView * UOffAxisLocalPlayer::CalcSceneView(FSceneViewFamily * ViewFamily, FVector & OutViewLocation, FRotator & OutViewRotation, FViewport * Viewport, FViewElementDrawer * ViewDrawer, EStereoscopicPass StereoPass)
{
	FSceneView* tmp = ULocalPlayer::CalcSceneView(ViewFamily, OutViewLocation, OutViewRotation, Viewport, ViewDrawer, StereoPass);

	if (s_bUseoffAxis && mOffAxisMatrixSetted)
	{
		SetOffAxisMatrix(GenerateOffAxisMatrix(s_Width, s_Height, s_EyePosition, StereoPass));
		UpdateProjectionMatrix(tmp, mOffAxisMatrix, StereoPass);
	}

	GEngine->AddOnScreenDebugMessage(200, 4, FColor::Red, FString::Printf(TEXT("CalcSceneViewLocalPlayer")));

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

void UOffAxisLocalPlayer::SetOffAxisMatrix(FMatrix OffAxisMatrix)
{
	mOffAxisMatrixSetted = true;
	mOffAxisMatrix = OffAxisMatrix;
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

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix_Internal(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon)
{
	FMatrix result;

	float width = _screenWidth;
	float height = _screenHeight;
	FVector eyePosition = _eyeRelativePositon;

	float l, r, b, t, n, f, nd;

	n = GNearClippingPlane;
	f = 10000.f;

	FMatrix matFlipZ;
	matFlipZ.SetIdentity();

	//FMatrix OffAxisProjectionMatrix;

	if (s_OffAxisVersion == 0)
	{
		FVector topLeftCorner(-width / 2.f, -height / 2.f, n);
		FVector bottomRightCorner(width / 2.f, height / 2.f, n);

		FVector eyeToTopLeft = topLeftCorner - _eyeRelativePositon;
		FVector eyeToTopLeftNear = n / eyeToTopLeft.Z * eyeToTopLeft;
		FVector eyeToBottomRight = bottomRightCorner - _eyeRelativePositon;
		FVector eyeToBottomRightNear = eyeToBottomRight / eyeToBottomRight.Z * n;

		l = -eyeToTopLeftNear.X;
		r = -eyeToBottomRightNear.X;
		t = -eyeToBottomRightNear.Y;
		b = -eyeToTopLeftNear.Y;


		//Frustum: l, r, b, t, near, far
		result = FrustumMatrix(l, r, b, t, n, f);

	}
	else
	{
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


	}

	// Move the apex of the frustum to the origin.
	result = FTranslationMatrix(-eyePosition) * result;
	//GEngine->AddOnScreenDebugMessage(41, 2, FColor::Red, FString::Printf(TEXT("FrustumMatrix_MOV: %s"), *result.ToString()));

	//scales matrix for UE4 and RHI
	result *= 1.0f / result.M[0][0];
	//GEngine->AddOnScreenDebugMessage(42, 2, FColor::Red, FString::Printf(TEXT("FrustumMatrix_DIV: %s"), *result.ToString()));

	result.M[2][2] = 0.f; //?
	result.M[3][2] = n; //?

	//GEngine->AddOnScreenDebugMessage(49, 2, FColor::Red, FString::Printf(TEXT("FrustumMatrix_MOD : %s"), *result.ToString()));

	return result;
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon)
{
	return GenerateOffAxisMatrix_Internal(_screenWidth, _screenHeight, _eyeRelativePositon);
}

FMatrix UOffAxisLocalPlayer::GenerateOffAxisMatrix(float _screenWidth, float _screenHeight, FVector _eyeRelativePositon, EStereoscopicPass _PassType)
{
	FVector tmpeye = _eyeRelativePositon;
	switch (_PassType)
	{
	case eSSP_FULL:
		break;
	case eSSP_LEFT_EYE:
		tmpeye += FVector(s_EyeOffsetVal, 0.f, 0.f);
		break;
	case eSSP_RIGHT_EYE:
		tmpeye -= FVector(s_EyeOffsetVal, 0.f, 0.f);
		break;
	case eSSP_MONOSCOPIC_EYE:
		break;
	default:
		break;
	}


	return GenerateOffAxisMatrix(_screenWidth, _screenHeight, tmpeye);
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