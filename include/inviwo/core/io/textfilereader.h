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

#ifndef IVW_TEXTFILEREADER_H
#define IVW_TEXTFILEREADER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>
#include <iostream>
#include <string>


namespace inviwo {
/** \class TextFileReader
 *
 * Reader for text files.
 */
class IVW_CORE_API TextFileReader {

public:
    TextFileReader(const std::string& filePath);
    virtual ~TextFileReader() {}

    /**
     * Read a file and return the content as a string.
     * Throws an exception if file could not be opened
     * and prints an error message to the error log.
     * @param filePath Path of file.
     * @return Content of file.
     */
    virtual std::string read(const std::string& filePath) throw (std::ifstream::failure);

    /**
     * Read a file and return the content as a string.
     * Throws an exception if file could not be opened
     * and prints an error message to the error log.
     * @return Content of file.
     */
    virtual std::string read() throw (std::ifstream::failure);

    void setFilePath(const std::string& filePath) { filePath_ = filePath; }
    const std::string& getFilePath() const { return filePath_; }
private:
    std::string filePath_;
};

} // namespace

#endif // IVW_TEXTFILEREADER_H
