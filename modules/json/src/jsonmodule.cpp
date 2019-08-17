/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <modules/json/jsonmodule.h>
#include <modules/json/io/json/boolpropertyjsonconverter.h>
#include <modules/json/io/json/buttonpropertyjsonconverter.h>
#include <modules/json/io/json/directorypropertyjsonconverter.h>
#include <modules/json/io/json/filepropertyjsonconverter.h>
#include <modules/json/io/json/minmaxpropertyjsonconverter.h>
#include <modules/json/io/json/optionpropertyjsonconverter.h>
#include <modules/json/io/json/ordinalpropertyjsonconverter.h>
#include <modules/json/io/json/templatepropertyjsonconverter.h>

#include <inviwo/core/properties/stringproperty.h>

namespace inviwo {
struct OrdinalReghelper {
    template <typename T>
    auto operator()(JSONModule& m) {
        using PropertyType = OrdinalProperty<T>;
        m.registerPropertyJSONConverter<PropertyType>();
    }
};

struct MinMaxReghelper {
    template <typename T>
    auto operator()(JSONModule& m) {
        using PropertyType = MinMaxProperty<T>;
        m.registerPropertyJSONConverter<PropertyType>();
    }
};

struct OptionReghelper {
    template <typename T>
    auto operator()(JSONModule& m) {
        using PropertyType = TemplateOptionProperty<T>;
        m.registerPropertyJSONConverter<PropertyType>();
    }
};

JSONModule::JSONModule(InviwoApplication* app) : InviwoModule(app, "JSON") {

    // Register JSON converters
    // Note: Also add WebBrowserModule::registerPropertyWidgetCEF if a conversion is
    // added here
    registerPropertyJSONConverter<BoolProperty>();
    registerPropertyJSONConverter<ButtonProperty>();
    registerPropertyJSONConverter<DirectoryProperty>();
    registerPropertyJSONConverter<FileProperty>();
    registerPropertyJSONConverter<StringProperty>();

    // Register ordinal property widgets
    using OrdinalTypes =
        std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4, dmat2,
                   dmat3, dmat4, int, ivec2, ivec3, ivec4, glm::i64, unsigned int, uvec2, uvec3,
                   uvec4, size_t, size2_t, size3_t, size4_t, glm::fquat, glm::dquat>;

    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<OrdinalTypes>{}(OrdinalReghelper{}, *this);

    // Register MinMaxProperty widgets
    util::for_each_type<ScalarTypes>{}(MinMaxReghelper{}, *this);

    // Register option property widgets
    using OptionTypes = std::tuple<unsigned int, int, size_t, float, double, std::string>;
    util::for_each_type<OptionTypes>{}(OptionReghelper{}, *this);

    // Add a directory to the search path of the Shadermanager
    // ShaderManager::getPtr()->addShaderSearchPath(getPath(ModulePath::GLSL));

    // Register objects that can be shared with the rest of inviwo here:

    // Processors
    // registerProcessor<jsonProcessor>();

    // Properties
    // registerProperty<jsonProperty>();

    // Readers and writes
    // registerDataReader(std::make_unique<jsonReader>());
    // registerDataWriter(std::make_unique<jsonWriter>());

    // Data converters
    // registerRepresentationConverter(std::make_unique<jsonDisk2RAMConverter>());

    // Ports
    // registerPort<jsonOutport>();
    // registerPort<jsonInport>();

    // PropertyWidgets
    // registerPropertyWidget<jsonPropertyWidget, jsonProperty>("Default");

    // Dialogs
    // registerDialog<jsonDialog>(jsonOutport);

    // Other things
    // registerCapabilities(std::make_unique<jsonCapabilities>());
    // registerSettings(std::make_unique<jsonSettings>());
    // registerMetaData(std::make_unique<jsonMetaData>());
    // registerPortInspector("jsonOutport", "path/workspace.inv");
    // registerProcessorWidget(std::string processorClassName, std::unique_ptr<ProcessorWidget>
    // processorWidget); registerDrawer(util::make_unique_ptr<jsonDrawer>());
}

void JSONModule::registerPropertyJSONConverter(
    std::unique_ptr<PropertyJSONConverterFactoryObject> propertyConverter) {
    if (propertyJSONConverterFactory_.registerObject(propertyConverter.get())) {
        propertyJSONConverters_.push_back(std::move(propertyConverter));
    }
}

}  // namespace inviwo
