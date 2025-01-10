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

#pragma once

#include <modules/webbrowser/webbrowsermoduledefine.h>
#include <inviwo/core/processors/processor.h>

#include <inviwo/core/ports/imageport.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/optionproperty.h>

#include <inviwo/dataframe/datastructures/dataframe.h>

#include <modules/brushingandlinking/ports/brushingandlinkingports.h>

#include <modules/json/jsonport.h>

#include <modules/webbrowser/processors/webbrowserbase.h>

namespace inviwo {

class IVW_MODULE_WEBBROWSER_API BasicWebBrowser : public Processor {
public:
    BasicWebBrowser(InviwoApplication* app);

    virtual void process() override;

    virtual const ProcessorInfo& getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    void render();

    DataFrameInport dataframe_;
    JSONInport json_;
    BrushingAndLinkingInport brushing_;
    ImageInport background_;
    ImageOutport outport_;

    StringProperty html_;
    StringProperty code_;
    ButtonProperty reload_;
    DoubleProperty zoom_;

    OptionPropertyString propertyTypes_;
    StringProperty name_;
    ButtonProperty add_;
    CompositeProperty extra_;

    CefRefPtr<WebBrowserBase> browser_;

    std::vector<std::shared_ptr<std::function<std::string(const std::string&)>>> callbacks_;

    bool loaded_ = false;
    std::string error_ = "";
};

}  // namespace inviwo
