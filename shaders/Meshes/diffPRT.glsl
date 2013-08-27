/* DiffPRT
 * Shader intended to render PRTMesh objects.
 */

--Vertex
#version 430

in vec4 vPosition;
in vec2 vTexCoord;

out vec2 smoothTex;

uniform mat4 modelToWorld;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec4 cameraPos;
	vec4 cameraDir;
};

void main()
{
	smoothTex = vTexCoord;
	gl_Position = worldToCamera * modelToWorld * vPosition;
}

--Fragment
#version 430 

in vec2 smoothTex;

out vec4 fragColor;

uniform sampler2DArray coefftTex;

layout(std140) uniform SHBlock
{
	vec4 lightCoeffts[$nSHCoeffts$];	
	int nLights;
};

void main()
{
	vec3 color = vec3(0.0, 0.0, 0.0);

	for(int i = 0; i < $nSHCoeffts$; ++i)
	{
		vec4 transfer = texture(coefftTex, vec3(smoothTex.x, (1.0 - smoothTex.y), i));
		vec3 lightCol =  vec3(transfer)* 
				vec3(lightCoeffts[i]);

		if(transfer.w < 0.5)
			color -= lightCol;
		else
			color += lightCol;
	}

	fragColor = vec4(color, 1.0);
}
