# version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;

const int ML=4; // max number of lights

uniform mat4	 vProj;
uniform mat4	 vView;
uniform vec3[ML] lPos;    // light position
uniform mat3[ML] lInt;    // light intensities: ambient, diffuse, and specular 
uniform int		 lNum;	  // Number of active lights
uniform vec3[4]  mColors; // material colors  : ambient, diffuse, specular, and emission 
uniform float[2] mParams; // material params  : shininess, transparency

flat out vec4 Color;

vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3 la, vec3 ld, vec3 ls, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha );

void main ()
{
	vec4 p4 = vec4(vPos,1.0f) * vView; // vertex pos in eye coords
	vec3 p = p4.xyz / p4.w;

	vec3 n = normalize ( vNorm*transpose(inverse(mat3(vView))) ); // vertex normal 

	Color = vec4(0,0,0,0);
	int i=0;
	if ( i<lNum ) { Color += shade ( p, n, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }
	if ( i<lNum ) { Color += shade ( p, n, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }
	if ( i<lNum ) { Color += shade ( p, n, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }
	if ( i<lNum ) { Color += shade ( p, n, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }

	gl_Position = vec4(p,1.0) * vProj;
}
