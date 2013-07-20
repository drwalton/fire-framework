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

uniform vec4 lightPos[30];
uniform vec4 lightDiffuse[30];
uniform vec4 lightSpecular[30];
uniform float lightAttenuation[30];

uniform vec4 ambLight;

uniform vec4 material_ambient;
uniform vec4 material_diffuse;
uniform vec4 material_specular;
uniform float material_exponent;

uniform vec3 cameraPos;

void main()
{
	fragColor = material_ambient * ambLight;

	vec3 norm = normalize(smoothNorm);

	vec3 view = normalize(cameraPos - vec3(worldPos));

	for(int i = 0; i < numLights; ++i)
	{
		if(lightPos[i].w == 0.0)// Directional light source
		{
			// Diffuse lighting
			vec3 lightDir = normalize(vec3(lightPos[i]));
			fragColor += max(dot(lightDir, norm), 0.0) * 
				material_diffuse * lightDiffuse[i];

			// Specular (Phong) lighting
			vec3 reflected = reflect(lightDir, norm);
			fragColor += pow(max(dot(reflected, view), 0.0), material_exponent) * 
				material_specular * lightSpecular[i];
		}

		else // Point light source
		{
			vec3 toLight = vec3(lightPos[i] - worldPos);
			float denom = lightAttenuation[i] * length(toLight);
			toLight = normalize(toLight);
			// Diffuse lighting
			fragColor += max(dot(toLight, norm), 0.0) * 
				material_diffuse * lightDiffuse[i] / denom;
			// Specular (Phong) lighting
			vec3 reflected = reflect(toLight, norm);
			fragColor += pow(max(dot(reflected, view), 0.0), material_exponent) * 
				material_specular * lightSpecular[i] / denom;
		}
	}
}
