# Using Inviwo from within Python
While Python Processors (TODO: link) allow you to embed Python code inside Inviwo processors, you can also use Inviwopy to use your Inviwo networks from within a Python application. This guide shows you first how to build Inviwopy, then how to load existing Inviwo workspaces for use in your Python scripts and lastly some ways to modify a loaded Inviwo network from within Python.

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

 Depending on your Python version and operating system this will generate two dynamic libraries with names similar to:
- `inviwopy.cpython-36m-x76_64-linux-gnu.so`
- `inviwopyapp.cpython-36m-x76_64-linux-gnu.so`

On Windows similarly named `.dll`s are generated. Note that those libraries depend heavily on the other libraries in the `bin` folder. Those libraries need to be in your `PYTHONPATH`.
So make sure to add the following lines to the top of your Python script:
```python
import sys
sys.path.append('<path to inviwo build>/bin')
```


## Loading an Inviwo workspace
Once your Python interpreter finds the Inviwo libraries, you can just import Inviwopy in Python:
```python
import inviwopy as ivw
import inviwopyapp as qt
```

From here you have to initialize a logger and the Inviwo application

```python
lc = ivw.LogCentral()
cl = ivw.ConsoleLogger()
lc.registerLogger(cl)

app = qt.InviwoApplicationQt()
app.registerModules()
```
The Inviwo application `app` gives you access to the network and its components. You can load the example workspaces, in this case the Boron example, as follows:
```python
app.network.load(app.getPath(ivw.PathType.Workspaces) + '/boron.inv')
```

From here on you can modify your network and its processors through `app.network.processors`. All that is left to do is to actually run the network, after possible modifications have been made:
```python
app.run()
```


In order to expose some Properties without starting the full GUI, you can add a `PropertyListWidget` and populate it as follows:
```python
plw = ivw.qt.PropertyListWidget(app)
plw.addProcessorProperties(app.network.VolumeRaycaster)
plw.show()
```

See the full example [here](https://github.com/inviwo/inviwo/blob/master/apps/inviwopyapp/inviwo.py).


## Modifying Inviwo networks with Python
This section presents some sensible modifications to the networks inside Python. Note that you should always define the very majority of your workspace inside the normal Inviwo GUI. Once your network works there, all you usually want to do is passing inputs from Python to your network and retrieving results from your network into Python.

TODO: find good way to do this
