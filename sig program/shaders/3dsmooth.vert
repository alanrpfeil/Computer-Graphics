# version 330

layout (location = 0) in vec3 vPos;
layout (location = 1) in vec4 vColor;

uniform mat4 vProj;
uniform mat4 vView;

out vec4 Color; // note no flat keyword here

void main ()
{
	Color = vColor / 255.0;
	gl_Position = vec4(vPos.x,vPos.y,vPos.z,1.0) * vView * vProj;
}
