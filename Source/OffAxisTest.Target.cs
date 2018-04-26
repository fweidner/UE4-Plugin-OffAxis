// Fill out your copyright notice in the Description page of Project Settings.

using UnrealBuildTool;
using System.Collections.Generic;

public class OffAxisTestTarget : TargetRules
{
	public OffAxisTestTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Game;
		ExtraModuleNames.Add("OffAxisTest");

	}

	//
	// TargetRules interface.
	//
}
