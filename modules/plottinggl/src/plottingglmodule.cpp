/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwomodule.h>                                        // for Inviw...
#include <inviwo/core/io/serialization/ticpp.h>                                     // for TxEle...
#include <inviwo/core/io/serialization/versionconverter.h>                          // for getEl...
#include <inviwo/core/rendering/datavisualizer.h>                                   // for DataV...
#include <modules/opengl/shader/shadermanager.h>                                    // for Shade...
#include <modules/plottinggl/datavisualizer/pcpdataframevisualizer.h>               // for PCPDa...
#include <modules/plottinggl/datavisualizer/scatterplotdataframevisualizer.h>       // for Scatt...
#include <modules/plottinggl/plotters/scatterplotgl.h>                              // for Scatt...
#include <modules/plottinggl/processors/axisrenderprocessor.h>                      // for AxisR...
#include <modules/plottinggl/processors/colorscalelegend.h>                         // for Color...
#include <modules/plottinggl/processors/imageplotprocessor.h>                       // for Image...
#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>  // for Paral...
#include <modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h>      // for PCPAx...
#include <modules/plottinggl/processors/persistencediagramplotprocessor.h>          // for Persi...
#include <modules/plottinggl/processors/scatterplotmatrixprocessor.h>               // for Scatt...
#include <modules/plottinggl/processors/scatterplotprocessor.h>                     // for Scatt...
#include <modules/plottinggl/processors/volumeaxis.h>                               // for Volum...

#include <functional>                                                               // for __base
#include <string>                                                                   // for opera...
#include <string_view>                                                              // for strin...

#include <fmt/core.h>                                                               // for format

namespace inviwo {
class InviwoApplication;

PlottingGLModule::PlottingGLModule(InviwoApplication* app) : InviwoModule(app, "PlottingGL") {

    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    registerProcessor<plot::AxisRenderProcessor>();
    registerProcessor<plot::ColorScaleLegend>();
    registerProcessor<plot::ImagePlotProcessor>();
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

int PlottingGLModule::getVersion() const { return 3; }

std::unique_ptr<VersionConverter> PlottingGLModule::getConverter(int version) const {
    return std::make_unique<Converter>(version);
}

PlottingGLModule::Converter::Converter(int version) : version_(version) {}

bool PlottingGLModule::Converter::convert(TxElement* root) {
    bool res = false;
    switch (version_) {
        case 0: {
            TraversingVersionConverter conv{[&](TxElement* node) -> bool {
                std::string key;
                node->GetValue(&key);
                if (key != "Processor") return true;
                const auto type = node->GetAttributeOrDefault("type", "");
                if (type != "org.inviwo.ParallelCoordinates") return true;

                auto props = xml::getElement(node, "Properties");
                if (auto tf = xml::getElement(node,
                                              "Properties/Property&identifier=colors/Properties/"
                                              "Property&identifier=tf")) {
                    props->InsertEndChild(*tf);
                    res = true;
                }

                if (auto color = xml::getElement(node,
                                                 "Properties/Property&identifier=colors/Properties/"
                                                 "Property&identifier=selectedColorAxis")) {
                    props->InsertEndChild(*color);
                    res = true;
                }

                return true;
            }};

            conv.convert(root);
            [[fallthrough]];
        }
        case 1: {
            TraversingVersionConverter conv{[&](TxElement* node) -> bool {
                std::string key;
                node->GetValue(&key);
                if (key != "Processor") return true;
                const auto type = node->GetAttributeOrDefault("type", "");
                if (type != "org.inviwo.ParallelCoordinates") return true;

                auto props = xml::getElement(node, "Properties");
                TxElement compNode{"Property"};
                compNode.SetAttribute("type", "org.inviwo.DataFrameColormapProperty");
                compNode.SetAttribute("identifier", "colormap");

                TxElement propNode{"Properties"};
                // Move old TF and selected color into new DataFrameColormapProperty
                if (auto tf = xml::getElement(props, "Property&identifier=tf")) {
                    propNode.InsertEndChild(*tf);
                }

                if (auto color = xml::getElement(props, "Property&identifier=selectedColorAxis")) {
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
            [[fallthrough]];
        }
        case 2: {
            TraversingVersionConverter conv{[&](TxElement* node) -> bool {
                std::string key;
                node->GetValue(&key);
                if (key != "Processor") return true;
                const auto type = node->GetAttributeOrDefault("type", "");
                if ((type != "org.inviwo.ScatterPlotProcessor") &&
                    (type != "org.inviwo.ScatterPlotMatrixProcessor")) {
                    return true;
                }

                const bool isScatterPlotProc = (type == "org.inviwo.ScatterPlotProcessor");

                std::string_view identifier =
                    isScatterPlotProc ? "scatterplot" : "scatterPlotproperties";

                if (auto elem = xml::getElement(node, "Properties/Property&identifier=fontFace")) {
                    elem->SetAttribute("type", "org.inviwo.FontFaceOptionProperty");
                    res = true;
                }
                if (auto elem =
                        xml::getElement(node, "Properties/Property&identifier=fontFaceStats")) {
                    elem->SetAttribute("type", "org.inviwo.FontFaceOptionProperty");
                    res = true;
                }
                if (auto elem =
                        xml::getElement(node, "Properties/Property&identifier=correlectionTF")) {
                    elem->SetAttribute("identifier", "correlationTF");
                    res = true;
                }

                if (auto plot = xml::getElement(
                        node,
                        fmt::format("Properties/Property&identifier={}/Properties", identifier))) {
                    // Rename color property
                    if (auto color = xml::getElement(plot, "Property&identifier=color")) {
                        color->SetAttribute("identifier", "defaultColor");
                    }

                    // Move hovering bool property to checked state of a BoolComposite
                    {
                        TxElement compNode{"Property"};
                        compNode.SetAttribute("type", "org.inviwo.BoolCompositeProperty");
                        compNode.SetAttribute("identifier", "showHighlighted");

                        TxElement propNode{"Properties"};
                        if (auto hovered = xml::getElement(plot, "Property&identifier=hovering")) {
                            hovered->SetAttribute("identifier", "checked");
                            hovered->SetAttribute("displayName", "");
                            propNode.InsertEndChild(*hovered);
                        }
                        // Change type of hover color to vec3, move to composite
                        if (auto highlightColor =
                                xml::getElement(plot, "Property&identifier=hoverColor")) {
                            highlightColor->SetAttribute("type", "org.inviwo.FloatVec3Property");
                            highlightColor->SetAttribute("identifier", "highlightColor");

                            propNode.InsertEndChild(*highlightColor);

                            if (auto value = xml::getElement(highlightColor, "value")) {
                                auto str = value->GetAttributeOrDefault("w", "1.0");

                                TxElement alphaNode{"Property"};
                                alphaNode.SetAttribute("type", "org.inviwo.FloatProperty");
                                alphaNode.SetAttribute("identifier", "highlightAlpha");
                                TxElement valueNode("value");
                                valueNode.SetAttribute("content", str);

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
                        if (auto highlightColor =
                                xml::getElement(plot, "Property&identifier=selectionColor")) {
                            highlightColor->SetAttribute("type", "org.inviwo.FloatVec3Property");
                            highlightColor->SetAttribute("identifier", "highlightColor");

                            propNode.InsertEndChild(*highlightColor);

                            if (auto value = xml::getElement(highlightColor, "value")) {
                                auto str = value->GetAttributeOrDefault("w", "1.0");

                                TxElement alphaNode{"Property"};
                                alphaNode.SetAttribute("type", "org.inviwo.FloatProperty");
                                alphaNode.SetAttribute("identifier", "selectionAlpha");
                                TxElement valueNode("value");
                                valueNode.SetAttribute("content", str);

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

        default:
            return false;  // No changes
    }
}

}  // namespace inviwo
