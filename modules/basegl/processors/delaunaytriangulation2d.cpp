/*********************************************************************************
*
* Inviwo - Interactive Visualization Workshop
*
* Copyright (c) 2013-2018 Inviwo Foundation
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

#include "delaunaytriangulation2d.h"

#include "basegl/algorithm/vec2_indexed.h"
#include "basegl/algorithm/edge.h"
#include "basegl/algorithm/triangle.h"
#include "basegl/algorithm/delaunay.h"

#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>

namespace inviwo {

    const ProcessorInfo DelaunayTriangulation2D::processorInfo_{
        "org.inviwo.DelaunayTriangulation2D", // Class identifier
        "Delaunay Triangulation 2D",          // Display name
        "Geometry Processing",                // Category
        CodeState::Experimental,              // Code state
        Tags::CPU,                            // Tags
    };
const ProcessorInfo DelaunayTriangulation2D::getProcessorInfo() const { return processorInfo_; }

    DelaunayTriangulation2D::DelaunayTriangulation2D()
        : Processor()
        , ptsInport_("meshport1")
        , meshOutport_("meshport2")
        , camera_("camera", "Camera", vec3(0.0f, 0.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f))
    {
        addPort(ptsInport_);
        addPort(meshOutport_);

        addProperty(camera_);
    }

    void DelaunayTriangulation2D::process() {
        const auto& view_mat = camera_.viewMatrix();
        const auto& pts = *ptsInport_.getData();
        const vec3 offset{0.0f};

        if (pts.size() >= 3) {
            std::vector<Vec2Indexed> pts_2d;
            pts_2d.reserve(pts.size());
            for (size_t idx = 0; idx < pts.size(); ++idx) {
                const auto& pt = pts[idx];
                // specify dimensions (x, y, or z) or project along normal before
                pts_2d.emplace_back(pt.z, pt.y, idx);
            }

            Delaunay delaunayTriangulation;
            const auto triangles = delaunayTriangulation.triangulate(pts_2d);

            auto mesh = std::make_shared<SimpleMesh>();
            mesh->setModelMatrix(mat4(1.f));

            for (const auto& p : pts) {
                // pos, color, texCoord
                // color and texCoord = pos to match volume coordinate for ray-casting later
                mesh->addVertex(p, p + offset, vec4(p + offset, 1.0f));
            }

            mesh->setIndicesInfo(DrawType::Triangles, ConnectivityType::None);
            for (const auto& t : triangles) {
                mesh->addIndex(static_cast<unsigned int>(t.p1.idx));
                mesh->addIndex(static_cast<unsigned int>(t.p2.idx));
                mesh->addIndex(static_cast<unsigned int>(t.p3.idx));
            }

            meshOutport_.setData(mesh);
        } else {
            LogWarn("not enough points for delaunay triangulation! (only " << pts.size() << " given)");
        }
    }

}  // namespace inviwo
