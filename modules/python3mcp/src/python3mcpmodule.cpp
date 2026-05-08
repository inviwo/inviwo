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

#include <inviwo/python3mcp/python3mcpmodule.h>

#include <pybind11/cast.h>
#include <pybind11/detail/common.h>
#include <pybind11/embed.h>
#include <pybind11/eval.h>
#include <pybind11/pybind11.h>
#include <pybind11/pytypes.h>

namespace inviwo {

Python3MCPModule::Python3MCPModule(InviwoApplication* app) : InviwoModule(app, "Python3MCP") {
    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    // registerProcessor<Python3MCPProcessor>();

    // Properties
    // registerProperty<Python3MCPProperty>();

    // Readers and writes
    // registerDataReader(std::make_unique<Python3MCPReader>());
    // registerDataWriter(std::make_unique<Python3MCPWriter>());

    // Data converters
    // registerRepresentationConverter(std::make_unique<Python3MCPDisk2RAMConverter>());

    // Ports
    // registerPort<Python3MCPOutport>();
    // registerPort<Python3MCPInport>();

    // PropertyWidgets
    // registerPropertyWidget<Python3MCPPropertyWidget, Python3MCPProperty>("Default");

    // Dialogs
    // registerDialog<Python3MCPDialog>(Python3MCPOutport);

    // Other things
    // registerCapabilities(std::make_unique<Python3MCPCapabilities>());
    // registerSettings(std::make_unique<Python3MCPSettings>());
    // registerMetaData(std::make_unique<Python3MCPMetaData>());
    // registerPortInspector("Python3MCPOutport", "path/workspace.inv");
    // registerProcessorWidget(std::string processorClassName, std::unique_ptr<ProcessorWidget> processorWidget); 
    // registerDrawer(std::make_unique_ptr<Python3MCPDrawer>());

    pybind11::exec(R"(print("hello world"))", pybind11::globals());
}

}  // namespace inviwo
