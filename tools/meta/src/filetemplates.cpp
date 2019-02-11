/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 *********************************************************************************/

#include <inviwo/meta/filetemplates.hpp>
#include <inviwo/meta/util.hpp>

#include <exception>
#include <fmt/format.h>

namespace inviwo::meta {

FileTemplates::FileTemplates() {
    // clang-format off
    templates["file"] = FileTemplate{"File",
        File{"File header", "h", "file.h", "General header file", "HEADER_FILES"},
        File{"File source", "cpp", "file.cpp", "General source file", "SOURCE_FILES"},
        {},
        {}
    };

    templates["processor"] = FileTemplate{"Processor",
        File{"Processor header", "h", "processor.h", "Processor header file", "HEADER_FILES"},
        File{"Processor source", "cpp", "processor.cpp", "Processor source file", "SOURCE_FILES"},
        {},
        {"processor"}
    };

    templates["test"] = FileTemplate{"Test", 
        {}, 
        {},
        File{"Unit Test", "cpp", "test.cpp", "Unit test file", "TEST_FILES"}, 
        {}
    };

    templates["testmain"] = FileTemplate{"Test Main",
        {},
        {},
        File{"Unit Test Main", "cpp", "unittest-main.cpp", "Unit test main file", "TEST_FILES"},
        {}
    };

    templates["frag"] = FileTemplate{"Fragment",
        {},
        {},
        File{"Fragment", "frag", "fragment.frag", "Fragment source file", "SHADER_FILES"},
        {}
    };

    templates["vert"] = FileTemplate{"Vertex",
        {},
        {},
        File{"Vertex", "vert", "vertex.vert", "Vertex source file", "SHADER_FILES"},
        {}
    };

    templates["geom"] = FileTemplate{"Geometry",
        {},
        {},
        File{"Geometry", "geom", "geometry.geom", "Geometry source file", "SHADER_FILES"},
        {}
    };

    templates["module"] = FileTemplate{"Module",
        File{"Module header", "h", "module.h", "Module header file", "HEADER_FILES"},
        File{"Module source", "cpp", "module.cpp", "Module source file", "SOURCE_FILES"},
        {},
        {}
    };

    templates["api"] = FileTemplate{"API",
        File{"Module API", "h", "moduledefine.h", "Module API define header", "HEADER_FILES"},
        {},
        {},
        {}
    };

    templates["cmakelists"] = FileTemplate{"CMakeLists",
        {},
        {},
        File{"CMakeLists", "txt", "CMakeLists.txt", "CMake project definition", {}},
        {}
    };
    templates["readme"] = FileTemplate{"Readme",
        {},
        {},
        File{"Readme", "md", "readme.md", "Description of the module, used by CMake", {}},
        {}
    };
    templates["depends"] = FileTemplate{"Depends",
        {},
        {},
        File{"Depends", "cmake", "depends.cmake", "List of dependencies to other modules", {}},
        {}
    };
    // clang-format on
}

const FileTemplate& FileTemplates::operator[](const std::string& key) const {
    auto it = templates.find(key);
    if (it != templates.end()) {
        return it->second;
    } else {
        throw util::makeError("Error: FileTemplates invalid key '{}'", key);
    }
}

}  // namespace inviwo::meta
