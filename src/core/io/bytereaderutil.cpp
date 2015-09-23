/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2015 Inviwo Foundation
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

#include <inviwo/core/io/bytereaderutil.h>
#include <inviwo/core/io/datareaderexception.h>
#include <inviwo/core/util/raiiutils.h>

namespace inviwo {

void util::readBytesIntoBuffer(const std::string& file, size_t offset, size_t bytes,
                               bool littleEndian, size_t elementSize, void* dest) {
    std::fstream fin(file.c_str(), std::ios::in | std::ios::binary);
    OnScopeExit close([&fin]() { fin.close(); });

    if (fin.good()) {
        fin.seekg(offset);
        fin.read(static_cast<char*>(dest), bytes);

        if (!littleEndian && elementSize > 1) {
            char* temp = new char[elementSize];

            for (std::size_t i = 0; i < bytes; i += elementSize) {
                for (std::size_t j = 0; j < elementSize; j++)
                    temp[j] = static_cast<char*>(dest)[i + j];

                for (std::size_t j = 0; j < elementSize; j++)
                    static_cast<char*>(dest)[i + j] = temp[elementSize - j - 1];
            }

            delete[] temp;
        }
    } else {
        throw DataReaderException("Error: Could not read from file: " + file,
                                  IvwContextCustom("readBytesIntoBuffer"));
    }
}

}  // namespace
