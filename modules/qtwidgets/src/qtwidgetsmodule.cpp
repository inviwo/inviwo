/*********************************************************************************
 *
 * Inviwo - Interactive Visualization Workshop
 *
 * Copyright (c) 2014-2019 Inviwo Foundation
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

#include <inviwo/core/properties/boolproperty.h>
#include <inviwo/core/properties/boolcompositeproperty.h>
#include <inviwo/core/properties/buttonproperty.h>
#include <inviwo/core/properties/cameraproperty.h>
#include <inviwo/core/properties/compositeproperty.h>
#include <inviwo/core/properties/eventproperty.h>
#include <inviwo/core/properties/fileproperty.h>
#include <inviwo/core/properties/imageeditorproperty.h>
#include <inviwo/core/properties/isovalueproperty.h>
#include <inviwo/core/properties/isotfproperty.h>
#include <inviwo/core/properties/listproperty.h>
#include <inviwo/core/properties/multifileproperty.h>
#include <inviwo/core/properties/optionproperty.h>
#include <inviwo/core/properties/ordinalproperty.h>
#include <inviwo/core/properties/stringproperty.h>
#include <inviwo/core/properties/transferfunctionproperty.h>
#include <inviwo/core/util/fileextension.h>

#include <modules/qtwidgets/properties/anglepropertywidgetqt.h>
#include <modules/qtwidgets/properties/boolpropertywidgetqt.h>
#include <modules/qtwidgets/properties/boolcompositepropertywidgetqt.h>
#include <modules/qtwidgets/properties/buttongrouppropertywidgetqt.h>
#include <modules/qtwidgets/properties/buttonpropertywidgetqt.h>
#include <modules/qtwidgets/properties/collapsiblegroupboxwidgetqt.h>
#include <modules/qtwidgets/properties/colorpropertywidgetqt.h>
#include <modules/qtwidgets/properties/compositepropertywidgetqt.h>
#include <modules/qtwidgets/properties/eventpropertywidgetqt.h>
#include <modules/qtwidgets/properties/filepropertywidgetqt.h>
#include <modules/qtwidgets/properties/fontsizepropertywidgetqt.h>
#include <modules/qtwidgets/properties/isotfpropertywidgetqt.h>
#include <modules/qtwidgets/properties/isovaluepropertywidgetqt.h>
#include <modules/qtwidgets/properties/listpropertywidgetqt.h>
#include <modules/qtwidgets/properties/lightpropertywidgetqt.h>
#include <modules/qtwidgets/properties/multifilepropertywidgetqt.h>
#include <modules/qtwidgets/properties/optionpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalminmaxpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalminmaxtextpropertywidgetqt.h>
#include <modules/qtwidgets/properties/ordinalpropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringpropertywidgetqt.h>
#include <modules/qtwidgets/properties/stringmultilinepropertywidgetqt.h>
#include <modules/qtwidgets/properties/texteditorwidgetqt.h>
#include <modules/qtwidgets/properties/tfprimitivesetwidgetqt.h>
#include <modules/qtwidgets/properties/tfpropertywidgetqt.h>

#include <inviwo/core/io/rawvolumereader.h>
#include <modules/qtwidgets/rawdatareaderdialogqt.h>
#include <modules/qtwidgets/inviwofiledialog.h>
#include <modules/qtwidgets/qtwidgetssettings.h>

#include <warn/push>
#include <warn/ignore/all>
#include <QApplication>
#include <warn/pop>

#ifndef INVIWO_ALL_DYN_LINK
struct InitQtResources {
    // Needed for loading of resources when building statically
    // see https://wiki.qt.io/QtResources#Q_INIT_RESOURCE
    InitQtResources() { Q_INIT_RESOURCE(inviwo); }
    ~InitQtResources() { Q_CLEANUP_RESOURCE(inviwo); }
} initQtResources;
#endif

namespace inviwo {

struct ColorWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const std::string& semantics) {
        using PropertyType = OrdinalProperty<T>;
        using PropertyWidget = ColorPropertyWidgetQt<T>;

        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
    }
};

struct OrdinalWidgetReghelper {
    template <typename T>
    auto operator()(QtWidgetsModule& qm, const std::string& semantics) {
        using PropertyType = OrdinalProperty<T>;
        using PropertyWidget = OrdinalPropertyWidgetQt<T>;

        qm.registerPropertyWidget<PropertyWidget, PropertyType>(semantics);
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
        using PropertyType = TemplateOptionProperty<T>;

        qm.registerPropertyWidget<OptionPropertyWidgetQt, PropertyType>(semantics);
    }
};

QtWidgetsModule::QtWidgetsModule(InviwoApplication* app)
    : InviwoModule(app, "QtWidgets"), tfMenuHelper_(std::make_unique<TFMenuHelper>()) {
    if (!qApp) {
        throw ModuleInitException("QApplication must be constructed before QtWidgetsModule");
    }
    registerSettings(std::make_unique<QtWidgetsSettings>());

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
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::ShaderEditor);
    registerPropertyWidget<FilePropertyWidgetQt, FileProperty>(PropertySemantics::PythonEditor);

    registerPropertyWidget<MultiFilePropertyWidgetQt, MultiFileProperty>("Default");

    // Register color property widgets
    using ColorTypes = std::tuple<ivec3, ivec4, vec3, vec4, dvec3, dvec4>;
    util::for_each_type<ColorTypes>{}(ColorWidgetReghelper{}, *this, "Color");

    // Register ordinal property widgets
    using OrdinalTypes =
        std::tuple<float, vec2, vec3, vec4, mat2, mat3, mat4, double, dvec2, dvec3, dvec4, dmat2,
                   dmat3, dmat4, int, ivec2, ivec3, ivec4, glm::i64, unsigned int, uvec2, uvec3,
                   uvec4, size_t, size2_t, size3_t, size4_t, glm::fquat, glm::dquat>;
    util::for_each_type<OrdinalTypes>{}(OrdinalWidgetReghelper{}, *this, "Default");
    util::for_each_type<OrdinalTypes>{}(OrdinalWidgetReghelper{}, *this, "Text");
    util::for_each_type<OrdinalTypes>{}(OrdinalWidgetReghelper{}, *this, "SpinBox");

    // Register MinMaxProperty widgets
    using ScalarTypes = std::tuple<float, double, int, glm::i64, size_t>;
    util::for_each_type<ScalarTypes>{}(MinMaxWidgetReghelper{}, *this, "Default");
    util::for_each_type<ScalarTypes>{}(MinMaxTextWidgetReghelper{}, *this, "Text");

    // Register option property widgets
    using OptionTypes =
        std::tuple<unsigned int, int, size_t, float, double, std::string, FileExtension>;
    util::for_each_type<OptionTypes>{}(OptionWidgetReghelper{}, *this, "Default");

    // Register string property widgets
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>("Default");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>("Password");
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::TextEditor);
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::ShaderEditor);
    registerPropertyWidget<StringPropertyWidgetQt, StringProperty>(PropertySemantics::PythonEditor);
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

    registerPropertyWidget<FloatVec3PropertyWidgetQt, FloatVec3Property>("Spherical");
    registerPropertyWidget<DoubleVec3PropertyWidgetQt, DoubleVec3Property>("Spherical");
    registerPropertyWidget<FloatVec3PropertyWidgetQt, FloatVec3Property>("SphericalSpinBox");
    registerPropertyWidget<DoubleVec3PropertyWidgetQt, DoubleVec3Property>("SphericalSpinBox");

    registerPropertyWidget<LightPropertyWidgetQt, FloatVec3Property>("LightPosition");

    // Register dialogs
    registerDialog<RawDataReaderDialogQt>("RawVolumeReader");
    registerDialog<InviwoFileDialog>("FileDialog");
}

QtWidgetsModule::~QtWidgetsModule() = default;

TFHelpWindow* QtWidgetsModule::getTFHelpWindow() const { return tfMenuHelper_->getWindow(); }

void QtWidgetsModule::showTFHelpWindow() const { tfMenuHelper_->showWindow(); }

}  // namespace inviwo
