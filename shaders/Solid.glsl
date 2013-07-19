-- Vertex
#version 420

in vec4	vPosition;
in vec3 vNorm;

out vec3 smoothNorm;
out vec4 worldPos;

uniform mat4 modelToWorld;
uniform mat4 worldToCamera;

void main()
{
	smoothNorm = mat3(modelToWorld) * vNorm; //!Not strictly accurate!
	worldPos = modelToWorld * vPosition;
	gl_Position = worldToCamera * worldPos;
}

-- Fragment
#version 420

in vec3 smoothNorm;
in vec4 worldPos;

out vec4 fragColor;

const int numLights = 30;

struct Material
{
	vec4 ambient;
	vec4 diffuse;
	vec4 specular;
	float exponent;
};

uniform vec4 lightPos[numLights];
uniform vec4 lightDiffuse[numLights];
uniform vec4 lightSpecular[numLights];
uniform float lightAttenuation[numlights];

uniform vec4 ambLight;

uniform Material materal;

uniform vec3 cameraPos;

void main()
{
	fragColor = material.ambient * ambLight;

	vec3 norm = normalize(smoothNorm);

	vec3 view = normalize(vec3(cameraPos - worldPos));

	for(int i = 0; i < numLights; ++i)
	{
		if(lightPos[i].w == 0.0)// Directional light source
		{
			// Diffuse lighting
			vec3 lightDir = normalize(vec3(lightPos[i]));
			fragColor += max(dot(lightDir, norm), 0.0) * 
				material.diffuse * lightDiffuse[i];

			// Specular (Phong) lighting
			vec3 reflected = reflect(lightDir, norm);
			fragColor += pow(max(dot(reflected, view), 0.0), material.exponent) * 
				material.specular * lightSpecular[i];
		}

		else // Point light source
		{
			vec3 toLight = vec3(lightPos[i] - worldPos);
			float denom = lightAttenuation[i] * length(toLight);
			toLight = normalize(toLight);
			// Diffuse lighting
			fragColor += max(dot(toLight, norm), 0.0) * 
				material.diffuse * lightDiffuse[i] / denom;
			// Specular (Phong) lighting
			vec3 reflected = reflect(toLight, norm);
			fragColor += pow(max(dot(reflected, view), 0.0), material.exponent) * 
				material.specular * lightSpecular[i] / denom;
		}
	}
}
