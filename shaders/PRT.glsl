-- Vertex
#version 420

in vec4 vPos;
in vec3 transferCoeffts[numCoeffts];

in mat4 modelToWorld;
in mat4 worldToCamera;

out vec3 color;

uniform vec3 lightCoeffts[numCoeffts * numLights];

void main()
{
	gl_Position = worldToCamera * modelToWorld * vPos;

	color = vec3(0.0, 0.0, 0.0);

	for(int l = 0; l < numLights; ++l)
	{
		for(int c = 0; c < numCoeffts; ++c)
		{
			color += transferCoeffts[c] * lightCoeffts[c + (l * numCoeffts)];
		}
	}
}

-- Fragment
#version 420

in vec3 color;

out vec4 fragColor;

void main()
{
	fragColor = vec4(color, 1.0);
}
