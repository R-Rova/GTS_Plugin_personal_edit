ScriptName GtsSM_HotKeys Extends Quest

Actor Property PlayerRef Auto

Function GrowPlayer()
  GtsScale.ModTargetScale(PlayerRef, 0.1)
EndFunction

Function ShrinkPlayer()
  GtsScale.ModTargetScale(PlayerRef, -0.1)
EndFunction

Function PlayerGrowthHalfLife(Float rate)
  GtsPlugin.SetGrowthHalfLife(PlayerRef, rate)
EndFunction

Function Experiment(Float value)
  GtsPlugin.SetExperimentFloat(value)
EndFunction


Function GrowFollowers()
  GtsScale.ModTeammateScale(0.1)
EndFunction

Function ShrinkFollowers()
  GtsScale.ModTeammateScale(-0.1)
EndFunction
