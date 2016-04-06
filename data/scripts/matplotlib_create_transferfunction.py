# Inviwo Python script 
import matplotlib.cm as cm
import matplotlib.pyplot as plt
import inviwo 

#http://matplotlib.org/examples/color/colormaps_reference.html

#Perceptually Uniform Sequential :  #['viridis', 'inferno', 'plasma', 'magma'] 
#Sequential  :  #['Blues', 'BuGn', 'BuPu','GnBu', 'Greens', 'Greys', 'Oranges', 'OrRd', 'PuBu', 'PuBuGn', 'PuRd', 'Purples', 'RdPu','Reds', 'YlGn', 'YlGnBu', 'YlOrBr', 'YlOrRd']
#Diverging :  #['afmhot', 'autumn', 'bone', 'cool','copper', 'gist_heat', 'gray', 'hot','pink', 'spring', 'summer', 'winter']
#Qualitative :  #['BrBG', 'bwr', 'coolwarm', 'PiYG', 'PRGn', 'PuOr', 'RdBu', 'RdGy', 'RdYlBu', 'RdYlGn', 'Spectral', 'seismic']
#Miscellaneous :  #['Accent', 'Dark2', 'Paired', 'Pastel1', 'Pastel2', 'Set1', 'Set2', 'Set3']
#Sequential :  #['gist_earth', 'terrain', 'ocean', 'gist_stern','brg', 'CMRmap', 'cubehelix','gnuplot', 'gnuplot2', 'gist_ncar', 'nipy_spectral', 'jet', 'rainbow', 'gist_rainbow', 'hsv', 'flag', 'prism']

inviwo.clearTransferfunction("VolumeRaycaster.transferFunction")

cmapName = "hot"

cmap=plt.get_cmap(cmapName)

for i in range(0,256,1):
   x = i / 256.0
   a = 1.0
   color = cmap(x)
   inviwo.addPointToTransferFunction("VolumeRaycaster.transferFunction",(x,a), (color[0:3])) 

