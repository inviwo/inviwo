/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2020 Inviwo Foundation
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

#include <inviwo/core/network/processornetwork.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>

#include <filesystem>
#include <fstream>

#include <inviwo/propertybasedtesting/propertybasedtestingmoduledefine.h>
#include <inviwo/propertybasedtesting/algorithm/propertyanalyzing.h>
#include <inviwo/propertybasedtesting/testresult.h>

namespace inviwo {

namespace pbt {

template<typename T>
class TestPropertyFactoryObjectTemplate;

class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyObservable;

class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyObserver : public Observer {
public:
    friend TestPropertyObservable;
    TestPropertyObserver() = default;
    virtual void onTestPropertyChange() {}
    virtual ~TestPropertyObserver() = default;
};

class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyObservable : public Observable<TestPropertyObserver> {
protected:
    TestPropertyObservable() = default;
    void notifyObserversAboutChange() {
        forEachObserver([](TestPropertyObserver* o) { o->onTestPropertyChange(); });
    }
    virtual ~TestPropertyObservable() = default;
};

/** 
 * A NetworkPath<T> is a reference to a Processor or Property of type T in the
 * ProcessorNetwork. If the pointer to the object is not known (i.e.
 * after deserialization) or one wants to deserialize a NetworkPath, 
 * the corresponding ProcessorNetwork must be provided via
 * setNetwork(...) before the deserialization/first access.
 */
template <typename T>
class NetworkPath : public Serializable {
private:
    mutable std::string path_;
    mutable T* ptr_;
    bool isProcessor_; // dynamic, since this may not be clear from T alone, e.g. if T is PropertyOwner 

    ProcessorNetwork* pn_;

    const std::string& path() const {
        if (ptr_ == nullptr) {
            IVW_ASSERT(!path_.empty(), "NetworkPath: ptr is null but path is empty");
            return path_;
        } else {
            if (auto p = dynamic_cast<Processor*>(ptr_)) {
                IVW_ASSERT(isProcessor_,
                        "NetworkPath: ptr points to Processor but isProcessor_ disagrees");
                return path_ = p->getIdentifier();
            } else {
                IVW_ASSERT(!isProcessor_,
                        "NetworkPath: ptr does not point to Processor but isProcessor_ disagrees");
                auto p2 = dynamic_cast<Property*>(ptr_);
                IVW_ASSERT(p2 != nullptr,
                        "NetworkPath: ptr points not to Processor and not to a Property");
                return path_ = p2->getPath();
            }
        }
    }
public:
    bool operator<(const NetworkPath& other) const { return path() < other.path(); }
    // only for deserialization
    NetworkPath() = default;

    NetworkPath(T* ptr, ProcessorNetwork* pn = nullptr)
        : path_("")
        , ptr_(ptr)
        , isProcessor_(dynamic_cast<Processor*>(ptr) != nullptr)
        , pn_(pn) {}
    void setNetwork(ProcessorNetwork* pn) {
        if (pn == nullptr) {
            path_ = path();
            ptr_ = nullptr;
			pn_ = nullptr;
        } else {
			pn_ = pn;
			maybeGet();
		}
    }

	/*
	 * Is true, if and only if the pointer to the referenced object or the
	 * the pointer to the network is known (and in the latter case, when the
	 * path is not empty).
	 */
	bool isValid() const {
		return ptr_ != nullptr || pn_ != nullptr;
	}
	/*
	 * Fails iff !isValid()
	 * Returns nullptr, if and only if ptr_ is null and the network does
	 * not contain the referenced object.
	 */
	T* maybeGet() const {
        if (ptr_ == nullptr) {
            IVW_ASSERT(pn_ != nullptr,
                "NetworkPath::get(): \"" + path_ + "\" ptr and network are both nullptr");
            if (isProcessor_) {
                Processor* const t = pn_->getProcessorByIdentifier(path_);
				if(t == nullptr) { // processor does not exist in the network
					return nullptr;
				}
                ptr_ = dynamic_cast<T*>(t);
                IVW_ASSERT(ptr_ != nullptr,
                    "NetworkPath::get(): \"" + path_ +"\" referenced processor has wrong type");
                return ptr_;
            } else {
                Property* const tmp = pn_->getProperty(path_);
				if(tmp == nullptr) { // property does not exist in the network
					return nullptr;
				}
                ptr_ = dynamic_cast<T*>(tmp);
                IVW_ASSERT(ptr_ != nullptr,
                    "NetworkPath::get(): \"" + path_ + "\" referenced object has wrong type");
            }
        }
        return ptr_;
	}
	/*
	 * Note: only works under the assumption that maybeGet() does not
	 * return nullptr
	 */
    T* get() const {
		T* const res = maybeGet();
		IVW_ASSERT(res != nullptr,
				"NetworkPath::get(): \"" + path_ + "\" The referenced object does not exist in the network");
		return res;
    }
    operator T*() const { return get(); }
    T* operator->() const { return get(); }
    T& operator*() const { return *get(); }

    void serialize(Serializer& s) const override {
        s.serialize("Path", path());
        s.serialize("IsProcessor", isProcessor_);
    }
    void deserialize(Deserializer& d) override {
        d.deserialize("Path", path_);
        d.deserialize("IsProcessor", isProcessor_);
        pn_ = nullptr;
        ptr_ = nullptr;
    }
};

/** \docpage{org.inviwo.PropertyAnalyzer, PropertyAnalyzer}
 * ![](org.inviwo.TestProperty.png?classIdentifier=org.inviwo.TestProperty)
 *
 * A TestProperty holds a reference to a testable property and maintains the
 * respective BoolCompositeProperty and drop-down menu.
 * It is used to provide a general interface to all properties, regardless of
 * their value_type.
 * It enables
 *    - generation of assignments,
 *    - a textual description of the expected effects on the counted number of pixels,
 *    - the expected effect on the counted number of pixels given two testcases, and
 *  - storing and resetting the value of the property before the tests have started.
 * Instantiation happes mainly by createTestableProperty (see below).
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API TestProperty
        : public Serializable
        , public TestPropertyObservable
        {
private:
    std::string displayName_, identifier_;
    NetworkPath<BoolCompositeProperty> boolComp_ = nullptr;
    std::vector<std::function<void(ProcessorNetwork*)>> onNetwork;

protected:
    void onNetworkReceive(std::function<void(ProcessorNetwork*)>);
    TestProperty() = default;
    TestProperty(const std::string& displayName, const std::string& identifier);

    const auto& options() const {
        const static std::vector<OptionPropertyOption<int>> options{
            {"EQUAL", "EQUAL", 0},     {"NOT_EQUAL", "NOT_EQUAL", 1},
            {"LESS", "LESS", 2},       {"LESS_EQUAL", "LESS_EQUAL", 3},
            {"GREATER", "GREATER", 4}, {"GREATER_EQUAL", "GREATER_EQUAL", 5},
            {"ANY", "ANY", 6},         {"NOT_COMPARABLE", "NOT_COMPARABLE", 7}};
        return options;
    }

public:
    /*
     * This needs to be called after construction/deserialization
     * in order to ensure that the NetworkPaths are correct
     */
    void setNetwork(ProcessorNetwork*);
    /*
     * returns the total number of checked (enabled) 
     * testable properties (i.e. excluding composite properties)
     */
    virtual size_t totalNumCheckedProperties() const = 0;
    /*
     * deactivated(i) returns a pointer to a boolean indicating
     * whether the i-th contained testable property is active
     * and is included in tests
     */
    virtual bool* deactivated(size_t) = 0;
    virtual const bool* deactivated(size_t) const = 0;

    virtual void serialize(Serializer&) const override;
    virtual void deserialize(Deserializer&) override;

    virtual BoolCompositeProperty* getBoolComp() const;

    const std::string& getDisplayName() const;
    const std::string& getIdentifier() const;

    /*
     * returns a textual description of the effects the active properties
     * should have on the score of the output when changed
     */
    virtual std::string textualDescription(unsigned int indent = 0) const = 0;

    /*
     * returns the values that the currently active testable properties had in a
     * test in a human-readable form
     */
    virtual std::string getValueString(std::shared_ptr<TestResult>) const = 0;
    /*
     * traverses the contained TestProperties in pre order and calls the given
     * function for each TestProperty with the current TestProperty as first
     * argument and its parent TestProperty as second argument (or nullptr when
     * there is no traversed parent)
     */
    void traverse(std::function<void(const TestProperty*, const TestProperty*)>) const;
    virtual void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
                          const TestProperty*) const = 0;

    /*
     * returns the PropertyEffect expected given the two test cases and the
     * set PropertyEffects of the contained currently active testable properties
     */
    virtual PropertyEffect getPropertyEffect(
        std::shared_ptr<TestResult>, std::shared_ptr<TestResult>) const = 0;

    /*
     * writes the values of all contained currently active testable properties
     * in both test results as well as the expected PropertyEffect (as returned
     * by getPropertyEffect) in human readable form to the std::ostream
     */
    virtual std::ostream& ostr(std::ostream&, std::shared_ptr<TestResult>,
                               std::shared_ptr<TestResult>) const = 0;

    /*
     * reset all contained testable properties to the stored default value
     * (i.e. the value the properties had before the testing started)
     */
    virtual void setToDefault() const = 0;
    /*
     * store the current value of all contained testable properties
     */
    virtual void storeDefault() = 0;
    /*
     * returns a vector containing an AssignmentComparator as well as a vector
     * of assignments for each contained currently active testable property
     */
    virtual std::vector<
        std::pair<pbt::AssignmentComparator,
        std::vector<std::shared_ptr<PropertyAssignment>>>>
    generateAssignmentsCmp(std::default_random_engine&) const = 0;
    virtual ~TestProperty();
};

/*
 * returns the combined expected effect (if any) of two values, given the
 * effect of each of the components.
 */
template <typename T, size_t numComp = DataFormat<T>::components()>
PropertyEffect propertyEffect(
    const T& val_old, const T& val_new,
    const std::array<pbt::PropertyEffect, numComp>& selectedEffects);

/*
 * Creates and returns a TestProperty for an arbitrary property, if possible.
 * That is, if the given property derives from CompositeProperty or one of
 * the properties in PropertyTypes (see below).
 */
std::unique_ptr<TestProperty> IVW_MODULE_PROPERTYBASEDTESTING_API
    createTestableProperty(Property* prop);

/*
 * Adds necessary callbacks for the proper behavior of the BoolComps of/in a
 * BoolCompositeProperty, i.e. the BoolProperty of the BoolCompositeProperty
 * is checked, if and only if at least one of the contained
 * BoolCompositeProperties is checked.
 */
void IVW_MODULE_PROPERTYBASEDTESTING_API makeOnChange(BoolCompositeProperty* const prop);

/*
 * Derived from TestProperty, for all PropertyOwners supporting
 * getDisplayName and getIdentifier, i.e. CompositeProperty and Processor
 */
class IVW_MODULE_PROPERTYBASEDTESTING_API TestPropertyComposite
        : public TestProperty
        , public TestPropertyObserver
        {
    NetworkPath<PropertyOwner> propertyOwner_;
    std::vector<std::unique_ptr<TestProperty>> subProperties;

    TestPropertyComposite() = default;
    TestPropertyComposite(PropertyOwner* original, const std::string& displayName,
                          const std::string& identifier);

    void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
                  const TestProperty*) const override;

public:
    TestPropertyComposite(Processor*);
    TestPropertyComposite(CompositeProperty*);
    PropertyOwner* getPropertyOwner() const;

    template <typename F>
    void for_each_checked(F) const;

    size_t totalNumCheckedProperties() const override;
    bool* deactivated(size_t) override;
    const bool* deactivated(size_t) const override;

    const static std::string& classIdentifier() {
        const static std::string name = "org.inviwo.TestPropertyComposite";
        return name;
    }
    friend class TestPropertyCompositeFactoryObject;
    friend class TestPropertyFactoryObjectTemplate<TestPropertyComposite>;

    void onTestPropertyChange() override;

    std::string textualDescription(unsigned int indent = 0) const override;

    std::string getValueString(std::shared_ptr<TestResult> testResult) const override;

    PropertyEffect getPropertyEffect(
        std::shared_ptr<TestResult> newTestResult,
        std::shared_ptr<TestResult> oldTestResult) const override;

    std::ostream& ostr(std::ostream& out, std::shared_ptr<TestResult> newTestResult,
                       std::shared_ptr<TestResult> oldTestResult) const override;

    virtual ~TestPropertyComposite() = default;
    void setToDefault() const override;

    void serialize(Serializer& s) const override;
    void deserialize(Deserializer& s) override;

    template <typename T>
    std::optional<typename T::value_type> getDefaultValue(const T* prop) const;

    void storeDefault();
    std::vector<std::pair<pbt::AssignmentComparator,
        std::vector<std::shared_ptr<PropertyAssignment>>>>
    generateAssignmentsCmp(std::default_random_engine&) const override;
};

/*
 * Derived from TestProperty, for all Properties having a value that is
 * supported by DataFormat<>.
 */
template <typename T>
class TestPropertyTyped : public TestProperty {
    using value_type = typename T::value_type;
    static constexpr size_t numComponents = DataFormat<value_type>::components();

    NetworkPath<T> typedProperty_;
    value_type defaultValue_;
    std::array<NetworkPath<OptionPropertyInt>, numComponents> effectOption_;

    std::array<pbt::PropertyEffect, numComponents> selectedEffects() const;
    bool deactivated_;

    void traverse(std::function<void(const TestProperty*, const TestProperty*)>,
                  const TestProperty*) const override;

    TestPropertyTyped() = default;
public:
    OptionPropertyInt* getEffectOption(size_t) const;
    T* getTypedProperty() const;

    size_t totalNumCheckedProperties() const override;
    bool* deactivated(size_t) override;
    const bool* deactivated(size_t) const override;

    const static std::string& classIdentifier() {
        const static std::string name =
            std::string("org.inviwo.TestPropertyTyped") + PropertyTraits<T>::classIdentifier();
        return name;
    }

    friend class TestPropertyFactoryObjectTemplate<TestPropertyTyped<T>>;

    std::string textualDescription(unsigned int indent) const override;

    std::string getValueString(std::shared_ptr<TestResult>) const override;

    PropertyEffect getPropertyEffect(
        std::shared_ptr<TestResult> newTestResult,
        std::shared_ptr<TestResult> oldTestResult) const override;

    std::ostream& ostr(std::ostream&, std::shared_ptr<TestResult>,
                       std::shared_ptr<TestResult>) const override;

    TestPropertyTyped(T* original);
    virtual ~TestPropertyTyped() = default;
    void setToDefault() const override;
    const value_type& getDefaultValue() const;

    void serialize(Serializer& s) const override;
    void deserialize(Deserializer& s) override;

    void storeDefault();
    std::vector<std::pair<pbt::AssignmentComparator,
        std::vector<std::shared_ptr<PropertyAssignment>>>>
    generateAssignmentsCmp(std::default_random_engine&) const override;
};


template <typename T, size_t numComp = DataFormat<T>::components()>
PropertyEffect propertyEffect(
    const T& val_old, const T& val_new,
    const std::array<pbt::PropertyEffect, numComp>& selectedEffects) {

    PropertyEffect res = PropertyEffect::ANY;
    for (size_t i = 0; i < numComp; i++) {
        auto compEff = propertyEffect(selectedEffects[i],
                                            GetComponent<T>::get(val_new, i),
                                            GetComponent<T>::get(val_old, i));
        res = pbt::combine(res, compEff);
    }
    return res;
}

template <typename F>
void TestPropertyComposite::for_each_checked(F f) const {
    for (const auto& prop : subProperties) {
        if (prop->getBoolComp()->isChecked()) f(prop.get());
    }
}

template <typename T>
std::optional<typename T::value_type> TestPropertyComposite::getDefaultValue(const T* prop) const {
    for (const auto& subProp : subProperties) {
        if (auto p = dynamic_cast<TestPropertyTyped<T>*>(subProp.get()); p != nullptr) {
            if (p->getTypedProperty() == prop) return p->getDefaultValue();
        } else if (auto p = dynamic_cast<TestPropertyComposite*>(subProp.get()); p != nullptr) {
            if (auto res = p->getDefaultValue(prop); res != std::nullopt) return res;
        }
    }
    return std::nullopt;
}

/*
 * Tuple containing all testable property types except CompositeProperty.
 * If you want to add support for a new property type, you probably want to add
 * it to this list.
 */
using PropertyTypes = std::tuple<IntProperty, FloatProperty, DoubleProperty, IntMinMaxProperty>;

} // namespace pbt

}  // namespace inviwo
