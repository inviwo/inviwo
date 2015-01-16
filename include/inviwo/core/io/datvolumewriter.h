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

#ifndef IVW_DATVOLUMEWRITER_H
#define IVW_DATVOLUMEWRITER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datawriter.h>
#include <inviwo/core/datastructures/volume/volume.h>

namespace inviwo {

class IVW_CORE_API DatVolumeWriter : public DataWriterType<Volume> {
public:
    DatVolumeWriter();
    DatVolumeWriter(const DatVolumeWriter& rhs);
    DatVolumeWriter& operator=(const DatVolumeWriter& that);
    virtual DatVolumeWriter* clone() const;
    virtual ~DatVolumeWriter() {};

    virtual void writeData(const Volume* data, const std::string filePath) const;

private:
    template<typename T>
    void writeKeyToString(std::stringstream& ss, const std::string& key, const glm::detail::tvec2<T, glm::defaultp>& vec) const;
    template<typename T>
    void writeKeyToString(std::stringstream& ss, const std::string& key, const glm::detail::tvec3<T, glm::defaultp>& vec) const;
    template<typename T>
    void writeKeyToString(std::stringstream& ss, const std::string& key, const glm::detail::tvec4<T, glm::defaultp>& vec) const;
    void writeKeyToString(std::stringstream& ss, const std::string& key, const std::string& str) const;
};

template<typename T>
void inviwo::DatVolumeWriter::writeKeyToString(std::stringstream& ss, const std::string& key,
        const glm::detail::tvec2<T, glm::defaultp>& vec) const {
    ss << key << ": " << vec.x << " " << vec.y << std::endl;
}
template<typename T>
void inviwo::DatVolumeWriter::writeKeyToString(std::stringstream& ss, const std::string& key,
        const glm::detail::tvec3<T, glm::defaultp>& vec) const {
    ss << key << ": " << vec.x << " " << vec.y << " " << vec.z << std::endl;
}
template<typename T>
void inviwo::DatVolumeWriter::writeKeyToString(std::stringstream& ss, const std::string& key,
        const glm::detail::tvec4<T, glm::defaultp>& vec) const {
    ss << key << ": " << vec.x << " " << vec.y << " " << vec.z << " " << vec.w << std::endl;
}





} // namespace

#endif // IVW_DATVOLUMEWRITER_H
