/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020-2022 Inviwo Foundation
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

#include <modules/base/processors/gridplanes.h>

#include <inviwo/core/algorithm/markdown.h>                             // for operator""_help
#include <inviwo/core/datastructures/geometry/geometrytype.h>           // for ConnectivityType
#include <inviwo/core/datastructures/geometry/typedmesh.h>              // for ColoredMesh, Type...
#include <inviwo/core/datastructures/representationconverter.h>         // for RepresentationCon...
#include <inviwo/core/datastructures/representationconverterfactory.h>  // for RepresentationCon...
#include <inviwo/core/ports/meshport.h>                                 // for MeshOutport
#include <inviwo/core/ports/volumeport.h>                               // for VolumeInport
#include <inviwo/core/processors/processor.h>                           // for Processor
#include <inviwo/core/processors/processorinfo.h>                       // for ProcessorInfo
#include <inviwo/core/processors/processorstate.h>                      // for CodeState, CodeSt...
#include <inviwo/core/processors/processortags.h>                       // for Tags, Tags::CPU
#include <inviwo/core/properties/boolproperty.h>                        // for BoolProperty
#include <inviwo/core/properties/minmaxproperty.h>                      // for FloatMinMaxProperty
#include <inviwo/core/properties/ordinalproperty.h>                     // for FloatProperty
#include <inviwo/core/util/glmvec.h>                                    // for vec3, vec4

#include <functional>     // for __base
#include <memory>         // for shared_ptr, share...
#include <string>         // for string
#include <type_traits>    // for remove_extent_t
#include <unordered_map>  // for unordered_map
#include <unordered_set>  // for unordered_set
#include <vector>         // for vector

#include <fmt/core.h>    // for format
#include <glm/vec2.hpp>  // for vec, vec<>::(anon...

namespace inviwo {

// The Class Identifier has to be globally unique. Use a reverse DNS naming scheme
const ProcessorInfo GridPlanes::processorInfo_{
    "org.inviwo.GridPlanes",  // Class identifier
    "Grid Planes",            // Display name
    "Information",            // Category
    CodeState::Experimental,  // Code state
    Tags::CPU,                // Tags
    R"(Creates a mesh that can be used to draw grid planes for the current coordinate system.
    )"_unindentHelp};

const ProcessorInfo GridPlanes::getProcessorInfo() const { return processorInfo_; }

GridPlanes::GridPlanes()
    : Processor()
    , basis_{"basis",
             "Optional volume inport. If a volume is connected the grid "
             "will be aligned to that volume."_help}
    , grid_{"grid",
            "A mesh containing the grid planes, can be rendered using, "
            "for example, the Mesh Renderer, Line Renderer or Tube Renderer."_help}
    , enable_{"enable", "Enable",
              "Toggles whether or not a given grid plane should be visible"_help, true}
    , spacing_{"spacing", "Spacing",
               "Set the distance between the each line along the given axis"_help, 0.1f}
    , extent_{"extent", "Extent", "Set the extent of the grid along the given axis."_help,
              -1.05f,   1.05f,    -100.f,
              100.f}
    , color_{"color", "Color",
             util::ordinalColor(vec4{0.5f, 0.5f, 0.5f, 1.f})
                 .set("Set the color of each grid plane."_help)} {

    basis_.setOptional(true);
    addPorts(basis_, grid_);
    addProperties(enable_, spacing_, extent_, color_);
}

void GridPlanes::process() {
    auto mesh = std::make_shared<ColoredMesh>();

    auto& ib = mesh->addIndexBuffer(DrawType::Lines, ConnectivityType::None)->getDataContainer();

    if (auto transform = basis_.getData()) {
        mesh->setModelMatrix(transform->getModelMatrix());
        mesh->setWorldMatrix(transform->getWorldMatrix());
    }

    std::array<std::vector<float>, 3> vals;
    for (unsigned i = 0; i < 3; i++) {
        const auto spacing = spacing_.get(i);
        const auto extent = extent_.get(i);

        int start = static_cast<int>(extent.x / spacing);
        const int end = static_cast<int>(extent.y / spacing);

        for (; start <= end; start++) {
            vals[i].emplace_back(start * spacing);
        }
    }

    auto createLines = [&](unsigned a, unsigned b, unsigned c) {
        const auto color = color_.get(a);
        const auto extent = extent_.get(c);

        vec3 start{0.0f};
        vec3 end{0.0f};
        start[c] = extent.x;
        end[c] = extent.y;

        for (auto v : vals[b]) {
            start[b] = v;
            end[b] = v;
            auto i0 = mesh->addVertex(start, color);
            auto i1 = mesh->addVertex(end, color);
            ib.push_back(i0);
            ib.push_back(i1);
        }
    };

    auto createGrid = [&](unsigned a, unsigned b, unsigned c) {
        if (!enable_.get(a)) return;
        createLines(a, b, c);
        createLines(a, c, b);
    };

    const static unsigned X = 0;
    const static unsigned Y = 1;
    const static unsigned Z = 2;

    createGrid(X, Y, Z);
    createGrid(Y, X, Z);
    createGrid(Z, X, Y);

    grid_.setData(mesh);
}
}  // namespace inviwo
