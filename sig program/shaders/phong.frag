# version 330

const int ML=4; // max number of lights

uniform vec3[ML] lPos;	   // light position
uniform mat3[ML] lInt;	   // light intensities: ambient, diffuse, and specular
uniform int		 lNum;	  // Number of active lights
uniform vec3[4]  mColors;  // material colors  : ambient, diffuse, specular, and emission
uniform float[2] mParams;  // material params  : shininess, transparency

in vec3 Pos;
in vec3 Norm;
out vec4 fColor;

vec4 shade ( vec3 p, vec3 n, vec3 lp, vec3 la, vec3 ld, vec3 ls, vec3 ka, vec3 kd, vec3 ks, vec3 emi, float sh, float alpha );

void main() 
{
	// fColor = shade ( Pos, Norm, lPos, lInt[0], lInt[1], lInt[2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] );

	fColor = vec4(0,0,0,0);
	int i=0;
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }
	if ( i<lNum ) { fColor += shade ( Pos, Norm, lPos[i], lInt[i][0], lInt[i][1], lInt[i][2], mColors[0], mColors[1], mColors[2], mColors[3], mParams[0], mParams[1] ); i++; }
} 
