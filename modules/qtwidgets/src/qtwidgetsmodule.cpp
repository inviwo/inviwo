/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2026 Inviwo Foundation
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

#include <modules/qtwidgets/qtwidgetsmodule.h>

#include <inviwo/core/common/inviwomodule.h>
#include <inviwo/core/datastructures/camera/camera.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/buttongroupproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/minmaxproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/ordinalrefproperty.h>
#include <inviwo/core/properties/propertysemantics.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/stringsproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/exception.h>
#include <inviwo/core/util/foreacharg.h>
#include <inviwo/core/util/glmmat.h>
#include <inviwo/core/util/glmvec.h>
#include <inviwo/core/util/staticstring.h>
#include <inviwo/core/util/zip.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/properties/anglepropertywidgetqt.h>
#include <modules/qtwidgets/properties/boolcompositepropertywidgetqt.h>
#include <modules/qtwidgets/properties/boolpropertywidgetqt.h>
#include <modules/qtwidgets/properties/buttongrouppropertywidgetqt.h>
#include <modules/qtwidgets/properties/buttonpropertywidgetqt.h>
#include <modules/qtwidgets/properties/colorpropertywidgetqt.h>
#include <modules/qtwidgets/properties/compositepropertywidgetqt.h>
#include <modules/qtwidgets/properties/eventpropertywidgetqt.h>
#include <modules/qtwidgets/properties/filepropertywidgetqt.h>
#include <modules/qtwidgets/properties/fontsizepropertywidgetqt.h>
#include <modules/qtwidgets/properties/isotfpropertywidgetqt.h>
#include <modules/qtwidgets/properties/isovaluepropertywidgetqt.h>
#include <modules/qtwidgets/properties/lightpropertywidgetqt.h>
#include <modules/qtwidgets/properties/listpropertywidgetqt.h>
#include <modules/qtwidgets/properties/multifilepropertywidgetqt.h>
#include <modules/qtwidgets/properties/optionpropertywidgetqt.h>             // IWYU pragma: keep
#include <modules/qtwidgets/properties/ordinalminmaxpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalminmaxtextpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringspropertywidgetqt.h>
#include <modules/qtwidgets/properties/tfprimitivesetwidgetqt.h>
#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>
#include <modules/qtwidgets/rawdatareaderdialogqt.h>
#include <modules/qtwidgets/tfhelpwindow.h>

#include <array>
#include <cstddef>
#include <functional>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <vector>

#include <QApplication>
#include <fmt/core.h>
#include <glm/common.hpp>
#include <glm/detail/type_quat.hpp>
#include <glm/ext/quaternion_double.hpp>
#include <glm/ext/scalar_relational.hpp>
#include <glm/fwd.hpp>
#include <glm/geometric.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/mat2x2.hpp>
#include <glm/mat3x3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/vec2.hpp>
#include <glm/vec3.hpp>
#include <glm/vec4.hpp>

#ifndef INVIWO_ALL_DYN_LINK
struct InitQtResources {
    // Needed for loading of resources when building statically
    // see https://wiki.qt.io/QtResources#Q_INIT_RESOURCE
    InitQtResources() { Q_INIT_RESOURCE(inviwo); }
    ~InitQtResources() { Q_CLEANUP_RESOURCE(inviwo); }
} initQtWidgetsResources;
#endif

namespace inviwo {

struct FileExtension;
class InviwoApplication;

namespace {

struct ColorWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const PropertySemantics& semantics) {
        using PropertyType = OrdinalProperty<T>;
        using PropertyWidget = ColorPropertyWidgetQt<T>;

        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
    }
};

template <OrdinalPropertyWidgetQtSemantics Sem>
struct OrdinalWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const PropertySemantics& semantics) {
        using PropertyType = OrdinalProperty<T>;
        using PropertyWidget = OrdinalPropertyWidgetQt<T, Sem>;
        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);

        using RefPropertyType = OrdinalRefProperty<T>;
        using RefPropertyWidget = OrdinalRefPropertyWidgetQt<T, Sem>;
        qm.registerPropertyWidget<RefPropertyWidget, RefPropertyType>(semantics);
    }
};

struct MinMaxWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const PropertySemantics& semantics) {
        using PropertyType = MinMaxProperty<T>;
        using PropertyWidget = OrdinalMinMaxPropertyWidgetQt<T>;
        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
    }
};
struct MinMaxTextWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const PropertySemantics& semantics) {
        using PropertyType = MinMaxProperty<T>;
        using DataType = std::conditional_t<std::is_integral<T>::value, int, double>;
        using PropertyWidget = OrdinalMinMaxTextPropertyWidgetQt<DataType, T>;
        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
    }
};

}  // namespace

QtWidgetsModule::QtWidgetsModule(InviwoApplication* app)
    : InviwoModule(app, "QtWidgets")
    , tfMenuHelper_(std::make_unique<TFMenuHelper>()) {
    if (!qApp) {
        throw ModuleInitException("QApplication must be constructed before QtWidgetsModule");
    }

    // Register bool property widgets
    registerPropertyWidget<BoolPropertyWidgetQt, BoolProperty>(PropertySemantics::Default);
    registerPropertyWidget<BoolPropertyWidgetQt, BoolProperty>(PropertySemantics::Text);

    // Register composite/list property widgets
    registerPropertyWidget<CompositePropertyWidgetQt, CompositeProperty>(
        PropertySemantics::Default);
    registerPropertyWidget<BoolCompositePropertyWidgetQt, BoolCompositeProperty>(
        PropertySemantics::Default);
    registerPropertyWidget<ListPropertyWidgetQt, ListProperty>(PropertySemantics::Default);

    // Register file property widgets
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::Default);
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::TextEditor);

    registerPropertyWidget<MultiFilePropertyWidgetQt, MultiFileProperty>(
        PropertySemantics::Default);
    registerPropertyWidget<MultiFileStringPropertyWidgetQt, MultiFileProperty>(
        PropertySemantics::Text);

    // Register color property widgets
    using ColorTypes = std::tuple<ivec3, ivec4, vec3, vec4, dvec3, dvec4>;
    util::for_each_type<ColorTypes>{}(ColorWidgetReghelper{}, *this, PropertySemantics::Color);

    // Register ordinal property widgets
    using OrdinalTypes =
        std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4, dmat2,
                   dmat3, dmat4, int, ivec2, ivec3, ivec4, glm::i64, unsigned int, uvec2, uvec3,
                   uvec4, size_t, size2_t, size3_t, size4_t, glm::fquat, glm::dquat>;
    util::for_each_type<OrdinalTypes>{}(
        OrdinalWidgetReghelper<OrdinalPropertyWidgetQtSemantics::Default>{}, *this,
        PropertySemantics::Default);
    util::for_each_type<OrdinalTypes>{}(
        OrdinalWidgetReghelper<OrdinalPropertyWidgetQtSemantics::Text>{}, *this,
        PropertySemantics::Text);
    util::for_each_type<OrdinalTypes>{}(
        OrdinalWidgetReghelper<OrdinalPropertyWidgetQtSemantics::SpinBox>{}, *this,
        PropertySemantics::SpinBox);

    // Register MinMaxProperty widgets
    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<ScalarTypes>{}(MinMaxWidgetReghelper{}, *this, PropertySemantics::Default);
    util::for_each_type<ScalarTypes>{}(MinMaxTextWidgetReghelper{}, *this, PropertySemantics::Text);

    // Register option property widgets
    registerPropertyWidget<OptionPropertyWidgetQt, OptionPropertyString>(
        PropertySemantics::Default);

    // Register string property widgets
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::Default);
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>("Password");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::TextEditor);

    registerPropertyWidget<StringsPropertyWidgetQt<1>, StringsProperty<1>>(
        PropertySemantics::Default);
    registerPropertyWidget<StringsPropertyWidgetQt<2>, StringsProperty<2>>(
        PropertySemantics::Default);
    registerPropertyWidget<StringsPropertyWidgetQt<3>, StringsProperty<3>>(
        PropertySemantics::Default);
    registerPropertyWidget<StringsPropertyWidgetQt<4>, StringsProperty<4>>(
        PropertySemantics::Default);

    registerPropertyWidget<StringMultilinePropertyWidgetQt, StringProperty>(
        PropertySemantics::Multiline);

    // Register TF property widgets
    registerPropertyWidget<IsoValuePropertyWidgetQt, IsoValueProperty>(PropertySemantics::Default);
    registerPropertyWidget<TFPrimitiveSetWidgetQt, IsoValueProperty>(PropertySemantics::Text);
    registerPropertyWidget<TFPrimitiveSetWidgetQt, IsoValueProperty>("Text (Normalized)");
    registerPropertyWidget<TFPropertyWidgetQt, TransferFunctionProperty>(
        PropertySemantics::Default);
    registerPropertyWidget<TFPrimitiveSetWidgetQt, TransferFunctionProperty>(
        PropertySemantics::Text);
    registerPropertyWidget<TFPrimitiveSetWidgetQt, TransferFunctionProperty>("Text (Normalized)");
    registerPropertyWidget<IsoTFPropertyWidgetQt, IsoTFProperty>(PropertySemantics::Default);
    registerPropertyWidget<CompositePropertyWidgetQt, IsoTFProperty>("Composite");

    // Register misc property widgets
    registerPropertyWidget<EventPropertyWidgetQt, EventProperty>(PropertySemantics::Default);
    registerPropertyWidget<FontSizePropertyWidgetQt, IntProperty>("Fontsize");
    registerPropertyWidget<ButtonGroupPropertyWidgetQt, ButtonGroupProperty>(
        PropertySemantics::Default);
    registerPropertyWidget<ButtonPropertyWidgetQt, ButtonProperty>(PropertySemantics::Default);

    registerPropertyWidget<FloatAnglePropertyWidgetQt, FloatProperty>("Angle");
    registerPropertyWidget<DoubleAnglePropertyWidgetQt, DoubleProperty>("Angle");

    registerPropertyWidget<
        OrdinalPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSemantics::Spherical>,
        FloatVec3Property>("Spherical");
    registerPropertyWidget<
        OrdinalPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSemantics::Spherical>,
        DoubleVec3Property>("Spherical");
    registerPropertyWidget<
        OrdinalPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSemantics::SphericalSpinBox>,
        FloatVec3Property>("SphericalSpinBox");
    registerPropertyWidget<
        OrdinalPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSemantics::SphericalSpinBox>,
        DoubleVec3Property>("SphericalSpinBox");

    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSemantics::Spherical>,
        FloatVec3RefProperty>("Spherical");
    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSemantics::Spherical>,
        DoubleVec3RefProperty>("Spherical");
    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSemantics::SphericalSpinBox>,
        FloatVec3RefProperty>("SphericalSpinBox");
    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSemantics::SphericalSpinBox>,
        DoubleVec3RefProperty>("SphericalSpinBox");

    registerPropertyWidget<LightPropertyWidgetQt, FloatVec3Property>("LightPosition");

    // Register dialogs
    registerDialog<RawDataReaderDialogQt>("RawVolumeReader");
    registerDialog<InviwoFileDialog>("FileDialog");
}

QtWidgetsModule::~QtWidgetsModule() = default;

TFHelpWindow* QtWidgetsModule::getTFHelpWindow() const { return tfMenuHelper_->getWindow(); }

void QtWidgetsModule::showTFHelpWindow() const { tfMenuHelper_->showWindow(); }

}  // namespace inviwo
