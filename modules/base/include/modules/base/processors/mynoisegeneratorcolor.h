/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2024 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>

#include <inviwo/core/ports/layerport.h>
#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/ports/meshport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <modules/base/properties/layerinformationproperty.h>
#include <modules/base/properties/basisproperty.h>
#include <inviwo/core/datastructures/gaussianorbital.h>
#include <random>

namespace inviwo {

/**
 * \brief A processor to generate a noise image
 */

/* class alignas(16) CustomType {
public:
    static constexpr uvec3 colorCode{188, 101, 101};
    static constexpr std::string_view classIdentifier{"org.inviwo.CustomType"};
    static constexpr std::string dataName{"CustomType"};
    CustomType() : p{}, coefs{} {}
    CustomType(const vec4& p,const vec3& coefs) : p{p}, coefs{coefs} {}
    CustomType(const CustomType& other) : p{other.p}, coefs{other.coefs} {}
    CustomType(CustomType&& other)
        : p{std::move(other.p)}, coefs{std::move(other.coefs)}
        {}
    CustomType& operator=(CustomType& other)
    {
        std::swap(p, other.p);
        std::swap(coefs, other.coefs);
        return *this;
    }
    CustomType& operator=(CustomType&& other) {
        p = std::move(other.p);
        coefs = std::move(other.coefs);
        return *this;
    }
    ~CustomType() = default;
    
    vec4 p;
    vec3 coefs;
};*/
class IVW_MODULE_BASE_API MyNoiseGeneratorColor : public Processor {
public:
    MyNoiseGeneratorColor();

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

protected:
    
    DataOutport<std::vector<vec4>> points_;
    
    DataOutport<std::vector<GaussianOrbital>> orbitals_;
    LayerOutport pointsLayer_;
    MeshOutport mesh_;
    
    
    IntSizeTProperty size_;           ///< Size of the output image.
    FloatProperty radii_;
    IntSizeTProperty seed_; 

private:
};

}  // namespace inviwo
