/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016-2024 Inviwo Foundation
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

#include <inviwo/dataframe/dataframemoduledefine.h>  // for IVW_MODULE_DATAFRAME_API

#include <inviwo/core/properties/optionproperty.h>      // for OptionPropertyInt
#include <inviwo/dataframe/datastructures/dataframe.h>  // for DataFrameInport, DataFrame

#include <functional>   // for function
#include <memory>       // for shared_ptr
#include <string>       // for string
#include <string_view>  // for string_view

namespace inviwo {
class Property;

class IVW_MODULE_DATAFRAME_API ColumnOptionProperty : public OptionPropertyInt {
public:
    enum class AddNoneOption { No, Yes };

    virtual std::string_view getClassIdentifier() const override;
    static constexpr std::string_view classIdentifier{"org.inviwo.DataFrameColumnProperty"};

    /**
     * Constructor, no options will be created unless \p noneOption is set to AddNoneOption::Yes
     *
     * @param identifier
     * @param displayName
     * @param noneOption if equal to AddNoneOption::Yes, "None" will be added as option
     * @param defaultIndex index of the selected column when the property options are updated
     */
    ColumnOptionProperty(std::string_view identifier, std::string_view displayName,
                         AddNoneOption noneOption = AddNoneOption::No, int defaultIndex = 0);
    /**
     * Constructor using an inport \p port to populate the options. The onChange event of \p port
     * will trigger an update of the options.
     *
     * @param identifier
     * @param displayName
     * @param port
     * @param noneOption if equal to AddNoneOption::Yes, "None" will be added as option
     * @param defaultIndex index of the selected column when the property options are updated
     */
    ColumnOptionProperty(std::string_view identifier, std::string_view displayName,
                         DataFrameInport& port, AddNoneOption noneOption = AddNoneOption::No,
                         int defaultIndex = 0);

    ColumnOptionProperty(const ColumnOptionProperty& rhs);
    virtual ColumnOptionProperty* clone() const override;

    virtual ~ColumnOptionProperty() = default;

    /**
     * Populate the options with the data owned by \p port. The onChange event of \p port
     * will trigger an update of the options.
     *
     * @note A reference to the port will be kept, so the port must outlive the scope of the
     * property.
     */
    void setPort(DataFrameInport& port);

    /**
     * Replace the options based on the columns in \p dataframe
     */
    void setOptions(const DataFrame& dataframe);

    /**
     * Set the \p index of the selected column when the property options are updated. If \p index is
     * negative and the property has a "None" option, that one will be the new default.
     *
     * @param index
     */
    void setDefaultSelectedIndex(int index);

    /**
     * Return the header of the currently selected column
     *
     * @return column header. If "None" is selected, an empty string is returned
     */
    const std::string& getSelectedColumnHeader() const;

    /**
     * Return whether no column is selected.
     *
     * @return true if there is a "None" option and it is selected
     */
    bool isNoneSelected() const;

    virtual void set(const Property* p) override;

private:
    DataFrameInport* inport_ = nullptr;
    const AddNoneOption noneOption_;
    int defaultColumnIndex_;

    std::shared_ptr<std::function<void()>> onChangeCallback_;
};

}  // namespace inviwo
