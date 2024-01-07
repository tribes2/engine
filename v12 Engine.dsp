# Microsoft Developer Studio Project File - Name="v12 Engine" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=v12 Engine - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "v12 Engine.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "v12 Engine.mak" CFG="v12 Engine - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "v12 Engine - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "v12 Engine - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "out.VC6.RELEASE"
# PROP Intermediate_Dir "out.VC6.RELEASE"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /GR /GX /O2 /I "../lib/directx" /I "../lib/zlib" /I "../lib/lungif" /I "../lib/lpng" /I "../lib/ljpeg" /I "../lib/mss" /I "../lib/openal/win32" /I "." /D "WIN32" /D "USEASSEMBLYTERRBLEND" /D "PNG_NO_READ_TIME" /D "PNG_NO_WRITE_TIME" /D OPENGL2D3D=\"glFOO.dll\" /D GLU2D3D=\"gluFOO.dll\" /D "NO_MILES_OPENAL" /YX /FD /c /Tp
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 COMCTL32.LIB COMDLG32.LIB USER32.LIB ADVAPI32.LIB GDI32.LIB WINMM.LIB WSOCK32.LIB ljpeg.lib lpng.lib lungif.lib zlib.lib Mss32.lib vfw32.lib /nologo /subsystem:console /machine:I386 /out:"out.VC6.RELEASE/v12_RELEASE.exe" /libpath:"../lib/out.VC6.RELEASE" /libpath:"../lib/mss"
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying v12_RELEASE.exe to example dir
PostBuild_Cmds=copy out.VC6.RELEASE\v12_RELEASE.exe ..\example
# End Special Build Tool

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "out.VC6.DEBUG"
# PROP Intermediate_Dir "out.VC6.DEBUG"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /Gm /GR /GX /ZI /Od /I "../lib/directx" /I "../lib/zlib" /I "../lib/lungif" /I "../lib/lpng" /I "../lib/ljpeg" /I "../lib/mss" /I "../lib/openal/win32" /I "." /D "DEBUG" /D "V12_DEBUG" /D "ENABLE_ASSERTS" /D "WIN32" /D "USEASSEMBLYTERRBLEND" /D "PNG_NO_READ_TIME" /D "PNG_NO_WRITE_TIME" /D "NO_MILES_OPENAL" /D OPENGL2D3D=\"glFOO.dll\" /D GLU2D3D=\"gluFOO.dll\" /YX /FD /GZ /c /Tp
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 COMCTL32.LIB COMDLG32.LIB USER32.LIB ADVAPI32.LIB GDI32.LIB WINMM.LIB WSOCK32.LIB ljpeg.lib lpng.lib lungif.lib zlib.lib Mss32.lib vfw32.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"LIBC" /out:"out.VC6.DEBUG/v12_DEBUG.exe" /pdbtype:sept /libpath:"../lib/out.VC6.DEBUG" /libpath:"../lib/mss"
# SUBTRACT LINK32 /pdb:none
# Begin Special Build Tool
SOURCE="$(InputPath)"
PostBuild_Desc=Copying v12_DEBUG.exe to example dir
PostBuild_Cmds=copy out.VC6.DEBUG\v12_DEBUG.exe ..\example
# End Special Build Tool

!ENDIF 

# Begin Target

# Name "v12 Engine - Win32 Release"
# Name "v12 Engine - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "ai"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\ai\aiConnection.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\aiConsole.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\aiDebug.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\aiNavJetting.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\aiNavStep.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\aiObjective.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\aiStep.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\aiTask.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graph.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphBase.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphBridge.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphBuildLOS.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphConjoin.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphData.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphDebug.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphDijkstra.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphFind.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphFloorPlan.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphFloorRender.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphForceField.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphGenUtils.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphGroundPlan.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphIndoors.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphIsland.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphJetting.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphLocate.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphLOS.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphMake.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphMath.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphOutdoors.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphPartition.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphPath.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphQueries.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphRender.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphSearchLOS.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphSmooth.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphSpawn.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphThreats.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphTransient.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ai\graphVolume.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ai"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ai"

!ENDIF 

# End Source File
# End Group
# Begin Group "audio"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\audio\audio.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\audioBuffer.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\audioCodec.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\audioCodecMiles.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\audioDataBlock.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\audioMss.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\audioNet.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\audioThread.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\audio\bufferQueue.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/audio"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/audio"

!ENDIF 

# End Source File
# End Group
# Begin Group "collision"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\collision\abstractPolyList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\boxConvex.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\clippedPolyList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\convex.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\depthSortList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\earlyOutPolyList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\extrudedPolyList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\gjk.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\planeExtractor.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\polyhedron.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\collision\polytope.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/collision"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/collision"

!ENDIF 

# End Source File
# End Group
# Begin Group "console"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\console\compiledEval.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\compiler.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\console.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleFunctions.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleInternal.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\consoleTypes.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\gram.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\scan.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\scriptObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\simBase.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\simDictionary.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\simManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\telnetConsole.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\console\telnetDebugger.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/console"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/console"

!ENDIF 

# End Source File
# End Group
# Begin Group "core"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\core\bitRender.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\bitStream.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\BitTables.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\dataChunker.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\dnet.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\fileObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\fileStream.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\filterStream.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\findMatch.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\idGenerator.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\memStream.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\nStream.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\nTypes.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\resDictionary.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\resizeStream.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\resManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\stringTable.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\tagDictionary.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\tVector.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\zipAggregate.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\zipHeaders.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\core\zipSubStream.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/core"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/core"

!ENDIF 

# End Source File
# End Group
# Begin Group "crypt"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\crypt\cryptMGF.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/crypt"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/crypt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptRandPool.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/crypt"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/crypt"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\crypt\cryptSHA1.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/crypt"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/crypt"

!ENDIF 

# End Source File
# End Group
# Begin Group "dgl"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\dgl\bitmapBM8.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapBMP.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapGIF.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapJpeg.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\bitmapPng.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\dgl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\dglMatrix.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gBitmap.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gFont.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gPalette.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\gTexManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\lensFlare.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\materialList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\materialPropertyMap.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\rectClipper.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\splineUtil.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\dgl\stripCache.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/dgl"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/dgl"

!ENDIF 

# End Source File
# End Group
# Begin Group "editor"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\editor\compTest.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\creator.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\editor.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\editorButtonCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\editorCheckboxCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\editTSCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\guiTerrPreviewCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\missionAreaEditor.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\terraformer.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\terraformer_noise.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\terraformerTexture.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\terrainActions.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\terrainEditor.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\editor\worldEditor.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/editor"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/editor"

!ENDIF 

# End Source File
# End Group
# Begin Group "game"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\game\ambientAudioManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\audioEmitter.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\banList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\bombSight.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\camera.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\cameraFXMgr.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\collisionTest.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\commanderMapIcon.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\debris.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\debugView.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\explosion.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\fireballAtmosphere.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\flyingVehicle.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\forceFieldBare.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\game.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\gameBase.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\gameConnection.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\gameConnectionEvents.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\gameConnectionMoves.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\gameFunctions.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\gameProcess.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\gameTSCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\guiNoMouseCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\guiPlayerView.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\guiServerBrowser.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\hoverVehicle.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\httpObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\item.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\lightning.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\linearProjectile.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\missionArea.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\missionMarker.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\motionBlurLine.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\net.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\netDispatch.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\netTest.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\particleEmitter.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\particleEngine.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\physicalZone.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\platTest.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\player.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\precipitation.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projBomb.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projectile.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projELF.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projEnergy.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projFlareGrenade.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projGrenade.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projLinearFlare.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projRepair.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projSeeker.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projShockLance.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projSniper.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projTargeting.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\projTracer.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\rigid.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\scopeAlwaysShape.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\sensor.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\serverQuery.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\shadow.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\shapeBase.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\shapeCollision.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\shapeImage.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\shieldImpact.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\shockwave.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\showTSShape.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\sphere.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\splash.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\staticShape.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\targetManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\tcpObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\trigger.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\tsStatic.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\turret.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\underLava.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\vehicle.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\vehicleBlocker.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\version.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\weaponBeam.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\game\wheeledVehicle.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/game"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/game"

!ENDIF 

# End Source File
# End Group
# Begin Group "gui"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\gui\channelVector.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiArrayCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiAviBitmapCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiBackgroundCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiBitmapCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiBubbleTextCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiButtonCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiCanvas.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiChannelVectorCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiChatMenuTreeCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiCheckBoxCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiChunkedBitmapCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiConsole.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleEditCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleTextCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiControl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiControlListPopup.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiDebugger.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiEditCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiFilterCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiFrameCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiInputCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiInspector.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiMessageVectorCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextEditCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiMouseEventCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiPopUpCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiProgressCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiRadioCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiScrollCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiSliderCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiTextCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditSliderCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiTextListCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiTreeViewCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiTSControl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiTypes.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiVoteCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\guiWindowCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gui\messageVector.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/gui"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/gui"

!ENDIF 

# End Source File
# End Group
# Begin Group "hud"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\hud\hudBarBaseCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudBitmapCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudClock.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudCompass.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudCrosshair.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudDamageCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudEnergyCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\hud\hudZoom.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/hud"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/hud"

!ENDIF 

# End Source File
# End Group
# Begin Group "interior"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\interior\FloorPlanRes.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\forceField.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interior.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorCollision.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorDebug.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorInstance.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorIO.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorLightAnim.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorLMManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorRender.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorRes.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorResObjects.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\interiorSubObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\itfdump.asm

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"
# Begin Custom Build
IntDir=.\out.VC6.RELEASE/interior
InputPath=.\interior\itfdump.asm
InputName=itfdump

"$(IntDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw.exe -f win32 $(InputPath) -o $(IntDir)/$(InputName).obj

# End Custom Build

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"
# Begin Custom Build
IntDir=.\out.VC6.DEBUG/interior
InputPath=.\interior\itfdump.asm
InputName=itfdump

"$(IntDir)/$(InputName).obj" : $(SOURCE) "$(INTDIR)" "$(OUTDIR)"
	nasmw.exe -f win32 $(InputPath) -o $(IntDir)/$(InputName).obj

# End Custom Build

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\lightUpdateGrouper.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\interior\mirrorSubObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/interior"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/interior"

!ENDIF 

# End Source File
# End Group
# Begin Group "math"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\math\mathTypes.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mathUtils.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mBox.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mConsoleFunctions.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMath_C.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMathAMD.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMathFn.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMathSSE.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mMatrix.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mPlaneTransformer.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mQuadPatch.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mQuat.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mRandom.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mSolver.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\math\mSplinePatch.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/math"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/math"

!ENDIF 

# End Source File
# End Group
# Begin Group "platform"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\platform\gameInterface.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformAssert.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformMemory.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformRedBook.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\platformVideo.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platform\profiler.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platform"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platform"

!ENDIF 

# End Source File
# End Group
# Begin Group "platformWIN32"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\platformWIN32\winAsmBlit.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winConsole.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winCPUInfo.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winD3DVideo.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"
# ADD CPP /D OPENGL2D3D=\"glFOO.dll\" /D GLU2D3D=\"gluFOO.dll\"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDInputDevice.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDirectInput.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winFileio.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winFont.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winGL.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winInput.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMath.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMath_ASM.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMemory.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winMutex.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winNet.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOGLVideo.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOpenAL.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winProcessControl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winRedbook.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winSemaphore.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winStrings.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winThread.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winTime.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winV2Video.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winWindow.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/platformWIN32"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/platformWIN32"

!ENDIF 

# End Source File
# End Group
# Begin Group "sceneGraph"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\scenegraph\detailManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\lightManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneGraph.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneLighting.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneRoot.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneState.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneTraversal.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\sgUtil.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\shadowVolumeBSP.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\scenegraph\windingClipper.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sceneGraph"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sceneGraph"

!ENDIF 

# End Source File
# End Group
# Begin Group "shell"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\shell\shellFancyArray.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/shell"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/shell"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shell\shellFancyTextList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/shell"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/shell"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shell\shellScrollCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/shell"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/shell"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\shell\shellTextEditCtrl.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/shell"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/shell"

!ENDIF 

# End Source File
# End Group
# Begin Group "sim"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\sim\actionMap.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\cannedChatDataBlock.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\decalManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\frameAllocator.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netConnection.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netEvent.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netGhost.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\netStringTable.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\pathManager.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\sceneObject.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\sim\simPath.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/sim"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/sim"

!ENDIF 

# End Source File
# End Group
# Begin Group "terrain"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\terrain\blender.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\bvQuadTree.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\FluidQuadTree.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\FluidRender.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\FluidSupport.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\Sky.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\Sun.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrCollision.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrData.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrLighting.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender2.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\terrain\waterBlock.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/terrain"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/terrain"

!ENDIF 

# End Source File
# End Group
# Begin Group "ts"

# PROP Default_Filter "cc"
# Begin Source File

SOURCE=.\ts\tsAnimate.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsCollision.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsDecal.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsDump.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsIntegerSet.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsLastDetail.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsMaterialList.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsMesh.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsPartInstance.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShape.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeAlloc.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeConstruct.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeInstance.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeOldRead.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsSortedMesh.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsThread.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\ts\tsTransform.cc

!IF  "$(CFG)" == "v12 Engine - Win32 Release"

# PROP Intermediate_Dir "out.VC6.RELEASE/ts"

!ELSEIF  "$(CFG)" == "v12 Engine - Win32 Debug"

# PROP Intermediate_Dir "out.VC6.DEBUG/ts"

!ENDIF 

# End Source File
# End Group
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Group "ai headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ai\aiConnection.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiNavJetting.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiNavStep.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiObjective.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiStep.h
# End Source File
# Begin Source File

SOURCE=.\ai\aiTask.h
# End Source File
# Begin Source File

SOURCE=.\ai\graph.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphBase.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphBridge.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphBSP.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphData.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphDefines.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphFloorPlan.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphForceField.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphGenUtils.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphGroundPlan.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphGroundVisit.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphJetting.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphLocate.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphLOS.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphMath.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphNodes.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphPartition.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphPath.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphSearches.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphThreats.h
# End Source File
# Begin Source File

SOURCE=.\ai\graphTransient.h
# End Source File
# Begin Source File

SOURCE=.\ai\oVector.h
# End Source File
# Begin Source File

SOURCE=.\ai\tBinHeap.h
# End Source File
# Begin Source File

SOURCE=.\ai\texturePreload.h
# End Source File
# End Group
# Begin Group "audio headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\audio\audio.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioBuffer.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioCodec.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioCodecMiles.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioDataBlock.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioMss.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioNet.h
# End Source File
# Begin Source File

SOURCE=.\audio\audioThread.h
# End Source File
# Begin Source File

SOURCE=.\audio\bufferQueue.h
# End Source File
# End Group
# Begin Group "collision headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\collision\abstractPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\boxConvex.h
# End Source File
# Begin Source File

SOURCE=.\collision\clippedPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\collision.h
# End Source File
# Begin Source File

SOURCE=.\collision\convex.h
# End Source File
# Begin Source File

SOURCE=.\collision\depthSortList.h
# End Source File
# Begin Source File

SOURCE=.\collision\earlyOutPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\extrudedPolyList.h
# End Source File
# Begin Source File

SOURCE=.\collision\gjk.h
# End Source File
# Begin Source File

SOURCE=.\collision\planeExtractor.h
# End Source File
# Begin Source File

SOURCE=.\collision\polyhedron.h
# End Source File
# Begin Source File

SOURCE=.\collision\polytope.h
# End Source File
# End Group
# Begin Group "console headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\console\ast.h
# End Source File
# Begin Source File

SOURCE=.\console\compiler.h
# End Source File
# Begin Source File

SOURCE=.\console\console.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleInternal.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleObject.h
# End Source File
# Begin Source File

SOURCE=.\console\consoleTypes.h
# End Source File
# Begin Source File

SOURCE=.\console\gram.h
# End Source File
# Begin Source File

SOURCE=.\console\objectTypes.h
# End Source File
# Begin Source File

SOURCE=.\console\simBase.h
# End Source File
# Begin Source File

SOURCE=.\console\simDictionary.h
# End Source File
# Begin Source File

SOURCE=.\console\telnetConsole.h
# End Source File
# Begin Source File

SOURCE=.\console\telnetDebugger.h
# End Source File
# End Group
# Begin Group "core headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\core\bitMatrix.h
# End Source File
# Begin Source File

SOURCE=.\core\bitRender.h
# End Source File
# Begin Source File

SOURCE=.\core\bitSet.h
# End Source File
# Begin Source File

SOURCE=.\core\bitStream.h
# End Source File
# Begin Source File

SOURCE=.\core\BitTables.h
# End Source File
# Begin Source File

SOURCE=.\core\bitVector.h
# End Source File
# Begin Source File

SOURCE=.\core\bitVectorW.h
# End Source File
# Begin Source File

SOURCE=.\core\color.h
# End Source File
# Begin Source File

SOURCE=.\core\coreRes.h
# End Source File
# Begin Source File

SOURCE=.\core\dataChunker.h
# End Source File
# Begin Source File

SOURCE=.\core\dnet.h
# End Source File
# Begin Source File

SOURCE=.\core\fileio.h
# End Source File
# Begin Source File

SOURCE=.\core\fileObject.h
# End Source File
# Begin Source File

SOURCE=.\core\fileStream.h
# End Source File
# Begin Source File

SOURCE=.\core\filterStream.h
# End Source File
# Begin Source File

SOURCE=.\core\findMatch.h
# End Source File
# Begin Source File

SOURCE=.\core\idGenerator.h
# End Source File
# Begin Source File

SOURCE=.\core\llist.h
# End Source File
# Begin Source File

SOURCE=.\core\memstream.h
# End Source File
# Begin Source File

SOURCE=.\core\polyList.h
# End Source File
# Begin Source File

SOURCE=.\core\realComp.h
# End Source File
# Begin Source File

SOURCE=.\core\resizeStream.h
# End Source File
# Begin Source File

SOURCE=.\core\resManager.h
# End Source File
# Begin Source File

SOURCE=.\core\stream.h
# End Source File
# Begin Source File

SOURCE=.\core\stringTable.h
# End Source File
# Begin Source File

SOURCE=.\core\tagDictionary.h
# End Source File
# Begin Source File

SOURCE=.\core\tAlgorithm.h
# End Source File
# Begin Source File

SOURCE=.\core\tSparseArray.h
# End Source File
# Begin Source File

SOURCE=.\core\tVector.h
# End Source File
# Begin Source File

SOURCE=.\core\zipAggregate.h
# End Source File
# Begin Source File

SOURCE=.\core\zipHeaders.h
# End Source File
# Begin Source File

SOURCE=.\core\zipSubStream.h
# End Source File
# End Group
# Begin Group "crypt headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\crypt\cryptMGF.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptRandPool.h
# End Source File
# Begin Source File

SOURCE=.\crypt\cryptSHA1.h
# End Source File
# End Group
# Begin Group "dgl headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\dgl\dgl.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gBitmap.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gChunkedTexManager.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gFont.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gPalette.h
# End Source File
# Begin Source File

SOURCE=.\dgl\gTexManager.h
# End Source File
# Begin Source File

SOURCE=.\dgl\lensFlare.h
# End Source File
# Begin Source File

SOURCE=.\dgl\materialList.h
# End Source File
# Begin Source File

SOURCE=.\dgl\materialPropertyMap.h
# End Source File
# Begin Source File

SOURCE=.\dgl\rectClipper.h
# End Source File
# Begin Source File

SOURCE=.\dgl\splineUtil.h
# End Source File
# Begin Source File

SOURCE=.\dgl\stripCache.h
# End Source File
# End Group
# Begin Group "editor headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\editor\creator.h
# End Source File
# Begin Source File

SOURCE=.\editor\editor.h
# End Source File
# Begin Source File

SOURCE=.\editor\editorButtonCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\editorCheckboxCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\editTSCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\guiTerrPreviewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\editor\missionAreaEditor.h
# End Source File
# Begin Source File

SOURCE=.\editor\terraformer.h
# End Source File
# Begin Source File

SOURCE=.\editor\terraformer_noise.h
# End Source File
# Begin Source File

SOURCE=.\editor\terrainActions.h
# End Source File
# Begin Source File

SOURCE=.\editor\terrainEditor.h
# End Source File
# Begin Source File

SOURCE=.\editor\worldEditor.h
# End Source File
# End Group
# Begin Group "game headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\game\ambientAudioManager.h
# End Source File
# Begin Source File

SOURCE=.\game\audioEmitter.h
# End Source File
# Begin Source File

SOURCE=.\game\auth.h
# End Source File
# Begin Source File

SOURCE=.\game\banList.h
# End Source File
# Begin Source File

SOURCE=.\game\bombSight.h
# End Source File
# Begin Source File

SOURCE=.\game\camera.h
# End Source File
# Begin Source File

SOURCE=.\game\cameraFXMgr.h
# End Source File
# Begin Source File

SOURCE=.\game\collisionTest.h
# End Source File
# Begin Source File

SOURCE=.\game\commanderMapIcon.h
# End Source File
# Begin Source File

SOURCE=.\game\debris.h
# End Source File
# Begin Source File

SOURCE=.\game\debugView.h
# End Source File
# Begin Source File

SOURCE=.\game\explosion.h
# End Source File
# Begin Source File

SOURCE=.\game\fireballAtmosphere.h
# End Source File
# Begin Source File

SOURCE=.\game\flyingVehicle.h
# End Source File
# Begin Source File

SOURCE=.\game\forceFieldBare.h
# End Source File
# Begin Source File

SOURCE=.\game\game.h
# End Source File
# Begin Source File

SOURCE=.\game\gameBase.h
# End Source File
# Begin Source File

SOURCE=.\game\gameConnection.h
# End Source File
# Begin Source File

SOURCE=.\game\gameConnectionEvents.h
# End Source File
# Begin Source File

SOURCE=.\game\gameFunctions.h
# End Source File
# Begin Source File

SOURCE=.\game\gameTSCtrl.h
# End Source File
# Begin Source File

SOURCE=.\game\guiPlayerView.h
# End Source File
# Begin Source File

SOURCE=.\game\guiServerBrowser.h
# End Source File
# Begin Source File

SOURCE=.\game\hoverVehicle.h
# End Source File
# Begin Source File

SOURCE=.\game\httpObject.h
# End Source File
# Begin Source File

SOURCE=.\game\item.h
# End Source File
# Begin Source File

SOURCE=.\game\lightning.h
# End Source File
# Begin Source File

SOURCE=.\game\linearProjectile.h
# End Source File
# Begin Source File

SOURCE=.\game\missionArea.h
# End Source File
# Begin Source File

SOURCE=.\game\missionMarker.h
# End Source File
# Begin Source File

SOURCE=.\game\motionBlurLine.h
# End Source File
# Begin Source File

SOURCE=.\game\moveManager.h
# End Source File
# Begin Source File

SOURCE=.\game\netDispatch.h
# End Source File
# Begin Source File

SOURCE=.\game\objectTypes.h
# End Source File
# Begin Source File

SOURCE=.\game\particleEmitter.h
# End Source File
# Begin Source File

SOURCE=.\game\particleEngine.h
# End Source File
# Begin Source File

SOURCE=.\game\physicalZone.h
# End Source File
# Begin Source File

SOURCE=.\game\player.h
# End Source File
# Begin Source File

SOURCE=.\game\precipitation.h
# End Source File
# Begin Source File

SOURCE=.\game\projBomb.h
# End Source File
# Begin Source File

SOURCE=.\game\projectile.h
# End Source File
# Begin Source File

SOURCE=.\game\projELF.h
# End Source File
# Begin Source File

SOURCE=.\game\projEnergy.h
# End Source File
# Begin Source File

SOURCE=.\game\projFlareGrenade.h
# End Source File
# Begin Source File

SOURCE=.\game\projGrenade.h
# End Source File
# Begin Source File

SOURCE=.\game\projLinearFlare.h
# End Source File
# Begin Source File

SOURCE=.\game\projRepair.h
# End Source File
# Begin Source File

SOURCE=.\game\projSeeker.h
# End Source File
# Begin Source File

SOURCE=.\game\projShockLance.h
# End Source File
# Begin Source File

SOURCE=.\game\projSniper.h
# End Source File
# Begin Source File

SOURCE=.\game\projTargeting.h
# End Source File
# Begin Source File

SOURCE=.\game\projTracer.h
# End Source File
# Begin Source File

SOURCE=.\game\resource.h
# End Source File
# Begin Source File

SOURCE=.\game\rigid.h
# End Source File
# Begin Source File

SOURCE=.\game\sensor.h
# End Source File
# Begin Source File

SOURCE=.\game\serverQuery.h
# End Source File
# Begin Source File

SOURCE=.\game\shadow.h
# End Source File
# Begin Source File

SOURCE=.\game\shapeBase.h
# End Source File
# Begin Source File

SOURCE=.\game\shieldImpact.h
# End Source File
# Begin Source File

SOURCE=.\game\shockwave.h
# End Source File
# Begin Source File

SOURCE=.\game\showTSShape.h
# End Source File
# Begin Source File

SOURCE=.\game\sphere.h
# End Source File
# Begin Source File

SOURCE=.\game\splash.h
# End Source File
# Begin Source File

SOURCE=.\game\staticShape.h
# End Source File
# Begin Source File

SOURCE=.\game\targetManager.h
# End Source File
# Begin Source File

SOURCE=.\game\tcpObject.h
# End Source File
# Begin Source File

SOURCE=.\game\tribesGame.h
# End Source File
# Begin Source File

SOURCE=.\game\trigger.h
# End Source File
# Begin Source File

SOURCE=.\game\tsStatic.h
# End Source File
# Begin Source File

SOURCE=.\game\turret.h
# End Source File
# Begin Source File

SOURCE=.\game\underLava.h
# End Source File
# Begin Source File

SOURCE=.\game\vehicle.h
# End Source File
# Begin Source File

SOURCE=.\game\vehicleBlocker.h
# End Source File
# Begin Source File

SOURCE=.\game\version.h
# End Source File
# Begin Source File

SOURCE=.\game\weaponBeam.h
# End Source File
# Begin Source File

SOURCE=.\game\wheeledVehicle.h
# End Source File
# End Group
# Begin Group "gui headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\gui\channelVector.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiArrayCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiAviBitmapCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiBackgroundCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiBitmapCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiBubbleTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiButtonCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiCanvas.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiChannelVectorCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiChatMenuTreeCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiCheckBoxCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsole.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiConsoleTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiControl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiDebugger.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiFilterCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiFrameCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiInputCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiInspector.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMessageVectorCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMLTextEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiMouseEventCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiPopUpCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiProgressCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiRadioCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiScrollCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextEditSliderCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTextListCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTreeViewCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTSControl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiTypes.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiVoteCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\guiWindowCtrl.h
# End Source File
# Begin Source File

SOURCE=.\gui\messageVector.h
# End Source File
# End Group
# Begin Group "hud headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\hud\hudBarBaseCtrl.h
# End Source File
# Begin Source File

SOURCE=.\hud\hudBitmapCtrl.h
# End Source File
# Begin Source File

SOURCE=.\hud\hudCtrl.h
# End Source File
# End Group
# Begin Group "interior headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\interior\FloorPlanRes.h
# End Source File
# Begin Source File

SOURCE=.\interior\forceField.h
# End Source File
# Begin Source File

SOURCE=.\interior\interior.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorInstance.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorLMManager.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorRes.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorResObjects.h
# End Source File
# Begin Source File

SOURCE=.\interior\interiorSubObject.h
# End Source File
# Begin Source File

SOURCE=.\interior\itf.h
# End Source File
# Begin Source File

SOURCE=.\interior\lightUpdateGrouper.h
# End Source File
# Begin Source File

SOURCE=.\interior\mirrorSubObject.h
# End Source File
# End Group
# Begin Group "math headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\math\mathIO.h
# End Source File
# Begin Source File

SOURCE=.\math\mathTypes.h
# End Source File
# Begin Source File

SOURCE=.\math\mathUtils.h
# End Source File
# Begin Source File

SOURCE=.\math\mBox.h
# End Source File
# Begin Source File

SOURCE=.\math\mConstants.h
# End Source File
# Begin Source File

SOURCE=.\math\mMath.h
# End Source File
# Begin Source File

SOURCE=.\math\mMathFn.h
# End Source File
# Begin Source File

SOURCE=.\math\mMatrix.h
# End Source File
# Begin Source File

SOURCE=.\math\mPlane.h
# End Source File
# Begin Source File

SOURCE=.\math\mPlaneTransformer.h
# End Source File
# Begin Source File

SOURCE=.\math\mPoint.h
# End Source File
# Begin Source File

SOURCE=.\math\mQuadPatch.h
# End Source File
# Begin Source File

SOURCE=.\math\mQuat.h
# End Source File
# Begin Source File

SOURCE=.\math\mRandom.h
# End Source File
# Begin Source File

SOURCE=.\math\mRect.h
# End Source File
# Begin Source File

SOURCE=.\math\mSphere.h
# End Source File
# Begin Source File

SOURCE=.\math\mSplinePatch.h
# End Source File
# Begin Source File

SOURCE=.\math\mTrig.h
# End Source File
# End Group
# Begin Group "platform headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\platform\3DFX.H
# End Source File
# Begin Source File

SOURCE=.\platform\event.h
# End Source File
# Begin Source File

SOURCE=.\platform\gameInterface.h
# End Source File
# Begin Source File

SOURCE=.\platform\platform.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformAssert.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformAudio.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformInput.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformMutex.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformRedBook.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformSemaphore.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformThread.h
# End Source File
# Begin Source File

SOURCE=.\platform\platformVideo.h
# End Source File
# Begin Source File

SOURCE=.\platform\profiler.h
# End Source File
# Begin Source File

SOURCE=.\platform\types.h
# End Source File
# Begin Source File

SOURCE=.\platform\typesWin32.h
# End Source File
# End Group
# Begin Group "platformWIN32 headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\platformWIN32\gllist.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformAL.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformGL.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\platformWin32.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winConsole.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winD3DVideo.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDInputDevice.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winDirectInput.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winOGLVideo.h
# End Source File
# Begin Source File

SOURCE=.\platformWIN32\winV2Video.h
# End Source File
# End Group
# Begin Group "sceneGraph headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\scenegraph\detailManager.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\lightManager.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneGraph.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneLighting.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneRoot.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sceneState.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\sgUtil.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\shadowVolumeBSP.h
# End Source File
# Begin Source File

SOURCE=.\scenegraph\windingClipper.h
# End Source File
# End Group
# Begin Group "shell headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\shell\shellFancyArray.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellFancyTextList.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellScrollCtrl.h
# End Source File
# Begin Source File

SOURCE=.\shell\shellTextEditCtrl.h
# End Source File
# End Group
# Begin Group "sim headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\sim\actionMap.h
# End Source File
# Begin Source File

SOURCE=.\sim\cannedChatDataBlock.h
# End Source File
# Begin Source File

SOURCE=.\sim\decalManager.h
# End Source File
# Begin Source File

SOURCE=.\sim\frameAllocator.h
# End Source File
# Begin Source File

SOURCE=.\sim\netConnection.h
# End Source File
# Begin Source File

SOURCE=.\sim\netObject.h
# End Source File
# Begin Source File

SOURCE=.\sim\netStringTable.h
# End Source File
# Begin Source File

SOURCE=.\sim\pathManager.h
# End Source File
# Begin Source File

SOURCE=.\sim\sceneObject.h
# End Source File
# Begin Source File

SOURCE=.\sim\simPath.h
# End Source File
# End Group
# Begin Group "terrain headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\terrain\blender.h
# End Source File
# Begin Source File

SOURCE=.\terrain\bvQuadTree.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Fluid.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Sky.h
# End Source File
# Begin Source File

SOURCE=.\terrain\Sun.h
# End Source File
# Begin Source File

SOURCE=.\terrain\terrData.h
# End Source File
# Begin Source File

SOURCE=.\terrain\terrRender.h
# End Source File
# Begin Source File

SOURCE=.\terrain\waterBlock.h
# End Source File
# End Group
# Begin Group "ts headers"

# PROP Default_Filter "h"
# Begin Source File

SOURCE=.\ts\tsDecal.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsIntegerSet.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsLastDetail.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsMesh.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsPartInstance.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShape.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeAlloc.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeConstruct.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsShapeInstance.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsSortedMesh.h
# End Source File
# Begin Source File

SOURCE=.\ts\tsTransform.h
# End Source File
# End Group
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
