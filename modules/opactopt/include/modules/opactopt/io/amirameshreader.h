/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2024 Inviwo Foundation
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

#include <modules/opactopt/opactoptmoduledefine.h>

#include <inviwo/core/datastructures/geometry/mesh.h>
#include <inviwo/core/io/datareader.h>

#include <any>          // for any
#include <memory>       // for shared_ptr
#include <string_view>  // for string_view

namespace inviwo {

/**
 * \ingroup dataio
 * \brief Inviwo Module OpactOpt
 *
 *  A GeometryReader (DataReaderType<Geometry>) using the Assimp Library.
 */
class IVW_MODULE_OPACTOPT_API AmiraMeshReader : public DataReaderType<Mesh> {
public:
    AmiraMeshReader();
    AmiraMeshReader(const AmiraMeshReader& rhs) = default;
    AmiraMeshReader& operator=(const AmiraMeshReader& that) = default;
    virtual AmiraMeshReader* clone() const override;
    virtual ~AmiraMeshReader() = default;

    virtual std::shared_ptr<Mesh> readData(const std::filesystem::path& filePath) override;

private:
    enum AmiraDataType { Lines, Vertices, Importance };

    void processLines(std::ifstream& fs, std::shared_ptr<Mesh> mesh);
    void processVertices(std::ifstream& fs, std::shared_ptr<Mesh> mesh);
    void processImportance(std::ifstream& fs, std::shared_ptr<Mesh> mesh);
};

}  // namespace inviwo
