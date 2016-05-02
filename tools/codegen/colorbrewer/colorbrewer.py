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

    defs = {};
    imps = [];

#colorbrewer.json is downlaoded from http://colorbrewer2.org/# 

    with open('colorbrewer.json','r') as cb_file:
        cb = json.load(cb_file);
        for fam,arr in cb.items():

            defs[fam] =  "IVW_CORE_API std::vector<dvec4> " + fam + "(int dataClasses);"
        
            arrs = {};
            for a,b in arr.items():
                try:
                    arrs[int(a)] = b;
                except:
                    pass

            imp = ["std::vector<dvec4> " + fam + "(int dataClasses){"
                ,  "\tswitch(dataClasses){"
            ];
            for a,b in sorted(arrs.items()):
                imp.append("\tcase " + str(a) + ":")
                colors = []
                for color in b:
                    r,g,b = color[4:-1].split(',')
                    r = int(r) / 255;
                    g = int(g) / 255;
                    b = int(b) / 255;
                    c = ','.join([str(r),str(g),str(b),"1.0"]);
                    colors.append('dvec4( '+c+' )');

                imp.append("\t\t return std::vector<dvec4>({"+','.join(colors)+"});")

            imp.append("\tdefault:")
            imp.append('\t\tthrow ColorBrewerException("unsupported number of colors");')
            imp.append('\t\treturn std::vector<dvec4>();')

            imp.append("\t};")
            imp.append("};")
            imp.append("")
            imps.append('\n'.join(imp));


    header = ""
    src = ""
    with open('colorbrewer_tmpl.h','r') as template_h:
        header = template_h.read();
    with open('colorbrewer_tmpl.cpp','r') as template_cpp:
        src = template_cpp.read();



    header = header.replace("##PLACEHOLDER##",'\n'.join(dictToOrderedList(defs)));
    src = src.replace("##PLACEHOLDER##",'\n'.join((imps)));


    with open(ivwpath + '/include/inviwo/core/util/colorbrewer.h','w') as header_file:
        print(header,file=header_file);
        header_file.close();
    with open(ivwpath + '/src/core/util/colorbrewer.cpp','w') as source_file:
        print(src,file=source_file);
        source_file.close();



