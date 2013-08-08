-- Vertex
#version 420

in vec4 vPosition;
in vec3 vColor;

uniform mat4 modelToWorld;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec3 cameraPos;
	vec3 cameraDir;
};

out vec4 smoothColor;

void main()
{
	gl_Position = worldToCamera * modelToWorld * vPosition;
	smoothColor = vec4(vColor, 1.0);
}

-- Fragment
#version 420

in vec4 smoothColor;

out vec4 fragColor;

void main()
{
	fragColor = smoothColor;
}
