/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2026 Inviwo Foundation
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

#include <modules/plottinggl/plottingglmodule.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/io/serialization/ticpp.h>
#include <inviwo/core/io/serialization/versionconverter.h>
#include <inviwo/core/rendering/datavisualizer.h>
#include <modules/opengl/shader/shadermanager.h>
#include <modules/plottinggl/datavisualizer/pcpdataframevisualizer.h>
#include <modules/plottinggl/datavisualizer/scatterplotdataframevisualizer.h>
#include <modules/plottinggl/plotters/scatterplotgl.h>
#include <modules/plottinggl/processors/axisrenderprocessor.h>
#include <modules/plottinggl/processors/colorscalelegend.h>
#include <modules/plottinggl/processors/imageplotprocessor.h>
#include <modules/plottinggl/processors/layeraxis.h>
#include <modules/plottinggl/processors/meshaxis.h>
#include <modules/plottinggl/processors/orthographicaxis2d.h>
#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>
#include <modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h>
#include <modules/plottinggl/processors/persistencediagramplotprocessor.h>
#include <modules/plottinggl/processors/scatterplotmatrixprocessor.h>
#include <modules/plottinggl/processors/scatterplotprocessor.h>
#include <modules/plottinggl/processors/volumeaxis.h>

#include <functional>
#include <string>
#include <string_view>

#include <fmt/core.h>

namespace inviwo {
class InviwoApplication;

PlottingGLModule::PlottingGLModule(InviwoApplication* app) : InviwoModule(app, "PlottingGL") {

    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    registerProcessor<OrthographicAxis2D>();
    registerProcessor<plot::AxisRenderProcessor>();
    registerProcessor<plot::ColorScaleLegend>();
    registerProcessor<plot::ImagePlotProcessor>();
    registerProcessor<plot::LayerAxis>();
    registerProcessor<plot::MeshAxis>();
    registerProcessor<plot::ParallelCoordinates>();
    registerProcessor<plot::PersistenceDiagramPlotProcessor>();
    registerProcessor<plot::ScatterPlotMatrixProcessor>();
    registerProcessor<plot::ScatterPlotProcessor>();
    registerProcessor<plot::VolumeAxis>();

    registerProperty<plot::ScatterPlotGL::Properties>();
    registerProperty<plot::PCPAxisSettings>();

    registerDataVisualizer(std::make_unique<PCPDataFrameVisualizer>(app));
    registerDataVisualizer(std::make_unique<ScatterPlotDataFrameVisualizer>(app));
}

int PlottingGLModule::getVersion() const { return 7; }

std::unique_ptr<VersionConverter> PlottingGLModule::getConverter(int version) const {
    return std::make_unique<Converter>(version);
}

PlottingGLModule::Converter::Converter(int version) : version_(version) {}

namespace {

void convertDoubleMinMaxPropertyToVec2(TxElement* node) {
    node->SetAttribute("type", "DoubleVec2Property");

    using ET = std::underlying_type_t<ConstraintBehavior>;
    const std::string ct = fmt::to_string(static_cast<ET>(ConstraintBehavior::Ignore));

    TxElement minConstraint{"minConstraint"};
    minConstraint.AddAttribute("content", ct);
    TxElement maxConstraint{"maxConstraint"};
    maxConstraint.AddAttribute("content", ct);
    node->InsertEndChild(minConstraint);
    node->InsertEndChild(maxConstraint);
    if (auto* range = xml::getElement(node, "range")) {
        const auto minVal = range->GetAttribute("x");
        const auto maxVal = range->GetAttribute("y");
        TxElement minValNode{"minvale"};
        minValNode.SetAttribute("x", minVal);
        minValNode.SetAttribute("y", minVal);
        TxElement maxValNode{"maxvale"};
        maxValNode.SetAttribute("x", maxVal);
        maxValNode.SetAttribute("y", maxVal);

        node->RemoveChild(range);
        node->InsertEndChild(minValNode);
        node->InsertEndChild(maxValNode);
    }
    if (auto* increment = xml::getElement(node, "increment")) {
        const auto inc = increment->GetAttribute("content");
        increment->RemoveAttribute("content");
        increment->SetAttribute("x", inc);
        increment->SetAttribute("y", inc);
    }
    if (auto* minSep = xml::getElement(node, "minSeparation")) {
        node->RemoveChild(minSep);
    }
}

bool updateV0(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&res](TxElement* node) -> bool {
        if (const auto& key = node->Value(); key != "Processor") return true;
        if (const auto& type = node->GetAttribute("type");
            type != "org.inviwo.ParallelCoordinates") {
            return true;
        }

        auto* props = xml::getElement(node, "Properties");
        if (auto* tf = xml::getElement(node,
                                       "Properties/Property&identifier=colors/Properties/"
                                       "Property&identifier=tf")) {
            props->InsertEndChild(*tf);
            res = true;
        }

        if (auto* color = xml::getElement(node,
                                          "Properties/Property&identifier=colors/Properties/"
                                          "Property&identifier=selectedColorAxis")) {
            props->InsertEndChild(*color);
            res = true;
        }

        return true;
    }};

    conv.convert(root);
    return res;
}

bool updateV1(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&res](TxElement* node) -> bool {
        const auto& key = node->Value();
        if (key != "Processor") return true;
        const auto& type = node->GetAttribute("type");
        if (type != "org.inviwo.ParallelCoordinates") return true;

        auto* props = xml::getElement(node, "Properties");
        TxElement compNode{"Property"};
        compNode.SetAttribute("type", "org.inviwo.DataFrameColormapProperty");
        compNode.SetAttribute("identifier", "colormap");

        TxElement propNode{"Properties"};
        // Move old TF and selected color into new DataFrameColormapProperty
        if (auto* tf = xml::getElement(props, "Property&identifier=tf")) {
            propNode.InsertEndChild(*tf);
        }

        if (auto* color = xml::getElement(props, "Property&identifier=selectedColorAxis")) {
            propNode.InsertEndChild(*color);
        }

        // Override colormap so that the old one is used
        TxElement boolNode{"Property"};
        boolNode.SetAttribute("type", "org.inviwo.BoolProperty");
        boolNode.SetAttribute("identifier", "overrideColormap");

        TxElement valNode{"value"};
        valNode.SetAttribute("content", "1");

        boolNode.InsertEndChild(valNode);
        propNode.InsertEndChild(boolNode);
        compNode.InsertEndChild(propNode);
        props->InsertEndChild(compNode);

        res = true;

        return true;
    }};
    conv.convert(root);
    return res;
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
bool updateV2(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&res](TxElement* node) -> bool {
        if (const auto& key = node->Value(); key != "Processor") return true;
        const auto& type = node->GetAttribute("type");
        if ((type != "org.inviwo.ScatterPlotProcessor") &&
            (type != "org.inviwo.ScatterPlotMatrixProcessor")) {
            return true;
        }

        if (auto* elem = xml::getElement(node, "Properties/Property&identifier=fontFace")) {
            elem->SetAttribute("type", "org.inviwo.FontFaceOptionProperty");
            res = true;
        }
        if (auto* elem = xml::getElement(node, "Properties/Property&identifier=fontFaceStats")) {
            elem->SetAttribute("type", "org.inviwo.FontFaceOptionProperty");
            res = true;
        }
        if (auto* elem = xml::getElement(node, "Properties/Property&identifier=correlectionTF")) {
            elem->SetAttribute("identifier", "correlationTF");
            res = true;
        }

        const bool isScatterPlotProc = (type == "org.inviwo.ScatterPlotProcessor");
        const std::string_view identifier =
            isScatterPlotProc ? "scatterplot" : "scatterPlotproperties";

        if (auto* plot = xml::getElement(
                node, fmt::format("Properties/Property&identifier={}/Properties", identifier))) {
            // Rename color property
            if (auto* color = xml::getElement(plot, "Property&identifier=color")) {
                color->SetAttribute("identifier", "defaultColor");
            }

            // Move hovering bool property to checked state of a BoolComposite
            {
                TxElement compNode{"Property"};
                compNode.SetAttribute("type", "org.inviwo.BoolCompositeProperty");
                compNode.SetAttribute("identifier", "showHighlighted");

                TxElement propNode{"Properties"};
                if (auto* hovered = xml::getElement(plot, "Property&identifier=hovering")) {
                    hovered->SetAttribute("identifier", "checked");
                    hovered->SetAttribute("displayName", "");
                    propNode.InsertEndChild(*hovered);
                }
                // Change type of hover color to vec3, move to composite
                if (auto* highlightColor =
                        xml::getElement(plot, "Property&identifier=hoverColor")) {
                    highlightColor->SetAttribute("type", "org.inviwo.FloatVec3Property");
                    highlightColor->SetAttribute("identifier", "highlightColor");

                    propNode.InsertEndChild(*highlightColor);

                    if (const auto* value = xml::getElement(highlightColor, "value")) {
                        TxElement alphaNode{"Property"};
                        alphaNode.SetAttribute("type", "org.inviwo.FloatProperty");
                        alphaNode.SetAttribute("identifier", "highlightAlpha");
                        TxElement valueNode("value");

                        valueNode.SetAttribute("content", value->Attribute("w").value_or("1.0"));

                        alphaNode.InsertEndChild(valueNode);
                        propNode.InsertEndChild(alphaNode);
                    }
                }
                compNode.InsertEndChild(propNode);
                plot->InsertEndChild(compNode);
            }

            // Change type of selection color to vec3, move to composite
            {
                TxElement compNode{"Property"};
                compNode.SetAttribute("type", "org.inviwo.BoolCompositeProperty");
                compNode.SetAttribute("identifier", "showSelected");

                TxElement propNode{"Properties"};
                if (auto* highlightColor =
                        xml::getElement(plot, "Property&identifier=selectionColor")) {
                    highlightColor->SetAttribute("type", "org.inviwo.FloatVec3Property");
                    highlightColor->SetAttribute("identifier", "highlightColor");

                    propNode.InsertEndChild(*highlightColor);

                    if (const auto* value = xml::getElement(highlightColor, "value")) {
                        TxElement alphaNode{"Property"};
                        alphaNode.SetAttribute("type", "org.inviwo.FloatProperty");
                        alphaNode.SetAttribute("identifier", "selectionAlpha");
                        TxElement valueNode("value");
                        valueNode.SetAttribute("content", value->Attribute("w").value_or("1.0"));

                        alphaNode.InsertEndChild(valueNode);
                        propNode.InsertEndChild(alphaNode);
                    }
                }
                compNode.InsertEndChild(propNode);
                plot->InsertEndChild(compNode);
            }
            res = true;
        }
        return true;
    }};
    conv.convert(root);
    return res;
}
// NOLINTEND(readability-function-cognitive-complexity)

bool updateV3(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&res](TxElement* node) -> bool {
        if (const auto& key = node->Value(); key != "Processor") return true;
        if (node->GetAttribute("type") != "org.inviwo.ParallelCoordinates") {
            return true;
        }
        if (auto* elem = xml::getElement(node,
                                         "Properties/Property&identifier=lines/Properties/"
                                         "Property&identifier=blendMode/selectedIdentifier")) {
            if (elem->GetAttribute("content") == "subractive") {
                elem->SetAttribute("content", "subtractive");
                res = true;
            }
        }
        return true;
    }};
    conv.convert(root);
    return res;
}

// NOLINTBEGIN(readability-function-cognitive-complexity)
bool updateV4(TxElement* root) {
    bool res = false;
    TraversingVersionConverter conv{[&res](TxElement* node) -> bool {
        if (const auto& key = node->Value(); key != "Property") return true;
        if (node->GetAttribute("type") != "org.inviwo.AxisProperty") {
            return true;
        }
        if (auto* useDataRange =
                xml::getElement(node, "Properties/Property&identifier=useDataRange")) {

            useDataRange->SetAttribute("identifier", "overrideRange");
            if (auto* elem = xml::getElement(useDataRange, "displayName")) {
                elem->SetAttribute("content", "Override Axis Range");
            }
            // flip checked state
            if (auto* value = xml::getElement(useDataRange, "value")) {
                if (value->GetAttribute("content") == "0") {
                    value->SetAttribute("content", "1");
                } else {
                    value->SetAttribute("content", "0");
                }
            }
            res = true;
        }
        if (auto* properties = xml::getElement(node, "Properties")) {
            if (auto* range = xml::getElement(properties, "Properties/Property&identifier=range")) {
                if (const auto* value = xml::getElement(range, "value")) {
                    TxElement customRange{"Property"};
                    customRange.SetAttribute("type", "org.inviwo.DoubleMinMaxProperty");
                    customRange.SetAttribute("identifier", "customRange");
                    TxElement valueNode("value");
                    valueNode.SetAttribute("x", value->Attribute("x").value_or("0.0"));
                    valueNode.SetAttribute("y", value->Attribute("y").value_or("100.0"));

                    customRange.InsertEndChild(valueNode);
                    properties->InsertEndChild(customRange);
                    res = true;
                }
            }
        }
        return true;
    }};
    conv.convert(root);
    return res;
}
// NOLINTEND(readability-function-cognitive-complexity)

bool updateV5(TxElement* root) {
    bool res = false;

    TraversingVersionConverter conv{[&res](TxElement* node) -> bool {
        if (const auto& key = node->Value(); key != "Processor") return true;
        if (const auto& type = node->GetAttribute("type"); type != "org.inviwo.MeshAxis" &&
                                                           type != "org.inviwo.VolumeAxis" &&
                                                           type != "org.inviwo.LayerAxis") {
            return true;
        }

        // convert DoubleMinMaxProperty to DoubleVec2Property
        if (auto* elem = xml::getElement(node, "Properties/Property&identifier=rangeX")) {
            convertDoubleMinMaxPropertyToVec2(elem);
            res = true;
        }
        if (auto* elem = xml::getElement(node, "Properties/Property&identifier=rangeY")) {
            convertDoubleMinMaxPropertyToVec2(elem);
            res = true;
        }
        if (auto* elem = xml::getElement(node, "Properties/Property&identifier=rangeZ")) {
            convertDoubleMinMaxPropertyToVec2(elem);
            res = true;
        }

        res = true;
        return true;
    }};

    conv.convert(root);
    return res;
}

constexpr bool is_flag(char c) { return c == '+' || c == '-' || c == ' ' || c == '#' || c == '0'; }
constexpr bool is_specifier(char c) {
    switch (c) {
        case 'd':
        case 'i':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
        case 's':
            return true;
        default:
            return false;
    }
}

std::string_view toStringView(auto begin, auto end) {
    return {&*begin, static_cast<size_t>(end - begin)};
}
// NOLINTBEGIN(llvm-qualified-auto, readability-qualified-auto)
std::string_view::const_iterator translateSpec(std::string_view::const_iterator it,
                                               std::string_view::const_iterator end,
                                               std::string& out) {

    auto percent = it++;
    if (it != end && *it == '%') {
        out.push_back('%');
        ++it;
        return it;
    }

    // ---- flags ----
    auto flags_begin = it;
    while (it != end && is_flag(*it)) ++it;
    const auto flags = toStringView(flags_begin, it);

    // ---- width ----
    auto width_begin = it;
    while (it != end && std::isdigit(static_cast<unsigned char>(*it))) ++it;
    const auto width = toStringView(width_begin, it);

    // ---- precision ----
    std::string_view precision;
    if (it != end && *it == '.') {
        ++it;
        auto prec_begin = it;
        while (it != end && std::isdigit(static_cast<unsigned char>(*it))) ++it;
        precision = toStringView(prec_begin, it);
    }

    // ---- specifier ----
    if (it == end || !is_specifier(*it)) {
        // invalid → copy literally
        out.append(percent, it);
        return it;
    }

    const char spec = *it++;
    out += "{:";

    // ---- flags mapping ----
    for (const char f : flags) {
        if (f == '0' && width.empty()) continue;  // zero-pad only with width
        if (f == '-') continue;                   // ignore left-align (semantic mismatch)
        out.push_back(f);
    }

    // ---- width ----
    out.append(width);

    // ---- precision ----
    if (!precision.empty()) {
        out.push_back('.');
        out.append(precision);
    }

    // ---- type ----
    switch (spec) {
        case 'd':
        case 'i':
        case 'u':
            break;
        case 'o':
        case 'x':
        case 'X':
            out.push_back(spec);
            break;
        case 'f':
        case 'F':
        case 'e':
        case 'E':
        case 'g':
        case 'G':
            out.push_back(static_cast<char>(std::tolower(spec)));
            break;
        case 's':
        default:
            break;
    }

    out.push_back('}');
    return it;
}

std::string printfToFmt(std::string_view input) {
    std::string out;
    out.reserve(input.size() + 16);

    auto it = input.begin();
    auto end = input.end();

    while (it != end) {
        if (*it != '%') {
            out.push_back(*it++);
        } else {
            it = translateSpec(it, end, out);
        }
    }

    return out;
}
// NOLINTEND(llvm-qualified-auto, readability-qualified-auto)

bool updateV6(TxElement* root) {
    bool res = false;

    TraversingVersionConverter conv{[&res](TxElement* node) -> bool {
        if (const auto& key = node->Value(); key != "Property") return true;
        if (const auto& type = node->GetAttribute("type"); type == "org.inviwo.AxisProperty") {
            if (auto* elem =
                    xml::getElement(node,
                                    "Properties"
                                    "/Property&type=org.inviwo.PlotTextProperty&identifier=labels"
                                    "/Properties"
                                    "/Property&type=org.inviwo.StringProperty&identifier=title"
                                    "/value")) {
                if (auto format = elem->Attribute("content").transform(printfToFmt)) {
                    elem->SetAttribute("content", *format);
                    res = true;
                }
            }
        }
        if (const auto& type = node->GetAttribute("type"); type == "org.inviwo.AxisStyleProperty") {
            if (auto* elem = xml::getElement(
                    node,
                    "Properties"
                    "/Property&type=org.inviwo.StringProperty&identifier=labelFormat"
                    "/value")) {
                if (auto format = elem->Attribute("content").transform(printfToFmt)) {
                    elem->SetAttribute("content", *format);
                    res = true;
                }
            }
        }

        return true;
    }};

    conv.convert(root);

    return res;
}

}  // namespace

bool PlottingGLModule::Converter::convert(TxElement* root) {
    bool res = false;
    switch (version_) {
        case 0: {
            res |= updateV0(root);
            [[fallthrough]];
        }
        case 1: {
            res |= updateV1(root);
            [[fallthrough]];
        }
        case 2: {
            res |= updateV2(root);
            [[fallthrough]];
        }
        case 3: {
            res |= updateV3(root);
            [[fallthrough]];
        }
        case 4: {
            res |= updateV4(root);
            [[fallthrough]];
        }
        case 5: {
            res |= updateV5(root);
            [[fallthrough]];
        }
        case 6: {
            res |= updateV6(root);
            return res;
        }
        default:
            return false;  // No changes
    }
}

}  // namespace inviwo
