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
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <modules/vectorfieldvisualization/datastructures/integrallineset.h>

namespace inviwo {

/** \docpage{org.inviwo.LineSourceASCII, Line Source ASCII}
 * ![](org.inviwo.LineSourceASCII.png?classIdentifier=org.inviwo.LineSourceASCII)
 * Loads GDP drifter data into an IntegralLineSet.
 * GDP drifter data is given in an ASCII table.
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API LineSourceASCII : public Processor {
public:
    LineSourceASCII();
    virtual ~LineSourceASCII() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    IntegralLineSetOutport linesOut_;

    FileProperty inputFile_;
    BoolProperty maxLines_;
    IntSizeTProperty maxNumLines_;
    BoolProperty overflowWrapping_;
    class SeedFilter : public BoolCompositeProperty {
    public:
        DoubleVec3Property center_;
        DoubleProperty radius_;

        SeedFilter(const std::string& identifier, const std::string& displayName)
            : BoolCompositeProperty(identifier, displayName)
            , center_("center", "Center", dvec3{0, 0, 0}, dvec3{-180, -90, 0}, dvec3{180, 90, 0})
            , radius_("radius", "Radius", 10, 0.001, 45) {
            addProperties(center_, radius_);
        }
    } filterSeed_;
    // DoubleVec2Property
};

}  // namespace inviwo
