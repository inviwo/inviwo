/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <inviwo/core/common/inviwomodule.h>            // for InviwoModule
#include <inviwo/core/properties/boolproperty.h>        // for BoolProperty
#include <inviwo/core/properties/buttonproperty.h>      // for ButtonProperty
#include <inviwo/core/properties/directoryproperty.h>   // for DirectoryProperty
#include <inviwo/core/properties/fileproperty.h>        // for FileProperty
#include <inviwo/core/properties/minmaxproperty.h>      // for MinMaxProperty
#include <inviwo/core/properties/optionproperty.h>      // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>     // for OrdinalProperty
#include <inviwo/core/properties/ordinalrefproperty.h>  // for OrdinalRefProperty
#include <inviwo/core/properties/stringproperty.h>      // for StringProperty
#include <inviwo/core/util/foreacharg.h>                // for for_each_type
#include <inviwo/core/util/glmmat.h>                    // for dmat2, dmat3, dmat4
#include <inviwo/core/util/glmvec.h>                    // for dvec2, dvec3, dvec4
#include <inviwo/core/util/staticstring.h>              // for operator+
#include <modules/base/processors/filecache.h>
#include <modules/base/processors/inputselector.h>
#include <modules/json/io/json/boolpropertyjsonconverter.h>        // for to_json
#include <modules/json/io/json/buttonpropertyjsonconverter.h>      // for to_json
#include <modules/json/io/json/directorypropertyjsonconverter.h>   // for to_json
#include <modules/json/io/json/filepropertyjsonconverter.h>        // for to_json
#include <modules/json/io/json/minmaxpropertyjsonconverter.h>      // for to_json
#include <modules/json/io/json/optionpropertyjsonconverter.h>      // for to_json
#include <modules/json/io/json/ordinalpropertyjsonconverter.h>     // for to_json
#include <modules/json/io/json/ordinalrefpropertyjsonconverter.h>  // for to_json
#include <modules/json/io/json/stringpropertyjsonconverter.h>      // for to_json
#include <modules/json/io/json/templatepropertyjsonconverter.h>    // for to_json
#include <modules/json/io/jsonreader.h>
#include <modules/json/io/jsonwriter.h>

#include <modules/json/jsonport.h>
#include <cstddef>      // for size_t
#include <string>       // for string, operator==
#include <tuple>        // for tuple
#include <type_traits>  // for make_unsigned_t
#include <utility>      // for move
#include <vector>

#include <glm/detail/type_quat.hpp>       // for qua::operator[]
#include <glm/ext/quaternion_double.hpp>  // for dquat
#include <glm/fwd.hpp>                    // for fquat
#include <glm/gtc/type_precision.hpp>     // for i64
#include <glm/mat2x2.hpp>                 // for operator+
#include <glm/mat3x3.hpp>                 // for operator+
#include <glm/mat4x4.hpp>                 // for operator+
#include <glm/vec2.hpp>                   // for operator+, operator!=
#include <glm/vec3.hpp>                   // for operator+
#include <glm/vec4.hpp>                   // for operator+

namespace inviwo {
class InviwoApplication;

template <typename T>
void to_json(json& j, const DataInport<T>& port) {
    if (auto data = port.getData()) {
        j = *data;
    } else {
        j.clear();
    }
}
template <typename T>
void from_json(const json&, DataInport<T>&) {
    throw Exception("It is not possible to assign a json object to an Inport");
}

template <typename T>
void to_json(json& j, const DataOutport<T>& port) {
    if (auto data = port.getData()) {
        j = *data;
    } else {
        j.clear();
    }
}
template <typename T>
void from_json(const json& j, DataOutport<T>& port) {
    port.setData(std::make_shared<T>(j.get<T>()));
}

JSONModule::JSONModule(InviwoApplication* app)
    : InviwoModule(app, "JSON")
    , JSONSupplier<Inport, PortTraits>{inportConverter_}
    , JSONSupplier<Outport, PortTraits>{outportConverter_}
    , JSONSupplier<Property, PropertyTraits>{propertyConverter_} {

    // Register JSON converters
    registerJSONConverter<BoolProperty>();
    registerJSONConverter<ButtonProperty>();
    registerJSONConverter<DirectoryProperty>();
    registerJSONConverter<FileProperty>();
    registerJSONConverter<StringProperty>();

    // Register JSON converters for properties
    using OrdinalTypes =
        std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4, dmat2,
                   dmat3, dmat4, int, ivec2, ivec3, ivec4, glm::i64, unsigned int, uvec2, uvec3,
                   uvec4, size_t, size2_t, size3_t, size4_t, glm::fquat, glm::dquat>;

    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<OrdinalTypes>{}(
        [&]<typename T>() { registerJSONConverter<OrdinalProperty<T>>(); });
    util::for_each_type<OrdinalTypes>{}(
        [&]<typename T>() { registerJSONConverter<OrdinalRefProperty<T>>(); });

    util::for_each_type<ScalarTypes>{}(
        [&]<typename T>() { registerJSONConverter<MinMaxProperty<T>>(); });
    using OptionTypes = std::tuple<unsigned int, int, size_t, float, double, std::string>;
    util::for_each_type<OptionTypes>{}(
        [&]<typename T>() { registerJSONConverter<OptionProperty<T>>(); });

    using OptionEnumTypes = std::tuple<char, int>;
    util::for_each_type<OptionEnumTypes>{}([&]<typename T>() {
        enum class e : T;
        registerJSONConverter<OptionProperty<e>>();

        enum class eU : std::make_unsigned_t<T>;
        registerJSONConverter<OptionProperty<eU>>();
    });

    registerProcessor<InputSelector<MultiDataInport<json>, DataOutport<json>>>();
    registerProcessor<FileCache<json>>();

    registerDataReader(std::make_unique<JSONReader>());
    registerDataWriter(std::make_unique<JSONWriter>());

    registerDefaultsForDataType<json>();

    registerJSONConverter<JSONInport>();
    registerJSONConverter<JSONOutport>();

    registerJSONConverter<DataInport<std::vector<float>>>();
    registerJSONConverter<DataInport<std::vector<double>>>();
    registerJSONConverter<DataOutport<std::vector<float>>>();
    registerJSONConverter<DataOutport<std::vector<double>>>();
}

JSONModule::~JSONModule() {
    JSONSupplier<Inport, PortTraits>::unregisterJSONConverters();
    JSONSupplier<Outport, PortTraits>::unregisterJSONConverters();
    JSONSupplier<Property, PropertyTraits>::unregisterJSONConverters();
}

}  // namespace inviwo
