/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2019 Inviwo Foundation
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

#ifndef IVW_CIMGSAVEBUFFER_H
#define IVW_CIMGSAVEBUFFER_H

#include <modules/cimg/cimgmoduledefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/util/memoryfilehandle.h>

#include <warn/push>
#include <warn/ignore/all>
#if (_MSC_VER)
#pragma warning(disable : 4146)
#pragma warning(disable : 4197)
#pragma warning(disable : 4293)
#pragma warning(disable : 4309)
#pragma warning(disable : 4319)
#pragma warning(disable : 4324)
#pragma warning(disable : 4456)
#pragma warning(disable : 4458)
#pragma warning(disable : 4611)
#pragma warning(disable : 5040)
#endif
struct IUnknown;  // Workaround for "combaseapi.h(229): error C2187: syntax error: 'identifier' was
                  // unexpected here" when using /permissive-
#include <CImg.h>
#include <warn/pop>

#include <cstdio>
#include <iterator>
#include <vector>

namespace inviwo {

namespace cimgutil {

template <typename T>
std::vector<unsigned char> saveCImgToBuffer(const cimg_library::CImg<T>& img,
                                            const std::string& ext);

template <typename T>
const cimg_library::CImg<T>& saveCImgToFileStream(FILE* handle, const cimg_library::CImg<T>& img,
                                                  const std::string& ext);

template <typename T>
std::vector<unsigned char> saveCImgToBuffer(const cimg_library::CImg<T>& img,
                                            const std::string& ext) {
    // estimate upper bound for size of the image with an additional header
    const size_t header = 1024;  // assume 1kB is enough for a header
    const size_t upperBound = img.width() * img.height() * img.spectrum() * sizeof(T) + header;

    util::MemoryFileHandle memfile(upperBound);
    saveCImgToFileStream(memfile.getHandle(), img, ext);

    if (memfile.checkForOverflow()) {
        // overflow detected buffer was too small, try again with twice the size
        memfile.resize(upperBound * 4u);
        saveCImgToFileStream(memfile.getHandle(), img, ext);

        if (memfile.checkForOverflow()) {
            throw Exception(
                "saveCImgToBuffer(): could not save image to buffer, exceeding buffer size.");
        }
    }

    auto it = memfile.getBuffer().begin();
    std::vector<unsigned char> data(it, it + memfile.getNumberOfBytesInBuffer());

    return data;
}

template <typename T>
const cimg_library::CImg<T>& saveCImgToFileStream(FILE* handle, const cimg_library::CImg<T>& img,
                                                  const std::string& extension) {
    // the following code was taken from CImg::save() and slightly adapted
    if (extension.empty()) {
        throw cimg_library::CImgIOException("specified extension is empty");
    }
    const char* ext = extension.c_str();
    if (!cimg_library::cimg::strcasecmp(ext, "cpp") ||
        !cimg_library::cimg::strcasecmp(ext, "hpp") || !cimg_library::cimg::strcasecmp(ext, "h") ||
        !cimg_library::cimg::strcasecmp(ext, "c"))
        return img.save_cpp(handle);

    // 2d binary formats
    else if (!cimg_library::cimg::strcasecmp(ext, "bmp"))
        return img.save_bmp(handle);
    else if (!cimg_library::cimg::strcasecmp(ext, "jpg") ||
             !cimg_library::cimg::strcasecmp(ext, "jpeg") ||
             !cimg_library::cimg::strcasecmp(ext, "jpe") ||
             !cimg_library::cimg::strcasecmp(ext, "jfif") ||
             !cimg_library::cimg::strcasecmp(ext, "jif"))
        return img.save_jpeg(handle);
    else if (!cimg_library::cimg::strcasecmp(ext, "rgb"))
        return img.save_rgb(handle);
    else if (!cimg_library::cimg::strcasecmp(ext, "rgba"))
        return img.save_rgba(handle);
    else if (!cimg_library::cimg::strcasecmp(ext, "pgm") ||
             !cimg_library::cimg::strcasecmp(ext, "ppm") ||
             !cimg_library::cimg::strcasecmp(ext, "pnm"))
        return img.save_pnm(handle);
    else if (!cimg_library::cimg::strcasecmp(ext, "pnk"))
        return img.save_pnk(handle);
    else if (!cimg_library::cimg::strcasecmp(ext, "pfm"))
        return img.save_pfm(handle);
    // these do not provide a FILE* stream interface:
    // else if (!cimg_library::cimg::strcasecmp(ext, "exr"))
    //    return img.save_exr(handle);
    // else if (!cimg_library::cimg::strcasecmp(ext, "tif") || !cimg_library::cimg::strcasecmp(ext,
    // "tiff"))
    //    return img.save_tiff(handle);

    // 3d binary formats
    else if (!cimg_library::cimg::strcasecmp(ext, "raw"))
        return img.save_raw(handle);
    else
        throw cimg_library::CImgIOException("unsupported format");
}

}  // namespace cimgutil

}  // namespace inviwo

#endif  // IVW_CIMGSAVEBUFFER_H
