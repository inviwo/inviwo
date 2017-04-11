import json 
import re
import sys
import argparse
sys.path.append('../../')
import ivwpy.ivwpaths 

def dictToOrderedList(d):
    for a,b in sorted(d.items()):
        yield b



if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='Add new modules to inviwo', 
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-i", "--inviwo", type=str, default="", dest="ivwpath", 
                        help="Path to the inviwo repository. Tries to find it in the current path")
    args = parser.parse_args()
    if args.ivwpath == "":
        ivwpath = ivwpy.ivwpaths.find_inv_path()
    else:
        ivwpath = args.ivwpath


    enum = "enum class Colormap{";


    impls = "";
    names = "";

    lastEnum = "";
    firstEnum = "";

#colorbrewer.json is downlaoded from http://colorbrewer2.org/# 

    with open('colorbrewer.json','r') as cb_file:
        cb = json.load(cb_file);
        for fam,arr in sorted(cb.items()):
            arrs = {};
            for a,b in arr.items():
                try:
                    arrs[int(a)] = b;
                except:
                    pass

            enum += "\n\t"
            imp = []
            for a,b in sorted(arrs.items()):
                enumname = fam +"_" + str(a);
                enum += enumname + ", ";
                lastEnum = enumname;
                if len(firstEnum) == 0:
                    firstEnum = enumname;

                impls += "\t\tcase Colormap::" + enumname + ": {"
                colors = []
                for color in b:
                    r,g,b = color[4:-1].split(',')
                    r = int(r) / 255;
                    g = int(g) / 255;
                    b = int(b) / 255;
                    c = ', '.join([str(r),str(g),str(b),"1.0"]);
                    colors.append('dvec4('+c+')');

#                vector = "std::vector<dvec4>({"+','.join(colors)+"});"
                vector = "std::vector<dvec4> "+ enumname.lower()  +"(\n\t\t\t\t{"+',\n\t\t\t\t '.join(colors)+"});"

                impls += "\n\t\t\tstatic const " + vector + "\n\t\t\treturn "+ enumname.lower() +";\n\t\t}\n"

                #impls += " return "+vector+"\n"
                names += "\tcase Colormap::" + enumname + ": os << \"" + enumname + "\"; break;\n";


    enum += "\n\tFirstMap=" + firstEnum + ", LastMap=" + lastEnum + "\n};\n\n"

    header = ""
    src = ""
    with open('colorbrewer_tmpl.h','r') as template_h:
        header = template_h.read();
    with open('colorbrewer_tmpl.cpp','r') as template_cpp:
        src = template_cpp.read();






    header = header.replace("##PLACEHOLDER##",enum);
    src = src.replace("##PLACEHOLDER##",impls);
    header = header.replace("##PLACEHOLDER_NAMES##",names);

    # replace tabs with spaces
    src = src.replace("\t","    ");
    header = header.replace("\t","    ");

    with open(ivwpath + '/include/inviwo/core/util/colorbrewer.h','w') as header_file:
        print(header,file=header_file);
        header_file.close();
    with open(ivwpath + '/src/core/util/colorbrewer.cpp','w') as source_file:
        print(src,file=source_file);
        source_file.close();



