# Manual {#manual}

\tableofcontents

\htmlonly <style>div.image img{align:left;}</style> \endhtmlonly
## Introduction

Inviwo is an extendable C++ framework for easy prototyping of interactive applications. 
It provides a network editor for the designing of data flow networks, which are automatically
evaluated and executed to produce output on one or more output processors (typically a canvas).
The data flow in such a network runs from top to bottom, and the nodes are referred to as 
processors. Besides these processors two more first class objects exist. Ports, which are 
used to exchange data in between processors, and properties, which define the state of a processor.

\htmlonly <style>div.image img[src="inviwoapp_annotated_new.png"]{width:500px;}</style> \endhtmlonly
\image html inviwoapp_annotated_new.png 

## Network

Ownership hierarchy of Inviwo network components
```
                                                      
                   ┌─────────────┐                    
                   │             │                    
                   │   Network   │                    
                   │             │                    
                   └──────┬──────┘                    
        ┌─────────────────┼─────────────────┐         
 ┌──────▼──────┐   ┌──────▼──────┐   ┌──────▼──────┐  
 │             ├┐  │             ├┐  │             ├┐ 
 │    Link     │├┐ │  Processor  │├┐ │ Connection  │├┐
 │             │││ │             │││ │             │││
 └─┬────┬──────┘││ └─┬────┬──────┘││ └─┬────┬──────┘││
   └─┬──────────┘│   └─┬──┼───────┘│   └─┬──────────┘│
     └──┼────────┘     └──┼────────┘     └──┼────────┘
        └───┐   ┌─────────┴────────┐    ┌───┘         
          ┌─▼───▼───────┐   ┌──────▼────▼─┐           
          │             ├┐  │             ├┐          
        ┌─►  Property   │├┐ │    Port     │├┐         
        │ │             │││ │             │││         
        │ └─┬────┬──────┘││ └─┬───────────┘││         
        │   └─┬──┼───────┘│   └─┬──────────┘│         
        │     └──┼────────┘     └───────────┘         
        └────────┘                                    
```
Solid line represent ownership, dashed lines represents references.  



### Processor

\htmlonly <style>div.image img[src="processor_annotated.png"]{width:500px;}</style> \endhtmlonly
\image html processor_annotated.png 

Processors are the primary objects the user interacts with inside the network editor. 
Usually, they are dragged from the processor list on the left onto the network editor,
before they are connected. Processors receive input data and generate output data through 
ports, whereby the ports on the top boundary are referred to as inports, and the ports on 
the bottom boundary are referred to as outports. Each processor has a set of properties, 
which define its current state. Upon selection of a processor in the network editor, 
its properties are shown in the property list on the right, where they can be edited.

Processors can exchange information in two ways. First, they can exchange data through 
their ports, whereby equally colored ports are of same type and can thus exchange data. 
Port connections can be established by connecting two ports via drag-and-drop. Besides the 
ports, the properties of a processor can be linked in order to synchronize their values. 
Links can be established by connecting two processors via drag-and-drop while pressing 
the ALT key.

To distinguish processors in a network, they have unique identifiers which can be edited 
by the user. Initially the identifier will be equal to the type of the processor, which 
is shown in italic below the identifier. Furthermore, the processor shows whether it is 
correctly connected through the status light.

#### Property
#### Port

##### ImagePorts
Resizing options for image ports
```
                                                ImageOutport                              
                                          isHandlingResizeEvents()                        
                                                                                          
                              True (default)                          False               
                   ┌──────────────────────────────────┬──────────────────────────────────┐
                   │ Outport::Size = max(Inports      │ Outport::Size = Outport::size    │
                   │ requested sizes)                 │ (no resize of data)              │
                   │ (resize the data in the outport  │                                  │
            False  │ if needed)                       │                                  │
          (default)│                                  │                                  │
                   │ Inport::Size = Inport requested  │ Inport::Size = Inport requested  │
                   │ size                             │ size                             │
 ImageInport       │ (return a resized copy if        │ (return a resized copy if        │
                   │ needed)                          │ needed)                          │
  isOutport-       │                                  │                                  │
 Determining-      ├──────────────────────────────────┼──────────────────────────────────┤
    Size()         │ Outport::Size = max(all inports  │ Outport::Size = Outport::Size    │
                   │ requested sizes)                 │ (no resize of data)              │
                   │ (resize the data in the outport  │                                  │
             True  │ if needed)                       │                                  │
                   │                                  │                                  │
                   │ Inport::Size = Outport::size     │ Inport::Size = Outport::size     │
                   │ (no copy)                        │ (no copy)                        │
                   │                                  │                                  │
                   │                                  │                                  │
                   └──────────────────────────────────┴──────────────────────────────────┘
```

### Connections
### Links

## Data Structures
### Data and Representations
In Inviwo the main core data structures (Volume, Image, Layer, Mesh, Buffer) use a pattering of handles and representations. The Volume data structure for example has a handle class called [Volume](@ref inviwo::Volume). The handle class by it self does not have any data, it only stores metadata, and a list of representations. The representations is were the actually data is stored. A Volume can have a set of different representations, for example a VolumeRAM representation and a VolumeGL representation. At any time at least one of the representations should be in a valid state. When ever we want to access the data of a volume we will ask the volume for a representation of some kind, and the handle is then responsible to try and provide that to us. If the requested representation is valid the handle can just return the that representation. If not, it will have to find a valid representation and try to either create the representation we wanted from the valid representation, if there was no representation of the kind we asked for around. Or if there is a invalid representation around, update that representation with the valid representation. To the this the handle one or several [RepresentationConverters](@ref inviwo::RepresentationConverters).

A typical use case can be that we start with a Volume handle with a Disk representation and we want to do raycasting using OpenGL. In our processor we will then ask the Volume for a VolumeGL representation. The volume will see that there are now such representation currently. It will then try and find a chain of RepresentationConverters to create that representation. In this case that might be a [VolumeDisk2RAMConverter](@ref inviwo::VolumeDisk2RAMConverter) that will read in the file from disk into ram, and a  [VolumeRAM2GLConverter](@ref inviwo::VolumeRAM2GLConverter) that will that the data in ram and upload it to the graphics card. The data handle will always try and find the shortest chain of converters. I.e. if there was a VolumeDisk2GLConverter that one would have been used instead. 

When asking a data handle for a representation there are two different calls we can make [getRepresentation](@ref inviwo::Data::getRepresentation()) and [getEditableRepresentation](@ref inviwo::Data::getEditableRepresentation) if we call getRepresentation we will get a read only data representation back. I.e. we can not edit that data. If we want to edit the data we have to call getEditableRepresentation. The we are able to modify the representation. This call has the side effect of invalidation all other representations in the handle, since the will not have gotten the edits and will hence be out of sync. This means that we in the example about had called get getEditableRepresentation<VolumeGL> and then called getRepresentation<VolumeRAM> the data handle would have to download the data from the graphics card to ram to update the VolumeRAM Representation using a [VolumeRAM2GLConverter](@ref inviwo::VolumeRAM2GLConverter).

### Spaces and Transforms

| Space | Description             | Range                                       |
|:------|:------------------------|:--------------------------------------------|
| Data  | raw data numbers        | generally (-inf, inf), ([0,1] for textures) |
| Model | model space coordinates | (data min, data max)                        |
| World | world space coordinates | (-inf, inf)                                 |
| View  | view space coordinates  | (-inf, inf)                                 |
| Clip  | clip space coordinates  | [-1,1]                                      |
| Index | voxel index coordinates | [0, number of voxels)                       |

\htmlonly <style>div.image img[src="coordinate_spaces.png"]{width:500px;}</style> \endhtmlonly
\image html coordinate_spaces.png 


## Evaluation
### Invalidate
### Process

## Processor Developemnt
### Creation
#### Ports
#### Properties
### Registration
For a processor to be able to be used in inviwo it has to be registered in a module. This is done in the module constructor
```{.cpp}
    registerProcessor<VolumeRaycaster>();
```
this will register a [ProcessorFactoryObject](@ref inviwo::ProcessorFactoryObject) that is used to create the processor at runtime. 

### Picking
Picking of specific objects is supported in Inviwo by the means of rendering to an additional texture. Each pickable object is assigned a globally unique color. The pool of available colors is managed internally by the [PickingManager](@ref inviwo::PickingManager) and supports about 16 million colors (2^(8*3)-1, black is not used). A processor developer would not use the PickingManager directly rather they use a [PickingMapper](@ref inviwo::PickingMapper) and its [PickingObject](@ref inviwo::PickingObject) to get unique colors. 


In the following sections we will give a short guide on how to enable picking in your processor. For an example processor see [MeshPicking](https://github.com/inviwo/inviwo/blob/master/modules/basegl/processors/meshpicking.h) in BaseGL and the example workspace [cube_sphere_picking.inv](https://github.com/inviwo/inviwo/blob/master/data/workspaces/cube_sphere_picking.inv). 

\htmlonly 
<div id="img_picking">
<style>
  #img_picking div.image {float: left; overflow: auto;}
  div.image img[src="picking_colorlayer.png"]{width:450px;}
  div.image img[src="picking_pickinglayer.png"]{width:450px;}
</style> 
\endhtmlonly
\image html picking_colorlayer.png "Color Layer" 
\image html picking_pickinglayer.png "Picking Layer"
\htmlonly 
</div>
<div style="clear: both;"></div>
\endhtmlonly



**Classes involved in picking** 
* [PickingMapper](https://inviwo.github.io/inviwo/doc/classinviwo_1_1PickingMapper.html) : Scoped variable to handle registration and unregistration of a Picking Object. Contains a callback that will be called whenever a picking event for this object is triggered. 
* [PickingObject](https://inviwo.github.io/inviwo/doc/classinviwo_1_1PickingObject.html) : A class which support converting between ids and picking color. Each PickingMapper has one PickingObject. Picking Objects has a size defining how many ids it can handle (each id gets a globaly unique color)
* [PickingContainer](https://inviwo.github.io/inviwo/doc/classinviwo_1_1PickingContainer.html) : Internal Class
* [PickingManager](https://inviwo.github.io/inviwo/doc/classinviwo_1_1PickingManager.html)  : Internal Class matching events with its picking object and calls the correct callbacks. 


#### Constructiong a PickingMapper
A processor that wants to utilize picking should add PickingMapper member object to its class `PickingMapper pickingMapper_;`. The construction of the PickingMapper should be done in the constructors member initializer list and takes a pointer to a processor (this), a size and a callback function as parameters:
```
pickingMapper_(this, 1, [&](const PickingObject* p) { updateWidgetPositionFromPicking(p); })
```
The size represents the number of unique colors will be associated with this mapper and controls how many unique object you can pick. The callback function supplied will be called as for when a picking event occurs and it matches.

__Available picking events__
  * Mouse Press
  * Mouse Release 
  * Mouse Move (while pressed)
  * Mouse Hover (while not pressed)
  * Mouse Wheel 
  * Touch events

If the number of pickable objects is unknown upon creation, one can specify a low number (eg. 1) as initial size and then call the function `PickableMapper::resize(size_t newSize)` to set a new size. Note: when the mapper is resized it gets a new set of colors and old colors will stop working. 

To have more than one callback function a processor can consist of more than one PickingMapper (one per callback) 

#### Rendering
**The short story:** 
Add a uniform to your fragment shader and set it to the color you get from the PickingObject. 
Getting the color and setting the uniform in you processor:
```c++
auto color = pickingMapper_.getPickingObject()->getColor(id);
shader_.setUniform("pickingColor",color);
```
and use it in your shader
```glsl
uniform vec3 pickingColor;
...
int main(){
    ...
    FragData0 = color;
    PickingData = vec4(pickingColor,1.0)
}
```

## Modules
In Inviwo most functionality comes from modules. The core framework defines a collection of extension 
points where modules can add functionality. For example a module can supply [Processors](#processor), 
DataReaders, [Properties](#property), Ports, Widgets, etc. 

### Structure
The physical structure of a Inviwo module is as follows

```                                                                                                        
                                                                                                        
File / Folder                 Type   Description                                                        
                                                                                                        
└──▶examplemodule              M     <lowercase name>module                                             
    │                                                                                                   
    ├──▶CMakeLists.txt         M     CMake project definition                                           
    │                                                                                                   
    ├──▶examplemodule.h        M     <lowercase name>module.h for module registration                   
    │                                                                                                   
    ├──▶examplemodule.cpp      M     <lowercase name>module.cpp for module registration                 
    │                                                                                                   
    ├──▶examplemoduledefine.h  S     <lowercase name>moduledefine.h declspec defines                    
    │                                                                                                   
    ├──▶depends.cmake          P     List of dependencies to other modules / cmake packages             
    │                                                                                                   
    ├──▶readme.md              P     Description of the module, used by CMake             
    │                                                                                                   
    ├──▶data                   P     Folder for non code stuff                                          
    │   │                                                                                               
    │   ├──▶images             S     Image resources                                                    
    │   │                                                                                               
    │   ├──▶portinspectors     S     Port Inspectors                                                    
    │   │                                                                                               
    │   ├──▶scripts            S     Script resources                                                   
    │   │                                                                                               
    │   ├──▶volumes            S     Volume resources                                                   
    │   │                                                                                               
    │   └──▶workspaces         P     Workspaces, listed in File::Examples::ExampleModule                
    │                                                                                                   
    ├──▶docs                   P     Put documentation here.                                            
    │   │                                                                                               
    │   └──▶images             P     Put images that should show up in doxygen here                     
    │                                                                                                   
    ├──▶ext                    P     Put all external lib under ext. So they can easily be excluded     
    │   │                                                                                               
    │   ├──▶externallib1       S     External lib 1                                                     
    │   │                                                                                               
    │   └──▶externallib2       S     External lib 2                                                     
    │                                                                                                   
    ├──▶datastructures         S     Put data structures here                                           
    │                                                                                                   
    ├──▶processors             S     Put processors here                                                
    │                                                                                                   
    ├──▶properties             S     Put properties here                                                
    │                                                                                                   
    ├──▶glsl                   S     If using OpenGL, put shaders here                                  
    │                                                                                                   
    ├──▶cl                     S     If using OpenCL, put kernels here                                  
    │                                                                                                   
    └──▶tests                  P     Test related things               
        │                                                                                               
        ├──▶images             S     Image test resources                                               
        │                                                                                               
        ├──▶volumes            S     Volume resources                                                   
        │                                                                                               
        ├──▶unittests          S     Put unittests here
        │                                                                                               
        └──▶regression         P     Regression Test workspaces, listed in File::Test::ExampleModule.   
                                     Automatically Run in regression tests on jenkins                   
                                                                                                        
  Type   Explanation                                                                                    
───────────────────────────────────────────────────────────────────────────────────                     
    M    Mandatory, these are needed for the module to work.                                            
    P    Programmatically used, these are files and folders that the framework                          
         will look for or take advantage of in some way.                                                
    S    Suggested, these a just suggestions to keep modules looking                                    
```

A new module can easily be create by using [make-new-module](#module-creation) script.


## Testing

### Unittest

### Regression tests
> Regression testing is a type of software testing that verifies that software previously developed and tested still performs correctly after it was changed or interfaced with other software. - [Wikipedia](https://en.wikipedia.org/wiki/Regression_testing)

In our setting, each regression test generate a set of images which are compared to a reference image. The test will fail if the generated image differs from the reference image. 
The current regression tests running on the server can be found at [the Jenkins regression job](http://130.236.145.152:8080/job/Regression/Regression_Report/).

#### Running regression tests on your computer
The regression.py script in the tools folder it use to generate regression reports.
```
usage: regression.py [-h] -i INVIWO [-c CONFIG] [-o OUTPUT]
                     [-r [REPOS [REPOS ...]]] [-m [MODULES [MODULES ...]]]
                     [-s [SLICE]] [--include [INCLUDE]] [--exclude [EXCLUDE]]
                     [-l] [--imagetolerance IMAGETOLERANCE]

Run regression tests

optional arguments:
  -h, --help            show this help message and exit
  -i INVIWO, --inviwo INVIWO
                        Paths to inviwo executable (default: None)
  -c CONFIG, --config CONFIG
                        A configure file (default: )
  -o OUTPUT, --output OUTPUT
                        Path to output (default: None)
  -r [REPOS [REPOS ...]], --repos [REPOS [REPOS ...]]
                        Paths to inviwo repos (default: None)
  -m [MODULES [MODULES ...]], --modules [MODULES [MODULES ...]]
                        Paths to folders with modules (default: [])
  -s [SLICE], --slice [SLICE]
                        Specifiy a specific slice of tests to run (default: )
  --include [INCLUDE]   Include filter (default: )
  --exclude [EXCLUDE]   Exclude filter (default: )
  -l, --list            List all tests (default: False)
  --imagetolerance IMAGETOLERANCE
                        Tolerance when comparing images (default: 0.0)
```

The most basic usage is by running
```
python3 regression.py -i <path/to/inviwo.exe>
```
the script will the automatically try and find a configfile called pyconfig.ini somewhere in that path
and that file is automatically generated there by CMake by default.

After the script has run it will by default generete a html report in the ```<buildfolder>/regress/report.html```

#### Creating regression tests 
The regression script will by default look for regression test at the ```<module>/test/regression```
where it will assume a folder structure like this. The script will use the first ".ivw" file as is finds as workspace 
and the first "*.py" file as a script file. The script file is optional. All "*.png" will be assumed to be reference files
and will be compare to the genrerated images from the workspace/script.
```
────▶regression               
     │                        
     ├───▶test1               
     │    │                   
     │    ├──▶test1.inv   
     │    │                   
     │    └──▶canvas1.png     
     │                        
     └───▶test2               
          │                   
          ├──▶test2.inv   
          │                   
          ├──▶script.py          
          │                   
          ├──▶config.json          
          │                   
          ├──▶canvas1.png     
          │                   
          └──▶canvas2.png     
```
Aditionally a "config.json" file can optionally be added, where one can specify a toloerance per reference image 
```
{
  "image_test" :  {
    "differenceTolerance" : {
       "canvas1.png"   : 0.00001
    }
  }
}
```

## Developer Utils

### Module creation
To create a new module there is a utility python script available in the tools folders.
The script will create all the files for you and fill them in with the needed information.

```
usage: make-new-module.py [-h] [-d] [-v] [-i IVWPATH] [-c BUILDDIR]
                          modules [modules ...]

Add new modules to inviwo

positional arguments:
  modules               Modules to add, form: path/name1 path/name2 ...

optional arguments:
  -h, --help            show this help message and exit
  -d, --dummy           Don't write actual files (default: False)
  -v, --verbose         Print extra information (default: False)
  -i IVWPATH, --inviwo IVWPATH
                        Path to the inviwo repository. Tries to find it in the
                        current path (default: )
  -c BUILDDIR, --cmake BUILDDIR
                        Rerun CMake in the specified build directory (default:
                        None)
```


### File/Processor creation
```
usage: make-new-files.py [-h] [-p] [-i IVWPATH] [-c BUILDDIR] [-nh] [-ns] [-f]
                         [-v] [-g] [-d] [--force]
                         names [names ...]

Add new files to Inviwo. typical usage: python.exe ./make-new-files.py --cmake
../build ../modules/mymodule/path/to/h-file/MyNewClass

positional arguments:
  names                 Classes to add, form: path/to/h-file/NewClassName
                        Note: the path should be to where the header should be
                        even if you do not genereate a header

optional arguments:
  -h, --help            show this help message and exit
  -p, --processor       Make a skeleton inviwo processor (default: False)
  -i IVWPATH, --inviwo IVWPATH
                        Path to the inviwo repository. If now given the script
                        tries to find it in the current path (default: None)
  -c BUILDDIR, --cmake BUILDDIR
                        Rerun CMake in the specified build directory (default:
                        None)
  -nh, --no-header      Don't add header file (default: False)
  -ns, --no-source      Don't add source file (default: False)
  -f, --frag            Add fragment shader file (default: False)
  -v, --vert            Add vertex shader file (default: False)
  -g, --geom            Add geometry shader file (default: False)
  -d, --dummy           Write local testfiles instead (default: False)
  --force               Overwrite exsting files (default: False)
```


### Pythons Settings
The python scripts take advantage of a settings file __tools/pyconfig.ini__ where the path to _CMake__ can be defined
```
[CMake]
path = C:\Program Files (x86)\CMake\bin\cmake.exe
``` 