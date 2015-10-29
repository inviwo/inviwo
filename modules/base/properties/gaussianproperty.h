/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_GAUSSIANPROPERTY_H
#define IVW_GAUSSIANPROPERTY_H

#include <modules/base/basemoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/propertyowner.h>

namespace inviwo {


template<typename T>
class GaussianProperty : public CompositeProperty { 
public:
    GaussianProperty(const std::string &identifier,const std::string &displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = PropertySemantics::Default) 
        : CompositeProperty(identifier, displayName, invalidationLevel, semantics)
        , height_("height_","Height",1.0)
        , sigma_("sigma","Sigma",1.0)
        , center_("center","Center",T(0),T(-1),T(1))
    {
        addProperty(height_);
        addProperty(sigma_);
        addProperty(center_);
    }
    virtual ~GaussianProperty(){}

    virtual double evaluate(const T &inV)const{
        T v = inV - center_.get();
        return height_.get() * std::exp(-glm::pow<double>(static_cast<double>(glm::distance(center_.get() , v)),2.0)/glm::pow<double>(2.0*sigma_.get(),2.0));
    }

    DoubleProperty height_;
    DoubleProperty sigma_;

    OrdinalProperty<T> center_;

private:

};


class IVW_MODULE_BASE_API Gaussian1DProperty : public GaussianProperty<double>{
public:
    Gaussian1DProperty(const std::string &identifier,const std::string &displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = PropertySemantics::Default);
    virtual ~Gaussian1DProperty();

    InviwoPropertyInfo();
    virtual double evaluate(const double &v)const override;
};

class IVW_MODULE_BASE_API Gaussian2DProperty : public GaussianProperty<dvec2>{
public:
    Gaussian2DProperty(const std::string &identifier,const std::string &displayName,
        InvalidationLevel invalidationLevel = InvalidationLevel::InvalidOutput,
        PropertySemantics semantics = PropertySemantics::Default);
    virtual ~Gaussian2DProperty();

    InviwoPropertyInfo();
    virtual double evaluate(const dvec2 &v)const override;
};




} // namespace

#endif // IVW_GAUSSIANPROPERTY_H

