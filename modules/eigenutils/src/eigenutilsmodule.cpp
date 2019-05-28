/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015-2019 Inviwo Foundation
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

#include <modules/eigenutils/eigenutilsmodule.h>
#include <modules/eigenutils/processors/eigenmatrixtoimage.h>
#include <modules/eigenutils/eigenports.h>
#include <modules/eigenutils/processors/eigenmix.h>
#include <modules/eigenutils/processors/eigennormalize.h>

#include <modules/eigenutils/processors/testmatrix.h>
namespace inviwo {

EigenUtilsModule::EigenUtilsModule(InviwoApplication* app) : InviwoModule(app, "EigenUtils") {
    registerProcessor<EigenMatrixToImage>();
    registerProcessor<EigenMix>();
    registerProcessor<EigenNormalize>();
    registerProcessor<TestMatrix>();
    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(PathType::Modules, "/eigenutils/glsl");

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    // registerProcessor<EigenUtilsProcessor>());

    // Properties
    // registerProperty<EigenUtilsProperty>());

    // Readers and writes
    // registerDataReader(std::make_unique<EigenUtilsReader>());
    // registerDataWriter(std::make_unique<EigenUtilsWriter>());

    // Data converters
    // registerRepresentationConverter(std::make_unique<EigenUtilsDisk2RAMConverter>());

    // Ports
    registerDefaultsForDataType<Eigen::MatrixXf>();

    registerPortInspector(PortTraits<DataOutport<Eigen::MatrixXf>>::classIdentifier(),
                          this->getPath(ModulePath::PortInspectors) + "/eigenmatrix.inv");

    // PropertyWidgets
    // registerPropertyWidget<EigenUtilsPropertyWidget, EigenUtilsProperty>("Default");

    // Dialogs
    // registerDialog<EigenUtilsDialog>(EigenUtilsOutport));

    // Other varius things
    // registerCapabilities(std::make_unique<EigenUtilsCapabilities>()));
    // registerSettings(std::make_unique<EigenUtilsSettings>());
    // registerMetaData(std::make_unique<EigenUtilsMetaData>());
    // registerProcessorWidget(std::string processorClassName, std::unique_ptr<ProcessorWidget>
    // processorWidget);
    // registerDrawer(util::make_unique_ptr<EigenUtilsDrawer>());
}

}  // namespace inviwo
