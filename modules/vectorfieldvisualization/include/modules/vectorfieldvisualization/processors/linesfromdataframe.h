/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021 Inviwo Foundation
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

#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <unordered_map>
#include <string>

namespace inviwo {

/** \docpage{org.inviwo.LinesFromDataFrame, Lines From Data Frame}
 * ![](org.inviwo.LinesFromDataFrame.png?classIdentifier=org.inviwo.LinesFromDataFrame)
 * Create an IntegralLineSet from a DataFrame.
 *
 * Midges: id x z y t vx vz vy ax az ay
 * Column 1, Column 2 ....
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API LinesFromDataFrame : public Processor {
public:
    LinesFromDataFrame();
    virtual ~LinesFromDataFrame() = default;

    virtual void process() override;
    void updateColumns();

    void deserialize(Deserializer& d) override;
    void serialize(Serializer& s) const override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataFrameInport dataIn_;
    IntegralLineSetOutport linesOut_;

    OptionPropertyString timeColumn_;
    DoubleProperty startTime_;
    CompositeProperty columnsForPosition_;
    BoolCompositeProperty columnsForVelocity_, columnsForAcceleration_;
    BoolProperty updateOutput_;

    enum PointData : char { None = 0, Position = 1, Velocity = 2, Acceleration = 4 };
    // Maps from the DataFrame column name to a flag of the line data it is assigned to.
    std::unordered_map<std::string, char> columnDataMap_;
};

}  // namespace inviwo
