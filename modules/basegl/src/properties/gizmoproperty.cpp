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

#include <modules/basegl/properties/gizmoproperty.h>

namespace inviwo {

std::string_view GizmoProperty::getClassIdentifier() const { return classIdentifier; }

GizmoProperty::GizmoProperty(std::string_view identifier, std::string_view displayName,
                             Document help, InvalidationLevel invalidationLevel,
                             PropertySemantics semantics)
    : CompositeProperty{identifier, displayName, std::move(help), invalidationLevel,
                        std::move(semantics)}
    , widgetSize{"widgetSize",
                 "Widget Size (px)",
                 GizmoConfig::defaultWidgetSize,
                 {30, ConstraintBehavior::Immutable},
                 {300, ConstraintBehavior::Ignore}}
    , lineWidth{"lineWidth", "Line Width (px)",
                util::ordinalLength(GizmoConfig::defaultLineWidth, 10.0f)}
    , pickingLineWidth{"pickingLineWidth", "Picking Line Width (px)",
                       util::ordinalLength(GizmoConfig::defaultPickingLineWidth, 10.0f)}
    , trackballVisible{"trackballVisible", "Trackball Visible",
                       GizmoConfig::defaultTrackballVisible}
    , axisColors{FloatVec3Property{"colorX", "Color X Axis",
                                   util::ordinalColor(GizmoConfig::defaultXAxisColor)},
                 FloatVec3Property{"colorY", "Color Y Axis",
                                   util::ordinalColor(GizmoConfig::defaultYAxisColor)},
                 FloatVec3Property{"colorZ", "Color Z Axis",
                                   util::ordinalColor(GizmoConfig::defaultZAxisColor)}}
    , neutralColor{"neutralColor", "Neutral Element Color",
                   util::ordinalColor(GizmoConfig::defaultNeutralColor)}
    , alpha{"alpha",
            "Alpha",
            GizmoConfig::defaultAlpha,
            {0.0f, ConstraintBehavior::Immutable},
            {1.0f, ConstraintBehavior::Immutable},
            0.001f} {

    addProperties(widgetSize, lineWidth, pickingLineWidth, trackballVisible, axisColors[0],
                  axisColors[1], axisColors[2], neutralColor, alpha);
}

GizmoProperty::GizmoProperty(const GizmoProperty& rhs)
    : CompositeProperty{rhs}
    , widgetSize{rhs.widgetSize}
    , lineWidth{rhs.lineWidth}
    , pickingLineWidth{rhs.pickingLineWidth}
    , trackballVisible{rhs.trackballVisible}
    , axisColors{rhs.axisColors}
    , neutralColor{rhs.neutralColor}
    , alpha{rhs.alpha} {

    addProperties(widgetSize, lineWidth, pickingLineWidth, trackballVisible, axisColors[0],
                  axisColors[1], axisColors[2], neutralColor, alpha);
}

GizmoProperty* GizmoProperty::clone() const { return new GizmoProperty(*this); }

void GizmoProperty::update(GizmoConfig& config) const {
    config.widgetSize = widgetSize.get();
    config.lineWidth = lineWidth.get();
    config.pickingLineWidth = pickingLineWidth.get();
    config.trackballVisible = trackballVisible.get();
    config.xAxisColor = axisColors[0].get();
    config.yAxisColor = axisColors[1].get();
    config.zAxisColor = axisColors[2].get();
    config.neutralColor = neutralColor.get();
    config.alpha = alpha.get();
}

}  // namespace inviwo
