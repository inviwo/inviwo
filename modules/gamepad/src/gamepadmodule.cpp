/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2022 Inviwo Foundation
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

#include <inviwo/gamepad/gamepadmodule.h>
#include <inviwo/gamepad/processors/gamepadordinals.h>
#include <inviwo/gamepad/processors/gamepadbuttonsswitches.h>
#include <inviwo/gamepad/processors/gamepadcamera.h>
#include <inviwo/gamepad/processors/gamepadinput.h>
#include <inviwo/gamepad/processors/gamepadordinals.h>
#include <inviwo/gamepad/properties/gamepadvaluesproperty.h>
#include <inviwo/gamepad/properties/gamepadtextproperty.h>

namespace inviwo {

GamepadModule::GamepadModule(InviwoApplication* app) : InviwoModule(app, "Gamepad") {
    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<GamepadButtonsSwitches>();
    registerProcessor<GamepadCamera>();
    registerProcessor<GamepadInput>();
    registerProcessor<GamepadOrdinals>();

    // Properties
    registerProperty<GamepadValuesProperty>();
    registerProperty<GamepadTextProperty>();
    util::for_each_type<GamepadOrdinals::Types>{}(RegHelper{}, *this);

    // Readers and writes
    // registerDataReader(std::make_unique<GamepadReader>());
    // registerDataWriter(std::make_unique<GamepadWriter>());

    // Data converters
    // registerRepresentationConverter(std::make_unique<GamepadDisk2RAMConverter>());

    // Ports
    // registerPort<GamepadOutport>();
    // registerPort<GamepadInport>();

    // PropertyWidgets
    // registerPropertyWidget<GamepadPropertyWidget, GamepadProperty>("Default");

    // Dialogs
    // registerDialog<GamepadDialog>(GamepadOutport);

    // Other things
    // registerCapabilities(std::make_unique<GamepadCapabilities>());
    // registerSettings(std::make_unique<GamepadSettings>());
    // registerMetaData(std::make_unique<GamepadMetaData>());
    // registerPortInspector("GamepadOutport", "path/workspace.inv");
    // registerProcessorWidget(std::string processorClassName, std::unique_ptr<ProcessorWidget>
    // processorWidget); registerDrawer(util::make_unique_ptr<GamepadDrawer>());
    registerCompositeProcessor(this->getPath(ModulePath::Workspaces) + "/composites/GamepadUI.inv");
}

}  // namespace inviwo
