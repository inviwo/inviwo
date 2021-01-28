# Render random volume
import inviwopy

# Get app, etc.
app = inviwopy.app
app.resizePool(0) # Pool size 0 necessary at the moment
network = app.network

# Get src, sink, raycaster
src = app.network.VolumeInjector
canvas = app.network.Canvas
vr = app.network.VolumeRaycaster
vol = np.random.rand(200, 200, 200).astype(np.float32)

# Set data from Py
app.network.lock() # stops evaluation
src.setArray(vol)
# Make object fit in view
vr.properties.camera.fitData()
app.network.unlock() # re-enables evaluation
app.waitForPool() # wait until render is done

canvas.snapshot('snapshot.png')