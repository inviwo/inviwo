/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2022 Inviwo Foundation
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

#include <modules/json/io/json/filepropertyjsonconverter.h>

#include <inviwo/core/properties/fileproperty.h>  // for FileProperty
#include <inviwo/core/util/filedialogstate.h>     // for AcceptMode, FileMode
#include <inviwo/core/util/fileextension.h>       // for FileExtension

#include <stdexcept>                              // for out_of_range
#include <string>                                 // for string, basic_string

#include <nlohmann/json.hpp>                      // for basic_json<>::object_t, basic_json

namespace inviwo {

void to_json(json& j, const FileExtension& p) {
    j = json{{"extension", p.extension_}, {"description", p.description_}};
}

void from_json(const json& j, FileExtension& p) {
    j["extension"].get_to(p.extension_);
    j["description"].get_to(p.description_);
}

void to_json(json& j, const FileProperty& p) {
    j = json{{"value", p.get()},
             {"selectedExtension", p.getSelectedExtension()},
             {"acceptMode", p.getAcceptMode()},
             {"fileMode", p.getFileMode()},
             {"contentType", p.getContentType()},
             {"nameFilters", p.getNameFilters()}};
}

void from_json(const json& j, FileProperty& p) {
    if (j.count("value") > 0) {
        p.set(j.at("value").get<std::string>());
    }
    if (j.count("selectedExtension") > 0) {
        p.setSelectedExtension(j.at("selectedExtension").get<FileExtension>());
    }
    if (j.count("acceptMode") > 0) {
        p.setAcceptMode(j.at("acceptMode").get<AcceptMode>());
    }
    if (j.count("fileMode") > 0) {
        p.setFileMode(j.at("fileMode").get<FileMode>());
    }
    if (j.count("contentType") > 0) {
        p.setContentType(j.at("contentType").get<std::string>());
    }
    if (j.count("requestFile") > 0) {
        p.requestFile();
    }
}

}  // namespace inviwo
