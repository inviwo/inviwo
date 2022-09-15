/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2021-2022 Inviwo Foundation
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

#include <modules/webbrowser/webbrowsersettings.h>

#include <inviwo/core/properties/constraintbehavior.h>  // for ConstraintBehavior, ConstraintBeh...
#include <inviwo/core/properties/invalidationlevel.h>   // for InvalidationLevel, InvalidationLe...
#include <inviwo/core/properties/ordinalproperty.h>     // for IntProperty
#include <inviwo/core/properties/propertysemantics.h>   // for PropertySemantics, PropertySemant...
#include <inviwo/core/util/settings/settings.h>         // for Settings

#include <string_view>                                  // for string_view

namespace inviwo {

WebBrowserSettings::WebBrowserSettings()
    : Settings("Web Browser")
    , refreshRate_("refreshRate", "Refresh Rate (Hz)", 60, {1, ConstraintBehavior::Immutable},
                   {240, ConstraintBehavior::Editable}, 1, InvalidationLevel::InvalidOutput,
                   PropertySemantics::Text) {

    addProperty(refreshRate_);

    load();
}

}  // namespace inviwo
