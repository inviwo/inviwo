/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024-2025 Inviwo Foundation
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

#include <inviwo/webqt/webqtmodule.h>

#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/fileproperty.h>

#include <inviwo/webqt/htmlsyntaxhighlight.h>
#include <inviwo/webqt/javascriptsyntaxhighlight.h>

#include <inviwo/webqt/properties/htmlpropertywidgetqt.h>
#include <inviwo/webqt/properties/javascriptpropertywidgetqt.h>

namespace inviwo {

WebQtModule::WebQtModule(InviwoApplication* app) : InviwoModule(app, "WebQt") {
    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    // registerProcessor<WebQtProcessor>();

    registerSettings(std::make_unique<HtmlSyntaxHighlight>());
    registerSettings(std::make_unique<JavascriptSyntaxHighlight>());

    registerPropertyWidget<HtmlFilePropertyWidgetQt, FileProperty>(PropertySemantics("HtmlEditor"));
    registerPropertyWidget<HtmlPropertyWidgetQt, StringProperty>(PropertySemantics("HtmlEditor"));

    registerPropertyWidget<JavascriptFilePropertyWidgetQt, FileProperty>(
        PropertySemantics("JavascriptEditor"));
    registerPropertyWidget<JavascriptPropertyWidgetQt, StringProperty>(
        PropertySemantics("JavascriptEditor"));
}

}  // namespace inviwo
