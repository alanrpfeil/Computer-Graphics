# version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec3 vNorm;
layout (location = 2) in vec2 vTexc;

uniform mat4 vProj;
uniform mat4 vView;

out vec3 Norm;
out vec3 Pos;
out vec2 Texc;

void main ()
{
	vec4 p4 = vec4(vPos,1.0f)*vView; // vertex pos in eye coords
	vec3 p = p4.xyz / p4.w;

	Texc = vTexc;
	Norm = normalize ( vNorm*transpose(inverse(mat3(vView))) );
	Pos = p4.xyz / p4.w;

	gl_Position = vec4(p,1.0)*vProj;
}
