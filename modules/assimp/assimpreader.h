/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2012-2015 Inviwo Foundation
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

#ifndef IVW_ASSIMPREADER_H
#define IVW_ASSIMPREADER_H

#include <modules/assimp/assimpmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/datareader.h>
#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/util/logcentral.h>
#include <assimp/LogStream.hpp>

namespace inviwo {

/** @brief Inviwo Module Assimp
*
*  A GeometryReader (DataReaderType<Geometry>) using the Assimp Library.*/
class IVW_MODULE_ASSIMP_API AssimpReader : public DataReaderType<Mesh> {
public:
    AssimpReader();
    AssimpReader(const AssimpReader& rhs) = default;
    AssimpReader& operator=(const AssimpReader& that) = default;
    virtual AssimpReader* clone() const override;
    virtual ~AssimpReader() = default;

    virtual std::shared_ptr<Mesh> readData(const std::string filePath) override;
};

/** @brief Assimp LogStream => Inviwo LogCentral
*
*  derive Assimp::LogStream to forward logged messages from the library to Inviwos LogCentral.*/
class InviwoAssimpLogStream : public Assimp::LogStream {
private:
    LogLevel loglevel;

public:
    InviwoAssimpLogStream(LogLevel ploglevel) { loglevel = ploglevel; }

    virtual ~InviwoAssimpLogStream() = default;

    void write(const char* message) {
        std::string tmp(message);
        while ('\n' == tmp.back()) tmp.pop_back();

        inviwo::LogCentral::getPtr()->log("Assimp Geometry Importer", loglevel, LogAudience::User,
                                          "<Assimp Bibliothek>", "<Funktion>", 0, tmp);
    }
};

}  // namespace

#endif  // IVW_ASSIMPREADER_H
