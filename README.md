# UE4-Plugin-OffAxis

![OffAxisExample](https://thumbs.gfycat.com/VastFewFruitfly-max-14mb.gif)




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
- Update viewport class in Edit->Project Settings->General Settings 
- Adjust Input mappings in Config/DefaultInput.ini
- Drag the OffAxisActor in your scene
- Set position of OffAxisActor to 0,0,0: This is what you see.

Ok, here's the catch. The cube of the OffAxisActor do not specifiy the extents of the position of your FishTank Virtual Reality. They specifiy just the height and width. Currently, the origin of the FishTankVR/OffAxisProjection is always at (0,0,0) with the size of width/height specified by the cube ExtentPoint.



##Input Mappings
Restart after updating DefaultInput.ini!

+ActionMappings=(ActionName="HomePosReset",Key=End,bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="ToggleOffAxisMethod",Key=SpaceBar,bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+ActionMappings=(ActionName="ResetEyeOffset",Key=Three,bShift=False,bCtrl=False,bAlt=False,bCmd=False)
+AxisMappings=(AxisName="Up",Key=Up,Scale=1.000000)
+AxisMappings=(AxisName="Up",Key=Down,Scale=-1.000000)
+AxisMappings=(AxisName="Left",Key=Left,Scale=-1.000000)
+AxisMappings=(AxisName="Left",Key=Right,Scale=1.000000)
+AxisMappings=(AxisName="Forward",Key=PageUp,Scale=1.000000)
+AxisMappings=(AxisName="Forward",Key=PageDown,Scale=-1.000000)
+AxisMappings=(AxisName="ChangeEyeOffset",Key=One,Scale=0.100000)
+AxisMappings=(AxisName="ChangeEyeOffset",Key=Two,Scale=-0.100000)

## Tips
* Show > Advanced > Camera Frustums
* Works with "-emulatestereo"

