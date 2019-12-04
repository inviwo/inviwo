Here we document changes that affect the public API or changes that needs to be communicated to other developers. 

## 2019-12-02 PoolProcessor
Added a processor base class to make background processing easier. Here is a basic example of how the process function might look like:
```c++
const auto calc = [image = inport_.getData()](pool::Stop stop, pool::Progress progress) 
    -> std::shared_ptr<const Image> {
    if (stop) return nullptr;
    auto newImage = std::shared_ptr<Image>(image->clone());
    progress(0.5f);
    // Do some work with the image
    return newImage;
};
dispatchOne(calc, [this](std::shared_ptr<const Image> result) {
    outport_.setData(result);
    newResults();  // Let the network know that the processor has new results on the outport.
});
```
The `PoolProcessor` automatically manages the Activity Indicator and the Progress Bar. It handles stopping old jobs and it manages the lifetimes of the jobs and the processor. The Example module has an `ExampleProgressBar` processor with example code. And the `PoolProcessor` has examples in its documentation as well.

## 2019-11-05 Mesh Plane Clipping
The mesh clipping processor can now handle most mesh types.

## 2019-10-02 DataFrameQt
A new sink processor that displays a spreadsheet view of a data frame, with selection support. 

## 2019-09-16 Auto Save
Inviwo now has a auto save feature that lets you restore a workspace on restart even if it was never saved.

## 2019-09-02 New Inviwo Version 0.9.11
Major change since the last release include:

* We have updated a number of build requirements:
    * A compiler supporting C++17 (We have built Inviwo with VS 2017, Clang 7, GCC 8, and XCode 10)
    * CMake version >= 3.12
    * Qt version >= 5.12
* New transfer function transforms: flip positions, interpolate alpha, equalize alpha (#618)
* Better import of transfer functions from images (#626)
* Source processors now have an option to see and explicitly set the data reader used. (#635)
* We now group add the module targets in a folder per external modules directory. The folder name can be customized by adding a ` meta.cmake` file to the external modules directory with `set(group_name <name>)` otherwise the folder name is used. (#637)
* Image port resize refactoring. The handling of image resize event is now more robust. (#645, #658)
* The Volume '.dat' reader now supports a ByteOffset option. 
* The Camera now has a set off buttons to easily fit data into the view. The buttons are also available via the canvas context menu. (#656)
* The Welcome widget got a search feature (#654, #662)
* Jenkins got better at tracking warning and format issues.
* Lots of fixes for static builds (#627, #631, #633)
* Observables no longer makes its observers also observe its clones (#643)
* PropertyOnwer will let all its observers know about the property removal on destruction (#632)

## 2019-08-20 Parallel coordinates plot margins
Removed autoMargins property and replaced it with includeLabels making it possible to specify margins with labels. 

## 2019-08-16 Property Assignment
We decided to remove the assignment operator from Property and related classes. The reason being that it is hard to understand what is does and hard to implement. Hence if you have implemented your own properties in your code you will have to remove the assignment operators from them. And given that we couldn't find any uses of it we decided to remove it. If you want to assign the "value" of a property to an other property the Property::set(const Property* src) function should be used.


## 2019-08-05 New version requirements
We now require a C++17 compatible compiler, and CMake version of at least 3.12 

## 2019-07-16 Transfer Function Editing
Added some utility functions for editing transfer functions: 

 * **flip positions** - swap the positions of all TF primitives with respect to their range
 * **interpolate alpha** - interpolates the alpha values of all TF primitives in between the left-most and right-most TF primitive
 * **equalize alpha** - averages the alpha value of all selected TF primitives

These functions are accessible under `Transform` in the context menus of both TF editor and respective TF properties. All functions are applied to the current selection or the entire TF, if nothing is selected.

## 2019-07-05 Plotting Improvements
We added a `AxisStyleProperty` for simplifying the setup of multiple axes in plots. Multiple axis properties can be registered with this new style property (`AxisStyleProperty::registerProperty()`). Changes to any of the style properties (line and text color, font size, tick length, etc.) are propagated to all registered axes, i.e. all axis share the same style. Note: this modifications can be overwritten in the individual axis.

There is now also an `Image Plot Processor` which allows to plot a 2D image with matching x-y axes. Interactions are forwarded to the image port which allows, e.g., to browse through volume slices. See image plot example in `PlottingGL`.

## 2019-07-04 New Inviwo version 0.9.10
Major change since the last release include:

 * Embedded web browser support
 * Get Started widget
 * Python processors
 * New module structure
 * New meta tool for adding new modules/processors

Moreover this will be that last version to not require c++17.

## 2019-06-11 Webbrowser property synchronization
Changed way of synchronizing/setting properties in javascript. Instead of adding properties to the webbrowser processor one can now set them using their path from javascript.
See web_property_sync.html in the Webbrowser module

This means that you need to update workspaces/webpages which used the Webbrowser processor.
```js
// Update html inputs when corresponding Inviwo properties change
inviwo.subscribe("ordinalProperty", "MeshCreator.torusRadius2_");
var slider = document.getElementById("ordinalProperty");
var ordinalPropertyValue = document.getElementById("ordinalPropertyValue");

slider.oninput = function() {
    inviwo.setProperty("MeshCreator.torusRadius2_", {value: Number(this.value)});
    ordinalPropertyValue.innerHTML = this.value;
}
```

## 2019-04-30 Python Processors
Inviwo Processors can now be implemented directly in Python by creating a python class and deriving from inviwopy.Processor.
Bellow follows an example of a python processor:
```py
# Name: PythonExample 

import inviwopy as ivw

class PythonExample(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.VolumeInport("inport")
        self.addInport(self.inport)
        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport)

        self.slider = ivw.properties.IntProperty("slider", "slider", 0, 0, 100, 1)
        self.addProperty(self.slider)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
            classIdentifier = "org.inviwo.PythonExample",
            displayName = "Python Example", 
            category = "Python",
            codeState = ivw.CodeState.Stable,
            tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return PythonExample.processorInfo()

    def initializeResources(self):
        print("init")

    def process(self):
        print("process: ", self.slider.value)
        self.outport.setData(self.inport.getData())

```
The initial '# Name:' comment is needed for Inviwo to know what python class that is should look for.
To register an Inviwo python processor one can put in in the <user setttings folder>/python_processor or by adding a `PythonProcessorFolderObserver` to an Inviwo module and have that observe a folder.


## 2019-04-30 Python Applications
It is now possible to run inviwo directly from python using a new inviwopyapp python package found in `apps/inviwopyapp`.
An example of running inviwo can be found in `apps/inviwopyapp/inviwo.py`. 
```py
import inviwopy as ivw
import inviwopyapp as qt

if __name__ == '__main__':
    # Inviwo requires that a logcentral is created.
    lc = ivw.LogCentral()
    
    # Create and register a console logger
    cl = ivw.ConsoleLogger()
    lc.registerLogger(cl)

    # Create the inviwo application
    app = qt.InviwoApplicationQt()
    app.registerModules()

    # load a workspace
    app.network.load(app.getPath(ivw.PathType.Workspaces) + "/boron.inv")

    # Make sure the app is ready
    app.update()
    app.waitForPool()
    app.update()
    # Save a snapshot
    app.network.Canvas.snapshot("snapshot.png") 

    # run the app event loop
    app.run()
```

## 2019-04-26 Parallel Coordinate Plot Update: flipped axes
Axes of the parallel coordinate plot can now be inverted via a double click with the left mouse button or using the corresponding `Invert Range` property of the axis. In addition, the filtering handles can be hidden if necessary (`Axes Settings` → `Handles Visible`).

## 2019-04-16 Moved DataFrame data structure from Plotting to new module (DataFrame)
Types of breaking changes:
```c++
#include <modules/plotting/datastructures/dataframe.h>
#include <modules/plotting/datastructures/dataframeutil.h>
#include <modules/plotting/datastructures/column.h>

plot::DataFrame
plot::Column
```

which need to be changed to 
```c++
#include <inviwo/dataframe/datastructures/dataframe.h>
#include <inviwo/dataframe/datastructures/dataframeutil.h>
#include <inviwo/dataframe/datastructures/column.h>
// Namespace plot removed
DataFrame
Column
```

Also added a reader for JSON-files which outputs a DataFrame.

## 2019-03-06 
New processor `Geometry Entry Exit Points` generates entry point and exit point images from any closed mesh to be used in raycasting. The positions of the input mesh are directly mapped to texture coordinates of a volume. This enables volume rendering within arbitrary bounding geometry.

## 2019-02-24 Unified line rendering
Line renderer vertex shader outputs `flat out uint pickID_;` and geometry shader takes `flat in uint pickID_[];` as input.
Vertex shaders depending on linerenderer.geom must write the pickID_ instead of the picking color.

Geometry shaders depending on previous linerenderer.vert should add the following:
`#include "utils/pickingutils.glsl"`

and modify the output to for example:
`pickColor_ = vec4(pickingIndexToColor(pickID_[0]), pickID_[0] == 0 ? 0.0 : 1.0);`

## 2019-01-17 Get Started and Workspace Annotations
Get Started screen provides an overview over recently used workspaces and available examples next to the latest changes.

Inviwo workspaces now also feature annotations like title, author, tags, and a description stored along with the network. The annotation widget allows to edit the annotations in the Qt application (accessible via the "View" menu). A default author for new workspaces can be specified in the System Settings of Inviwo.

## 2018-12-18 Transfer Function Import/Export
Extended context menu of transfer function properties. It is now possible to import and export TFs directly in the property widget. Transfer functions located in `data/transferfunctions/` are accessible as TF presets in the context menu of both TF editor and TF property.

## 2018-11-22
Better handling of linking in port inspectors. Show auto links when dragging in processors, disable auto links by pressing alt. Pressing shift while dragging when dragging in processors enables auto connection for inports.

## 2018-11-19
Converted all Inviwo core modules to use the new structure with include and src folders. 

## 2018-11-19
Added a `--updateModule` option to inviwo-meta-cli.exe it will update a module to use include and src folders.
Move all `.h` file into the `include/<org>/<module>` sub folder and all `.cpp` into the `src ` folder. 
except for files under `/ext`, `/tests`, or paths excluded be the given filters.

## 2018-11-14
Added an option to control if a module should be on by default, and remove the old global setting.
To enable the module by default add the following to the module `depends.cmake` file:
```c++
set(EnableByDefault ON)
```

## 2018-11-14
A new inviwo-meta library and an inviwo-meta-cli commandline app has been added to supersede the `make-new-module.py` and `make-new-file.py` python scripts. The library is also exposed in the tools menu of inviwo. Note that the inviwo-meta library relies on C++17 features and, thus, requires a recent compiler.

## 2018-11-14
Generated files are now stored in the corresponding `CMAKE_CURRENT_BINAY_DIR` for the subdirectory in question.
For a module this means `{build folder}/modules/{module name}`. `CMAKE_CURRENT_BINAY_DIR/include` path is added as an include path for each module. Hence, the generated headers are put in 
```
{build folder}/modules/{module name}/include/{organization}/{module name}/
```
Same is true for the generated headers of inviwo core, like `moduleregistration.h`. They are now placed in:
```
{build folder}/modules/core/include/inviwo/core/
```
Which means that for the module loading in apps
```c++
#include <moduleregistration.h>
```

needs to be changed to 
```c++
#include <inviwo/core/moduleregistration.h>
```

## 2018-11-14
New Module structure. We have introduced a new module structure where we separate headers and source files in the same way as it was already done for the core part of inviwo. Hence, module headers should now be placed under the include folder like:
```
    .../{module name}/include/{organization}/{module name}/
```
and sources goes in the source folder:
```
    .../{module name}/src/
```
`{module name}` it the lower case name of the module, `{organization}` default to inviwo but can be user-specified. 
The headers can then be included using 
```c++
#include <{organization}/{module name}/header.h>
```
The implementation is backwards compatible so old modules can continue to exist, but the structure is considered deprecated.
The main reasons for the change are to make packaging of headers easier and to prevent accidentally including headers from modules without depending on them.

## 2018-11-05
The SplitImage processor now features a draggable handle. This handle allows to adjust the split position in the canvas with either mouse or touch.

## 2018-10-04 Color Scale Legend
Added a Color Scale Legend processor to the plottinggl module that draws a 1D transfer function and the corresponding value axis on top of an image.

## 2018-09-27 Full screen on startup
Core: Renamed key mapping to fullscreenEvent and instead use fullscreen for a bool property.
You will need to change the key mapping again if you have changed it from shift + f.

Python: Full screen is now exposed as a property in the canvas instead of a function.

## 2018-09-26 Initial DiscreteData
Adding the DiscreteData module. The idea is to handle datasets that support all kinds of grids, data representations and interpolations on them.
Current status: Dataset; Explicit and implicit channel; Structured and periodic structured grid.

## 2018-09-24 Color property improvements
Updated the color property widget which allows to edit colors directly. Supports floating point range `[0,1]`, int range `[0,255]`, and hex color codes (`#RGB`, `#RGBA`, `#RRGGBB`, `#RRGGBBAA`).
Invalid input is indicated by red border and changes discarded if either `<Esc>` is pressed or the widget looses focus.


## 2018-09-24 Spinbox semantics for ordinal properties
New property semantics for ordinal properties: `SpinBox`

This widget allows to adjust the value by dragging the arrow up/down indicator with the mouse. The rate of change per sec is shown in a tooltip. Alternatively, use the mouse wheel to adjust the value.


## 2018-09-21
Settings are no longer shared between executables. I.e. The Inviwo app and the integration test will not use the same settings any more. We now prefix the settings with the InviwoApplication display name. This also implies that any existing Inviwo app settings will be lost. To keep old setting one can prefix all the ".ivs" file in the inviwo settings folder with "Inviwo_".  On windows the inviwo settings can be found in `%APPDATA%/inviwo`.

Added System settings for breaking into the debugger on various log message levels, and on throwing exceptions. Also added an option to add stacktraces to exceptions. All to help with debugging. 

The inviwo app will now catch uncaught inviwo exceptions in main and present an dialog with information and an option to continue or abort. It will also give an option to save your workspace before closing.

InviwoApplicationQt now has the same order of constructor arguments as InviwoApplication.

## 2018-08-21
The property class identifier system no longer uses the `InviwoPropertyInfo` / `PropertyClassIdentifier` macros but rather implements
```c++
virtual std::string getClassIdentifier() const override
```
The static class identifier 
```c++
static const std::string CLASS_IDENTIFIER
```
can still be added manually, but the preferred way is to either use
```c++
static const std::string classIdentifier
```
or specialize the `PropertyTraits` like:
```c++
template <>
struct PropertyTraits<MyProperty> {
    static std::string classIdentifier() {
        return "org.somename.myproperty";
    }
};
```
To access a class identifier of a property type statically, the `PropertyTraits` class should be used  
```c++
    PropertyTraits<MyProperty>::classIdentifier()
```
instead of accessing the `CLASS_IDENTIFIER` directly.

An enum traits class has been added to help working with enums and serialization, especially in the case of OptionProperties. 
For example given an enum:
```c++
enum class MyEnum { a, b };
```
EnumTraits can be specialized to provided a name for `MyEnum`, i.e.
```c++
template <>
struct EnumTraits<MyEnum> {
    static std::string name() {return "MyEnum"; }
};
```
This name will then be used by the TemplateOptionProperty in its class identifier. 
```c++
TemplateOptionProperty<MyEnum> prop("test","test");    
prop.getClassIdentifier() == PropertyTraits<TemplateOptionProperty<MyEnum>>::classIdentifier == "org.inviwo.OptionPropertyMyEnum"
```
This makes it possible to differentiate `MyEnum` from other enum TemplateOptionPropertys.

## 2018-09-19 Web browser
Use html5 web pages inside of Inviwo. Uses Chromium Embedded Framework (CEF) to render web pages off-screen. The rendered web page is transferred to an Inviwo Image.
Inviwo properties can be synchronized using javascript, see the web browser module example.
 
## 2018-07-26 ListProperty

Added `ListProperty`, a new property for dynamically adding and removing properties.
A ListProperty holds a number of "prefab" objects, i.e. unique_ptr to properties, which are used to instantiate new list entries. The property widget features small "x" buttons for removing individual elements (if removal is enabled). Pressing the "+" button next to the property label adds new elements (if adding is enabled). In case multiple prefabs exist, a drop-down menu is shown when pressing the "+" button.
```c++
// using a single prefab object and at most 10 elements
ListProperty listProperty("myListProperty", "My ListProperty",
    std::make_unique<BoolProperty>("boolProp", "BoolProperty", true), 10);

// multiple prefab objects
ListProperty listProperty("myListProperty", "My List Property", 
    []() {
        std::vector<std::unique_ptr<Property>> v;
        v.emplace_back(std::make_unique<IntProperty>("template1", "Template 1", 5, 0, 10));
        v.emplace_back(std::make_unique<IntProperty>("template2", "Template 2", 2, 0, 99));
        return v;
    }());
```

This also works when using different types of properties as prefab objects:
```c++
ListProperty listProperty("myListProperty", "My List Property", 
    []() {
        std::vector<std::unique_ptr<Property>> v;
        v.emplace_back(std::make_unique<BoolProperty>("boolProperty1", "Boolean Flag", true));
        v.emplace_back(std::make_unique<TransferFunctionProperty>("customTF1", "Transfer Function"));
        v.emplace_back(std::make_unique<IntProperty>("template1", "Template 1", 5, 0, 10));
        return v;
    }());
```
Prefabs can be added later on as well using `ListProperty::addPrefab(std::unique_ptr<Property>&& p)`.

## 2018-07-25 Integral Line Tracing updates
Before this change we had two Tracers, one for streamlines and one for pathlines, both in three spatial dimensions only.
These two classes were very similar and have now been merged/rewritten into a single templated class supporting both 
streamline and pathline tracing and is no longer limited to three dimensions. 
A new processor has been added which uses this tracer class to integrate stream/pathlines. The old processors, i.e. 
`StreamLines`, `PathLines` and `StreamRibbons` are now deprecated and were renamed to `[Name]Deprecated` for backwards compatibility.
The processors that are recommended to use are `StreamLines3D`, `PathLines3D` and `StreamLines2D`, see example workspaces to 
see how they are used.  


## 2018-06-28 GLM Version Update
GLM was updated to the new 0.9.9.0 version. Major changes are listed at https://github.com/g-truc/glm/releases/tag/0.9.9.0
Notable changes include the include of the vector / matrix dimension as a template argument, so the main types are now
```c++
    template <glm::length_t L, typename T, glm::qualifier Q>
    glm::vec<L, T, Q> 
```
and
```c++
    template <glm::length_t C, glm::length_t R, typename T, glm::qualifier Q>
    glm::mat<C, R, T, Q>
```
Most code should continue working as before. Except for __default constructed values that now are left uninitialized__. Where as before vec where initialized to 0 and mat to identify. This change can __break__ user code we have seen.  

## 2018-05-02 Refactored Transfer Function

- Renamed `TransferFunctionDataPoint `to `TFPrimitive`
- Renamed `TransferFunctionDataPoint::getRGBA`. Instead use `TFPrimitive::getColor`
- Renamed `TransferFunctionDataPoint::getPos`. Instead use `TFPrimitive::getPosition`
- Renamed `TransferFunctionObserver`. Instead use `TFPrimitiveSetObserver`
- Renamed `TransferFunctionPointObserver`. Instead use `TFPrimitiveObserver`
- TF primitive position type changed from float to double
- Deprecated many Transfer function methods, see deprecation messages. 

## 2018-04-11 Deprecating Callback Registration of Member Functions

The interface of ports and properties offers callback registration using lambda expressions as well as member functions. It was decided to deprecate the usage of member functions in this context. Consider registering lambda expression instead.
The functions in question have been marked as deprecated (`[[deprecated]]`) and warnings should appear during compilation. Using pre-compiled headers on MSVC is known to suppress those, though.

This immediately affects `Inport`, `Outport`, and `Property` (in particular `onChange(T* object, void (T::*method)())`, `onInvalid()`, `removeOnChange()`, `removeOnInvalid()`.

We will remove the respective deprecated functions at some point in the future.

## 2018-01-15 Updates in UserInterfaceGL module

The UserInterfaceGL module has experienced some major updates. For one, sliders and range sliders are now available within `glui` and secondly touch interaction is now supported. This includes all glui elements (buttons, sliders, ...) as well as the widgets for camera manipulation (Camera Widget), widgets for volume cropping (Cropping Widget), and the presentation mode (Presentation Processor). The `Presentation Processor` can be used, e.g. during demonstrations, as it allows to show images in a PowerPoint fashion instead of its regular image input. 

So far, the following UI elements are supported by glui:

- [X] box layout (vertical and horizontal)
- [X] checkboxes
- [X] buttons and toolbuttons, which feature only an icon
- [X] sliders (vertical and horizontal)
- [X] range sliders (vertical and horizontal)

For all UI elements, matching property widgets exist, i.e. `glui::BoolPropertyWidget`, `glui::ButtonPropertyWidget`, `glui::IntPropertyWidget`, `glui::FloatPropertyWidget`, `glui::IntMinMaxPropertyWidget`, and `glui::FloatMinMaxPropertyWidget`. This allows to link the UI elements with the respective properties.

For examples and usage see `readme.md` in the UserInterfaceGL module and check out `modules/UserInterfaceGL/processors/GLUITestProcessor.h`. 

## 2018-01-12 Moved OpenEXR from ext to modules/cimg/ext
OpenEXR is only used by the CImg module. Root/ext folder is for core dependencies and libraries used by multiple modules.

## 2017-12-01 Composite Processors
We now support composite processor, i.e. a processor wrappig it own processors network. Composite processors can be created from a selection of processors in the networkeditor, or by adding it from the processor list. This makes reusing groups of processors and managing large networks easier.

## 2017-11-26 Use ZLIB::ZLIB as CMake target for zlib instead of inviwo::zlib1
We are now using the same target name for zlib as find_package(ZLIB) would give. 

## 2017-11-23 Drag&Drop of Inviwo Workspace files
Workspace files can now be dropped onto the Inviwo Qt Application which in turn will open the respective workspace.

## 2017-11-23 UTF-8 support
Internally, Inviwo relies on UTF-8 encoded strings, i.e. strings with multibyte encoding stored in a std::string. In order to access files which contain unicode characters in the file name, one should use `filesystem::fstream()`, `filesystem::ifstream()`, and `filesystem::ofstream()`. These functions create and return a `std::*stream` object for the given file name. 

The call `auto f = filesystem::fstream(filename, mode);` is functionally equivalent to the statement `std::fstream f(filename, mode);`. No checks whether the file exists or was successfully opened are performed. That is, the caller has to check it. For more details check the documentation of `std::fstream`.

This is necessary since the Visual Studio compiler does not have support for handling UTF-8 encoded strings in `std::*stream(filename)`. See also comments on `filesystem::*stream()` in `core/util/filesystem.h`.

## 2017-11-21 Dependency fixes
InviwoCore is now registered together with the rest of the modules, moved SystemSettings and SystemCapabilities from InviwoCore to InviwoApplication, since the app depends on them. And there was no reason the had to be owned by InviwoCore.

Added convenience getSystemSettings and getSystemCapabilities to InviwoApplication. Also added getModuleCapabilities and getCapabilitesByType to InviwoApplication, similarly to settings.

## 2017-11-16 Processor Display Name
Removed the static usedIdentifiers from Processor, the network now checks that the id is unique or increments it instead. The processor displayname is now used as the main userfaceing name instead of identifier, it does not have any of the format limitations of the identifier and does not need to be unique. Made the displayName is now user configurable.

## 2017-11-09 Lightsource refactoring, LightSourceType is now an enum class and uses lower case.
- Replace `LightSourceType::Enum` with `LightSourceType` 
- Replace `LIGHT_AREA`, `LIGHT_CONE`, `LIGHT_POINT`, `LIGHT_DIRECTIONAL` with `area`, `cone`, `point`, `directional`

## 2017-11-09 Port traits refactoring
The old port_traits was previously used to acquire information, mainly class identifiers, about data objects used in ports and ports them self. Here we have separated port_traits into PortTraits (only classIdentifier) and DataTraits (with classIdentifier, dataName, colorCode and info). Where DataTraits are only used for data objects and PortTraits only for ports. The naming has also been updated to match the inviwo style better.

Hence if you have your own port_traits specialization it has to be replaced by something like the following for a Port:
```c++
    #include <inviwo/core/ports/porttraits.h>

    template <typename T>
    struct PortTraits<MyPort<T>> {
        static std::string classIdentifier() {
            return generateMyPortClassIdentifier<T>();
        }
    };
```
And for a data object, i.e. something that you put in a port. 
```c++
    #include <inviwo/core/datastructures/datatraits.h>

    template <>
    struct DataTraits<MyDataType> {
        static std::string classIdentifier() {
            return "org.something.mydatatype";
        }
        static std::string dataName() {
            return "MyDataType";
        }
        static uvec3 colorCode() {
            return uvec3{55,66,77};
        }
        static Document info(const MyDataType& data) {
            Document doc;
            doc.append("p", data.someInfo());
            return doc;
        }
    };
```

Port registration also now gets the port classIdentifier via PortTraits, so no need to specify the class identifier that registering the port.

## 2017-11-07 Breaking changes: Static functioned moved from BasicMesh 
Before this change, BasicMesh had various static functions to create meshes. These methods have been moved to a new file and namespace. They are now located in `<modules/base/algorithm/meshutils.h>` and the namespace meshutil. 
Hence, where you have used methods like `BasicMesh::sphere(...)`: you have to change to use `meshutil::sphere(...)` and add include `#include <modules/base/algorithm/meshutils.h>`. You also have to add `InviwoBaseModule` to your modules `depends.cmake`

## 2017-10-22 New feature: Runtime module loading and module versioning
Modules can now be dynamically loaded when starting an Inviwo application instead if linking them into the application. This means that the application does not need to know about all modules at compile time. 
Enable `IVW_RUNTIME_MODULE_LOADING` in Cmake to use this feature.
You can choose to only load a subset of existing modules by modifying inviwo-enabled-modules.txt in the application binary directory. 

Modules can be automatically reloaded when changed while running inviwo by enabling runtime module reloading in the application settings. **This means that you can compile a module while running inviwo!** The workspace will automatically be serialized, modules will be reloaded, and then the workspace will be deserialized again.
### Module versioning
We automatically add a version for you and we have made it easy for you so you usually do not need to care about your module version:
- You only need to change your module version if you want to release a new version of your module in between Inviwo core versions.

We have also introduced a module version to ensure that modules are loading correctly. You only need to change the module version if your module changes in between inviwo releases since the module version is dependent on the inviwo application version. Change the module version by adding "IVW_MODULE_VERSION(1.0.0)", i.e. Major.Minor.Patch in your module CmakeLists.txt file. We follow semantic versioning: http://semver.org/

## 2017-10-01 Breaking changes for Processor::isReady, Processor::performEvaluationRequest
Before this change, we could not detect when the Processor::isReady status changed. We introduced a StateCoordinator to resolve this issue and improve network evaluation. Only processors that have a sink among its decedents will now be evaluated.

- Processor performEvaluationRequest has been removed, instead call `Processor::invalidate` to trigger a network evaluation. 
- If you happen to override ```Processor::isReady()```, that will no longer work. You instead have to set the updater for the ```isReady_``` StateCoordinator. Most likely, you will just need to move your isReady code to a functor and set it in the constructor of your processor:
```c++
    // (default isReady() behavior)
    isReady_.setUpdate([this]() { return allInportsAreReady(); });
```
- This also applies to `isSink_` and `isSource_` in a similar manner. To mark a processor as sink, use `isSink_.setUpdate([]() { return true; });`

Note that you can use `Inport::setOptional(true)` if you want your processor to process even though the port is not connected instead of changing isReady.

Ready status is now pushed from outport to inport to processor, making it an observable in ProcessorObserver.
As this is now a state that is pushed instead of pulled you should also call ```isReady_.update()``` when ever the outcome of the functor above might change. This does not apply to ports, e.g. `onConnect` and `onDisconnect`, since ports already call  `isReady_.update()` on connect and disconnect.

## 2016-11-11 - Moved the QtWidgets project into the modules folder.

Moved `InviwoApplicationQt` from `QtWidgets` into a new project InviwoQtApplicationBase.  
If a module was using `InviwoApplicationQt` to get the main window, for example:  
```cpp
    #include <inviwo/qt/qtwidgets/inviwoapplicationqt.h> 
    auto mainWindow = dynamic_cast<InviwoApplicationQt*>(InviwoApplication::getPtr())->getMainWindow();
``` 
This can be exchanged with:   
```cpp
    #include <modules/qtwidgets/inviwoqtutils.h>  
    auto mainWindow = utilqt::getApplicationMainWindow();
```

Perform search and replace to accommodate changes:  
Search:  `#include <inviwo/qt/widgets/inviwoapplicationqt.h>``
Replace: `#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>`  
Furthermore, make your project depend on `InviwoQtApplicationBase` instead of `InviwoQtWidgets`

Search: `inviwo/qt/widgets/`  
Replace: `modules/qtwidgets/`

##2016-11-08
The vector interpolation was removed from the Interpolation helper class. The reason for this is that having the function pointer as a parameter for the function made the function impossible to inline and hence much slower, the interpolation calls become about 50% faster when the argument was removed. 

## 2016-02-03
The `setValueFrom*` and `getValueAs*` in buffer-, layer- and volumeRAM has been renamed to `getAs*` and `setFrom*` Previously only get function would use normalization. Now there are instead an other set of functions `getAsNormalized*` and `setFromNormalized*` that will apply normalization. Where as neither `setFrom*` or `getAs*` will use any normalization, just plain casting.

##2015-12-03
* __Generic Types__ `BufferType`, `BufferUsage`, `DrawType`, and `ConnectivityType` have had their members renamed to pascal case. Use `tools/refactoring/enumfixes.py` to update your code.
* __Processor__ InviwoProcessorInfo macros has been removed. Use ProcessorInfo class. 
* __Processor__ Virtual initialize and deinitialize function has be removed. Use constructor.
* __Processor__ enable/disable evaluation function has been removed. It was rarely used and the new NetworkLock is easier to use.

## 2015-11-16
__Singeltons__ The factories are not singletons any longer. They are now owned by InviwoApplication, one can ask InviwoApplication for them. There is a script `tools/refactoring/factoryfixes.py` to update code. 

## 2015-11-04
__Serialization__ Removed the ivw prefix from all serialization classes, and related filenames.  Use `tools/refactoring/serializerename.py` to update you own code. Just modify the "path = [paths, to, code]" variable first.

## 2015-10-29
__Enum refactoring:__ 
* `enum DataFromatEnums::Id` -> `enum class DataFormatId`, camelcased 
* `enum DataFromatEnums::NumericType` -> `enum class NumericType`, camelcased 
* `enum ShadingFunctionEnum` -> `enum class ShadingFunctionKind`, camelcased 
* `enum UsageMode` -> `enum class UsageMode`, camelcased 
* `DrawMode` camelcased 
* `enum InteractionEventType` -> `enum class InteractionEventType`, camelcased 
* `enum GlVendor` -> `enum class GlVendor`, camelcased 
* `enum GLFormats::Normalization` -> `enum class GLFormats::Normalization`, camelcased
* `enum CLFormats::Normalization` -> `enum class CLFormats::Normalization`, camelcased
* `enum InvalidationLevel` -> `enum class InvalidationLevel`, camelcased

The enums has also been made into a consistent camel case.
To simplify refactoring there is a script in `tools/refactoring/enumfixes.py` that one can run to update code. You only have to specify the relevant paths in the script first.

## 2015-10-28
Processors: Updated to use new structure of ProcessorInfo. The macro
```c++
    InviwoProcessorInfo();
```
should be replaced with:
```c++
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
```
in the header file.

In the cpp file the macros, `ProcessorClassIdentifier(VolumeRaycaster, "org.inviwo.VolumeRaycaster");` etc. for the static members are replaced with:
```c++
    const ProcessorInfo VolumeRaycaster::processorInfo_{
        "org.inviwo.VolumeRaycaster",  // Class identifer
        "Volume Raycaster",            // Display name
        "Volume Rendering",            // Category
        CodeState::Stable,             // Code state
        Tags::GL                       // Tags
    };
    const ProcessorInfo VolumeRaycaster::getProcessorInfo() const {
        return processorInfo_;
    }
```
The old macros should still work, but will be deprecated before next release.

The name of the static member ```processorInfo_``` is important since that is what the ```ProcessorTraits``` looks for when it tries to find the information statically. If you want to have a different name or generate the information dynamically you can specialize the ```ProcessorTraits``` for your processor. Here is an example for the template processor BasisTransform:
```cpp
    template <>
    struct ProcessorTraits<BasisTransform<Mesh>> {
        static ProcessorInfo getProcessorInfo() {
            return {
                "org.inviwo.BasisTransformGeometry",  // Class identifier
                "Basis Transform Mesh",               // Display name
                "Coordinate Transforms",              // Category
                CodeState::Experimental,              // Code state
                Tags::CPU                             // Tags
            };
        }
    };
```

If you have many processor that needs updating there is a utility script 
`tools/refactoring/processorinfo.py`.
You will need to edit some path information in it, but otherwise it should be automatic.

**Converting files to UTF-8**

If you need to convert your files to UTF-8 you can use notepad++ and the following python script (python plugin installer can be found at http://sourceforge.net/projects/npppythonscript/files/):
```py
    import os;
    import sys;
    filePathSrc="C:\\inviwo\\vistinct\\" # Path to the folder with files to convert
    for root, dirs, files in os.walk(filePathSrc):
        for fn in files:
            if fn.endswith(".h") or fn.endswith(".cpp") or fn.endswith(".cl") or fn.endswith(".frag") or fn.endswith(".vert") or fn.endswith(".glsl") or fn.endswith(".geom"): # Specify type of the files
                notepad.open(root + "\\" + fn)
                notepad.runMenuCommand("Encoding", "Convert to UTF-8")
                notepad.save()
                notepad.close()
```

## 2015-10-27
__CodeState__ CodeState is now an enum class, i.e. `CODE_STATE_STABLE` -> `CodeState::Stable` etc

## 2015-10-07
__Modules__ The module registration has change a bit a module now has to take a  `InviwoApplication*` in the constructor, like:
```cpp 
    class IVW_MODULE_BASEGL_API BaseGLModule : public InviwoModule {
    public:
        BaseGLModule(InviwoApplication* app);
    };
```
And then pass that on to the base class together with the module name
```cpp
    BaseGLModule::BaseGLModule(InviwoApplication* app) : InviwoModule(app, "BaseGL") {
```
There is also not any `initialize()` do `deinitialize()` function anymore, just use the constructor and destructor.

The MACROS for registering object in the module are now replaced by proper function. The most common one for processor now look like this:
```cpp
    registerProcessor<Background>();
```
For other object refer to InviwoModule. 

Is is also generally encouraged to avoid using any singletons, especially during initialization but also in general. In the long run we are working on removing most of the.   

## 2015-10-01
* __Data__ `Data` and `DataGroup" is now templated with respect to the `DataRepresentation` that they have.
* __Converters__ The converter are now typed with From and To templates.
* __Geometry Types__ all the enums CoordinatePlane, CartesianCoordinateAxis, BufferUsage, DrawType, ConnectivityType are now enum classes. 
* __Buffers__ The buffers have been updated. The old `Buffer` is now a abstract `BufferBase` and the old `BufferPrecision<T>` is now `Buffer<T>` this is the class you should use. The BufferType member/template argument has been removed from both `Buffer` and `BufferRepresentation` and is now handled by the `Mesh`. Because of this most typedef for `Buffer` and `BufferRAM` has now been removed. To create a `BufferRAM` you would now to this: `auto repr = std::make_shared<BufferRAMPrecision<vec3>>()` and then add it to a `Buffer` with `auto buffer = std::make_shared<Buffer<vec3>>(repr);`
* __Multiple context__ there is now basic support for using OpenGL from multiple thread. Call `RenderContext::getPtr()->activateLocalRenderContext();` before using OpenGL.
* __Data Readers__ The Data Reader base class has been clean up. Now there is only one function to implement: `virtual std::shared_ptr<T> readData(const std::string filePath) = 0;`
* __DiskRepresentationLoader__ there is a new class DiskRepresentationLoader to handle the old `DataReader::readData`, and `DataReader::readDataInto` with a cleaner interface without any `void*`. and a `RawVolumeRAMLoader` class that is used in by the "dat", "ivf", "raw" readers. 

## 2015-10-01
Updates to Data:
   * Using type_index to refer to different representations
   * Data stores representations using shared_ptr
   * Reworked converter factory. Now automatically creates all converter packages.
   * Data always uses a converter package of size 1 or more. Simplifies code paths...
   * Data now has a mutex to protect mutable representations.
   * Each representations now have a isValid flag, that is updated by Data
   * The old bitflags in data are now gone.
   * Representations should now implement a getTypeIndex() method on the type erasure level above DataFormat, I.e. VolumeRAM VolumeGL, not any of the precision classes.
   * All representations should now be created as shared_ptr and Data::addRepresentation now only accepts a shared_ptr.


## 2015-09-10
* __Ports__ now uses `std::shared_ptr<const T>` for everything. I.e. `DataInport::getData()` now returns a `std::shared_ptr<const T>` and `DataOutport::setData(std::shared_ptr<const T> data)` also takes a shared ptr. Notably the argument to the `DataOutport::setData` is now a `std::shared_ptr<__const__ T>` this means that you can now do `outport.setData(inport.getData());` But on the other hand will you not be able to do:
`outport.getData()->getEditableRepresentation<T>()` since the data is now const. To solve this it is recommended to keep a copy of the `std::shared_ptr<T>` around in the processor instead. 
One should take special care when changing data that has already been added to a outport since a different processor might be using that data in a background thread. One might for example use `std::shared_ptr<T>::unique()` to check whether there is someone else holding the data.

* __Image port__ has special overloads for non-const data since they might need to resize the data during resize events. Hence `ImageOutport::setData(std::shared_ptr<T> data)` and `std::shared_ptr<T> ImageOutport::getEditableData()` exists.

* __utilgl__ All the opengl utility functions in shaderutils, textureutils, etc, now uses reference arguments instead of points. 
Notably the: ` void setShaderUniforms(Shader& shader, ...) `
functions now take the shader by reference not pointer. 

## 2015-09-09
* __DataSequence__ has been removed in favor of outputting the a actual vector of data. VolumeSource still supports time series (it keeps a vector internally) If you want access to the whole data set you can use the new VolumeVectorSource.
* __VectorData__ has also been removed in favor a just using a std::vector of data.

## 2015-08-25
* Paths, in general paths should not contain trailing slashes, InviwoApplication::getPath is change to reflect that behavior.

## 2015-07-16
* __Shaders__ A shader now has a `onReload(std::function<void()>)` callback that processors that want to be reloaded on shader reload has to use. So now only affected processors will be invalidated on shader modifications. To get the same behavior as before this needs to be added to an processor using shaders:
```cpp
    shader_.onReload([this]() { invalidate(INVALID_RESOURCES); });
```

## 2015-07-15
* __Timers__ The `InviwoApplication::createTimer()` factory has been removed in favor of a pure c++11 timer. Hence there is no need for a factory. The new class is very similar and can be found in `inviwo/core/util/timer.h`

## 2015-07-14
* __Observers__ The observers now use a unordered_set and the "observers_" member is now typed correctly hence there is no need to static cast. And since unordered_set does not have reverse iterators, if you use that you need to update. The current preferred use looks like this: 
```cpp
    void PropertyObservable::notifyObserversOnSetVisible(bool visible) const {
        for (auto o : observers_) o->onSetVisible(visible);
    }
```

* __Activity Indicator__ There is now activity indicator for processor similar to a progress bar. it will only show up as a yellow status indicator on the processor. It can either be active or not. See volumesubsample for an example.


## 2015-07-09
* __Threading__ For handling background work there are now two global functions 
```cpp
    template <class F, class... Args>
    auto dispatchFront(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>
    template <class F, class... Args>
    auto dispatchPool(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> 
```
`dispatchFront` is used to submit a task to the Front thread, i.e. the thread that does handles the GUI and so far all the evaluation. Everything that can have side effects should be run in this thread, changing properties, updating the GUI, triggering evaluation etc. 

`dispatchPool` is used to submit tasks to the thread pool. The size of the thread pool will by default be half of the number of cores and can be change is the system settings. It the pool size is 0 all submitted tasks will be run immediately. This is used in the regression to make sure everything is finished before we close inviwo. At this point we can not run OpenGL operations in the thread pool since we don't have any routines for how to handle the contexts.

The return value of both `dispatchFront` and `dispatchPool` is a future<...> of the return value of the task given. Apart for getting the result of the task this future can be used to check whether the task has finished or is still running. Here is a small example:
```cpp
    std::future<std::unique_ptr<Volume>> result_;
```
result here is a future holding the resulting volume from a subsampling. The following code submits the subsampling task to the thread pool and extracts the result from the future when the calculation is done.
```cpp
    void VolumeSubsample::process() { 
        if (result_.valid() &&
            result_.wait_for(std::chrono::duration<int, std::milli>(0)) ==
                std::future_status::ready) {

            std::unique_ptr<Volume> volume = std::move(result_.get());
            outport_.setData(volume.release());

        } else if (!result_.valid()) {
            const Volume* data = inport_.getData();
            const VolumeRAM* vol = data->getRepresentation<VolumeRAM>();

            result_ = dispatchPool(
                [this](const VolumeRAM* v, VolumeRAMSubSample::Factor f) -> std::unique_ptr<Volume> {
                    auto volume = util::make_unique<Volume>(VolumeRAMSubSample::apply(v, f));
                    dispatchFront([this]() { invalidate(INVALID_OUTPUT); });
                    return volume;
                },
                vol, subSampleFactor_.get());
        }
    }
```
Here we start by checking if there is a valid future, that means that there is either a result available or the calculation is running. In the case when the result is ready we extract the result and set it to the outport. If it is still running we do nothing since we don't what to fill the pool with jobs. To check if a future is ready we use wait_for with a timeout of 0. In the case that the result is invalid we submit a new task the pool using `dispatchPool` and assign the result to `result_`. To make sure that we get back to the process function when the task is done we also add a nested task submit inside of the pool task `dispatchFront([this]() { invalidate(INVALID_OUTPUT); });` this will run when that task is finished submitting an invalidation on the Front thread casing a new evaluation of the processor. Where we then will find the result of the task.
