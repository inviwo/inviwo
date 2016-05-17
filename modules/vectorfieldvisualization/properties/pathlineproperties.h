/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#ifndef IVW_PATHLINEPROPERTIES_H
#define IVW_PATHLINEPROPERTIES_H

#include <inviwo/core/common/inviwo.h>
#include <modules/vectorfieldvisualization/properties/integrallineproperties.h>
#include <modules/vectorfieldvisualization/vectorfieldvisualizationmoduledefine.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>

namespace inviwo {

/**
 * \class PathLineProperties
 * \brief VERY_BRIEFLY_DESCRIBE_THE_CLASS
 * DESCRIBE_THE_CLASS
 */
class IVW_MODULE_VECTORFIELDVISUALIZATION_API PathLineProperties : public IntegralLineProperties {
public:
    InviwoPropertyInfo();

    PathLineProperties(std::string identifier, std::string displayName);
    PathLineProperties(const PathLineProperties& rhs);
    PathLineProperties& operator=(const PathLineProperties& that);
    virtual PathLineProperties* clone() const override;
    virtual ~PathLineProperties();

    double getStartT()const { return startT_.get(); }


    void deserialize(Deserializer& d) override;

    bool isLoopingAllowed()const { return allowLooping_.get(); }

private:
    void setUpProperties();

protected:
    DoubleProperty startT_;
    BoolProperty allowLooping_;
};

}  // namespace

#endif  // IVW_PATHLINEPROPERTIES_H
