/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019-2025 Inviwo Foundation
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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/common/inviwoapplication.h>
#include <inviwo/dataframepython/dataframepythonmodule.h>
#include <modules/python3/python3module.h>
#include <modules/python3/pybindutils.h>

#include <inviwo/dataframe/datastructures/column.h>
#include <inviwo/dataframe/datastructures/dataframe.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eval.h>

#include <fmt/format.h>
#include <memory>

namespace inviwo {

namespace py = ::pybind11;

namespace {

struct AddColumnScript {
    template <typename T>
    auto operator()() {
        auto classname = Defaultvalues<T>::getName();

        const std::string source = fmt::format(R"delim(
import inviwopy
import ivwdataframe

d = ivwdataframe.DataFrame()
d.add{0}Column('{0}Col')
)delim",
                                               classname);

        auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

        py::eval<py::eval_statements>(source, dict);

        auto dataframe = dict["d"].cast<DataFrame>();

        ASSERT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";

        auto col = dataframe.getColumn(1);
        auto expectedFormat = DataFormat<T>::get();
        EXPECT_EQ(expectedFormat->getId(), col->getBuffer()->getDataFormat()->getId())
            << "Dataformat mismatch";
        const std::string expectedHeader = fmt::format("{}Col", classname).c_str();
        EXPECT_EQ(expectedHeader, col->getHeader()) << "Column header does not match";
    }
};

}  // namespace

TEST(DataFrameTests, Create) {
    pybind11::gil_scoped_acquire guard{};

    const std::string source = R"delim(
import inviwopy
import ivwdataframe

d = ivwdataframe.DataFrame()
d2 = ivwdataframe.DataFrame(10)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto dataframe = dict["d"].cast<DataFrame>();
    auto dataframe2 = dict["d2"].cast<DataFrame>();

    EXPECT_EQ(1, dataframe.getNumberOfColumns()) << "DataFrame contains more than index column";
    EXPECT_EQ(0, dataframe.getNumberOfRows()) << "DataFrame contains rows";

    EXPECT_EQ(1, dataframe2.getNumberOfColumns()) << "DataFrame contains more than index column";
    EXPECT_EQ(0, dataframe2.getNumberOfRows()) << "DataFrame contains rows";
    EXPECT_EQ(10, dataframe2.getIndexColumn()->getSize())
        << "Incorrect size of Index column buffer";
}

TEST(DataFrameTests, AddColumn) {
    pybind11::gil_scoped_acquire guard{};

    using Scalars = std::tuple<float, double, int, glm::i64, std::uint32_t>;

    util::for_each_type<Scalars>{}(AddColumnScript{});
}

TEST(DataFrameTests, AddCategoricalColumn) {
    pybind11::gil_scoped_acquire guard{};
    
    const std::string colname = "CatColumn";

    const std::string source = fmt::format(R"delim(
import inviwopy
import ivwdataframe

d = ivwdataframe.DataFrame()
d.addCategoricalColumn('{}')
)delim",
                                           colname);

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto dataframe = dict["d"].cast<DataFrame>();

    ASSERT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";

    auto col = dataframe.getColumn(1);
    auto expectedFormat = DataFormat<uint32_t>::get();
    EXPECT_EQ(expectedFormat->getId(), col->getBuffer()->getDataFormat()->getId())
        << "Dataformat mismatch";
    EXPECT_EQ(colname, col->getHeader()) << "Column header does not match";
}

TEST(DataFrameTests, AddCategoricalColumnData) {
    pybind11::gil_scoped_acquire guard{};

    const std::string colname = "CatColumn";

    const std::string source = fmt::format(R"delim(
import inviwopy
import ivwdataframe

d = ivwdataframe.DataFrame()
d.addCategoricalColumn('{}', ['a', 'c', 'b'])
d.updateIndex()
)delim",
                                           colname);

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto dataframe = dict["d"].cast<DataFrame>();

    ASSERT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";

    auto col = dataframe.getColumn(1);
    auto expectedFormat = DataFormat<uint32_t>::get();
    EXPECT_EQ(expectedFormat->getId(), col->getBuffer()->getDataFormat()->getId())
        << "Dataformat mismatch";
    EXPECT_EQ(colname, col->getHeader()) << "Column header does not match";

    auto catcol = dynamic_cast<CategoricalColumn*>(col.get());
    ASSERT_TRUE(catcol) << "Column is not categorical";

    const auto& result = catcol->getCategories();
    const std::vector<std::string> expected = {"a", "c", "b"};

    EXPECT_EQ(expected, result) << "Categories after append are not correct";
}

TEST(DataFrameTests, AddColumnFromBuffer) {
    pybind11::gil_scoped_acquire guard{};

    const std::string colName = "FloatCol";

    const std::string source = fmt::format(R"delim(
import inviwopy
import ivwdataframe
import numpy as np

buffer = inviwopy.data.Buffer(np.array([1.0, 2.0, 3.0], dtype=np.single))

d = ivwdataframe.DataFrame()
d.addColumnFromBuffer('{}', buffer)
d.updateIndex()
)delim",
                                           colName);

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto dataframe = dict["d"].cast<DataFrame>();

    EXPECT_EQ(2, dataframe.getNumberOfColumns()) << "Incorrect number of columns";
    EXPECT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";

    auto col = dataframe.getColumn(1);
    EXPECT_EQ(colName, col->getHeader());
    EXPECT_EQ(3, col->getSize()) << "Column row count incorrect";
}

TEST(DataFrameTests, RowAccess) {
    pybind11::gil_scoped_acquire guard{};

    const size_t rowIndex = 1;

    const std::string source = R"delim(
import inviwopy
import ivwdataframe
import numpy as np

buffer = inviwopy.data.Buffer(np.array([1.0, 2.0, 3.0], dtype=np.single))

d = ivwdataframe.DataFrame()
d.addColumnFromBuffer('FloatCol', buffer)
col = d.addIntColumn('IntCol', 3)
col.set(0, 4)
col.set(1, 5)
col.set(2, 6)
d.updateIndex()
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto dataframe = dict["d"].cast<DataFrame>();
    ASSERT_EQ(3, dataframe.getNumberOfRows()) << "Incorrect number of rows";
    ASSERT_EQ(3, dataframe.getNumberOfColumns()) << "Incorrect number of columns";

    py::eval<py::eval_single_statement>(fmt::format("value = col.get({})", rowIndex), dict);

    auto pyValue = dict["value"].cast<int>();
    EXPECT_EQ(5, pyValue) << "Wrong data value in int column";

    auto indexcol = dataframe.getIndexColumn();
    auto floatcol = std::dynamic_pointer_cast<TemplateColumn<float>>(dataframe.getColumn(1));
    auto intcol = std::dynamic_pointer_cast<const TemplateColumn<int>>(dataframe.getColumn(2));

    ASSERT_TRUE(floatcol) << "column not found";
    ASSERT_TRUE(intcol) << "column not found";

    EXPECT_EQ(rowIndex, indexcol->get(rowIndex)) << "Incorrect row index";
    EXPECT_EQ(2.0f, floatcol->get(rowIndex));
    EXPECT_EQ(5, intcol->get(rowIndex));
}

}  // namespace inviwo
