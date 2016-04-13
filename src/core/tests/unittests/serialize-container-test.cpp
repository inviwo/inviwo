/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2016 Inviwo Foundation
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

#include <inviwo/core/common/inviwo.h>
#include <inviwo/core/io/serialization/serializable.h>

namespace inviwo {

TEST(ContainerSerialitionTest, Minimal) {
    std::stringstream ss;
    Serializer serializer("");
    
    std::vector<int> vector {1,2,3,4,5};
    
    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);
    
    vector[2] = 10;
    vector.pop_back();
    
    Deserializer deserializer(nullptr, ss, "");
    deserializer.deserialize("Vector", vector, "Item");
    
    ASSERT_EQ(5, vector.size());
    ASSERT_EQ(1, vector[0]);
    ASSERT_EQ(2, vector[1]);
    ASSERT_EQ(3, vector[2]);
    ASSERT_EQ(4, vector[3]);
    ASSERT_EQ(5, vector[4]);
}
    
    
TEST(ContainerSerialitionTest, ContainerTest1) {
    std::stringstream ss;
    Serializer serializer("");
    
    std::vector<int> vector {1,2,3,4,5};
    
    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);
    
    vector[2] = 10;
    vector.push_back(12);

    int tmp = 0;
    std::vector<bool> visited(vector.size(), false);
    ContainerWrapper<int> cont(
        "Item", [&](std::string id,
                    size_t ind) -> ContainerWrapper<int>::Item {
            if (ind < vector.size()) {
                return {true, vector[ind], [&visited, ind](int& val) { visited[ind] = true; }};
            } else {
                return {true, tmp, [&visited, &vector](int& val) {
                            visited.push_back(true);
                            vector.push_back(val);
                        }};
            }
        });

    ASSERT_EQ(vector.size(), visited.size());
    
    Deserializer deserializer(nullptr, ss, "");
    deserializer.deserialize("Vector", cont);

    size_t ind = 0;
    util::erase_remove_if(vector, [&](int& i) {
        return !visited[ind++];
    });
    
    ASSERT_EQ(5, vector.size());
    ASSERT_EQ(1, vector[0]);
    ASSERT_EQ(2, vector[1]);
    ASSERT_EQ(3, vector[2]);
    ASSERT_EQ(4, vector[3]);
    ASSERT_EQ(5, vector[4]);
}

TEST(ContainerSerialitionTest, ContainerTest2) {
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

    std::vector<Item> vector{{"a", 1}, {"b", 2}, {"c", 3}};

    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);

    vector[0].id_ = "c";
    vector.pop_back();
    vector.insert(vector.begin(), Item("d",4));

    Item tmp;
    std::vector<std::string> visited;
    ContainerWrapper<Item> cont(
        "Item", [&](std::string id,
                    size_t ind) -> ContainerWrapper<Item>::Item {
            visited.push_back(id);
            auto it = util::find_if(vector, [&](const Item& i) { return i.id_ == id; });
            if (it != vector.end()) {
                return {true, *it, [&](Item& val) {}};
            } else {
                return {true, tmp, [&](Item& val) { vector.push_back(val); }};
            }
        });

    Deserializer deserializer(nullptr, ss, "");
    deserializer.deserialize("Vector", cont);

    util::erase_remove_if(vector, [&](Item& i) { return !util::contains(visited, i.id_); });

    ASSERT_EQ(3, vector.size());

    ASSERT_EQ("a", vector[2].id_);
    ASSERT_EQ("b", vector[1].id_);
    ASSERT_EQ("c", vector[0].id_);

    ASSERT_EQ(1, vector[2].value_);
    ASSERT_EQ(2, vector[1].value_);
    ASSERT_EQ(3, vector[0].value_);
}

TEST(ContainerSerialitionTest, ContainerTest3) {
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
    
    vector.push_back(new Item("a",1));
    vector.push_back(new Item("b",2));
    vector.push_back(new Item("c",3));

    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);

    vector[0]->id_ = "c";
    vector.pop_back();
    vector.insert(vector.begin(), new Item("d", 1));

    Item* tmp = nullptr;
    std::vector<std::string> visited;
    ContainerWrapper<Item*> cont(
        "Item", [&](std::string id,
                    size_t ind) -> ContainerWrapper<Item*>::Item {
            visited.push_back(id);
            auto it = util::find_if(vector, [&](Item*& i) { return i->id_ == id; });
            if (it != vector.end()) {
                return {true, *it, [&](Item*& val) {}};
            } else {
                tmp = new Item();
                return {true, tmp, [&](Item*& val) { vector.push_back(val); }};
            }
        });

    Deserializer deserializer(nullptr, ss, "");
    deserializer.deserialize("Vector", cont);

    util::erase_remove_if(vector, [&](Item*& i) {
        if (!util::contains(visited, i->id_)) {
            delete i;
            return true;
        } else {
            return false;
        }
    });

    ASSERT_EQ(3, vector.size());

    ASSERT_EQ("a", vector[2]->id_);
    ASSERT_EQ("b", vector[1]->id_);
    ASSERT_EQ("c", vector[0]->id_);

    ASSERT_EQ(1, vector[2]->value_);
    ASSERT_EQ(2, vector[1]->value_);
    ASSERT_EQ(3, vector[0]->value_);
    
    for (auto& item : vector) delete item;
}


TEST(ContainerSerialitionTest, ContainerTest4) {
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
    
    vector.push_back(new Item("a",1));
    vector.push_back(new Item("b",2));
    vector.push_back(new Item("c",3));

    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);

    vector[0]->id_ = "c";
    vector.pop_back();
    vector.insert(vector.begin(), new Item("d",1));

    Deserializer deserializer(nullptr, ss, "");
    auto des = util::IdentifiedDeserializer<std::string, Item*>("Vector", "Item")
                   .setGetId([](Item* const& i) { return i->id_; })
                   .setMakeNew([]() { return new Item(); })
                   .onNew([&](Item*& i) { vector.push_back(i); })
                   .onRemove([&](const std::string& id) {
                       util::erase_remove_if(vector, [&](Item* i) {
                           if (id == i->id_) {
                               delete i;
                               return true;
                           } else {
                               return false;
                           }
                       });
                   });

    des(deserializer, vector);

    ASSERT_EQ(3, vector.size());

    ASSERT_EQ("a", vector[2]->id_);
    ASSERT_EQ("b", vector[1]->id_);
    ASSERT_EQ("c", vector[0]->id_);

    ASSERT_EQ(1, vector[2]->value_);
    ASSERT_EQ(2, vector[1]->value_);
    ASSERT_EQ(3, vector[0]->value_);
    
    for (auto& item : vector) delete item;
}

TEST(ContainerSerialitionTest, ContainerTest5) {
    std::stringstream ss;
    Serializer serializer("");
    
    std::vector<int> vector {1,2,3,4,5};
    
    serializer.serialize("Vector", vector, "Item");
    serializer.writeFile(ss);
    
    vector[2] = 10;
    vector.push_back(12);

    Deserializer deserializer(nullptr, ss, "");
    auto des = util::IndexedDeserializer<int>("Vector", "Item")
                   .setMakeNew([]() { return 0; })
                   .onNew([&](int& i) { vector.push_back(i); })
                   .onRemove([](int& i) {return true;});
    
    des(deserializer, vector);
    
   
    ASSERT_EQ(5, vector.size());
    ASSERT_EQ(1, vector[0]);
    ASSERT_EQ(2, vector[1]);
    ASSERT_EQ(3, vector[2]);
    ASSERT_EQ(4, vector[3]);
    ASSERT_EQ(5, vector[4]);
}


TEST(ContainerSerialitionTest, ContainerTest6) {
    std::stringstream ss;
    Serializer serializer("");
    
    std::unordered_map<std::string, int> map = {{"a", 1}, {"b", 2}, {"c", 3}};
    
    serializer.serialize("Map", map, "Item");
    serializer.writeFile(ss);
    
    std::string str = ss.str();
    
    map["a"] = 10;
    map.erase("b");
    map["d"] = 20;

    using Item = std::pair<const std::string, int>;

    Deserializer deserializer(nullptr, ss, "");
    auto des = util::MapDeserializer<std::string, int>("Map", "Item")
                   .setMakeNew([]() { return 0; })
                   .onNew([&](const std::string& k, int& v) { map[k] = v; })
                   .onRemove([&](const std::string& k) {map.erase(k);});

    des(deserializer, map);
    
    ASSERT_EQ(map.size(), 3);
    ASSERT_EQ(map["a"], 1);
    ASSERT_EQ(map["b"], 2);
    ASSERT_EQ(map["c"], 3);
}

TEST(ContainerSerialitionTest, ContainerTest7) {
    std::stringstream ss;
    Serializer serializer("");
    
    std::unordered_map<int, std::string> map = {{1, "a"}, {2, "b"}, {3, "c"}};
    
    serializer.serialize("Map", map, "Item");
    serializer.writeFile(ss);
    
    std::string str = ss.str();
    
    map[1] = "g";
    map.erase(2);
    map[4] = "h";

    using Item = std::pair<const int, std::string>;

    Deserializer deserializer(nullptr, ss, "");
    auto des = util::MapDeserializer<int, std::string>("Map", "Item")
                   .setMakeNew([]() { return ""; })
                   .onNew([&](const int& k, std::string& v) { map[k] = v; })
                   .onRemove([&](const int& k) {map.erase(k);});

    des(deserializer, map);
    
    ASSERT_EQ(map.size(), 3);
    ASSERT_EQ(map[1], "a");
    ASSERT_EQ(map[2], "b");
    ASSERT_EQ(map[3], "c");
}


} // namespace

