

From the build dir

	cmake -S ../../inviwo -B . --preset iwyu
	../../inviwo/tools/iwyu/run.sh mymodule

you need to rerun cmake to make any change to the mappingfile (.imp) to take affect 


Example preset to for running iwyu

{
  "version": 1,
  "cmakeMinimumRequired": { "major": 3, "minor": 19, "patch": 0 },
    {
      "name": "iwyu",
      "displayName": "IWYU ninja build",
      "generator": "Ninja",
      "binaryDir": "${sourceParentDir}/build/iwyu",
      "inherits" : ["modules", "develop"],
      "environment": { "PATH" : "$penv{PATH}",
        "CC": "/opt/homebrew/opt/llvm/bin/clang",
        "CXX": "/opt/homebrew/opt/llvm/bin/clang++"
      },  
      "cacheVariables": {
        "CMAKE_CXX_INCLUDE_WHAT_YOU_USE": { 
          "type": "PATH", 
          "value": "/opt/homebrew/opt/include-what-you-use/bin/include-what-you-use;-Xiwyu;--keep=*warn/*;-Xiwyu;--mapping_file=${sourceDir}/tools/iwyu/inviwo.imp;-Xiwyu;--max_line_length=100"}
      }
    }
  ]
}




