// Uniform blocks:

//INDEX = 0
layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec3 cameraPos;
	vec3 cameraDir;
};

//INDEX = 1
layout(std140) uniform ambBlock
{
	vec4 ambLight;
};

//INDEX = 2
layout(std140) uniform phongBlock
{
	vec4 lightPos[50];
	vec4 lightDiffuse[50];
	vec4 lightSpecular[50];
	float lightAttenuation[50];
	uint nLights;
};

//INDEX = 3
layout(std140) uniform SHBlock
{
	vec3 lightCoeffts[9 * 10];
	uint nLights;
};
