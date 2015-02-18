/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
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

#ifndef IVW_IMAGEDISK_H
#define IVW_IMAGEDISK_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/datastructures/diskrepresentation.h>
#include <inviwo/core/datastructures/image/imagerepresentation.h>

namespace inviwo {

class IVW_CORE_API ImageDisk : public ImageRepresentation, public DiskRepresentation {

public:
    ImageDisk();
    ImageDisk(std::string url);
    virtual ~ImageDisk();
    virtual void initialize();
    virtual void deinitialize();
    virtual void resize(uvec2 dimensions);
    virtual DataRepresentation* clone() const;
    virtual std::string getClassName() const { return "ImageDisk"; };
    virtual bool copyAndResizeImage(DataRepresentation*) { return false;};
    /**
     * \brief loads data from url.
     *
     * @return void* return the raw data
     */
    void* loadFileData() const;
    /**
     * \brief loads and rescales data from url.
     *
     * @param uvec2 dst_dimesion destination dimension
     * @return void* returns the raw data that has been rescaled to dst_dimension
     */
    void* loadFileDataAndRescale(uvec2 dst_dimesion) const;
};

} // namespace

#endif // IVW_IMAGEDISK_H
