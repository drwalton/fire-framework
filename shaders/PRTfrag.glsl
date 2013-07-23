-- Vertex
#version 420

in vec4 vPos;
in vec4 transferCoeffts[$nSHCoeffts$];

uniform mat4 modelToWorld;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec3 cameraPos;
	vec3 cameraDir;
};

out vec4 smoothCoeffts[$nSHCoeffts$];

void main()
{
	gl_Position = worldToCamera * modelToWorld * vPos;

	

	for(int c = 0; c < $nSHCoeffts$; ++c)
		smoothCoeffts[c] = transferCoeffts[c];
}

-- Fragment
#version 420

in vec4 smoothCoeffts[$nSHCoeffts$];

layout(std140) uniform SHBlock
{
	vec4 lightCoeffts[$nSHCoeffts$ * $maxSHLights$];	
	int nLights;
};

out vec4 fragColor;

void main()
{
	vec3 color = vec3(0.0, 0.0, 0.0);

	for(int l = 0; l < $maxSHLights$; ++l)
	{
		for(int c = 0; c < $nSHCoeffts$; ++c)
		{
			color += smoothCoeffts[c].xyz * lightCoeffts[c + (l * $nSHCoeffts$)].xyz;
		}
	}

	fragColor = vec4(color, 1.0);
}
