import sys
import os
import matplotlib.pyplot as plt




def createColorMap(colorMap,colors):
	f = open(colorMap + ".itf",'w');

	points = "";

	t1 = "        ";
	t2 = "            ";
	l = len(colors);

	dl = 1.0 / (l-1);

	for i in range(0,l):
	    x = i * dl;
	    r = colors[i][0];
	    g = colors[i][1];
	    b = colors[i][2];
	    a = colors[i][3]
	    points += t1 + "<point>\n";
	    points += t2 + "<pos x=\"" + str(x)+ "\" y=\"" + str(a)+ "\" />\n"
	    points += t2 + "<rgba x=\"" + str(r)+ "\" y=\"" + str(g)+ "\" z=\"" + str(b)+ "\" w=\"" + str(a)+ "\" />\n"
	    points += t1 + "</point>\n";


	f.write("<?xml version=\"1.0\" ?>\n");
	f.write("<InviwoTreeData version=\"1.0\">\n")
	f.write("    <!-- Don't edit the following code -->\n")
	f.write("    <maskMin content=\"0\" />\n")
	f.write("    <maskMax content=\"1\" />\n")
	f.write("    <dataPoints>\n")
	f.write(points);
	f.write("    </dataPoints>\n");
	f.write("    <interpolationType_ content=\"0\" />\n");
	f.write("</InviwoTreeData>\n");


cmaps = [('Sequential',     ['Blues', 'BuGn', 'BuPu',
                             'GnBu', 'Greens', 'Greys', 'Oranges', 'OrRd',
                             'PuBu', 'PuBuGn', 'PuRd', 'Purples', 'RdPu',
                             'Reds', 'YlGn', 'YlGnBu', 'YlOrBr', 'YlOrRd']),
         ('Sequential (2)', ['afmhot', 'autumn', 'bone', 'cool', 'copper',
                             'gist_heat', 'gray', 'hot', 'pink',
                             'spring', 'summer', 'winter']),
         ('Diverging',      ['BrBG', 'bwr', 'coolwarm', 'PiYG', 'PRGn', 'PuOr',
                             'RdBu', 'RdGy', 'RdYlBu', 'RdYlGn', 'Spectral',
                             'seismic']),
         ('Qualitative',    ['Accent', 'Dark2', 'Paired', 'Pastel1',
                             'Pastel2', 'Set1', 'Set2', 'Set3']),
         ('Miscellaneous',  ['gist_earth', 'terrain', 'ocean', 'gist_stern',
                             'brg', 'CMRmap', 'cubehelix',
                             'gnuplot', 'gnuplot2', 'gist_ncar',
                             'nipy_spectral', 'jet', 'rainbow',
                             'gist_rainbow', 'hsv', 'flag', 'prism'])]


numPoints = 256
maps = sorted(m for m in plt.cm.datad if not m.endswith("_r"))
print(maps)

def ensure_dir(f):
    if not os.path.exists(f):
    	os.makedirs(f)

#ensure_dir("matplotlib")

for colormap in maps:
	for colormap in maps:
		colors = [];
		cmap=plt.get_cmap(colormap)
		for i in range(0,numPoints):
			t = i / (numPoints-1);
			colors.append(cmap(t))
		createColorMap("matplotlib/" + colormap,colors);

	