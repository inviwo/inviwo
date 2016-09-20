/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <modules/base/io/stlwriter.h>

#include <fstream>

namespace inviwo {

StlWriter::StlWriter() : DataWriterType<Mesh>() {
    addExtension(FileExtension("std","ASCII stl file format"));
}


StlWriter* StlWriter::clone() const {
    return new StlWriter(*this);
}

void StlWriter::writeData(const Mesh* data, const std::string filePath) const {
    std::ofstream f(filePath.c_str());
    
    f << "solid inviwo stl file\n";
    
    auto model = data->getModelMatrix();
    auto modelNormal = mat3(glm::transpose(glm::inverse(model)));
    
    std::shared_ptr<BufferBase> posBuffer;
    
    for (const auto& buf : data->getBuffers()) {
        if(buf.first.type == BufferType::PositionAttrib) {
            posBuffer = buf.second;
            break;
        }
    }
    
    if(!posBuffer) return;
    
    auto ram = posBuffer->getRepresentation<BufferRAM>();
    
    for (const auto& inds : data->getIndexBuffers()) {
        auto buffer = inds.second->getRAMRepresentation();
        for (const auto& i : *buffer->getDataContainer()) {
            
        
        }
    }

    
}

} // namespace

