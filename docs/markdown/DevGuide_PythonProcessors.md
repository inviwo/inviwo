# Python Processors
You can define Inviwo processors using Python instead of C++. Python often offers simpler means to implement certain functionality and brings a large palette of libraries that can easy processor development. However note, that Python in general runs a lot slower than C++ and should thus be avoided for performance critical processors. The following explains how Python processors can be created, how an Inviwo processor is properly defined in Python and how you can easily access data using NumPy.
The code from the following sections builds up to a simple processor that reads serialized Numpy arrays (`.npy`) from disk and serves them as `Volume` to the network.

## Building InviwoPy
To use Inviwopy, you have to build the appropriate `.dll`/`.so` yourself, since it is currently not available through Pypi. To do so, enable the `IVW_MODULE_PYTHON3` and `IVW_MODULE_PYTHON3QT` CMake flags. Next you need to specify the Python executable to which the produced library shall be compatible in the `PYTHON_EXECUTABLE` flag.
<details>
<summary>
Using Inviwopy with Anaconda environments
</summary>
<p>

1. Set the `PYTHON_EXECUTABLE` flag to your environment's executable (e.g. `~/.conda/envs/inviwo/bin/python3`)

2. If not set automatically, also adapt the `PYTHON_LIBRARY` flag to `<conda env>/lib/libpython3.6m.so` (according to your Python version).

</p>
</details>

## Processor Creation
In order to create a Python Processor, Inviwo must be built with the Python3 module enabled in CMake. To create a new Python processor, open Inviwo and select `Python > New Python Processor` from the menu. After specifying a processor name, the Python script containing the new processor is created in `$HOME/.inviwo/python_processors/<processor name>.py`. The script is already filled with a processor template containing the required methods etc. The newly created processor is also automatically added to your processor list for immediate use.

## Processor Definition
First of all `inviwopy` needs to be imported (called `ivw` in the following) to get access to `ivw.Processor`, `ivw.properties.*`, `ivw.ports.*` and `ivw.glm.*` wrappers. In our NumpySource example the imports are:
```python
import inviwopy as ivw
from inviwopy.properties import FileProperty, InvalidationLevel
from inviwopy.data       import Volume, VolumeOutport

import numpy as np
from pathlib import Path
```

Using those wrappers you can define Inviwo processors very similar to how it is done in C++. The actual processor definition happens inside your own processors class, inheriting from `ivw.Processor` and defining all the following methods:
```python
class NumpySource(ivw.Processor):
    def __init__(self, id, name):    # Default processor signature
        super().__init__(id, name)   # Call super class (Processor) with id, name

        [ Here comes normal constructor contents like adding ports, properties ]

    @staticmethod
    def processorInfo():                 # General information on this processor
        return ivw.ProcessorInfo(
            classIdentifier = "org.inviwo.numpysource",
    		displayName = "NumpySource",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self): return NumpySource.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        pass
```

As you can see, just as in C++, a processor needs to define a constructor to define all ports, properties etc., some information about the processor itself, the `initializeResources()` and the `process()` method. If you don't need `initializeResources()`, just define it with the `pass` no-op as function body.
Also do not forget the call to `ivw.Processor.__init__` in your processors `__init__`.

In our `NumpySource` example we can use the following `__init__`:
```python
class NumpySource(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = VolumeOutport("outport")  # Define Outport
        self.addOutport(self.outport)            # Add port to processor

        self.file = FileProperty("file", "Numpy Volume (.npy)",
            invalidationLevel=InvalidationLevel(2)) # invalidate resources on change
        self.addProperty(self.file)
        self.array = None    # Init to None
```
This `__init__` defines a volume outport to pass the loaded array to the network and a file property to locate the serialized Numpy array. Note that this `FileProperty` has its `invalidationLevel=InvalidationLevel(2)`, which lets the `FileProperty` invalidate the processors resources upon changing the property. This will automatically call the `initializeResources()` method which will take care of actually loading the Numpy file (see below).

## NumPy Compatibility
In order to transfer data between Python and C++, the Inviwo data structures  `Volume` (example below), `Layer` (for `Image`s, [example](https://inviwo.org/assets/media/inviwo-vcbm2019.pdf) slide 34-35) and `Buffer` (for `Mesh`es, [example](https://github.com/inviwo/modules/blob/2f07a0fffe916c413a520644b9fe2e45a3ee60a9/misc/vasp/python/vasputil.py#L109-L123)) can take Numpy arrays (`numpy.ndarray`) for initialization.

Loading a Numpy array from disk, wrapping it in a `Volume` and outputting it to the network can be realized as follows:
```python
def initializeResources(self):
    if Path(self.file.value).exists() and self.file.value.endswith('.npy'):
        self.array = np.load(self.file.value)
    else: print('Invalid file path.')

def process(self):
    if self.array is not None:
        vol = Volume(self.array)
        self.outport.setData(vol)
```
Note that `Path` is a class from Python's `pathlib` that handles file paths and in this case is just used to check whether the given file path exists.

That is all the code necesssary to making Inviwo able to read Numpy-serialized arrays from disk and supplying them as `Volume` to the network.
From here on you can do arbitrarily complex stuff using Python with your favorite Python libraries or you can wrap your existing Python-based algorithms in Inviwo processors to use them in your visualization.
