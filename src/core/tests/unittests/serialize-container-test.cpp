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

#include <warn/push>
#include <warn/ignore/all>
#include <gtest/gtest.h>
#include <warn/pop>

#include <inviwo/core/io/serialization/serialization.h>
#include <vector>
#include <charconv>

namespace inviwo {

TEST(SerializationContainerTest, Minimal) {
    std::stringstream ss;
    Serializer serializer("");

    std::vector<int> vector{1, 2, 3, 4, 5};

    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);

    vector[2] = 10;
    vector.pop_back();

    Deserializer deserializer(ss, "");
    deserializer.deserialize("Vector", vector, "Item");

    ASSERT_EQ(5, vector.size());
    ASSERT_EQ(1, vector[0]);
    ASSERT_EQ(2, vector[1]);
    ASSERT_EQ(3, vector[2]);
    ASSERT_EQ(4, vector[3]);
    ASSERT_EQ(5, vector[4]);
}

TEST(SerialitionContainerTest, IdentifierFunctions) {
    struct Item : Serializable {
        Item() = default;
        Item(std::string id, int value) : id_(id), value_(value) {}

        virtual void serialize(Serializer& s) const override {
            s.serialize("identifier", id_, SerializationTarget::Attribute);
            s.serialize("value", value_);
        }
        virtual void deserialize(Deserializer& d) override {
            d.deserialize("identifier", id_, SerializationTarget::Attribute);
            d.deserialize("value", value_);
        }

        std::string id_;
        int value_;
    };

    std::stringstream ss;
    Serializer serializer("");

    std::vector<Item*> vector;

    vector.push_back(new Item("a", 1));
    vector.push_back(new Item("b", 2));
    vector.push_back(new Item("c", 3));

    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);

    vector[0]->id_ = "c";
    delete vector.back();
    vector.pop_back();
    vector.insert(vector.begin(), new Item("d", 1));

    Deserializer deserializer(ss, "");
    deserializer.deserialize("Vector", vector, "Item",
                             deserializer::IdentifierFunctions{
                                 .getID = [](Item* const& i) -> std::string_view { return i->id_; },
                                 .makeNew = []() { return new Item(); },
                                 .onNew = [&](Item*& i, size_t) { vector.push_back(i); },
                                 .onRemove =
                                     [&](std::string_view id) {
                                         std::erase_if(vector, [&](Item* i) {
                                             if (id == i->id_) {
                                                 delete i;
                                                 return true;
                                             } else {
                                                 return false;
                                             }
                                         });
                                     },
                                 .onMove =
                                     [&](Item*& i, size_t newIndex) {
                                         if (auto it = std::find(vector.begin(), vector.end(), i);
                                             it != vector.end()) {
                                             Item* item = i;
                                             vector.erase(it);
                                             vector.insert(vector.begin() + newIndex, item);
                                         }
                                     }});

    ASSERT_EQ(3, vector.size());

    ASSERT_EQ("a", vector[0]->id_);
    ASSERT_EQ("b", vector[1]->id_);
    ASSERT_EQ("c", vector[2]->id_);

    ASSERT_EQ(1, vector[0]->value_);
    ASSERT_EQ(2, vector[1]->value_);
    ASSERT_EQ(3, vector[2]->value_);

    for (auto& item : vector) delete item;
}

TEST(SerialitionContainerTest, IndexFunctions) {
    std::stringstream ss;
    Serializer serializer("");

    std::vector<int> vector{1, 2, 3, 4, 5};

    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);

    vector[2] = 10;
    vector.push_back(12);

    Deserializer deserializer(ss, "");
    deserializer.deserialize("Vector", vector, "Item",
                             deserializer::IndexFunctions{
                                 .makeNew = []() { return 0; },
                                 .onNew = [&](int& i, size_t) {},
                                 .onRemove = [](int&) {},
                             });

    ASSERT_EQ(5, vector.size());
    ASSERT_EQ(1, vector[0]);
    ASSERT_EQ(2, vector[1]);
    ASSERT_EQ(3, vector[2]);
    ASSERT_EQ(4, vector[3]);
    ASSERT_EQ(5, vector[4]);
}

TEST(SerialitionContainerTest, MapFunctions1) {
    std::stringstream ss;
    Serializer serializer("");

    std::unordered_map<std::string, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};

    serializer.serialize("Map", map, "Item");
    serializer.writeFile(ss);

    std::string str = ss.str();

    map["a"] = 10;
    map.erase("b");
    map["d"] = 20;

    Deserializer deserializer(ss, "");

    deserializer.deserialize(
        "Map", map, "Item",
        deserializer::MapFunctions{.idTransform = [](std::string_view s) { return std::string{s}; },
                                   .makeNew = []() { return 0; },
                                   .onNew = [&](const std::string& k, int& v) { map[k] = v; },
                                   .onRemove = [&](const std::string& k) { map.erase(k); }

        });

    ASSERT_EQ(map.size(), 3);
    ASSERT_EQ(map["a"], 1);
    ASSERT_EQ(map["b"], 2);
    ASSERT_EQ(map["c"], 3);
}

TEST(SerialitionContainerTest, MapFunctions2) {
    std::stringstream ss;
    Serializer serializer("");

    std::unordered_map<int, std::string> map = {{1, "a"}, {2, "b"}, {3, "c"}};

    serializer.serialize("Map", map, "Item");
    serializer.writeFile(ss);

    std::string str = ss.str();

    map[1] = "g";
    map.erase(2);
    map[4] = "h";

    Deserializer deserializer(ss, "");
    deserializer.deserialize(
        "Map", map, "Item",
        deserializer::MapFunctions{.idTransform = [](std::string_view s) -> int {
                                       int val{};
                                       std::from_chars(s.data(), s.data() + s.size(), val);
                                       return val;
                                   },
                                   .makeNew = []() { return std::string{}; },
                                   .onNew = [&](const int& k, std::string& v ) { map[k] = v; },
                                   .onRemove = [&](const int& k) { map.erase(k); }

        });

    ASSERT_EQ(map.size(), 3);
    ASSERT_EQ(map[1], "a");
    ASSERT_EQ(map[2], "b");
    ASSERT_EQ(map[3], "c");
}

}  // namespace inviwo
