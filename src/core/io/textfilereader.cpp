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

#include <inviwo/core/io/textfilereader.h>

namespace inviwo {

TextFileReader::TextFileReader(const std::string& filePath): filePath_(filePath)
{}

std::string TextFileReader::read(const std::string& filePath) throw (std::ifstream::failure)
{
    filePath_ = filePath;
    return read();
}

std::string TextFileReader::read() throw (std::ifstream::failure)
{
    std::ifstream file;
    // Make ifstream throw exception if file could not be opened
    file.exceptions(std::ifstream::failbit);

    try {
        file.open(filePath_.c_str());
    } catch (std::ifstream::failure& ex) {
        LogError("Error opening file " << filePath_);
        throw ex;
    }

    std::string data(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));

    if (data.empty()) {
        // When editing files using Visual Studio it may happen that the file is empty.
        // Wait a bit and hope that the content is there later.
#ifdef WIN32
        Sleep(400);
#endif
        std::ifstream srcFile(filePath_.c_str());
        data = std::string(std::istreambuf_iterator<char>(file), (std::istreambuf_iterator<char>()));
    }

    file.close();
    return data;
}



} // namespace
