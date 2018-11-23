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

#include "polylinegrabber.h"
#include "basegl/algorithm/vec2_indexed.h"
#include "basegl/algorithm/edge.h"
#include "basegl/algorithm/triangle.h"
#include "basegl/algorithm/delaunay.h"

#include <inviwo/core/datastructures/geometry/simplemeshcreator.h>

#include <fstream>

namespace inviwo {

    const ProcessorInfo PolylineGrabber::processorInfo_{
        "org.inviwo.PolylineGrabber",  // Class identifier
        "Polyline Grabber",              // Display name
        "Input",          // Category
        CodeState::Experimental,           // Code state
        Tags::CPU,                    // Tags
    };
    const ProcessorInfo PolylineGrabber::getProcessorInfo() const { return processorInfo_; }

    PolylineGrabber::PolylineGrabber()
        : Processor()
        , pt_("pt", "Point to Add")
        , readyToRecord_(false)
        , clearPolyline_("clearpolyline", "Clear Points")
        , loadExamplePolyline_("loadexamplepolyline", "Load Example Polyline")
        , performDelaunayOnPts_("performdelaunay", "Perform Delaunay")
        , offset_("offset", "Offset", 0.0f, -1.0f, 1.0f, 0.001f)
        , polyline_(std::make_shared<std::vector<vec3>>())
        , clip_("clip", "Clip Polyline", 0.0f, 1.0f)
        , outport_("polylineport")
        , meshOutport1_("meshport1")
        , meshOutport2_("meshport2")
    {
        outport_.setData(polyline_);
        addPort(outport_);
        addPort(meshOutport1_);
        addPort(meshOutport2_);

        clearPolyline_.onChange([this]() { 
            polyline_->clear();
            invalidate(InvalidationLevel::InvalidOutput);
        });
        addProperty(clearPolyline_);

        loadExamplePolyline_.onChange([this]() {
            polyline_->clear();

            // volume 2143.dat dimensions: 256 x 256 x 255
            std::ifstream inputStream("D:/data/2134.dijkstra");
            if (inputStream.is_open()) {
                vec3 pt;
                while (inputStream >> pt.x >> pt.y >> pt.z) {
                    LogInfo("point = " << pt);
                    addPoint(pt / vec3(256, 256, 255));
                }
            }
            inputStream.close();

            invalidate(InvalidationLevel::InvalidOutput);
        });
        addProperty(loadExamplePolyline_);

        performDelaunayOnPts_.onChange([this]() {
            if (polyline_->size() >= 3) {
                std::vector<Vec2Indexed> pts_2d;
                pts_2d.reserve(polyline_->size());
                for (size_t idx = 0; idx < polyline_->size(); ++idx) {
                    const auto& pt = polyline_->at(idx);
                    // specify dimensions (x, y, or z) or project along normal before
                    pts_2d.emplace_back(pt.z, pt.y, idx);
                }

                Delaunay delaunayTriangulation;
                const auto triangles = delaunayTriangulation.triangulate(pts_2d);

                auto mesh = std::make_shared<SimpleMesh>();
                mesh->setModelMatrix(mat4(1.f));
                for (const auto& v : *polyline_) {
                    mesh->addVertex(v, v, vec4(v, 1.0f));
                }
                mesh->setIndicesInfo(DrawType::Triangles, ConnectivityType::None);
                for (const auto& t : triangles) {
                    mesh->addIndex(static_cast<unsigned int>(t.p1.idx));
                    mesh->addIndex(static_cast<unsigned int>(t.p2.idx));
                    mesh->addIndex(static_cast<unsigned int>(t.p3.idx));
                }

                for (size_t idx = 0; idx < polyline_->size(); ++idx) {
                    vec3 normal{0.0f};
                    size_t n_faces{0};
                    for (const auto& t : triangles) {
                        if (idx == t.p1.idx || idx == t.p2.idx || idx == t.p3.idx) {
                            const vec3 triangle_normal = glm::normalize(glm::cross(
                                polyline_->at(t.p1.idx) - polyline_->at(t.p2.idx),
                                polyline_->at(t.p1.idx) - polyline_->at(t.p3.idx)
                            ));
                            normal += triangle_normal;
                            n_faces++;
                        }
                    }
                    
                    if (n_faces > 0) {
                        normal = glm::normalize(normal);
                    }
                }

                meshOutport1_.setData(mesh);

                mesh = std::make_shared<SimpleMesh>();
                mesh->setModelMatrix(mat4(1.f));
                for (const auto& v : *polyline_) {
                    mesh->addVertex(v, v + vec3(offset_), vec4(v + vec3(offset_), 1.0f));
                }
                mesh->setIndicesInfo(DrawType::Triangles, ConnectivityType::None);
                for (const auto& t : triangles) {
                    mesh->addIndex(static_cast<unsigned int>(t.p1.idx));
                    mesh->addIndex(static_cast<unsigned int>(t.p2.idx));
                    mesh->addIndex(static_cast<unsigned int>(t.p3.idx));
                }

                for (size_t idx = 0; idx < polyline_->size(); ++idx) {
                    vec3 normal{0.0f};
                    size_t n_faces{0};
                    for (const auto& t : triangles) {
                        if (idx == t.p1.idx || idx == t.p2.idx || idx == t.p3.idx) {
                            const vec3 triangle_normal = glm::normalize(
                                glm::cross(polyline_->at(t.p1.idx) - polyline_->at(t.p2.idx),
                                           polyline_->at(t.p1.idx) - polyline_->at(t.p3.idx)));
                            normal += triangle_normal;
                            n_faces++;
                        }
                    }

                    if (n_faces > 0) {
                        normal = glm::normalize(normal);
                    }
                }

                meshOutport2_.setData(mesh);

                invalidate(InvalidationLevel::InvalidOutput);
            } else {
                LogWarn("not enough points for triangulation!");
            }
        });
        addProperty(performDelaunayOnPts_);

        addProperty(offset_);

        clip_.onChange([this]() {
            const auto from =
                static_cast<size_t>(clip_.getStart() * static_cast<float>(polyline_->size()));
            const size_t to =
                static_cast<size_t>(clip_.getEnd() * static_cast<float>(polyline_->size()));
            std::vector<vec3>::const_iterator first = polyline_->begin() + from;
            std::vector<vec3>::const_iterator last = polyline_->begin() + to;
            auto newVec = std::make_shared<std::vector<vec3>>(first, last);
            outport_.setData(newVec);
            invalidate(InvalidationLevel::InvalidOutput);
        });
        addProperty(clip_);

        //pt_.setVisible(false);
        pt_.setReadOnly(true);
        pt_.onChange([this]() { addPoint(pt_); });
        addProperty(pt_);

        //TODO smoothen the normal

        //TODO color points whether they are on slice or above or below

        //TODO implement move and delete points
    }

    void PolylineGrabber::addPoint(const vec3& pt)
    {
        if (readyToRecord_) {
            polyline_->push_back(pt);
        }

        readyToRecord_ = true;
        invalidate(InvalidationLevel::InvalidOutput);
    }

}  // namespace inviwo
