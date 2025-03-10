project "alt:M"
 language "C++"
 kind "ConsoleApp"

 links { "Shared", "CitiCore" }

 add_dependencies { 'vender:fmtlib', 'vendor:breakpad', 'vendor:cpu_features' }

 if os.istarget('windows') then
 links { "psapi", "wininet", "winhttp" }
 flags { "NoManifest", "NoImportLib" }
 files { "server.rc" }

 -- match the 4MB stack size set on Windows in Main.cpp
 -- again: 1.5 MB is required for V1
 linkoptions '/STACK:0x400000'
 else
 links { 'dl', 'pthread' }
 end

 includedirs
 {
	 "../../client/citicore/",
	 "include/"
 }

 files
 {
	 "**.cpp", "**.h", "../../client/common/Error.cpp"
 }

 pchsource "src/StdInc.cpp"
 pchheader "StdInc.h"

 target "AMServer"