/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/isovaluecollection.h>

#include <inviwo/core/util/zip.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/vectoroperations.h>
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/io/datawriterexception.h>
#include <inviwo/core/io/datareaderexception.h>

namespace inviwo {

IsoValueCollection::IsoValueCollection(const std::vector<TFPrimitiveData>& values,
                                       TFPrimitiveSetType type)
    : TFPrimitiveSet(values, type) {}

std::vector<FileExtension> IsoValueCollection::getSupportedExtensions() const {
    return {{"iiv", "Inviwo Isovalues"}};
}

void IsoValueCollection::save(const std::string& filename, const FileExtension& ext) const {
    std::string extension = toLower(filesystem::getFileExtension(filename));

    if (ext.extension_ == "iiv" || (ext.empty() && extension == "iiv")) {
        Serializer serializer(filename);
        serialize(serializer);
        serializer.writeFile();
    } else {
        throw DataWriterException("Unsupported format for saving isovalues", IVW_CONTEXT);
    }
}

void IsoValueCollection::load(const std::string& filename, const FileExtension& ext) {
    std::string extension = toLower(filesystem::getFileExtension(filename));

    if (ext.extension_ == "iiv" || (ext.empty() && extension == "iiv")) {
        Deserializer deserializer(filename);
        deserialize(deserializer);
    } else {
        throw DataReaderException("Unsupported format for loading isovalues", IVW_CONTEXT);
    }
}

std::string IsoValueCollection::getTitle() const { return "Isovalues"; }

std::string IsoValueCollection::serializationKey() const { return "IsoValues"; }

std::string IsoValueCollection::serializationItemKey() const { return "IsoValue"; }

}  // namespace inviwo
