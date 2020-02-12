# version 330

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec4 vColor;
layout (location = 2) in vec2 vTexc;

uniform mat4 vProj;
uniform mat4 vView;
uniform float zCoord;

out vec4 fColor;
out vec2 fTexc;

void main ()
{
	fColor = vColor / 255.0;
	fTexc = vTexc;
	gl_Position = vec4 ( vPos.x, vPos.y, zCoord, 1.0 ) * vView * vProj;
}
