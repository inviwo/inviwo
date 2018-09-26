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

#ifndef IVW_GDCMVOLUMEREADER_H
#define IVW_GDCMVOLUMEREADER_H

#include <modules/dicom/dicommoduledefine.h>
#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/datastructures/data.h>
#include <inviwo/core/datastructures/volume/volume.h>
#include <inviwo/core/datastructures/volume/volumedisk.h>
#include <inviwo/core/datastructures/volume/volumeramprecision.h>

#include <inviwo/core/io/datareader.h>
#include <inviwo/core/io/datareaderexception.h>

#include <warn/push>
#include <warn/ignore/all>
#include <MediaStorageAndFileFormat/gdcmImageReader.h>
#include <warn/pop>

namespace inviwo {

	struct DICOMDIRImage {
		std::string path;
	};

	struct DICOMDIRSeries {
		std::vector<DICOMDIRImage> images;
		size_t totalByteCount = 0;
		std::string modality; // e.g. "CT"
	};

	struct DICOMDIRStudy {
		std::vector<DICOMDIRSeries> series;
	};

	struct DICOMDIRPatient {
		std::vector<DICOMDIRStudy> studies;
	};

	using SharedVolumeSequence = std::shared_ptr<VolumeSequence>;
	using SharedVolume = std::shared_ptr<Volume>;

	class IVW_MODULE_DICOM_API GdcmVolumeReader : public DataReaderType<VolumeSequence> {
	public:
		GdcmVolumeReader();
		virtual GdcmVolumeReader* clone() const;
		virtual ~GdcmVolumeReader() = default;

		// interface gdcm - mevis reader (for single volumes)
		SharedVolume generateVolume(const gdcm::Image &image, const gdcm::File &file);
		const DataFormatBase* getFormat() { return format_; }
		size3_t getDimension() { return dimension_; }

		// the entry point of the reader
		virtual std::shared_ptr<VolumeSequence> readData(const std::string& filePath);

	private:
		// file or folder from where volume(s) from last readData call
		std::string file_;

		// format and dimension only relevant when a single volume is read
		const DataFormatBase* format_;
		size3_t dimension_;

		// volumes from last readData call
		SharedVolumeSequence volumes_;

		static SharedVolumeSequence tryReadDICOMDIR(const std::string& fileOrDirectory);

		static SharedVolumeSequence tryReadDICOMsequence(const std::string& sequenceDirectory);

		static SharedVolumeSequence tryReadDICOMsequenceRecursive(const std::string& directory);

		static SharedVolume getVolumeDescription(DICOMDIRSeries series);
	};

	
	/**
	 * \brief A loader for dcm files. Used to create VolumeRAM representations.
	 * This class us used by the GdcmVolumeReader.
	 */
	class IVW_MODULE_DICOM_API GCDMVolumeRAMLoader : public DiskRepresentationLoader<VolumeRepresentation> {
	public:
		GCDMVolumeRAMLoader(std::string file, size3_t dimension, const DataFormatBase* format,
			bool isPartOfSequence = false, DICOMDIRSeries series = {})
			: file_(file), dimension_(dimension), format_(format) ,
				isPartOfSequence_(isPartOfSequence), series_(series) {}

		virtual GCDMVolumeRAMLoader* clone() const override { return new GCDMVolumeRAMLoader(*this); }
		virtual ~GCDMVolumeRAMLoader() = default;
		
		virtual std::shared_ptr<VolumeRepresentation> createRepresentation() const override {
			return format_->dispatch(*this);
		}

		virtual void updateRepresentation(std::shared_ptr<VolumeRepresentation> dest) const override;
		
		using type = std::shared_ptr<VolumeRAM>;
		
		template <class T>
		std::shared_ptr<VolumeRAM> dispatch() const;
		
	private:
		static void getVolumeData(DICOMDIRSeries series, void* outData);
		std::string file_; // only relevant for single volumes
		size3_t dimension_;
		const DataFormatBase* format_;
		bool isPartOfSequence_;
		DICOMDIRSeries series_;
	};

} // namespace

#endif // IVW_GDCMVOLUMEREADER_H
