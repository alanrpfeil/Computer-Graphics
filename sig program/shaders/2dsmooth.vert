# version 330

layout (location = 0) in vec2 vPos;
layout (location = 1) in vec4 vColor;

uniform mat4 vProj;
uniform mat4 vView;
uniform float zCoord;

out vec4 Color; // note no flat keyword here

void main ()
{
	Color = vColor / 255.0;
	gl_Position = vec4 ( vPos.x, vPos.y, zCoord, 1.0 ) * vView * vProj;
}
