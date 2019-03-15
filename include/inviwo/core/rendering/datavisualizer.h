/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2018-2019 Inviwo Foundation
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

#ifndef IVW_DATAVISUALIZER_H
#define IVW_DATAVISUALIZER_H

#include <inviwo/core/common/inviwocoredefine.h>
#include <inviwo/core/common/inviwo.h>

#include <inviwo/core/ports/inport.h>
#include <inviwo/core/ports/outport.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/util/fileextension.h>

#include <inviwo/core/util/document.h>

namespace inviwo {

/**
 * A DataVisualizer can be used to easily add a visualization of a data type.
 * A DataVisualizer has two tasks. The first is to create a "Source Processor" which can be used to
 * load data of the supported type. The second is to create a "Visualizer Network", which gets data
 * (from an Outport) or loads data (from a "Source Processor") and generates some useful
 * visualization. Both parts a optional, and can be used independently or together, like:
 *     1) Adding a source processor that can load the requested file
 *     2) Adding a set of processors that will take input from a existing port to create
 *        a visualization
 *     3) Both 1 and 2
 *
 * An Inviwo module can add new DataVisualizer by deriving from DataVisualizer,
 * implementing all the virtual functions, and then add register it using
 * DataVisualizerManager::registerDataVisualizer.
 * @see DataVisualizerManager
 */
class IVW_CORE_API DataVisualizer {
public:
    DataVisualizer() = default;
    virtual ~DataVisualizer() = default;

    /**
     * A descriptive name of the visualizer
     */
    virtual std::string getName() const = 0;

    /**
     * A detailed description
     */
    virtual Document getDescription() const = 0;

    /**
     * A list of file formats that the visualizer can open
     */
    virtual std::vector<FileExtension> getSupportedFileExtensions() const = 0;

    /**
     * Test if the visualizer can be connected to the given outport
     */
    virtual bool isOutportSupported(const Outport* port) const = 0;

    /**
     * Does the visualizer provide a source processor
     */
    virtual bool hasSourceProcessor() const = 0;

    /**
     * Does the visualizer provide a visualization network
     */
    virtual bool hasVisualizerNetwork() const = 0;

    /**
     * Adds a source processor with the requested file to the network.
     * hasSourceProcessor should return true for this function to be called.
     * @param filename The file to load in the source processor. The extension of the filename
     *        should be in the list of extensions returned by getSupportedFileExtensions.
     * @param network The network to add the processor to.
     * @return The added source processor and the outport in the source processor
     *         containing data from the given file.
     */
    virtual std::pair<Processor*, Outport*> addSourceProcessor(const std::string& filename,
                                                               ProcessorNetwork* network) const = 0;
    /**
     * Adds a set of processors visualizing the data in the given outport.
     * Nothing will be added to the network if outport is not supported (isOutportSupported returns
     * false). hasVisualizationNetwork should return true for this function to be called.
     * @param outport The port to get data from. It will be connected to the added network.
     * @param network The network to add the Visualizer Network into.
     * @return A list of added processors.
     */
    virtual std::vector<Processor*> addVisualizerNetwork(Outport* outport,
                                                         ProcessorNetwork* network) const = 0;

    /**
     * Adds a source processor with the requested file and a set of processors
     * visualizing the data.
     * Nothing will be added to the network if outport is not supported (isOutportSupported returns
     * false). Only source processor will be added if hasVisualizationNetwork returns false.
     * @param filename The file to load in the source processor. The extension of the filename
     *        should be in the list of extensions returned by getSupportedFileExtensions.
     * @param network The network to add the processor to.
     * @return A list of added processors.
     */
    virtual std::vector<Processor*> addSourceAndVisualizerNetwork(
        const std::string& filename, ProcessorNetwork* network) const = 0;
};

}  // namespace inviwo

#endif  // IVW_DATAVISUALIZER_H
