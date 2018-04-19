# Change log  {#changelog}


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
Search: 
`#include <inviwo/qt/widgets/inviwoapplicationqt.h>`  
Replace: `#include <inviwo/qt/applicationbase/inviwoapplicationqt.h>`  
Furthermore, make your project depend on `InviwoQtApplicationBase` instead of `InviwoQtWidgets`

Search: `inviwo/qt/widgets/`  
Replace: `modules/qtwidgets/`  
##2016-11-08
The vector interpolation was removed from the Interpolation helper class. The reason for this is that having the function pointer as a parameter for the function made the function impossible to inline and hence much slower, the interpolation calls become about 50% faster when the argument was removed. 

## 2016-02-03
The ```setValueFrom*``` and ```getValueAs*``` in buffer-, layer- and volumeRAM has been renamed to ```getAs*``` and ```setFrom*``` Previously only get function would use normalization. Now there are instead an other set of functions ```getAsNormalized*``` and ```setFromNormalized*``` that will apply normalization. Where as neither ```setFrom*``` or ```getAs*``` will use any normalization, just plain casting.

##2015-12-03
* __GenetryTypes__ BufferType,BufferUsage, DrawType, and ConnectivityType have had their members renamed to pascal case. Use tools/refactoring/enumfixes.py to update your code.
* __Processor__ InviwoProcessorInfo macros has been removed. Use ProcessorInfo class. 
* __Processor__ Virtual initialize and deinitialize function has be removed. Use constructor.
* __Processor__ enable/disable evaluation function has been removed. It was rarely used and the new NetworkLock is easier to use.

## 2015-11-16
__Singeltons__ The factories are not singletons any longer. They are now owned by InviwoApplication, one can ask InviwoApplication for them. There is a script "tools/refactoring/factoryfixes.py" to update code. 

## 2015-11-04
__Serialization__ Removed the ivw prefix from all serialization classes, and related filenames.  Use "tools/refactoring/serializerename.py" to update you own code. Just modify the "path = [paths, to, code]" variable first.

## 2015-10-29
__Enum refactoring:__ 
* enum DataFromatEnums::Id -> enum class DataFormatId, camelcased 
* enum DataFromatEnums::NumericType, -> enum class NumericType, camelcased 
* enum ShadingFunctionEnum -> enum class ShadingFunctionKind, camelcased 
* enum UsageMode -> enum class UsageMode, camelcased 
* DrawMode camelcased 
* enum InteractionEventType-> enum class InteractionEventType, camelcased 
* enum GlVendor-> enum class GlVendor, camelcased 
* enum GLFormats::Normalization-> enum class GLFormats::Normalization, camelcased
* enum CLFormats::Normalization-> enum class CLFormats::Normalization, camelcased
* enum InvalidationLevel -> enum class InvalidationLevel, camelcased

The enums has also been made into a consistent camel case.
To simplify refactoring there is a script in tools/refactoring/enumfixes.py that one can run to update code. You only have to specify the relevant paths in the script first.

## 2015-10-28
Processors: Updated to use new structure of ProcessorInfo:
the macro:
    ```InviwoProcessorInfo();```
is should be replaced with:
```cpp
    virtual const ProcessorInfo getProcessorInfo() const override;
    static const ProcessorInfo processorInfo_;
```
in the header file.

In the cpp file the macros,
```ProcessorClassIdentifier(VolumeRaycaster, "org.inviwo.VolumeRaycaster");``` etc.
 for the static members are replaced with:
```cpp
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
"tools/refactoring/processorinfo.py".
You will need to edit some path information in it, but otherwise it should be automatic.

**Converting files to UTF-8**

If you need to convert your files to UTF-8 you can use notepad++ and the following python script (python plugin installer can be found at http://sourceforge.net/projects/npppythonscript/files/):
```py
import os;
import sys;
filePathSrc="C:\\inviwo-dev\\vistinct\\" # Path to the folder with files to convert
for root, dirs, files in os.walk(filePathSrc):
    for fn in files:
        if fn.endswith(".h") or fn.endswith(".cpp") or fn.endswith(".cl") or fn.endswith(".frag") or fn.endswith(".vert") or fn.endswith(".glsl") or fn.endswith(".geom"): # Specify type of the files
            notepad.open(root + "\\" + fn)
            notepad.runMenuCommand("Encoding", "Convert to UTF-8")
            notepad.save()
            notepad.close()
```
## 2015-10-27
__CodeState__ CodeState is now an enum class, i.e. CODE_STATE_STABLE -> CodeState::Stable etc

## 2015-10-07
__Modules__ The module registration has change a bit a module now has to take a  ```InviwoApplication*``` in the constructor, like:
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
There is also not any ```initialize()``` do ```deinitialize()``` function anymore, just use the constructor and destructor.

The MACROS for registering object in the module are now replaced by proper function. The most common one for processor now look like this:
```cpp
registerProcessor<Background>();
```
For other object refer to InviwoModule. 

Is is also generally encouraged to avoid using any singletons, especially during initialization but also in general. In the long run we are working on removing most of the.   

## 2015-10-01
* __Data__ ```Data``` and ```DataGroup" is now templated with respect to the ```DataRepresentation``` that they have.
* __Converters__ The converter are now typed with From and To templates.
* __Geometry Types__ all the enums CoordinatePlane, CartesianCoordinateAxis, BufferUsage, DrawType, ConnectivityType are now enum classes. 
* __Buffers__ The buffers have been updated. The old ```Buffer``` is now a abstract ```BufferBase``` and the old ```BufferPrecision<T>``` is now ```Buffer<T>``` this is the class you should use. The BufferType member/template argument has been removed from both ```Buffer``` and ```BufferRepresentation``` and is now handled by the ```Mesh```. Because of this most typedef for ```Buffer``` and ```BufferRAM``` has now been removed. To create a ```BufferRAM``` you would now to this: ```auto repr = std::make_shared<BufferRAMPrecision<vec3>>()``` and then add it to a ```Buffer``` with ```auto buffer = std::make_shared<Buffer<vec3>>(repr);```
* __Multiple context__ there is now basic support for using OpenGL from multiple thread. Call ```RenderContext::getPtr()->activateLocalRenderContext();``` before using OpenGL.
* __Data Readers__ The Data Reader base class has been clean up. Now there is only one function to implement: ```virtual std::shared_ptr<T> readData(const std::string filePath) = 0;```
* __DiskRepresentationLoader__ there is a new class DiskRepresentationLoader to handle the old ```DataReader::readData```, and ```DataReader::readDataInto``` with a cleaner interface without any ```void*```. and a ```RawVolumeRAMLoader``` class that is used in by the "dat", "ivf", "raw" readers. 

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
* __Ports__ now uses ```std::shared_ptr<const T>``` for everything. I.e. ```DataInport::getData()``` now returns a ```std::shared_ptr<const T>``` and ```DataOutport::setData(std::shared_ptr<const T> data)``` also takes a shared ptr. Notably the argument to the ```DataOutport::setData``` is now a ```std::shared_ptr<__const__ T>``` this means that you can now do ```outport.setData(inport.getData());``` But on the other hand will you not be able to do:
```outport.getData()->getEditableRepresentation<T>()``` since the data is now const. To solve this it is recommended to keep a copy of the ```std::shared_ptr<T>``` around in the processor instead. 
One should take special care when changing data that has already been added to a outport since a different processor might be using that data in a background thread. One might for example use ```std::shared_ptr<T>::unique()``` to check whether there is someone else holding the data.

* __Image port__ has special overloads for non-const data since they might need to resize the data during resize events. Hence ```ImageOutport::setData(std::shared_ptr<T> data)``` and ```std::shared_ptr<T> ImageOutport::getEditableData()``` exists.

* __utilgl__ All the opengl utility functions in shaderutils, textureutils, etc, now uses reference arguments instead of points. 
Notably the: ``` void setShaderUniforms(Shader& shader, ...) ```
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
