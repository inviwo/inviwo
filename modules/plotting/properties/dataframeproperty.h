/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2017 Inviwo Foundation
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

#ifndef IVW_DATAFRAMEPROPERTY_H
#define IVW_DATAFRAMEPROPERTY_H

#include <modules/plotting/plottingmoduledefine.h>
#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/properties/optionproperty.h>
#include <modules/plotting/datastructures/dataframe.h>

namespace inviwo {

namespace plot {

class IVW_MODULE_PLOTTING_API DataFrameColumnProperty : public OptionPropertyInt {
public:
    InviwoPropertyInfo();
    DataFrameColumnProperty(std::string identifier, std::string displayName, bool allowNone = false,
                            size_t firstIndex = 0);
    DataFrameColumnProperty(std::string identifier, std::string displayName,
                            DataInport<DataFrame> &port, bool allowNone = false,
                            size_t firstIndex = 0);
    virtual ~DataFrameColumnProperty() = default;

    void setOptions(std::shared_ptr<const DataFrame> dataframe);

    std::shared_ptr<const Column> getColumn();
    std::shared_ptr<const BufferBase> getBuffer();

    virtual std::string getClassIdentifierForWidget() const override {
        return TemplateOptionProperty<int>::getClassIdentifier();
    }

    virtual void set(const Property *p) override;

private:
    std::shared_ptr<const DataFrame> dataframe_;
    bool allowNone_;
    size_t firstIndex_;
};

}  // namespace plot

}  // namespace inviwo

#endif  // IVW_DATAFRAMEPROPERTY_H
