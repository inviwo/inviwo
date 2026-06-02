/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2026 Inviwo Foundation
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

#pragma once

#include <modules/plottinggl/plottingglmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/ports/volumeport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/datastructures/geometry/geometrytype.h>
#include <inviwo/core/datastructures/geometry/typedmesh.h>
#include <modules/plotting/properties/axisproperty.h>
#include <modules/plotting/properties/axisstyleproperty.h>
#include <modules/plottinggl/utils/axisrenderer.h>
#include <modules/basegl/rendering/linerenderer.h>

namespace inviwo::plot {

struct MeasurementSM;

class IVW_MODULE_PLOTTINGGL_API MeasureDistance3D : public Processor {
public:
    MeasureDistance3D();
    ~MeasureDistance3D();

    virtual void process() override;

    virtual void serialize(Serializer& s) const override;
    virtual void deserialize(Deserializer& d) override;

    virtual void invokeEvent(Event* event) override;

    void setLocator(size_t index, dvec3 position);
    void reset();

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    ImageInport inport_;
    MeshInport mesh_;
    LayerInport layer_;
    VolumeInport volume_;
    ImageOutport outport_;

    BoolProperty enabled_;
    std::array<DoubleVec3Property, 2> positions_;
    DoubleProperty distance_;
    ButtonProperty clearMeasurement_;
    FloatVec4Property locatorColor_;
    IntProperty locatorSize_;
    FloatProperty locatorLineWidth_;
    BoolProperty renderOnTop_;
    AxisStyleProperty style_;
    AxisProperty measurementAxis_;
    CameraProperty camera_;

    EventProperty placeLocatorEvent_;
    EventProperty cancelMeasurementEvent_;

    AxisRenderer3D axisRenderer_;
    algorithm::LineRenderer lineRenderer_;

    // MeasurementState measurementState_;

    std::unique_ptr<MeasurementSM> sm;

    using PositionMesh = TypedMesh<buffertraits::PositionsBuffer3D>;
    PositionMesh locatorMesh_{DrawType::Lines,
                              ConnectivityType::None,
                              {
                                  {{-1.0f, 0.0f, 0.0f}},
                                  {{1.0f, 0.0f, 0.0f}},
                                  {{0.0f, -1.0f, 0.0f}},
                                  {{0.0f, 1.0f, 0.0f}},
                                  {{0.0f, 0.0f, -1.0f}},
                                  {{0.0f, 0.0f, 1.0f}},
                              },
                              {0, 1, 2, 3, 4, 5}};
};

}  // namespace inviwo::plot
