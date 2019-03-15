/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#include <modules/userinterfacegl/userinterfaceglmodule.h>

#include <modules/userinterfacegl/processors/camerawidget.h>
#include <modules/userinterfacegl/processors/cropwidget.h>
#include <modules/userinterfacegl/processors/gluiprocessor.h>
#include <modules/userinterfacegl/processors/gluitestprocessor.h>
#include <modules/userinterfacegl/processors/presentationprocessor.h>

#include <modules/opengl/shader/shadermanager.h>

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>

#include <modules/userinterfacegl/glui/widgets/boolpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/buttonpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/floatminmaxpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/floatpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/intminmaxpropertywidget.h>
#include <modules/userinterfacegl/glui/widgets/intpropertywidget.h>

namespace inviwo {

UserInterfaceGLModule::UserInterfaceGLModule(InviwoApplication* app)
    : InviwoModule(app, "UserInterfaceGL"), WidgetSupplier(*this) {
    // Add a directory to the search path of the Shadermanager
    ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    registerProcessor<CameraWidget>();
    registerProcessor<CropWidget>();
    registerProcessor<glui::GLUIProcessor>();
    registerProcessor<PresentationProcessor>();

    registerProcessor<GLUITestProcessor>();

    // Properties
    // registerProperty<UserInterfaceGLProperty>();

    // PropertyWidgets

    // GLUI Widgets
    registerGLUIWidget<glui::BoolPropertyWidget, BoolProperty>();
    registerGLUIWidget<glui::ButtonPropertyWidget, ButtonProperty>();
    registerGLUIWidget<glui::FloatPropertyWidget, FloatProperty>();
    registerGLUIWidget<glui::FloatMinMaxPropertyWidget, FloatMinMaxProperty>();
    registerGLUIWidget<glui::IntPropertyWidget, IntProperty>();
    registerGLUIWidget<glui::IntMinMaxPropertyWidget, IntMinMaxProperty>();
}

UserInterfaceGLModule::~UserInterfaceGLModule() {
    // Unregister everything from the factory since this module _owns_ the factory. This is
    // neccessary even though the base class destructor, i.e. glui::WidgetSupplier, takes
    // care of this. Otherwise the supplier will unregister the items _after_ the factory is
    // destroyed.
    //
    // Other modules do not have to do this!
    unregisterAll();
}

glui::WidgetFactory& UserInterfaceGLModule::getGLUIWidgetFactory() { return widgetFactory_; }

const glui::WidgetFactory& UserInterfaceGLModule::getGLUIWidgetFactory() const {
    return widgetFactory_;
}

}  // namespace inviwo
