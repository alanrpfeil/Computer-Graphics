# version 330

layout (location = 0) in vec3 vPos;

uniform mat4 vProj;
uniform mat4 vView;
uniform vec4 vColor;

out vec4 Color; // note no flat keyword here

void main ()
{
	Color = vColor;
	gl_Position = vec4(vPos.x,vPos.y,vPos.z,1.0) * vView * vProj;
}
