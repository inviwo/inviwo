/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2026 Inviwo Foundation
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
    templates["file"] = FileTemplate{.name="File",
        .header=File{.name="File header", .extension="h", .templateFile="file.h", .description="General header file", .cmakeGroup="HEADER_FILES"},
        .source=File{.name="File source", .extension="cpp", .templateFile="file.cpp", .description="General source file", .cmakeGroup="SOURCE_FILES"},
        .other={},
        .registration={}
    };

    templates["processor"] = FileTemplate{.name="Processor",
        .header=File{.name="Processor header", .extension="h", .templateFile="processor.h", .description="Processor header file", .cmakeGroup="HEADER_FILES"},
        .source=File{.name="Processor source", .extension="cpp", .templateFile="processor.cpp", .description="Processor source file", .cmakeGroup="SOURCE_FILES"},
        .other={},
        .registration={"processor"}
    };

    templates["test"] = FileTemplate{.name="Test", 
        .header={}, 
        .source={},
        .other=File{.name="Unit Test", .extension="cpp", .templateFile="test.cpp", .description="Unit test file", .cmakeGroup="TEST_FILES"}, 
        .registration={}
    };

    templates["testmain"] = FileTemplate{.name="Test Main",
        .header={},
        .source={},
        .other=File{.name="Unit Test Main", .extension="cpp", .templateFile="unittest-main.cpp", .description="Unit test main file", .cmakeGroup="TEST_FILES"},
        .registration={}
    };

    templates["frag"] = FileTemplate{.name="Fragment",
        .header={},
        .source={},
        .other=File{.name="Fragment", .extension="frag", .templateFile="fragment.frag", .description="Fragment source file", .cmakeGroup="SHADER_FILES"},
        .registration={}
    };

    templates["vert"] = FileTemplate{.name="Vertex",
        .header={},
        .source={},
        .other=File{.name="Vertex", .extension="vert", .templateFile="vertex.vert", .description="Vertex source file", .cmakeGroup="SHADER_FILES"},
        .registration={}
    };

    templates["geom"] = FileTemplate{.name="Geometry",
        .header={},
        .source={},
        .other=File{.name="Geometry", .extension="geom", .templateFile="geometry.geom", .description="Geometry source file", .cmakeGroup="SHADER_FILES"},
        .registration={}
    };

    templates["module"] = FileTemplate{.name="Module",
        .header=File{.name="Module header", .extension="h", .templateFile="module.h", .description="Module header file", .cmakeGroup="HEADER_FILES"},
        .source=File{.name="Module source", .extension="cpp", .templateFile="module.cpp", .description="Module source file", .cmakeGroup="SOURCE_FILES"},
        .other={},
        .registration={}
    };

    templates["api"] = FileTemplate{.name="API",
        .header=File{.name="Module API", .extension="h", .templateFile="moduledefine.h", .description="Module API define header", .cmakeGroup="HEADER_FILES"},
        .source={},
        .other={},
        .registration={}
    };

    templates["cmakelists"] = FileTemplate{.name="CMakeLists",
        .header={},
        .source={},
        .other=File{.name="CMakeLists", .extension="txt", .templateFile="CMakeLists.txt", .description="CMake project definition", .cmakeGroup={}},
        .registration={}
    };
    templates["readme"] = FileTemplate{.name="Readme",
        .header={},
        .source={},
        .other=File{.name="Readme", .extension="md", .templateFile="readme.md", .description="Description of the module, used by CMake", .cmakeGroup={}},
        .registration={}
    };
    templates["depends"] = FileTemplate{.name="Depends",
        .header={},
        .source={},
        .other=File{.name="Depends", .extension="cmake", .templateFile="depends.cmake", .description="List of dependencies to other modules", .cmakeGroup={}},
        .registration={}
    };
    // clang-format on
}

const FileTemplate& FileTemplates::operator[](const std::string& key) const {
    const auto it = templates.find(key);
    if (it != templates.end()) {
        return it->second;
    } else {
        throw util::makeError("Error: FileTemplates invalid key '{}'", key);
    }
}

}  // namespace inviwo::meta
