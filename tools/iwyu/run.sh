#!/bin/zsh

module=$1

sed=gsed # osx sed version is odd
fix=/Users/peter/Documents/Inviwo/inviwo/tools/iwyu/fix_includes.py
#fix=/opt/homebrew/bin/fix_includes.py

#ninja -r -t clean ${module}
ninja -t clean -r CXX_COMPILER__inviwo-module-${module}_Release

ninja inviwo-module-${module} | tee inc.fix 

${sed} -i -E 's/#include "([A-Za-z0-9./\-_]+)"/#include <\1>/g' inc.fix
${sed} -i 's/#include <stdint.h>/#include <cstdint> /g' inc.fix
${sed} -i 's/#include <stddef.h>/#include <cstddef> /g' inc.fix
${sed} -i 's/#include <math.h>/#include <cmath> /g' inc.fix
${sed} -i 's/#include <string.h>/#include <cstring> /g' inc.fix

#${fix} --only_re=".*/opengl/.*" --comments --blank_lines --reorder  --safe_headers --separate_project_includes="<tld>" < inc.fix
${fix} --only_re="(?!.*/ext/.*).*(inviwo|modules)/${module}/.*" --blank_lines --reorder --comments --update_comments --nosafe_headers --separate_project_includes="<tld>" --basedir "/Users/peter/Documents/Inviwo/inviwo/" < inc.fix | tee fixed.txt

# Clang format the fixes
grep ">>> Fixing" fixed.txt | sed "s/>>> Fixing #includes in '//" | sed "s/'//" | xargs clang-format -i

