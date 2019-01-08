/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2018 Inviwo Foundation
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

#ifndef IVW_CONVEXHULL2DPROCESSOR_H
#define IVW_CONVEXHULL2DPROCESSOR_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/ports/meshport.h>

namespace inviwo {

/** \docpage{org.inviwo.ConvexHull2DProcessor, Convex Hull2DProcessor}
 * ![](org.inviwo.ConvexHull2DProcessor.png?classIdentifier=org.inviwo.ConvexHull2DProcessor)
 * Computes the convex hull of a 2D mesh.
 *
 * ### Inports
 *   * __Inport__  Input geometry, only the first two dimensions are considered
 *
 * ### Outports
 *   * __Geometry__ 2D convex hull of the input geometry
 *
 * ### Properties
 *   * __Normal__ Normal of the plane onto which all points are projected prior to computing the
 *                convex hull
 */

/**
 * \class ConvexHull2DProcessor
 * \brief Processor computing the convex hull of a given 2D input geometry
 */
class IVW_MODULE_BASE_API ConvexHull2DProcessor : public Processor {
public:
    ConvexHull2DProcessor();
    virtual ~ConvexHull2DProcessor() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    static std::shared_ptr<Mesh> convertHullToMesh(const std::vector<dvec2> hull);

    MeshInport inport_;
    MeshOutport outport_;
    FloatVec3Property normal_;
};

}  // namespace inviwo

#endif  // IVW_CONVEXHULL2DPROCESSOR_H
