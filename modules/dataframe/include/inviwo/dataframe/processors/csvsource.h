/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2019 Inviwo Foundation
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

#ifndef IVW_CSVSOURCE_H
#define IVW_CSVSOURCE_H

#include <inviwo/dataframe/dataframemoduledefine.h>

#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/processors/processor.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/ports/dataoutport.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/buttonproperty.h>

namespace inviwo {

/** \docpage{org.inviwo.CSVSource, CSVSource}
 * ![](org.inviwo.CSVSource.png?classIdentifier=org.inviwo.CSVSource)
 * Reads comma separated values (CSV) and converts it into a DataFrame.
 *
 * ### Outports
 *   * __data__  DataFrame representation of the CSV input file
 *
 * ### Properties
 *   * __First Row Headers__   if true, the first row is used as column names in the DataFrame
 *   * __Delimiters__          defines the delimiter between values (default ',')
 */

class IVW_MODULE_DATAFRAME_API CSVSource : public Processor {
public:
    CSVSource(const std::string& file = "");
    virtual ~CSVSource() = default;

    virtual void process() override;

    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;

private:
    DataOutport<DataFrame> data_;
    BoolProperty firstRowIsHeaders_;
    FileProperty inputFile_;
    StringProperty delimiters_;
    ButtonProperty reloadData_;
};

}  // namespace inviwo

#endif  // IVW_CSVSOURCE_H
