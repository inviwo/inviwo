/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2017-2023 Inviwo Foundation
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

#include <modules/base/basemoduledefine.h>  // for IVW_MODULE_BASE_API

#include <inviwo/core/datastructures/volume/volume.h>  // for VolumeSequence
#include <inviwo/core/io/datawriter.h>                 // for DataWriterType, Overwrite, Overwri...
#include <modules/base/io/ivfvolumewriter.h>           // for IvfVolumeWriter

#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
/**
 * \ingroup dataio
 * \brief Writter for *.ivfs sequnce files
 *
 * Supports writing a volume sequence to disk. Will create one main file ([name].ivfs) and a series
 * of ivf volumes ([name]xx.ivf), one for each element in the sequence.
 *
 * The output structure of the ivfs sequence files is:
 * \verbatim
<?xml version="1.0" ?>
<InviwoWorkspace version="2">
    <volumes>
        <volume content="./relative/path/to/volume00.ivf" />
        <volume content="./relative/path/to/volume01.ivf" />
        ...
        <volume content="./relative/path/to/volumeNN.ivf" />
    </volumes>
</InviwoWorkspace>
 * \endverbatim
 *
 * @see inviwo::IvfSequenceVolumeReader inviwo::util::writeIvfVolumeSequence
 *
 */
class IVW_MODULE_BASE_API IvfSequenceVolumeWriter : public DataWriterType<VolumeSequence> {
public:
    IvfSequenceVolumeWriter() = default;
    IvfSequenceVolumeWriter(const IvfSequenceVolumeWriter& rhs) = default;
    IvfSequenceVolumeWriter& operator=(const IvfSequenceVolumeWriter& that) = default;
    virtual IvfSequenceVolumeWriter* clone() const override {
        return new IvfSequenceVolumeWriter(*this);
    }
    virtual ~IvfSequenceVolumeWriter() = default;

    /**
     * \brief Writes a volume sequence to disk
     *
     * Writes a volume sequence to disk. Will create one main file ([name].ivfs) and a series of
     * ivf volumes ([name]xx.ivf), one for each element in the sequence.
     *
     * @param volumes The volume sequence to export
     * @param filePath path to where the files will be written
     */
    virtual void writeData(const VolumeSequence* volumes,
                           const std::filesystem::path& filePath) const override;

    /**
     * \brief Writes a volume sequence to disk
     *
     * Writes a volume sequence to disk. Will create one main file ([name].ivfs) and a series of
     * ivf volumes ([name]xx.ivf), one for each element in the sequence.
     *
     * @param volumes The volume sequence to export
     * @param name the name of the dataset, will be used for to name the output files [name].ivfs
     * and [name]xx.ivf
     * @param path path to the folder to put the main file
     * @param relativePathToTimeSteps relative path (from the path to the main file) to where the
     * sequence elements will be written.
     */
    void writeData(const VolumeSequence* volumes, std::string_view name,
                   const std::filesystem::path& path,
                   std::string_view relativePathToTimeSteps = "") const;

private:
    IvfVolumeWriter writer_;
};

namespace util {
/**
 * \brief Writes a volume sequence to disk
 *
 * Writes a volume sequence to disk. Will create one main file ([name].ivfs) and a series of ivf
 * volumes ([name]xx.ivf), one for each element in the sequence.
 *
 * @param volumes The volume sequence to export
 * @param name the name of the dataset, will be used for to name the output files [name].ivfs and
 * [name]xx.ivf
 * @param path path to the folder to put the main file
 * @param relativePathToElements relative path (from the path to the main file) to where the
 * sequence elements will be written
 * @param overwrite whether or not to overwrite existing files.
 * @return path to the created main-file
 * @see inviwo::IvfSequenceVolumeWriter
 * @see inviwo::IvfSequenceVolumeReader
 */
IVW_MODULE_BASE_API std::filesystem::path writeIvfVolumeSequence(
    const VolumeSequence& volumes, std::string_view name, const std::filesystem::path& path,
    std::string_view relativePathToElements = "", Overwrite overwrite = Overwrite::Yes);
}  // namespace util

}  // namespace inviwo
