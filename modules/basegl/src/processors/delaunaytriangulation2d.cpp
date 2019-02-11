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

#include <modules/basegl/processors/delaunaytriangulation2d.h>

#include <modules/basegl/algorithm/edge.h>
#include <modules/basegl/algorithm/triangle.h>
#include <modules/basegl/algorithm/delaunay.h>

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
    , axisOfProjection_("project_along_axis", "Axis of Projection")
    , volumeVoxelSpacing_("volumeVoxelSpacing", "Volume Voxel Spacing", vec3{1.0f}, vec3{1e-9f},
                          vec3{1e9f}, vec3{1e-3f})
    , triangulateInViewSpace_("triangulate_in_view_space", "Triangulate in View Space", false)
    , camera_("camera", "Camera", vec3(0.0f, 0.0f, 10.0f), vec3(0.0f, 0.0f, 0.0f),
              vec3(0.0f, 1.0f, 0.0f))
    , planeNormal_("planeNormal", "Plane Normal", vec3(1.0f, 0.0f, 0.0f), vec3(-1.0f),
                   vec3(1.0f), vec3(0.001f)) {
    addPort(ptsInport_);
    addPort(meshOutport_);
    addPort(meshEdgesOutport_);

    axisOfProjection_.addOption("x", "y-z plane (X axis)",
        static_cast<int>(CartesianCoordinateAxis::X));
    axisOfProjection_.addOption("y", "z-x plane (Y axis)",
        static_cast<int>(CartesianCoordinateAxis::Y));
    axisOfProjection_.addOption("z", "x-y plane (Z axis)",
        static_cast<int>(CartesianCoordinateAxis::Z));
    axisOfProjection_.addOption("p", "Plane Equation", 3);
    axisOfProjection_.set(
        static_cast<int>(CartesianCoordinateAxis::X));
    axisOfProjection_.setCurrentStateAsDefault();
    addProperty(axisOfProjection_);

    addProperty(triangulateInViewSpace_);
    addProperty(camera_);
    addProperty(planeNormal_);
}

std::vector<Vec2Indexed> DelaunayTriangulation2D::setupPoints2D(
    const std::vector<vec3>& pts_3d)
{
    std::vector<Vec2Indexed> pts_2d;

    if (pts_3d.size() >= 3 && glm::length(planeNormal_.get()) > 0.0f) {
        planeNormal_.setReadOnly(true);

        // find smalles component of current normal and set up 1st. basis vector of new
        // local coordinate system
        const auto n = glm::normalize(planeNormal_.get());
        vec3 tmp;
        if (glm::abs(n.x) <= glm::abs(n.y) &&
            glm::abs(n.x) <= glm::abs(n.z)) {  // x component is smallest
            tmp = vec3(1.0f, 0.0f, 0.0f);
        } else if (glm::abs(n.y) <= glm::abs(n.z)) {  // y component is smallest
            tmp = vec3(0.0f, 1.0f, 0.0f);
        } else {  // z component is smallest
            tmp = vec3(0.0f, 0.0f, 1.0f);
        }

        // setup new basis if general plane equation is used
        const auto basis_1 = glm::cross(n, tmp);
        const auto basis_2 = glm::cross(n, basis_1);

        pts_2d.reserve(pts_3d.size());
        for (size_t idx = 0; idx < pts_3d.size(); ++idx) {
            const auto& pt = pts_3d[idx];
            if (triangulateInViewSpace_) {
                const auto& view_mat = camera_.viewMatrix();
                const auto pt_view_space = view_mat * vec4(pt, 1.0f);
                pts_2d.emplace_back(pt_view_space.x, pt_view_space.y, idx);
            } else {
                switch (axisOfProjection_.get()) {
                    case static_cast<int>(CartesianCoordinateAxis::X): {
                        pts_2d.emplace_back(pt.y, pt.z, idx);
                        break;
                    }
                    case static_cast<int>(CartesianCoordinateAxis::Y): {
                        pts_2d.emplace_back(pt.x, pt.z, idx);
                        break;
                    }
                    case static_cast<int>(CartesianCoordinateAxis::Z): {
                        pts_2d.emplace_back(pt.x, pt.y, idx);
                        break;
                    }
                    case 3: {  // general plane equation
                        planeNormal_.setReadOnly(false);

                        // ### projection along normal ###
                        // add 2D point projected into new basis
                        pts_2d.emplace_back(glm::dot(basis_1, pt), glm::dot(basis_2, pt), idx);
                        break;
                    }
                    default: {
                        LogError("This should not happen!");
                        break;
                    }
                }
            }
        }
    }

    return pts_2d;
}

std::pair<std::shared_ptr<SimpleMesh>, std::shared_ptr<SimpleMesh>>
DelaunayTriangulation2D::createMeshFrom2dPts(
    const std::vector<Vec2Indexed>& pts_2d,
    const std::vector<vec3>& pts_3d) const
{
    std::shared_ptr<SimpleMesh> mesh = std::make_shared<SimpleMesh>(), mesh_edges;

    if (pts_2d.size() >= 3) {
        Delaunay delaunayTriangulation;
        const auto triangles = delaunayTriangulation.triangulate(pts_2d);

        if (!triangles.empty()) {
            mesh->setModelMatrix(mat4(1.f));

            for (const auto& p : pts_3d) {
                // color is the important attribute for the ray-caster
                mesh->addVertex(volumeVoxelSpacing_.get() * p, p, vec4(p, 1.0f));
            }

            // TODO: fix this conversion
            mesh_edges = std::shared_ptr<SimpleMesh>(mesh->clone());

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
        } else {
            LogWarn("triangulation yielded no triangles, all points are potentially in one plane.");
        }
    } else {
        LogWarn("not enough points for delaunay triangulation! (only " << pts_2d.size()
                                                                       << " given)");
    }

    return {mesh, mesh_edges};
}

void DelaunayTriangulation2D::process() {
    // check for all properties that have to be changes in order to trigger a new triangulation
    // otherwise, a re-calculation is always triggered, even if you rotate the mesh in the 3D canvas
    const auto readyToProcess = ptsInport_.isChanged() ||
                              axisOfProjection_.isModified() ||
                              planeNormal_.isModified() ||
                              triangulateInViewSpace_;
    if (!readyToProcess) {
        return;
    }

    const auto& pts_3d = *ptsInport_.getData();
    const auto pts_2d = setupPoints2D(pts_3d);

    std::shared_ptr<SimpleMesh> mesh, mesh_edges;
    std::tie(mesh, mesh_edges) = createMeshFrom2dPts(pts_2d, pts_3d);

    if (mesh && mesh_edges) {
        meshOutport_.setData(mesh);
        meshEdgesOutport_.setData(mesh_edges);
    }
}

}  // namespace inviwo
