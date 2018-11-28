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
#include <inviwo/core/datastructures/geometry/geometrytype.h>

namespace inviwo {

const ProcessorInfo DelaunayTriangulation2D::processorInfo_{
    "org.inviwo.DelaunayTriangulation2D",  // Class identifier
    "Delaunay Triangulation 2D",           // Display name
    "Geometry Processing",                 // Category
    CodeState::Experimental,               // Code state
    Tags::CPU,                             // Tags
};

const ProcessorInfo DelaunayTriangulation2D::getProcessorInfo() const { return processorInfo_; }

DelaunayTriangulation2D::DelaunayTriangulation2D()
    : Processor()
    , ptsInport_("pts_in")
    , meshOutport_("mesh_out")
    , meshEdgesOutport_("mesh_edges_out")
    , projectAlongAxis_("project_along_axis", "Axis of Projection")
    , triangulateInViewSpace_("triangulate_in_view_space", "Triangulate in View Space", false)
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f)) {
    addPort(ptsInport_);
    addPort(meshOutport_);
    addPort(meshEdgesOutport_);

    projectAlongAxis_.addOption("x", "y-z plane (X axis)",
        static_cast<int>(CartesianCoordinateAxis::X));
    projectAlongAxis_.addOption("y", "z-x plane (Y axis)",
        static_cast<int>(CartesianCoordinateAxis::Y));
    projectAlongAxis_.addOption("z", "x-y plane (Z axis)",
        static_cast<int>(CartesianCoordinateAxis::Z));
    projectAlongAxis_.set(
        static_cast<int>(CartesianCoordinateAxis::X));
    projectAlongAxis_.setCurrentStateAsDefault();
    addProperty(projectAlongAxis_);

    addProperty(triangulateInViewSpace_);
    addProperty(camera_);
}

void DelaunayTriangulation2D::process() {
    if (ptsInport_.isChanged() || triangulateInViewSpace_ || projectAlongAxis_.isModified()) {
        const auto& pts = *ptsInport_.getData();

        if (pts.size() >= 3) {
            std::vector<Vec2Indexed> pts_2d;
            pts_2d.reserve(pts.size());
            for (size_t idx = 0; idx < pts.size(); ++idx) {
                const auto& pt = pts[idx];
                if (triangulateInViewSpace_) {
                    const auto& view_mat = camera_.viewMatrix();
                    const auto pt_view_space = view_mat * vec4(pt, 1.0f);
                    pts_2d.emplace_back(pt_view_space.x, pt_view_space.y, idx);
                } else {
                    switch (projectAlongAxis_.get()) {
                        case static_cast<int>(CartesianCoordinateAxis::X):
                            pts_2d.emplace_back(pt.y, pt.z, idx);
                            break;
                        case static_cast<int>(CartesianCoordinateAxis::Y):
                            pts_2d.emplace_back(pt.x, pt.z, idx);
                            break;
                        case static_cast<int>(CartesianCoordinateAxis::Z):
                            pts_2d.emplace_back(pt.x, pt.y, idx);
                            break;
                        default:
                            LogError("This should not happen!");
                    }
                }
            }

            Delaunay delaunayTriangulation;
            const auto triangles = delaunayTriangulation.triangulate(pts_2d);

            auto mesh = std::make_shared<SimpleMesh>();
            mesh->setModelMatrix(mat4(1.f));

            for (const auto& p : pts) {
                // color is the important attribute for the ray-caster
                mesh->addVertex(p, p, vec4(p, 1.0f));

                // TODO: add pixel spacing and slice thickness
            }

            auto mesh_edges = mesh->clone();

            mesh->setIndicesInfo(DrawType::Triangles, ConnectivityType::None);
            mesh_edges->setIndicesInfo(DrawType::Lines, ConnectivityType::None);
            for (const auto& t : triangles) {
                // full triangle
                mesh->addIndex(static_cast<unsigned int>(t.p1.idx));
                mesh->addIndex(static_cast<unsigned int>(t.p2.idx));
                mesh->addIndex(static_cast<unsigned int>(t.p3.idx));

                // edges of triangle
                mesh_edges->addIndex(static_cast<unsigned int>(t.p1.idx));
                mesh_edges->addIndex(static_cast<unsigned int>(t.p2.idx));

                mesh_edges->addIndex(static_cast<unsigned int>(t.p2.idx));
                mesh_edges->addIndex(static_cast<unsigned int>(t.p3.idx));

                mesh_edges->addIndex(static_cast<unsigned int>(t.p1.idx));
                mesh_edges->addIndex(static_cast<unsigned int>(t.p3.idx));
            }

            meshOutport_.setData(mesh);
            meshEdgesOutport_.setData(mesh_edges);
        } else {
            LogWarn("not enough points for delaunay triangulation! (only " << pts.size()
                                                                           << " given)");
        }
    }
}

}  // namespace inviwo
