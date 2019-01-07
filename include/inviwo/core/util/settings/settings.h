/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2018 Inviwo Foundation
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

#ifndef IVW_SETTINGS_H
#define IVW_SETTINGS_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/propertyowner.h>
#include <inviwo/core/properties/property.h>

namespace inviwo {

class InviwoApplication;

class IVW_CORE_API Settings : public PropertyOwner {

public:
    Settings(const std::string& id, InviwoApplication* app);
    Settings(const std::string& id = "");
    virtual ~Settings();

    virtual void addProperty(Property* property, bool owner = true) override;
    virtual void addProperty(Property& property) override;

    /**
     * Load setting from disk. Each derived class needs to take care of calling this in the
     * constructor
     */
    void load();

    /**
     * Save is called automatically whenever a property changes.
     */
    void save();

    virtual std::string getIdentifier();
    virtual InviwoApplication* getInviwoApplication() override;

protected:
    std::string getFileName() const;
    std::string identifier_;
    bool isDeserializing_;
    InviwoApplication* app_;
};

}  // namespace inviwo

#endif  // IVW_SETTINGS_H
