/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/hdf5/hdf5moduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <modules/hdf5/ports/hdf5port.h>
#include <modules/hdf5/hdf5utils.h>
#include <modules/hdf5/properties/dimselectionsproperty.h>
#include <inviwo/core/datastructures/image/layer.h>
#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>

#include <modules/base/properties/layerinformationproperty.h>

#include <memory>
#include <vector>

namespace inviwo::hdf5 {

class IVW_MODULE_HDF5_API HDF5ToLayer : public Processor {
public:
    HDF5ToLayer();
    HDF5ToLayer(const HDF5ToLayer&) = delete;
    HDF5ToLayer& operator=(const HDF5ToLayer&) = delete;
    HDF5ToLayer(HDF5ToLayer&&) = delete;
    HDF5ToLayer& operator=(HDF5ToLayer&&) = delete;
    virtual ~HDF5ToLayer();

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    virtual void process() override;
    virtual void deserialize(Deserializer& d) override;

private:
    void makeLayer();
    void onDataChange();
    void onSelectionChange();

    std::vector<DataSetInfo> layerMatches_;

    Inport inport_;
    LayerOutport outport_;
    std::shared_ptr<Layer> layer_;

    OptionPropertyString layerSelection_;

    BoolProperty automaticEvaluation_;
    ButtonProperty evaluate_;

    LayerInformationProperty information_;

    CompositeProperty outputGroup_;
    OptionPropertyInt datatype_;
    DimSelectionsProperty selection_;

    bool dirty_;
    bool deserialized_ = false;
};

}  // namespace inviwo::hdf5
