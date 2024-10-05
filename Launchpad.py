import os, sys, re

def ImportFiles(Target, RelPath):
  MacroRegex = re.compile(r'!import\s+(\S+)')
  Matches = re.findall(MacroRegex, Target)
  Result = Target
  for Match in Matches:
    RelPath.append(Match)
    SubH = open('/'.join(RelPath), 'r')
    SubS = SubH.read()
    SubH.close()
    Result = re.sub(MacroRegex, SubS, Result)
  return Result

def ReplaceConstants(Target):
  MacroRegex = re.compile(r'!constant\s+([a-zA-Z_]\w*)\s*(\S*)\n')
  Matches = re.findall(MacroRegex, Target)
  Result = re.sub(MacroRegex, '', Target)
  for Match in Matches:
    Result = re.sub(fr'{Match[0]}', Match[1], Result)
  return Result

def ReplaceMacros(Target):
  MacroRegex = re.compile(r'!macro\s+([a-zA-Z_]\w*)((?:\s+[a-zA-Z_]\w*)*)\n([^!]*)!endmacro\n')
  Matches = re.findall(MacroRegex, Target)
  Result = re.sub(MacroRegex, '', Target)
  for Match in Matches:
    if len(Match) > 1:
      ParameterList = Match[1].split()
    ReplaceRegex = re.compile(fr'{Match[0]}((?:\s+[a-zA-Z_]\w*){{{len(ParameterList)}}})\n')
    MacroString = Match[-1]
    for RMatch in re.findall(ReplaceRegex, Result):
      RenamedMacro = MacroString
      if len(ParameterList) > 0:
        Parameters = RMatch.split()
        for i in range(len(Parameters)):
          RenamedMacro = re.sub(fr'{ParameterList[i]}', Parameters[i], RenamedMacro)
      Result = re.sub(ReplaceRegex, RenamedMacro, Result)
  return Result

def RemoveEmptyLines(Target):
  return re.sub(r'\n\n+', '\n', Target)

if __name__ == "__main__":
  for File in sys.argv[1:]:
    Target = open(File, 'r')
    Buffer = Target.read()
    Target.close()
    
    Buffer = ImportFiles(Buffer, File.split('/')[:-1])
    Buffer = ReplaceConstants(Buffer)
    Buffer = ReplaceMacros(Buffer)
    Buffer = RemoveEmptyLines(Buffer)

    Result = open(File + ".ppc", 'w')
    Result.write(Buffer)
    Result.close()
