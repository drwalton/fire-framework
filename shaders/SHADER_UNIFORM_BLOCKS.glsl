/* Shader Uniform Blocks
 * This file contains a list of some of the uniform blocks 
 * used in this application, and their formats. These may be
 * copied into shaders which require them. 
 * ========================================================
 * Note that the application is designed to automatically 
 * substitute constants such as $maxPhongLights$ in the source
 * with the appropriate literal values.
 */

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
	vec4 lightPos[$maxPhongLights$];
	vec4 lightDiffuse[$maxPhongLights$];
	vec4 lightSpecular[$maxPhongLights$];
	float lightAttenuation[$maxPhongLights$];
	uint nLights;
};

//INDEX = 3
layout(std140) uniform SHBlock
{
	vec3 lightCoeffts[$nSHCoeffts$ * $maxSHLights$];
	uint nLights;
};
