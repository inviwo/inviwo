# Inviwo Python script 
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import inviwopy
from inviwopy.glm import vec2,vec3,vec4

#http://matplotlib.org/examples/color/colormaps_reference.html

#Perceptually Uniform Sequential :  #['viridis', 'inferno', 'plasma', 'magma'] 
#Sequential  :  #['Blues', 'BuGn', 'BuPu','GnBu', 'Greens', 'Greys', 'Oranges', 'OrRd', 'PuBu', 'PuBuGn', 'PuRd', 'Purples', 'RdPu','Reds', 'YlGn', 'YlGnBu', 'YlOrBr', 'YlOrRd']
#Diverging :  #['afmhot', 'autumn', 'bone', 'cool','copper', 'gist_heat', 'gray', 'hot','pink', 'spring', 'summer', 'winter']
#Qualitative :  #['BrBG', 'bwr', 'coolwarm', 'PiYG', 'PRGn', 'PuOr', 'RdBu', 'RdGy', 'RdYlBu', 'RdYlGn', 'Spectral', 'seismic']
#Miscellaneous :  #['Accent', 'Dark2', 'Paired', 'Pastel1', 'Pastel2', 'Set1', 'Set2', 'Set3']
#Sequential :  #['gist_earth', 'terrain', 'ocean', 'gist_stern','brg', 'CMRmap', 'cubehelix','gnuplot', 'gnuplot2', 'gist_ncar', 'nipy_spectral', 'jet', 'rainbow', 'gist_rainbow', 'hsv', 'flag', 'prism']


tf = inviwopy.app.network.VolumeRaycaster.transferFunction
tf.clear()

cmapName = "viridis"

cmap=plt.get_cmap(cmapName)

N = 128

for i in range(0,N,1):
   x = i / (N-1)
   a = 1.0
   color = cmap(x)
   tf.add(x, vec4(color[0],color[1],color[2], a)) 

