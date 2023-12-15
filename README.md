# UE4-Plugin-OffAxis

![OffAxisExample](https://github.com/fweidner/UE4-Plugin-OffAxis/blob/master/2018-06-13.gif)

## Current version
- Works with 4.23 :)
- check out the branch "UE5" with info on how to run it for UE5

## General info: 
It works. But it is (always) experimental :)

* OffAxisProjection / FishTank VR
  * in 2D Mode: OffAxisActor can be placed arbitrarily.
  * in 3D Mode: OffAxisActor can be translated in x,y, z and rotated using pitch and roll.
* Also shows a possibility of how to modify the projection matrix. 

## How to use:
Assuming you start with a new project:
- If your project is a C++ project, you are good to go. If you have a pure blueprint project, add a random C++ class via File -> Add C++ class (empty class is fine).
- Copy the Plugin in your (you might need to create a Plugin folder in the directory where your uproject file is).
- Start (or restart) Unreal Engine. You might get a warning that the plugin needs to be rebuild. That's normal.
- Update LocalPlayer class in Edit->Project Settings->General Settings 
- Adjust Input mappings to your Config/DefaultInput.ini (just copy the below mappins to your Config/DefaultInput.ini. If you don't have one, create a random new mapping in Settings --> Project Settings --> Input. Then the file should be there. Then copy the mappings. Restart UE4.
- Drag the OffAxisActor in your scene
- Set position of OffAxisActor to 0,0,0
- In the OffAxisActor, place the child actors P_a, P_b, and P_c so that the correspond to the corners of your "virtual window".
- Change your NearClippingPlane .1f (Settings --> Project Settings --> Engine --> General Settings --> Near Clip Plane)
- Select the OffAxisActor in your Project Outliner: set "Use Off Axis on Start" to true and change "Tracking Device" to None (the Set Tracking Device Setting is still in early stages).
- Hit play. If it is not working now, close UE4, right-click on your uproject file and regenerate Visual Studio Project Files, try again :)
- If it starts but the screen is black, try to either move the STartPositionEye of the OffAxisActor along X or (if you have copied the input mappings) play around with pageUp and pageDown.

## Input Mappings
* Here are the current input mappings the plugin supports/uses. Just copy and paste this bunch in your DefaultInput.ini and restart after updating DefaultInput.ini!
* For some inputs, you might need to change the key if they are already in use in your project.

### OffAxis
+ActionMappings=(ActionName="HomePosReset",bShift=True,bCtrl=False,bAlt=False,bCmd=False,Key=Home)
+ActionMappings=(ActionName="ResetEyeOffset",bShift=True,bCtrl=False,bAlt=False,bCmd=False,Key=Nine)
+ActionMappings=(ActionName="ResetProjectionPlaneOffset",bShift=True,bCtrl=False,bAlt=False,bCmd=False,Key=Six)
+ActionMappings=(ActionName="ToggleShowDebugMessages",bShift=True,bCtrl=False,bAlt=False,bCmd=False,Key=V)
+ActionMappings=(ActionName="ToggleUseOffAxis",bShift=True,bCtrl=False,bAlt=False,bCmd=False,Key=P)
+ActionMappings=(ActionName="Pick",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=LeftMouseButton)
+ActionMappings=(ActionName="ToggleVisOffAxisMenu",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=F9)
+AxisMappings=(AxisName="EyeUp",Scale=1.500000,Key=Up)
+AxisMappings=(AxisName="EyeUp",Scale=-1.500000,Key=Down)
+AxisMappings=(AxisName="EyeLeft",Scale=-1.500000,Key=Left)
+AxisMappings=(AxisName="EyeLeft",Scale=1.500000,Key=Right)
+AxisMappings=(AxisName="EyeForward",Scale=1.000000,Key=PageUp)
+AxisMappings=(AxisName="EyeForward",Scale=-1.000000,Key=PageDown)
+AxisMappings=(AxisName="Pick_away",Scale=0.100000,Key=W)
+AxisMappings=(AxisName="Pick_left",Scale=-0.100000,Key=A)
+AxisMappings=(AxisName="Pick_down",Scale=0.100000,Key=Q)
+AxisMappings=(AxisName="Pick_away",Scale=-0.100000,Key=S)
+AxisMappings=(AxisName="Pick_left",Scale=0.100000,Key=D)
+AxisMappings=(AxisName="Pick_down",Scale=-0.100000,Key=E)
+AxisMappings=(AxisName="Wheel",Scale=1.000000,Key=MouseWheelAxis)
+AxisMappings=(AxisName="MouseX",Scale=1.000000,Key=MouseX)
+AxisMappings=(AxisName="MouseY",Scale=1.000000,Key=MouseY)
+AxisMappings=(AxisName="ChangeProjectionPlaneOffset",Scale=0.100000,Key=Five)
+AxisMappings=(AxisName="ChangeProjectionPlaneOffset",Scale=-0.100000,Key=Six)
+AxisMappings=(AxisName="MoveUp",Scale=0.100000,Key=I)
+AxisMappings=(AxisName="MoveDown",Scale=-0.100000,Key=K)
+AxisMappings=(AxisName="MoveLeft",Scale=-0.100000,Key=J)
+AxisMappings=(AxisName="MoveRight",Scale=0.100000,Key=L)
+AxisMappings=(AxisName="Away",Scale=0.100000,Key=U)
+AxisMappings=(AxisName="Towards",Scale=-0.100000,Key=O)
+AxisMappings=(AxisName="ChangeEyeOffset",Scale=0.001000,Key=Seven)
+AxisMappings=(AxisName="ChangeEyeOffset",Scale=-0.010000,Key=Eight)

### Optitrack related:
+ActionMappings=(ActionName="TryToConnectToServer",bShift=True,bCtrl=False,bAlt=False,bCmd=False,Key=One)
+ActionMappings=(ActionName="ConnectToOptitrack",bShift=True,bCtrl=False,bAlt=False,bCmd=False,Key=Y)

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
