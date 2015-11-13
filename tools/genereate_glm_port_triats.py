


def getGlmType(t,i):
	s = "";
	if t == "double":
		s = "d"
	if t == "int":
		s = "i"
	return s + "vec" + str(i);

def color(t,i):
	r = 0;
	g = 0;
	b = 0;

	s = 50 + 25 * (i-2); #// 50, 75 or 100
	if(t == "double"):
		r = 2*s
		g = s;
	if(t == "float"):
		g = 2*s
		r = s;
	if(t == "int"):
		b = 2*s
		g = s;
	

	return str(r) + ", " + str(g) + ", " + str(b);

type = ["float","double","int"]

for i in range(2,5):
    for t in type:
        print("template <>")
        print("struct port_traits<Vector<"+str(i)+", "+t+">> {");
        print("\tstatic std::string class_identifier() { return \"" + getGlmType(t,i) + "\"; }")
        print("\tstatic uvec3 color_code() { return uvec3("+color(t,i)+"); }")
        print("\tstatic std::string data_info(const Vector<"+str(i)+", "+t+">* data) {")
        print("\t\treturn class_identifier();")
        print("}\t")
        print("};")
        print("")
        print("")



for i in range(2,5):
    for t in type:
    	s = getGlmType(t,i);
    	print("registerPort<DataInport<"+s+">>(\""+s+"Inport\");")
    	print("registerPort<DataInport<"+s+",0>>(\""+s+"MutliInport\");")
    	print("registerPort<DataInport<"+s+",0,true>>(\""+s+"FlatMultiInport\");")
    	print("registerPort<DataOutport<std::vector<"+s+">>>(\""+s+"VectorOutport\");")
    	print("registerPort<DataOutport<"+s+">>(\""+s+"Outport\");")
