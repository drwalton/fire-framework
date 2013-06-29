-- Vertex
#version 420

in vec4	vPosition;
in vec3 vNorm;

out vec3 norm;
out vec4 worldPos;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;

void main()
{
	norm = mat3(modelToWorld) * vNorm; //!Not strictly accurate!
	worldPos = modelToWorld * vPosition;
	gl_Position = worldToCamera * worldPos;
}

-- Fragment
#version 420

const int maxDirLights = 10;
const int maxPointLights = 10;

in vec3 norm;
in vec4 worldPos;

out vec4 fragColor;

uniform float ambLight;

uniform uint dirLightOn[maxDirLights];
uniform vec3 dirLightDir[maxDirLights];
uniform float dirIntensity[maxDirLights];

uniform uint pointLightOn[maxPointLights];
uniform vec4 pointLightPos[maxPointLights];
uniform float pointIntensity[maxPointLights];

void main()
{
	float intensity = 0.0;

	//Ambient lighting.
	intensity += ambLight;

	int i;
	//Directional lighting.
	for(i = 0; i < maxDirLights; ++i)
	{
		if(dirLightOn[i] != 0)
			intensity += clamp(dot(norm, normalize(-dirLightDir[i])), 0, 1) * dirIntensity[i];
	}

	//Point lighting.
	for(i = 0; i < maxPointLights; ++i)
	{
		if(pointLightOn[i] != 0)
		{
			vec3 toLight = normalize((pointLightPos[i] - worldPos).xyz);
			intensity += clamp(dot(normalize(norm), toLight), 0, 1) * pointIntensity[i];
		}
	}

	intensity = clamp(intensity, 0, 1);

	fragColor = vec4(intensity, intensity, intensity, 1.0);
}
