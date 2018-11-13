/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2018 Inviwo Foundation
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

#ifndef IVW_MEVISVOLUMEREADER_H
#define IVW_MEVISVOLUMEREADER_H

#include <modules/dicom/dicommoduledefine.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeram.h>
#include <inviwo/core/io/datareader.h>

#include <tiff.h>
#include <tiffio.h>

#include <warn/push>
#include <warn/ignore/all>
#include <DataStructureAndEncodingDefinition/gdcmTag.h>
#include <DataStructureAndEncodingDefinition/gdcmAttribute.h>
#include <warn/pop>

#include <functional>

namespace inviwo {

class IVW_MODULE_DICOM_API MevisVolumeReader : public DataReaderType < Volume > {
public:
		MevisVolumeReader();
		virtual MevisVolumeReader* clone() const;
		virtual ~MevisVolumeReader() = default;

		// interface gdcm - mevis reader
		bool setFilenames(std::string filePath);

        virtual std::shared_ptr<Volume> readData(const std::string& filePath);

private:
		std::string dcm_file_;
		std::string tif_file_;
		const DataFormatBase* format;
        size3_t dimension_;
};
/**
 * \brief A loader for dcm files. Used to create VolumeRAM representations.
 * This class us used by the GdcmVolumeReader.
 */
class IVW_MODULE_DICOM_API MevisVolumeRAMLoader : public DiskRepresentationLoader<VolumeRepresentation> {
public:
    MevisVolumeRAMLoader(std::string file, size3_t dimension, const DataFormatBase* format)
        : tif_file_(file), dimension_(dimension), format_(format) {}
    virtual MevisVolumeRAMLoader* clone() const override { return new MevisVolumeRAMLoader(*this); }
    virtual ~MevisVolumeRAMLoader() = default;

    virtual std::shared_ptr<VolumeRepresentation> createRepresentation() const override {
        return format_->dispatch(*this);
        }
        virtual void updateRepresentation(std::shared_ptr<VolumeRepresentation> dest) const override {
            auto volumeDst = std::static_pointer_cast<VolumeRAM>(dest);
            auto data = volumeDst->getData();
            
            readDataInto(reinterpret_cast<void*>(data));
            
        }
        
        using type = std::shared_ptr<VolumeRAM>;
        
        template <class T>
        std::shared_ptr<VolumeRAM> dispatch() const {
            typedef typename T::type F;
            
            const std::size_t size = dimension_[0] * dimension_[1] * dimension_[2];
            
            auto data = util::make_unique<F[]>(size);
            readDataInto(reinterpret_cast<char*>(data.get()));
            auto repr = std::make_shared<VolumeRAMPrecision<F>>(data.get(), dimension_);
            data.release();
            return repr;
        }
        
private:
        void readDataInto(void* destination) const;
        std::string tif_file_;
        size3_t dimension_;
        const DataFormatBase* format_;


};

/*
@Brief A simple Resource Acquisition is Initialization (RAII) implementation.

Used to wrap the tif resource managment calls to get safe behavior guaranteed
by the compiler: destructors are always called (even if an exception occurs)
ie: we put a tif_free call in a lambda function and create a tifRAII instance with it.
*/
class tifRAII {
private:
	std::function<void()> fct;
	bool destr;
public:
	void done() {
		if (!destr) {
			fct();
			destr = true;
		}
	}

	tifRAII(std::function<void()> f) : fct(f), destr(false) {}
	~tifRAII() {
		done();
	}
};
}

#endif

