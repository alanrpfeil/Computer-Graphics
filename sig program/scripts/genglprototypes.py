# Marcelo Kallmann - 2015

# Parse glcorearb.h and generates OpenGL definitions

inp = open('../include/sigogl/glcorearb.h')
out1 = open('glcorearb_functions.h', 'w')
out2 = open('gl_loader_functions.inc', 'w')

header = ( "#ifndef glcorearb_functions_h\n"
           "#define glcorearb_functions_h\n\n"
           "#ifndef PFNGLDECLTYPE\n"
           "#define PFNGLDECLTYPE extern\n"
           "#endif\n\n" )

out1.writelines ( header );

maxfuncs=645
nfuncs=0

for line in inp:
	proto = line.find("APIENTRY gl")
	if proto>=0:
		nb = line.find("APIENTRY")
		ne = line.find("(")
		if nb>=0 and ne>=0:
			name = line[nb+9:ne-1]
			type = "PFN"+name.upper()+"PROC"
			decl = "PFNGLDECLTYPE " + type + " " + name + ";\n"
			init = name + "=("+type + ")GetProc(\"" + name + "\"); if(++c==n)return;\n"
			print(decl)
			print(init)
			out1.writelines(decl)
			out2.writelines(init)
			nfuncs = nfuncs+1
			if nfuncs==maxfuncs:
				break

out1.writelines("\n#endif\n")

inp.close()
out1.close()
out2.close()

print("Total:", nfuncs)