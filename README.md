# UE4-Plugin-OffAxis

![OffAxisExample](https://github.com/fweidner/UE4-Plugin-OffAxis/blob/master/2018-06-13.gif)

## Current version
Works with 4.21 :)

## General info: 



http://nttl.ru/en

https://perspectiveresources.blogspot.de/2013/04/i3d-head-coupled-perspective.html

http://iihm.imag.fr/en/demo/hcpmobile/

https://www.youtube.com/watch?v=hvrT7FqpPQE

https://www.youtube.com/watch?v=-foNLFnrNRc

https://www.youtube.com/watch?v=PP38yj3zdqo

https://www.youtube.com/watch?v=5ibPBGCAWKo

http://csc.lsu.edu/~kooima/articles/genperspective/

Michael Deering. 1992. High resolution virtual reality. ACM SIGGRAPH Computer Graphics 26, 2: 195–202. https://doi.org/10.1145/142920.134039

## Based on...
https://answers.unrealengine.com/questions/65003/howto-modify-the-projection-matrix.html

## How to use:
Assuming you start with a Basic Code C++ project:
- Update LocalPlayer class in Edit->Project Settings->General Settings 
- Adjust Input mappings to your Config/DefaultInput.ini
- Drag the OffAxisActor in your scene
- Set position of OffAxisActor to 0,0,0: This is what you see.
- Change your NearClippingPlane .1f 

### Ok, here's the catch...
The cube of the OffAxisActor does not specifiy the extents of the position of your FishTank Virtual Reality. It specifies just the height and width. Currently, the origin of the FishTankVR/OffAxisProjection is always at (0,0,0) with the size of width/height specified by the cube ExtentPoint.

### Some explanations...
When you select the OffAxisActor in the World Outliner, There are two new categories: "OffAxis" and "Tracking Optitrack".
* "OffAxis", 
** You can specify the tracking device (None uses keyboard input. Optitrack works with my Optitrack plugin. SteamVR is just a wrapper and does nothing at the moment)
** You can set the default player start position. That specifies the position where the Unreal Engine camera is located on startup.
* "Tracking Optitrack",
** You can specify the coordinates OptiX, OptiY, OptiZ. They specify the the start position of the player with respect to the virtual screen (in the default version: 2.8m in front of the screen and 1.2m above the ground). They also form the variable OptiTransform. Basically: (OptiX,OptiY,OptiZ) shifts the origin of the tracking system to the origin of the UE4 application. If you need additional shifts to have relative transforms, ReferenceTransformTwo does another shift.
** The variable "Head Name" specifies the name of the tracking target in Motive that represents the head.

## Input Mappings
Restart after updating DefaultInput.ini!

+ActionMappings=(ActionName="HomePosReset",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=End)
+ActionMappings=(ActionName="ToggleOffAxisMethod",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=SpaceBar)
+ActionMappings=(ActionName="ResetEyeOffset",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Three)
+ActionMappings=(ActionName="StartOptitrack",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Four)
+ActionMappings=(ActionName="UseOptitrack",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Eight)
+ActionMappings=(ActionName="ToggleUseOffAxis",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=P)
+ActionMappings=(ActionName="ResetProjectionPlaneOffset",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Seven)
+ActionMappings=(ActionName="Quit",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Escape)
+ActionMappings=(ActionName="ConnectToOptitrack",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=I)
+ActionMappings=(ActionName="UseSteamVR",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Nine)
+ActionMappings=(ActionName="UseNoneTracking",bShift=False,bCtrl=False,bAlt=False,bCmd=False,Key=Zero)
+AxisMappings=(AxisName="Up",Scale=1.500000,Key=Up)
+AxisMappings=(AxisName="Up",Scale=-1.500000,Key=Down)
+AxisMappings=(AxisName="Left",Scale=-1.500000,Key=Left)
+AxisMappings=(AxisName="Left",Scale=1.500000,Key=Right)
+AxisMappings=(AxisName="Forward",Scale=1.000000,Key=PageUp)
+AxisMappings=(AxisName="Forward",Scale=-1.000000,Key=PageDown)
+AxisMappings=(AxisName="ChangeEyeOffset",Scale=0.100000,Key=One)
+AxisMappings=(AxisName="ChangeEyeOffset",Scale=-0.100000,Key=Two)
+AxisMappings=(AxisName="TmpZUp",Scale=0.500000,Key=W)
+AxisMappings=(AxisName="TmpZUp",Scale=-0.500000,Key=S)
+AxisMappings=(AxisName="Forward",Scale=0.100000,Key=NumPadEight)
+AxisMappings=(AxisName="Forward",Scale=-0.100000,Key=NumPadTwo)
+AxisMappings=(AxisName="ChangeProjectionPlaneOffset",Scale=0.100000,Key=Five)
+AxisMappings=(AxisName="ChangeProjectionPlaneOffset",Scale=-0.100000,Key=Six)

## Tips
* Show > Advanced > Camera Frustums
* Works with "-emulatestereo"
