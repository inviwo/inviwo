/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 * Version 0.9
 *
 * Copyright (c) 2013-2015 Inviwo Foundation
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

#ifndef IVW_DIRECTORY_PROPERTY_H
#define IVW_DIRECTORY_PROPERTY_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/properties/templateproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>

namespace inviwo {
/** class DirectoryProperty
 *  A class for file representations.
 *  Holds the value of the path to a file as a string.
 *
 * @see FileProperty, StringProperty
 */

class IVW_CORE_API DirectoryProperty : public TemplateProperty<std::string> {
public:
    InviwoPropertyInfo();
    DirectoryProperty(
        std::string identifier, 
        std::string displayName, 
        std::string value = "",
        std::string contentType = "default",
        InvalidationLevel invalidationLevel = INVALID_OUTPUT,
        PropertySemantics semantics = PropertySemantics::Default);
    
    DirectoryProperty(const DirectoryProperty& rhs);
    DirectoryProperty& operator=(const DirectoryProperty& that);
    virtual DirectoryProperty* clone() const;
    virtual ~DirectoryProperty();

    virtual std::vector<std::string> getDirectoryTree() const;
    virtual std::vector<std::string> getFiles(std::string filters = "*.*") const;
    virtual void setDirectoryTree(std::vector<std::string> dirTree);
    void updateDirectoryTree();
    virtual void serialize(IvwSerializer& s) const;
    virtual void deserialize(IvwDeserializer& d);
    virtual void registerFileIndexingHandle(IntProperty* indexHandle) {
        // TODO: use composite property if possible.
        fileIndexingHandle_ = indexHandle;
    }
    virtual IntProperty* getFileIndexingHandle() {
        // TODO: use composite property if possible.
        return fileIndexingHandle_;
    }

    void setContentType(const std::string& contentType);
    std::string getContentType() const;

protected:
    // TODO: currently tree contains file names only.
    std::vector<std::string> directoryTree_;
    IntProperty* fileIndexingHandle_; // Owning!?! //Peter
    std::string contentType_;
};

}  // namespace

#endif  // IVW_DIRECTORY_PROPERTY_H