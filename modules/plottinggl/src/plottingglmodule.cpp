/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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
#include <modules/plotting/plottingmodule.h>

#include <inviwo/core/common/inviwoapplication.h>
#include <modules/opengl/shader/shadermanager.h>

#include <modules/plottinggl/processors/axisrenderprocessor.h>
#include <modules/plottinggl/processors/colorscalelegend.h>
#include <modules/plottinggl/processors/parallelcoordinates/parallelcoordinates.h>
#include <modules/plottinggl/processors/scatterplotmatrixprocessor.h>
#include <modules/plottinggl/processors/scatterplotprocessor.h>
#include <modules/plottinggl/processors/volumeaxis.h>
#include <modules/plottinggl/processors/persistencediagramplotprocessor.h>
#include <modules/plottinggl/processors/parallelcoordinates/pcpaxissettings.h>

#include <modules/plottinggl/datavisualizer/pcpdataframevisualizer.h>

namespace inviwo {

PlottingGLModule::PlottingGLModule(InviwoApplication* app) : InviwoModule(app, "PlottingGL") {

    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    registerProcessor<plot::AxisRenderProcessor>();
    registerProcessor<plot::ColorScaleLegend>();
    registerProcessor<plot::ParallelCoordinates>();
    registerProcessor<plot::PersistenceDiagramPlotProcessor>();
    registerProcessor<plot::ScatterPlotMatrixProcessor>();
    registerProcessor<plot::ScatterPlotProcessor>();
    registerProcessor<plot::VolumeAxis>();

    registerProperty<plot::PCPAxisSettings>();

    registerDataVisualizer(std::make_unique<PCPDataFrameVisualizer>(app));
}

int PlottingGLModule::getVersion() const { return 1; }

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
                }

                if (auto color = xml::getElement(node,
                                                 "Properties/Property&identifier=colors/Properties/"
                                                 "Property&identifier=selectedColorAxis")) {
                    props->InsertEndChild(*color);
                }

                res = true;
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
