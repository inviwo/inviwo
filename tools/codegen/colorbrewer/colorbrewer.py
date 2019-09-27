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
    parser = argparse.ArgumentParser(description='Generate C++ source and header files exposing colorbrewer colors', 
                                     formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument("-i", "--inviwo", type=str, default="", dest="ivwpath", 
                        help="Path to the inviwo repository. If ommited, tries to find it in the current path")
    args = parser.parse_args()
    if args.ivwpath == "":
        ivwpath = ivwpy.ivwpaths.find_inv_path()
    else:
        ivwpath = args.ivwpath

    enum = "enum class Colormap {";

    cat = [];
    categories = "enum class Category { ";

    families = "enum class Family {";

    familiesInCategory = dict();
    getFamiliesForCategoryImpl = "";

    maxElementsForFamily = dict();
    getMaxNumberOfColorsForFamilyImpl = "";
    
    impls = "";
    names = "";
    families_os = "";
    categories_os = "";

    lastEnum = "";
    firstEnum = "";

	#colorbrewer.json is downlaoded from http://colorbrewer2.org/# 

    with open('colorbrewer.json','r') as cb_file:
        cb = json.load(cb_file);
        famID = 0;
        for fam,arr in sorted(cb.items()):
            if famID % 7 == 0:
                families += "\n\t"
            famID += 1    
            families_os += "\t\tcase Family::" + fam + ": os << \"" + fam + "\"; break;\n";
            families +=  fam + ", "
            arrs = {}
            for a,b in arr.items():
                try:
                    arrs[int(a)] = b
                except:
                    pass
            if not arr["type"] in cat:
                familiesInCategory[arr["type"]] = [];

            cat.append(arr["type"]);
            familiesInCategory[arr["type"]].append(fam);

            enum += "\n\t"
            imp = []
            i=0;
            for a,b in sorted(arrs.items()):
                if i == len(sorted(arrs.items())) - 1:
                    if not a in maxElementsForFamily:
                        maxElementsForFamily[a] = [];
                    maxElementsForFamily[a].append(fam);
                i+=1;
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

                vector = "std::vector<dvec4> "+ enumname.lower()  +"(\n\t\t\t\t{"+',\n\t\t\t\t '.join(colors)+"});"

                impls += "\n\t\t\tstatic const " + vector + "\n\t\t\treturn "+ enumname.lower() +";\n\t\t}\n"

                names += "\t\tcase Colormap::" + enumname + ": os << \"" + enumname + "\"; break;\n";
                
        families += "\n\tNumberOfColormapFamilies, Undefined\n};"
        families_os += "\t\tcase Family::NumberOfColormapFamilies: os << \"NumberOfColormapFamilies\"; break;\n"
        families_os += "\t\tcase Family::Undefined: os << \"Undefined\"; break;"
        catset = set(cat);
        catlist = list(catset);

        for c in sorted(catlist):
            categories += c + ", ";
            getFamiliesForCategoryImpl += "\t\tcase Category::";
            if c == "div":
                getFamiliesForCategoryImpl += "Diverging:\n"
                categories_os += "\t\tcase Category::Diverging: os << \"Diverging\"; break;\n";
            if c == "qual":
                getFamiliesForCategoryImpl += "Qualitative:\n"
                categories_os += "\t\tcase Category::Qualitative: os << \"Qualitative\"; break;\n";
            if c == "seq":
                getFamiliesForCategoryImpl += "Sequential:\n"
                categories_os += "\t\tcase Category::Sequential: os << \"Sequential\"; break;\n";
            for f in sorted(familiesInCategory[c]):
                getFamiliesForCategoryImpl += "\t\t\tv.emplace_back(Family::" + f + ");\n";
            getFamiliesForCategoryImpl += "\t\t\tbreak;\n";
        getFamiliesForCategoryImpl += "\t\tdefault:\n\t\t\tbreak;";

        categories_os += "\t\tcase Category::NumberOfColormapCategories: os << \"NumberOfColormapCategories\"; break;\n";
        categories_os += "\t\tcase Category::Undefined: os << \"Undefined\"; break;";

        r = 0
        for a in maxElementsForFamily:
            getMaxNumberOfColorsForFamilyImpl += "\tif (";
            for z in maxElementsForFamily[a]:
                getMaxNumberOfColorsForFamilyImpl += "family == Family::" + z + " || "
                if r % 2:
                    getMaxNumberOfColorsForFamilyImpl += "\n\t\t";
                r=r+1
            if not r % 2:
                getMaxNumberOfColorsForFamilyImpl = getMaxNumberOfColorsForFamilyImpl[:-7];
            else:
                getMaxNumberOfColorsForFamilyImpl = getMaxNumberOfColorsForFamilyImpl[:-4];
            getMaxNumberOfColorsForFamilyImpl += ") {\n\t\treturn " + str(a) + ";\n\t}\n";

    categories += "NumberOfColormapCategories, Undefined };\n";

    categories = categories.replace("seq", "Sequential");
    categories = categories.replace("div", "Diverging");
    categories = categories.replace("qual", "Qualitative");

    enum += "\n\tFirstMap=" + firstEnum + ", LastMap=" + lastEnum + "\n};\n\n"

    header = ""
    src = ""
    with open('colorbrewer_tmpl.h','r') as template_h:
        header = template_h.read();
    with open('colorbrewer_tmpl.cpp','r') as template_cpp:
        src = template_cpp.read();

    while names.endswith("\n"):
        names = names[0:-1]

    while getMaxNumberOfColorsForFamilyImpl.endswith("\n"):
        getMaxNumberOfColorsForFamilyImpl = getMaxNumberOfColorsForFamilyImpl[0:-1]

    

    header = header.replace("##PLACEHOLDER##",enum + categories + "\n" + families);
    src = src.replace("##PLACEHOLDER##",impls);
    src = src.replace("##GETFAMILIESIMPL##", getFamiliesForCategoryImpl);
    src = src.replace("##GETMAXIMPL##", getMaxNumberOfColorsForFamilyImpl);
    header = header.replace("##PLACEHOLDER_NAMES##",names);
    header = header.replace("##PLACEHOLDER_CATEGORIES##",categories_os);
    header = header.replace("##PLACEHOLDER_FAMILIES##",families_os);

    # replace tabs with spaces
    src = src.replace("\t","    ");
    header = header.replace("\t","    ");

    #with open(ivwpath + '/include/inviwo/core/util/colorbrewer-generated.h','w') as header_file:
    #    print(header, file=header_file, end='')
    #    header_file.close();
    with open(ivwpath + '/src/core/util/colorbrewer-generated.cpp','w') as source_file:
        print(src, file=source_file, end='')
        source_file.close();
