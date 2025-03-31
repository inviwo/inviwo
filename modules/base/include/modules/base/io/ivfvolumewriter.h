/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2013-2025 Inviwo Foundation
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

#include <inviwo/core/datastructures/volume/volume.h>  // for DataWriterType
#include <inviwo/core/io/datawriter.h>                 // for Overwrite, Overwrite::No, DataWrit...

#include <string_view>  // for string_view

namespace inviwo {

/**
 * \ingroup dataio
 * \brief Writer for *.ivf volume files
 *
 * Supports writing a single volume to disk. Creates one main file ([name].ivf) and one raw file
 * ([name].raw or [name]xx.raw.gz if zlib compression is available).
 *
 * The output structure of the ivf file is:
 * \verbatim
<?xml version="1.0" ?>
<InviwoVolume version="2">
    <RawFiles>
        <RawFile content="../data/CLOUDf01.bin.gz">
            <MetaDataMap>
                <MetaDataItem type="org.inviwo.DoubleMetaData" key="timestamp">
                    <MetaData content="1" />
                </MetaDataItem>
                <MetaDataItem type="org.inviwo.IntMetaData" key="index">
                    <MetaData content="1" />
                </MetaDataItem>
            </MetaDataMap>
        </RawFile>
    </RawFiles>
    <ByteOrder content="1" />
    <Compression content="1" />
    <Format content="FLOAT32" />
    <BasisAndOffset>
        <col0 x="1941.7" y="0" z="0" w="0" />
        <col1 x="0" y="1996.25" z="0" w="0" />
        <col2 x="0" y="0" z="990.0" w="0" />
        <col3 x="-970.85" y="-998.125" z="1.75" w="1" />
    </BasisAndOffset>
    <WorldTransform>
        <col0 x="1" y="0" z="0" w="0" />
        <col1 x="0" y="1" z="0" w="0" />
        <col2 x="0" y="0" z="1" w="0" />
        <col3 x="0" y="0" z="0" w="1" />
    </WorldTransform>
    <Dimension x="500" y="500" z="100" />
    <DataRange x="0" y="0.00332" />
    <ValueRange x="0" y="0.00332" />
    <ValueUnit content="kg/kg" />
</InviwoVolume>
 * \endverbatim
 *
 * @see inviwo::IvfVolumeReader inviwo::util::writeIvfVolume
 */
class IVW_MODULE_BASE_API IvfVolumeWriter : public DataWriterType<Volume> {
public:
    IvfVolumeWriter();
    IvfVolumeWriter(const IvfVolumeWriter& rhs);
    IvfVolumeWriter(IvfVolumeWriter&& rhs) noexcept;
    IvfVolumeWriter& operator=(const IvfVolumeWriter& that);
    IvfVolumeWriter& operator=(IvfVolumeWriter&& that) noexcept;
    virtual IvfVolumeWriter* clone() const;
    virtual ~IvfVolumeWriter() = default;

    virtual void writeData(const Volume* data, const std::filesystem::path& filePath) const;
};

/**
 * \ingroup dataio
 * \brief Writer for *.ivfs volume sequence files
 *
 * Supports writing a volume sequence to disk. Creates one main file ([name].ivfs) and a series
 * of raw files ([name]xx.raw or [name]xx.raw.gz if zlib compression is available), one for each
 * volume.
 *
 * The output structure of the ivfs sequence files is:
 * \verbatim
<?xml version="1.0" ?>
<InviwoVolume version="2">
    <RawFiles>
        <RawFile content="../data/CLOUDf01.bin.gz">
            <MetaDataMap>
                <MetaDataItem type="org.inviwo.DoubleMetaData" key="timestamp">
                    <MetaData content="1" />
                </MetaDataItem>
            </MetaDataMap>
        </RawFile>
        <RawFile content="../data/CLOUDf02.bin.gz">
            <MetaDataMap>
                <MetaDataItem type="org.inviwo.DoubleMetaData" key="timestamp">
                    <MetaData content="2" />
                </MetaDataItem>
            </MetaDataMap>
        </RawFile>
        ...
    </RawFiles>
    <ByteOrder content="0" />
    <Compression content="1" />
    <Format content="FLOAT32" />
    ...
</InviwoVolume>
 * \endverbatim
 *
 * @see inviwo::IvfVolumeSequenceReader inviwo::util::writeIvfVolumeSequence
 */
class IVW_MODULE_BASE_API IvfVolumeSequenceWriter : public DataWriterType<VolumeSequence> {
public:
    IvfVolumeSequenceWriter();
    IvfVolumeSequenceWriter(const IvfVolumeSequenceWriter& rhs);
    IvfVolumeSequenceWriter(IvfVolumeSequenceWriter&& rhs) noexcept;
    IvfVolumeSequenceWriter& operator=(const IvfVolumeSequenceWriter& that);
    IvfVolumeSequenceWriter& operator=(IvfVolumeSequenceWriter&& that) noexcept;
    virtual IvfVolumeSequenceWriter* clone() const;
    virtual ~IvfVolumeSequenceWriter() = default;

    virtual void writeData(const VolumeSequence* data, const std::filesystem::path& filePath) const;
};

namespace util {

IVW_MODULE_BASE_API void writeIvfVolume(const Volume& data, const std::filesystem::path& filePath,
                                        Overwrite overwrite = Overwrite::Yes);

/**
 * \brief Writes a volume sequence to disk
 *
 * Supports writing a volume sequence to disk. Creates one main file (<tt>[name].ivfs</tt>) and a
 * series of raw files (<tt>[name]xx.raw</tt> or <tt>[name]xx.raw.gz</tt> if zlib compression is
 * available), one for each volume. The raw files are
 *
 * @param data    the volume sequence to export
 * @param name    file name of the dataset and raw files, that is [name].ivfs and [name]xx.raw
 * @param parentFolder    parent folder
 * @param relativePathToElements    path of raw files relative to \p path
 * @param overwrite       whether or not to overwrite existing files.
 * @return path to the created main file
 *
 * \see inviwo::IvfVolumeSequenceWriter inviwo::IvfVolumeSequenceReader
 */
IVW_MODULE_BASE_API std::filesystem::path writeIvfVolumeSequence(
    const VolumeSequence& data, std::string_view name, const std::filesystem::path& parentFolder,
    const std::filesystem::path& relativePathToElements = {}, Overwrite overwrite = Overwrite::Yes);

}  // namespace util

}  // namespace inviwo
