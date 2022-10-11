

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

Fixes for qt, modify iwyu to avoid some circular include in global.h see
https://github.com/include-what-you-use/include-what-you-use/issues/424

diff --git a/iwyu_include_picker.cc b/iwyu_include_picker.cc
index 26b55d8..abfb00a 100644
--- a/iwyu_include_picker.cc
+++ b/iwyu_include_picker.cc
@@ -1065,6 +1065,7 @@ void MakeNodeTransitive(IncludePicker::IncludeMap* filename_map,
                         const string& key) {
   // If we've already calculated this node's transitive closure, we're done.
   const TransitiveStatus status = (*seen_nodes)[key];
+
   if (status == kCalculating) {   // means there's a cycle in the mapping
     // TODO: Reconsider cycle handling; the include_cycle test fails without
     // this special-casing, but it seems we should handle this more generally.
@@ -1072,6 +1073,10 @@ void MakeNodeTransitive(IncludePicker::IncludeMap* filename_map,
       VERRS(4) << "Ignoring a cyclical mapping involving " << key << "\n";
       return;
     }
+     if (key.find("qglobal.h") != string::npos) {
+      VERRS(4) << "Ignoring a cyclical mapping involving " << key << "\n";
+      return;
+    }
   }
   if (status == kCalculating) {
     VERRS(0) << "Cycle in include-mapping:\n";

