
spatialCoordinateTransformer_spaces = ["Data" , "Model" , "World"]
spatialCameraCoordinateTransformer_spaces = ["Data" , "Model" , "World" , "Clip" , "View"]
structuredCoordinateTransformer_spaces = ["Data" , "Model" , "World" , "Texture" , "Index"]
structuredCameraCoordinateTransformer_spaces = ["Data" , "Model" , "World" , "Texture" , "Index" , "Clip" , "View"]

'''
private:
    enum class SPACES{
        Data , Model , World
    };
public:
    const Matrix <H + 1, float> getMatrix(SPACES from, SPACES to)const{
        switch (from){
        case Data:
            switch (to){
            case Data:
                return Matrix<N + 1, float>(1.0f);
            case Model:
                return getDataToModelMatrix();
            case World:
                return getDataToWorldMatrix();
            }
        case Model:
            switch (to){
            case Data:
                return getModelToWorldMatrix();
            case Model:
                return Matrix<N + 1, float>(1.0f);
            case World:
                return getModelToWorldMatrix();
            }
        case World:
            switch (to){
            case Data:
                return getWorldToDataMatrix();
            case Model:
                return getWorldToDataMatrix()();
            case World:
                return Matrix<N + 1, float>(1.0f);
            }
        }
    }
'''




def generateSpaces(spaces):
    print("")
    print("")
    print("")
    print("//AUTO GENERATED CODE STARTS HERE")
    print("\tenum class SPACES{")
    print("\t\t" + (", ".join(spaces)))
    print("\t};")
    print("public:")
    print("\tconst Matrix <N + 1, float> getMatrix(SPACES from, SPACES to)const{")
    print("\t\tswitch(from){")
    for f in spaces:
        print("\t\tcase SPACES::" + f + ":")
        print("\t\t\tswitch(to){")
        for t in spaces:
            print("\t\t\tcase SPACES::" + t + ":")
            if (f == t) or (f == "Data" and t == "Texture" ) or (t == "Data" and f == "Texture" ):
                print("\t\t\t\treturn Matrix<N + 1, float>(1.0f);")
            else:
                print("\t\t\t\treturn get"+f+"To"+t+"Matrix();")
        print("\t\t\t}")
    print("\t\t}")
    print("\t\tthrow std::exception(\"getMatrix is not implatemented for the given spaces\");")
    print("\t}")
    print("//AUTO GENERATED CODE ENDS HERE")
    



generateSpaces(spatialCoordinateTransformer_spaces)
generateSpaces(structuredCoordinateTransformer_spaces)
generateSpaces(spatialCameraCoordinateTransformer_spaces)
generateSpaces(structuredCameraCoordinateTransformer_spaces)
