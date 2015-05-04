/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_VOLUMEINDICATORPROPERTY_H
#define IVW_VOLUMEINDICATORPROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/baseoptionproperty.h>
#include <inviwo/core/properties/planeproperty.h>

namespace inviwo {

/**
 * \class VolumeIndicatorProperty
 *
 * \brief A property for highlighting a plane, line of point in 3D
 *
 * The property is used to represent an indicator in a 3D volume the indicator
 * can be a set of planes, a set of lines, or some other.
 */
class IVW_CORE_API VolumeIndicatorProperty : public CompositeProperty { 
public:
    InviwoPropertyInfo();

    VolumeIndicatorProperty(std::string identifier, std::string displayName,
                            InvalidationLevel invalidationLevel = INVALID_RESOURCES,
                            PropertySemantics semantics = PropertySemantics::Default);

    VolumeIndicatorProperty(const VolumeIndicatorProperty& rhs);
    VolumeIndicatorProperty& operator=(const VolumeIndicatorProperty& that);
    virtual VolumeIndicatorProperty* clone() const;
    virtual ~VolumeIndicatorProperty();
    virtual std::string getClassIdentifierForWidget() const;

    BoolProperty enable_;

    OptionPropertyInt mode_;

    PlaneProperty plane1_;
    PlaneProperty plane2_;
    PlaneProperty plane3_;

private:
    void onModeChange();
};

} // namespace

#endif // IVW_VOLUMEINDICATORPROPERTY_H

