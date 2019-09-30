/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2019 Inviwo Foundation
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

#include <inviwo/core/datastructures/buffer/buffer.h>
#include <inviwo/core/datastructures/buffer/bufferram.h>
#include <inviwo/core/datastructures/buffer/bufferramprecision.h>

#include <inviwo/core/util/defaultvalues.h>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/eval.h>

#include <fmt/format.h>

namespace inviwo {

namespace py = ::pybind11;

namespace {

struct CreateColumnScript {
    template <typename T>
    auto operator()() {
        auto classname = Defaultvalues<T>::getName();

        const std::string source = fmt::format(R"delim(
import inviwopy
import ivwdataframe

col = ivwdataframe.{0}Column('{0}Col')
)delim",
                                               classname);

        auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

        py::eval<py::eval_statements>(source, dict);

        auto pyCol = dict["col"];
        auto col = pyCol.cast<TemplateColumn<T>>();

        auto expectedFormat = DataFormat<T>::get();
        EXPECT_EQ(expectedFormat->getId(), col.getBuffer()->getDataFormat()->getId())
            << "Dataformat mismatch";

        const std::string expectedHeader = fmt::format("{}Col", classname).c_str();
        EXPECT_STREQ(expectedHeader.c_str(), col.getHeader().c_str())
            << "Column header does not match";
    }
};

}  // namespace

TEST(ColumnTests, Create) {
    using Scalars = std::tuple<float, double, int, glm::i64, size_t, std::uint32_t>;

    util::for_each_type<Scalars>{}(CreateColumnScript{});
}

TEST(ColumnTests, CategoricalColumn) {
    const std::string source = R"delim(
import inviwopy
import ivwdataframe

col = ivwdataframe.CategoricalColumn('Column')
col.add('a')
col.add('c')
col.add('b')
col.add('a')
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto pyCategories = dict["col"].attr("categories");
    auto result = pyCategories.cast<std::vector<std::string>>();

    const std::vector<std::string> expected = {"a", "c", "b"};

    auto rows = dict["col"].attr("size").cast<size_t>();

    EXPECT_EQ(4, rows) << "Row count differs";
    EXPECT_EQ(expected, result) << "Categories in column are not correct";
}

TEST(ColumnTests, FloatColumn) {
    const std::string source = R"delim(
import inviwopy
import ivwdataframe
import numpy as np

col = ivwdataframe.FloatColumn('Column')
col.buffer.size = 6
col.buffer.data = np.array([0.0, 0.1, 0.5, 1.0, 2.0, 10.0], dtype=np.single)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto buffer = dict["col"].attr("buffer").cast<std::shared_ptr<BufferBase>>();
    auto rows = dict["col"].attr("size").cast<size_t>();

    ASSERT_EQ(DataFormat<float>::id(), buffer->getDataFormat()->getId())
        << "column buffer has incorrect data format";
    EXPECT_EQ(6, rows) << "Row count differs";

    auto bufferram =
        static_cast<const BufferRAMPrecision<float>*>(buffer->getRepresentation<BufferRAM>());
    const std::vector<float> expected = {0.0f, 0.1f, 0.5f, 1.0f, 2.0f, 10.0f};

    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Categories in column are not correct";
}

TEST(ColumnTests, DataAccess) {
    const std::string source = R"delim(
import inviwopy
import ivwdataframe
import numpy as np

col = ivwdataframe.FloatColumn('Column')
col.buffer.size = 6
col.buffer.data = np.array([0.0, 0.1, 0.5, 1.0, 2.0, 10.0], dtype=np.single)

value = col.get(2)
col.set(2, 5.0)
)delim";

    auto dict = py::cast<py::dict>(PyDict_Copy(py::globals().ptr()));

    py::eval<py::eval_statements>(source, dict);

    auto buffer = dict["col"].attr("buffer").cast<std::shared_ptr<BufferBase>>();
    auto rows = dict["col"].attr("size").cast<size_t>();

    ASSERT_EQ(DataFormat<float>::id(), buffer->getDataFormat()->getId())
        << "column buffer has incorrect data format";
    EXPECT_EQ(6, rows) << "Row count differs";

    auto bufferram =
        static_cast<const BufferRAMPrecision<float>*>(buffer->getRepresentation<BufferRAM>());
    const std::vector<float> expected = {0.0f, 0.1f, 5.0f, 1.0f, 2.0f, 10.0f};

    EXPECT_EQ(expected, bufferram->getDataContainer()) << "Categories in column are not correct";
}

}  // namespace inviwo
