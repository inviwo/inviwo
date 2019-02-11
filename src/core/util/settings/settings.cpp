/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2019 Inviwo Foundation
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

#include <inviwo/core/util/settings/settings.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/util/raiiutils.h>
#include <inviwo/core/util/utilities.h>
#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/core/properties/propertyfactory.h>
#include <inviwo/core/metadata/metadatafactory.h>

namespace inviwo {

Settings::Settings(const std::string& id) : Settings(id, InviwoApplication::getPtr()) {}

Settings::Settings(const std::string& id, InviwoApplication* app)
    : identifier_(id), isDeserializing_(false), app_(app) {}

Settings::~Settings() = default;

void Settings::addProperty(Property* property, bool owner) {
    PropertyOwner::addProperty(property, owner);
    property->onChange([this]() { save(); });
}

void Settings::addProperty(Property& property) { addProperty(&property, false); }

std::string Settings::getIdentifier() { return identifier_; }

std::string Settings::getFileName() const {
    auto settingname = identifier_;
    replaceInString(settingname, " ", "_");
    const auto appname = util::stripIdentifier(app_->getDisplayName());
    return filesystem::getPath(PathType::Settings, "/" + appname + "_" + settingname + ".ivs",
                               true);
}

InviwoApplication* Settings::getInviwoApplication() { return app_; }

void Settings::load() {
    util::KeepTrueWhileInScope guard{&isDeserializing_};
    auto filename = getFileName();

    if (filesystem::fileExists(filename)) {
        // An error is not critical as the default setting will be used.
        try {
            Deserializer d(filename);
            d.registerFactory(app_->getPropertyFactory());
            d.registerFactory(app_->getMetaDataFactory());
            deserialize(d);
        } catch (const Exception& e) {
            util::log(e.getContext(), e.getMessage(), LogLevel::Error);
        } catch (const std::exception& e) {
            LogError(e.what());
        }
    }
}

void Settings::save() {
    if (isDeserializing_) return;
    try {
        Serializer s(getFileName());
        serialize(s);
        s.writeFile();
    } catch (const Exception& e) {
        util::log(e.getContext(), e.getMessage(), LogLevel::Error);
    } catch (const std::exception& e) {
        LogWarn(e.what());
    }
}

}  // namespace inviwo
