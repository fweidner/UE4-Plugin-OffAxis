# UE4-Plugin-OffAxis

![OffAxisExample](https://github.com/fweidner/UE4-Plugin-OffAxis/blob/master/2018-06-13.gif)

## Current version
Works with 4.22 :)

## General info: 
It works. But it is (always) experimental :)

* OffAxisProjection / FishTank VR
* in 2D Mode: OffAxisActor can be placed arbitrarily.
* in 3D Mode: OffAxisActor can be placed parallel to the Z-Y-Plane and rotated using pitch and roll.

## How to use:
Assuming you start with a Basic Code C++ project:
- Update LocalPlayer class in Edit->Project Settings->General Settings 
- Adjust Input mappings to your Config/DefaultInput.ini
- Drag the OffAxisActor in your scene
- Set position of OffAxisActor to 0,0,0
- In the OffAxisActor, place the child actors P_a, P_b, and P_c so that the correspond to the corners of your "virtual window".
- Change your NearClippingPlane .1f 

## Input Mappings
Restart after updating DefaultInput.ini!

+ActionMappings=(ActionName="HomePosReset",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=End)
+ActionMappings=(ActionName="ToggleOffAxisMethod",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Y)
+ActionMappings=(ActionName="ResetEyeOffset",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Three)
+ActionMappings=(ActionName="StartOptitrack",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Four)
+ActionMappings=(ActionName="UseOptitrack",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Eight)
+ActionMappings=(ActionName="ToggleUseOffAxis",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=P)
+ActionMappings=(ActionName="ResetProjectionPlaneOffset",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Seven)
+ActionMappings=(ActionName="Quit",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Escape)
+ActionMappings=(ActionName="ConnectToOptitrack",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=I)
+ActionMappings=(ActionName="UseSteamVR",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Nine)
+ActionMappings=(ActionName="UseNoneTracking",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Zero)
+ActionMappings=(ActionName="Select",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=LeftMouseButton)
+ActionMappings=(ActionName="ResetPickableObjects",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=None)
+AxisMappings=(AxisName="EyeUp",Scale=1.500000,Key=Up)
+AxisMappings=(AxisName="EyeUp",Scale=-1.500000,Key=Down)
+AxisMappings=(AxisName="EyeLeft",Scale=-1.500000,Key=Left)
+AxisMappings=(AxisName="EyeLeft",Scale=1.500000,Key=Right)
+AxisMappings=(AxisName="EyeForward",Scale=1.000000,Key=PageUp)
+AxisMappings=(AxisName="EyeForward",Scale=-1.000000,Key=PageDown)
+AxisMappings=(AxisName="ChangeEyeOffset",Scale=0.100000,Key=One)
+AxisMappings=(AxisName="ChangeEyeOffset",Scale=-0.100000,Key=Two)
+AxisMappings=(AxisName="EyeForward",Scale=0.100000,Key=NumPadEight)
+AxisMappings=(AxisName="EyeForward",Scale=-0.100000,Key=NumPadTwo)
+AxisMappings=(AxisName="ChangeProjectionPlaneOffset",Scale=0.100000,Key=Five)
+AxisMappings=(AxisName="ChangeProjectionPlaneOffset",Scale=-0.100000,Key=Six)
+AxisMappings=(AxisName="MouseX",Scale=1.000000,Key=MouseX)
+AxisMappings=(AxisName="MouseY",Scale=1.000000,Key=MouseY)
+AxisMappings=(AxisName="Wheel",Scale=1.000000,Key=MouseWheelAxis)
+AxisMappings=(AxisName="Pick_away",Scale=0.100000,Key=W)
+AxisMappings=(AxisName="Pick_left",Scale=-0.100000,Key=A)
+AxisMappings=(AxisName="Pick_down",Scale=0.100000,Key=Q)
+AxisMappings=(AxisName="Pick_away",Scale=-0.100000,Key=S)
+AxisMappings=(AxisName="Pick_left",Scale=0.100000,Key=D)
+AxisMappings=(AxisName="Pick_down",Scale=-0.100000,Key=E)
DefaultTouchInterface=/Engine/MobileResources/HUD/DefaultVirtualJoysticks.DefaultVirtualJoysticks

## Based on...

http://nttl.ru/en

https://perspectiveresources.blogspot.de/2013/04/i3d-head-coupled-perspective.html

http://iihm.imag.fr/en/demo/hcpmobile/

https://www.youtube.com/watch?v=hvrT7FqpPQE

https://www.youtube.com/watch?v=-foNLFnrNRc

https://www.youtube.com/watch?v=PP38yj3zdqo

https://www.youtube.com/watch?v=5ibPBGCAWKo

http://csc.lsu.edu/~kooima/articles/genperspective/

Michael Deering. 1992. High resolution virtual reality. ACM SIGGRAPH Computer Graphics 26, 2: 195–202. https://doi.org/10.1145/142920.134039

https://answers.unrealengine.com/questions/65003/howto-modify-the-projection-matrix.html
