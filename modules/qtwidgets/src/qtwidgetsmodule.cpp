/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2022 Inviwo Foundation
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

#include <inviwo/core/common/inviwomodule.h>                                 // for InviwoModule
#include <inviwo/core/datastructures/camera/camera.h>                        // for mat4
#include <inviwo/core/properties/boolcompositeproperty.h>                    // for BoolComposit...
#include <inviwo/core/properties/boolproperty.h>                             // for BoolProperty
#include <inviwo/core/properties/buttongroupproperty.h>                      // for ButtonGroupP...
#include <inviwo/core/properties/buttonproperty.h>                           // for ButtonProperty
#include <inviwo/core/properties/compositeproperty.h>                        // for CompositePro...
#include <inviwo/core/properties/eventproperty.h>                            // for EventProperty
#include <inviwo/core/properties/fileproperty.h>                             // for FileProperty
#include <inviwo/core/properties/isotfproperty.h>                            // for IsoTFProperty
#include <inviwo/core/properties/isovalueproperty.h>                         // for IsoValueProp...
#include <inviwo/core/properties/listproperty.h>                             // for ListProperty
#include <inviwo/core/properties/minmaxproperty.h>                           // for MinMaxProperty
#include <inviwo/core/properties/multifileproperty.h>                        // for MultiFilePro...
#include <inviwo/core/properties/optionproperty.h>                           // for OptionProperty
#include <inviwo/core/properties/ordinalproperty.h>                          // for OrdinalProperty
#include <inviwo/core/properties/ordinalrefproperty.h>                       // for OrdinalRefPr...
#include <inviwo/core/properties/propertysemantics.h>                        // for PropertySema...
#include <inviwo/core/properties/stringproperty.h>                           // for StringProperty
#include <inviwo/core/properties/stringsproperty.h>                          // for StringsProperty
#include <inviwo/core/properties/transferfunctionproperty.h>                 // for TransferFunc...
#include <inviwo/core/util/exception.h>                                      // for ModuleInitEx...
#include <inviwo/core/util/foreacharg.h>                                     // for for_each_type
#include <inviwo/core/util/glmmat.h>                                         // for dmat2, dmat3
#include <inviwo/core/util/glmvec.h>                                         // for dvec3, vec3
#include <inviwo/core/util/sourcecontext.h>                                  // for IVW_CONTEXT
#include <inviwo/core/util/staticstring.h>                                   // for operator+
#include <inviwo/core/util/zip.h>                                            // for zipper
#include <modules/qtwidgets/inviwofiledialog.h>                              // for InviwoFileDi...
#include <modules/qtwidgets/properties/anglepropertywidgetqt.h>              // for DoubleAngleP...
#include <modules/qtwidgets/properties/boolcompositepropertywidgetqt.h>      // for BoolComposit...
#include <modules/qtwidgets/properties/boolpropertywidgetqt.h>               // for BoolProperty...
#include <modules/qtwidgets/properties/buttongrouppropertywidgetqt.h>        // for ButtonGroupP...
#include <modules/qtwidgets/properties/buttonpropertywidgetqt.h>             // for ButtonProper...
#include <modules/qtwidgets/properties/colorpropertywidgetqt.h>              // for ColorPropert...
#include <modules/qtwidgets/properties/compositepropertywidgetqt.h>          // for CompositePro...
#include <modules/qtwidgets/properties/eventpropertywidgetqt.h>              // for EventPropert...
#include <modules/qtwidgets/properties/filepropertywidgetqt.h>               // for FileProperty...
#include <modules/qtwidgets/properties/fontsizepropertywidgetqt.h>           // for FontSizeProp...
#include <modules/qtwidgets/properties/isotfpropertywidgetqt.h>              // for IsoTFPropert...
#include <modules/qtwidgets/properties/isovaluepropertywidgetqt.h>           // for IsoValueProp...
#include <modules/qtwidgets/properties/lightpropertywidgetqt.h>              // for LightPropert...
#include <modules/qtwidgets/properties/listpropertywidgetqt.h>               // for ListProperty...
#include <modules/qtwidgets/properties/multifilepropertywidgetqt.h>          // for MultiFilePro...
#include <modules/qtwidgets/properties/optionpropertywidgetqt.h>             // IWYU pragma: keep
#include <modules/qtwidgets/properties/ordinalminmaxpropertywidgetqt.h>      // for OrdinalMinMa...
#include <modules/qtwidgets/properties/ordinalminmaxtextpropertywidgetqt.h>  // for OrdinalMinMa...
#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>            // for OrdinalPrope...
#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>    // for StringMultil...
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>             // for StringProper...
#include <modules/qtwidgets/properties/stringspropertywidgetqt.h>            // for StringsPrope...
#include <modules/qtwidgets/properties/tfprimitivesetwidgetqt.h>             // for TFPrimitiveS...
#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>                 // for TFPropertyWi...
#include <modules/qtwidgets/rawdatareaderdialogqt.h>                         // for RawDataReade...
#include <modules/qtwidgets/tfhelpwindow.h>                                  // for TFMenuHelper

#include <array>        // for array
#include <cstddef>      // for size_t
#include <functional>   // for __base
#include <string>       // for string
#include <string_view>  // for string_view
#include <tuple>        // for tuple
#include <type_traits>  // for conditional_t
#include <vector>       // for vector

#include <QApplication>                   // for QApplication
#include <fmt/core.h>                     // for format, basi...
#include <glm/common.hpp>                 // for max, min, clamp
#include <glm/detail/type_quat.hpp>       // for qua::operator[]
#include <glm/ext/quaternion_double.hpp>  // for dquat
#include <glm/ext/scalar_relational.hpp>  // for equal
#include <glm/fwd.hpp>                    // for fquat
#include <glm/geometric.hpp>              // for length
#include <glm/gtc/type_precision.hpp>     // for i64
#include <glm/mat2x2.hpp>                 // for operator+
#include <glm/mat3x3.hpp>                 // for operator+
#include <glm/mat4x4.hpp>                 // for operator+
#include <glm/vec2.hpp>                   // for operator!=
#include <glm/vec3.hpp>                   // for operator+, vec
#include <glm/vec4.hpp>                   // for operator+

struct InitQtResources {
    // Needed for loading of resources when building statically
    // see https://wiki.qt.io/QtResources#Q_INIT_RESOURCE
    InitQtResources() { Q_INIT_RESOURCE(inviwo); }
    ~InitQtResources() { Q_CLEANUP_RESOURCE(inviwo); }
};

namespace inviwo {

class FileExtension;
class InviwoApplication;

namespace {

struct ColorWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const std::string& semantics) {
        using PropertyType = OrdinalProperty<T>;
        using PropertyWidget = ColorPropertyWidgetQt<T>;

        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
    }
};

template <OrdinalPropertyWidgetQtSematics Sem>
struct OrdinalWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const std::string& semantics) {
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
    auto operator()(QtWidgetsModule& qm, const std::string& semantics) {
        using PropertyType = MinMaxProperty<T>;
        using PropertyWidget = OrdinalMinMaxPropertyWidgetQt<T>;
        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
    }
};
struct MinMaxTextWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const std::string& semantics) {
        using PropertyType = MinMaxProperty<T>;
        using DataType = std::conditional_t<std::is_integral<T>::value, int, double>;
        using PropertyWidget = OrdinalMinMaxTextPropertyWidgetQt<DataType, T>;
        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
    }
};

struct OptionWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const std::string& semantics) {
        using PropertyType = OptionProperty<T>;

        qm.registerPropertyWidget<OptionPropertyWidgetQt, PropertyType>(semantics);
    }
};

}  // namespace

QtWidgetsModule::QtWidgetsModule(InviwoApplication* app)
    : InviwoModule(app, "QtWidgets")
    , resources_{std::make_unique<InitQtResources>()}
    , tfMenuHelper_(std::make_unique<TFMenuHelper>()) {
    if (!qApp) {
        throw ModuleInitException("QApplication must be constructed before QtWidgetsModule",
                                  IVW_CONTEXT);
    }

    // Register bool property widgets
    registerPropertyWidget<BoolPropertyWidgetQt, BoolProperty>("Default");
    registerPropertyWidget<BoolPropertyWidgetQt, BoolProperty>("Text");

    // Register composite/list property widgets
    registerPropertyWidget<CompositePropertyWidgetQt, CompositeProperty>("Default");
    registerPropertyWidget<BoolCompositePropertyWidgetQt, BoolCompositeProperty>("Default");
    registerPropertyWidget<ListPropertyWidgetQt, ListProperty>("Default");

    // Register file property widgets
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>("Default");
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::TextEditor);

    registerPropertyWidget<MultiFilePropertyWidgetQt, MultiFileProperty>("Default");

    // Register color property widgets
    using ColorTypes = std::tuple<ivec3, ivec4, vec3, vec4, dvec3, dvec4>;
    util::for_each_type<ColorTypes>{}(ColorWidgetReghelper{}, *this, "Color");

    // Register ordinal property widgets
    using OrdinalTypes =
        std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4, dmat2,
                   dmat3, dmat4, int, ivec2, ivec3, ivec4, glm::i64, unsigned int, uvec2, uvec3,
                   uvec4, size_t, size2_t, size3_t, size4_t, glm::fquat, glm::dquat>;
    util::for_each_type<OrdinalTypes>{}(
        OrdinalWidgetReghelper<OrdinalPropertyWidgetQtSematics::Default>{}, *this, "Default");
    util::for_each_type<OrdinalTypes>{}(
        OrdinalWidgetReghelper<OrdinalPropertyWidgetQtSematics::Text>{}, *this, "Text");
    util::for_each_type<OrdinalTypes>{}(
        OrdinalWidgetReghelper<OrdinalPropertyWidgetQtSematics::SpinBox>{}, *this, "SpinBox");

    // Register MinMaxProperty widgets
    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<ScalarTypes>{}(MinMaxWidgetReghelper{}, *this, "Default");
    util::for_each_type<ScalarTypes>{}(MinMaxTextWidgetReghelper{}, *this, "Text");

    // Register option property widgets
    using OptionTypes = std::tuple<char, unsigned char, unsigned int, int, size_t, float, double,
                                   std::string, FileExtension>;
    util::for_each_type<OptionTypes>{}(OptionWidgetReghelper{}, *this, "Default");

    // Register string property widgets
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>("Default");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>("Password");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::TextEditor);

    registerPropertyWidget<StringsPropertyWidgetQt<1>, StringsProperty<1>>("Default");
    registerPropertyWidget<StringsPropertyWidgetQt<2>, StringsProperty<2>>("Default");
    registerPropertyWidget<StringsPropertyWidgetQt<3>, StringsProperty<3>>("Default");
    registerPropertyWidget<StringsPropertyWidgetQt<4>, StringsProperty<4>>("Default");

    registerPropertyWidget<StringMultilinePropertyWidgetQt, StringProperty>("Multiline");

    // Register TF property widgets
    registerPropertyWidget<IsoValuePropertyWidgetQt, IsoValueProperty>("Default");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, IsoValueProperty>("Text");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, IsoValueProperty>("Text (Normalized)");
    registerPropertyWidget<TFPropertyWidgetQt, TransferFunctionProperty>("Default");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, TransferFunctionProperty>("Text");
    registerPropertyWidget<TFPrimitiveSetWidgetQt, TransferFunctionProperty>("Text (Normalized)");
    registerPropertyWidget<IsoTFPropertyWidgetQt, IsoTFProperty>("Default");
    registerPropertyWidget<CompositePropertyWidgetQt, IsoTFProperty>("Composite");

    // Register misc property widgets
    registerPropertyWidget<EventPropertyWidgetQt, EventProperty>("Default");
    registerPropertyWidget<FontSizePropertyWidgetQt, IntProperty>("Fontsize");
    registerPropertyWidget<ButtonGroupPropertyWidgetQt, ButtonGroupProperty>("Default");
    registerPropertyWidget<ButtonPropertyWidgetQt, ButtonProperty>("Default");

    registerPropertyWidget<FloatAnglePropertyWidgetQt, FloatProperty>("Angle");
    registerPropertyWidget<DoubleAnglePropertyWidgetQt, DoubleProperty>("Angle");

    registerPropertyWidget<
        OrdinalPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSematics::Spherical>,
        FloatVec3Property>("Spherical");
    registerPropertyWidget<
        OrdinalPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSematics::Spherical>,
        DoubleVec3Property>("Spherical");
    registerPropertyWidget<
        OrdinalPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSematics::SphericalSpinBox>,
        FloatVec3Property>("SphericalSpinBox");
    registerPropertyWidget<
        OrdinalPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSematics::SphericalSpinBox>,
        DoubleVec3Property>("SphericalSpinBox");

    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSematics::Spherical>,
        FloatVec3RefProperty>("Spherical");
    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSematics::Spherical>,
        DoubleVec3RefProperty>("Spherical");
    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<vec3, OrdinalPropertyWidgetQtSematics::SphericalSpinBox>,
        FloatVec3RefProperty>("SphericalSpinBox");
    registerPropertyWidget<
        OrdinalRefPropertyWidgetQt<dvec3, OrdinalPropertyWidgetQtSematics::SphericalSpinBox>,
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
