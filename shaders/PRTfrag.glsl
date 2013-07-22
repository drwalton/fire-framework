-- Vertex
#version 420

in vec4 vPos;
in vec4 transferCoeffts[9];

uniform mat4 modelToWorld;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec3 cameraPos;
	vec3 cameraDir;
};

out vec4 smoothCoeffts[9];

void main()
{
	gl_Position = worldToCamera * modelToWorld * vPos;

	

	for(int c = 0; c < 9; ++c)
		smoothCoeffts[c] = transferCoeffts[c];
}

-- Fragment
#version 420

in vec4 smoothCoeffts[9];

layout(std140) uniform SHBlock
{
	vec4 lightCoeffts[9 * 10];	
	int nLights;
};

out vec4 fragColor;

void main()
{
	vec3 color = vec3(0.0, 0.0, 0.0);

	for(int l = 0; l < 10; ++l)
	{
		for(int c = 0; c < 9; ++c)
		{
			color += smoothCoeffts[c].xyz * lightCoeffts[c + (l * 9)].xyz;
		}
	}

	fragColor = vec4(color, 1.0);
}
