-- Vertex
#version 420

in vec4 vPos;
in vec3 transferCoeffts[9];

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;

out vec3 color;

uniform vec3 lightCoeffts[9 * 10];

void main()
{
	gl_Position = worldToCamera * modelToWorld * vPos;

	color = vec3(0.0, 0.0, 1.0);

	for(int l = 0; l < 10; ++l)
	{
		for(int c = 0; c < 9; ++c)
		{
			color += transferCoeffts[c] * lightCoeffts[c + (l * 9)];
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
