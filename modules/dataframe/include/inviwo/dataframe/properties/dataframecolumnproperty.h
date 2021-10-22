/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2021 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/core/properties/optionproperty.h>

namespace inviwo {

class IVW_MODULE_DATAFRAME_API DataFrameColumnProperty : public OptionPropertyInt {
public:
    enum class EmptySelection { No, Yes };
    enum class FlattenedView { No, Yes };

    virtual std::string getClassIdentifier() const override;
    static const std::string classIdentifier;

    /**
     * Constructor, no options will be created
     *
     * @param emptySelection   if equal to EmptySelection::Yes, "None" will be added as option
     * @param defaultIndex     index of the selected column when the property options are updated
     */
    DataFrameColumnProperty(std::string identifier, std::string displayName,
                            EmptySelection emptySelection = EmptySelection::No,
                            size_t defaultIndex = 0);
    /**
     * Constructor using an inport \p port to popuplate the options. The onChange event of \p port
     * will trigger an update of the options.
     *
     * @param port
     * @param emptySelection  if equal to EmptySelection::Yes, "None" will be added as option
     * @param defaultIndex    index of the selected column when the property options are updated
     */
    DataFrameColumnProperty(std::string identifier, std::string displayName, DataFrameInport& port,
                            EmptySelection emptySelection = EmptySelection::No,
                            size_t defaultIndex = 0);

    DataFrameColumnProperty(const DataFrameColumnProperty& rhs);
    virtual DataFrameColumnProperty* clone() const override;

    virtual ~DataFrameColumnProperty() = default;

    /**
     * populate the options with the data owned by \p port. The onChange event of \p port
     * will trigger an update of the options.
     */
    void setPort(DataFrameInport& port);
    /**
     * update the options based on the columns in \p dataframe
     */
    void setOptions(std::shared_ptr<const DataFrame> dataframe);

    void setDefaultIndex(int index);

    /**
     * return the header of the currently selected column
     *
     * @return column header. If "None" is selected, an empty string is returned
     */
    const std::string& getColumnHeader() const;

    // virtual std::string getClassIdentifierForWidget() const override {
    //    return TemplateOptionProperty<int>::getClassIdentifierForWidget();
    //}

    virtual void set(const Property* p) override;

private:
    DataFrameInport* inport_ = nullptr;
    const EmptySelection emptySelection_;
    size_t defaultIndex_;

    std::shared_ptr<std::function<void()>> onChangeCallback_;
};

}  // namespace inviwo
