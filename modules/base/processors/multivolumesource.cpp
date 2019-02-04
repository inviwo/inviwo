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

#include "multivolumesource.h"
#include <inviwo/core/util/filesystem.h>
#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/common/inviwoapplication.h>

namespace inviwo {

const ProcessorInfo MultiVolumeSource::processorInfo_{
    "org.inviwo.MultiVolumeSource", // Class identifier
    "Multi Volume Source",          // Display name
    "Data Input",                   // Category
    CodeState::Experimental,        // Code state
    Tags::CPU,                      // Tags
};
const ProcessorInfo MultiVolumeSource::getProcessorInfo() const {
    return processorInfo_;
}

MultiVolumeSource::MultiVolumeSource()
    : Processor()
    , baseDirectory_("baseDirectory_", "Base Directory")
    , numElements_("numElements_", "Num. Elements", 1, 1, 16, 1)
    , data_({{
            CompositeProperty("composite1_", "Composite 1"),
            StringProperty("composite1_string1_", "Pattern 1"),
            FileProperty("composite1_file1_", "File 1")
        }})
{
    numElements_.onChange([this]() {
        setupComposites();
    });

    addProperty(numElements_);
    addProperty(baseDirectory_);

    addDataElementsToProcessor();
}

void MultiVolumeSource::process()
{
    for (size_t idx = 0; idx < data_.size(); ++idx) {
        auto& element = data_[idx];
        StringProperty& pattern_prop = std::get<1>(element);
        FileProperty& file_prop = std::get<2>(element);

        const auto directory = baseDirectory_.get() + "/" + pattern_prop.get();
        if (filesystem::directoryExists(directory)) {
            const auto directory_content = filesystem::getDirectoryContents(directory);
            if (!directory_content.empty()) {
                const auto file_path = baseDirectory_.get() + "/" + pattern_prop.get() + "/" + directory_content[0];
                if (filesystem::fileExists(file_path)) {
                    file_prop = file_path;
                    LogInfo("found file: " << file_path);
                } else {
                    LogWarn("could not find file: " << file_path);
                }
            } else {
                LogWarn("directory: \"" << directory << "\" is empty!");
            }
        } else {
            LogWarn("directory: \"" << directory << "\" does not exist!");
        }
    }
}

void MultiVolumeSource::setupComposites()
{
    //findAndDeleteLinks(); // not neccessary?
    removeDataElementsFromProcessor();
    resizeCompositeData(numElements_.get());
    addDataElementsToProcessor();
}

void MultiVolumeSource::resizeCompositeData(size_t n)
{
    data_.resize(n, {
        CompositeProperty("composite_dummy_", "Composite Dummy"),
        StringProperty("composite_dummy_string_dummy_", "Pattern Dummy"),
        FileProperty("composite_dummy_file_dummy_", "File Dummy")}
    );
}

void MultiVolumeSource::findAndDeleteLinks()
{
    for (auto& element : data_) {
        CompositeProperty& c = std::get<0>(element);
        StringProperty& s = std::get<1>(element);
        FileProperty& f = std::get<2>(element);

        findAndDeleteLinks(s);
        findAndDeleteLinks(f);
        findAndDeleteLinks(c);
    }
}

void MultiVolumeSource::findAndDeleteLinks(Property& p)
{
    auto network = getInviwoApplication()->getProcessorNetwork();
    auto links = network->getLinks();

    for (const auto& l : links) {
        const auto s = l.getSource();
        const auto d = l.getDestination();
        if (s == &p || d == &p) {
            network->removeLink(l);
        }
    }
}

void MultiVolumeSource::addDataElementsToProcessor()
{
    for (size_t idx = 0; idx < data_.size(); ++idx) {
        auto& element = data_[idx];
        const auto idx_string = std::to_string(idx + 1);

        CompositeProperty& c = std::get<0>(element);
        const auto composite_string = "composite" + idx_string;
        c.setIdentifier(composite_string);
        c.setDisplayName("Composite " + idx_string);

        StringProperty& s = std::get<1>(element);
        s.setIdentifier(composite_string + "pattern" + idx_string);
        s.setDisplayName("Pattern " + idx_string);

        FileProperty& f = std::get<2>(element);
        f.setIdentifier(composite_string + "file" + idx_string);
        f.setReadOnly(true);
        f.setDisplayName("File " + idx_string);

        //LogInfo("adding: " << s.getIdentifier());
        c.addProperty(s);
        //LogInfo("adding: " << f.getIdentifier());
        c.addProperty(f);
        //LogInfo("adding: " << c.getIdentifier());
        addProperty(c);
    }
}

void MultiVolumeSource::removeDataElementsFromProcessor()
{
    for (auto& element : data_) {
        CompositeProperty& c = std::get<0>(element);
        StringProperty& s = std::get<1>(element);
        FileProperty& f = std::get<2>(element);

        //LogInfo("removing: " << s.getIdentifier());
        c.removeProperty(s);
        //LogInfo("removing: " << f.getIdentifier());
        c.removeProperty(f);
        //LogInfo("removing: " << c.getIdentifier());
        removeProperty(c);
    }
}

void MultiVolumeSource::serialize(Serializer& s) const {
    Processor::serialize(s);

    // save the number of composites
    s.serialize("num_composites", data_.size());
}

void MultiVolumeSource::deserialize(Deserializer& d) {
    // load the number of composites
    size_t num_composites{0};
    d.deserialize("num_composites", num_composites);

    // setup all composites to be prepared for their deserialization
    setupComposites();

    Processor::deserialize(d);
}

}  // inviwo namespace
