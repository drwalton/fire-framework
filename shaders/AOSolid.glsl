-- Vertex
#version 420

in vec4	vPosition;
in vec3 vNorm;
in uint vMatIndex;
in float vOccl;

out vec3 smoothNorm;
out vec4 worldPos;
out float smoothOccl;
out vec4 ambient;
out vec4 diffuse;
out vec4 specular;
out float exponent;

uniform mat4 modelToWorld;
uniform vec4 material_ambient[$maxMaterials$];
uniform vec4 material_diffuse[$maxMaterials$];
uniform vec4 material_specular[$maxMaterials$];
uniform float material_exponent[$maxMaterials$];

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec4 cameraPos;
	vec4 cameraDir;
};

void main()
{
	ambient = material_ambient[vMatIndex];
	diffuse = material_diffuse[vMatIndex];
	specular = material_specular[vMatIndex];
	exponent = material_exponent[vMatIndex];

	smoothNorm = mat3(modelToWorld) * vNorm; //!Not strictly accurate!
	smoothOccl = vOccl;
	worldPos = modelToWorld * vPosition;
	gl_Position = worldToCamera * worldPos;
}

-- Fragment
#version 420

in vec3 smoothNorm;
in vec4 worldPos;
in float smoothOccl;
in vec4 ambient;
in vec4 diffuse;
in vec4 specular;
in float exponent;

out vec4 fragColor;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec4 cameraPos;
	vec4 cameraDir;
};

layout(std140) uniform phongBlock
{
	vec4 lightPos[$maxPhongLights$];
	vec4 lightDiffuse[$maxPhongLights$];
	vec4 lightSpecular[$maxPhongLights$];
	float lightAttenuation[$maxPhongLights$];
};

layout(std140) uniform ambBlock
{
	vec4 ambLight;
};

void main()
{
	fragColor = ambient * ambLight;

	vec3 norm = normalize(smoothNorm);

	vec3 view = normalize(-vec3(cameraPos) - vec3(worldPos));

	for(int i = 0; i < $maxPhongLights$; ++i)
	{
		// Check if light is off.
		// Lights that are on must have diffuse.w and specular.w equal to 1.0
		if(lightDiffuse[i].w < 0.01 || lightSpecular[i].w < 0.01) continue;

		if(lightPos[i].w < 0.01)// Directional light source
		{
			// Diffuse lighting
			vec3 lightDir = normalize(vec3(lightPos[i]));
			float nDotL = max(dot(lightDir, norm), 0.0);
			fragColor += nDotL * diffuse * lightDiffuse[i];

			// Specular (Phong) lighting
			if(dot(lightDir, norm) > 0.0)
			{
				vec3 reflected = reflect(lightDir, norm);
				fragColor += smoothOccl * nDotL * pow(max(dot(reflected, view), 0.0), exponent) * 
					specular * lightSpecular[i];
			}
		}

		else // Point light source
		{
			vec3 toLight = vec3(lightPos[i] - worldPos);
			float denom = max(lightAttenuation[i] * length(toLight), 0.01);
			toLight = normalize(toLight);
			float nDotL = max(dot(toLight, norm), 0.0);
			// Diffuse lighting
			fragColor += nDotL * diffuse * lightDiffuse[i] / denom;
			// Specular (Phong) lighting
			if(dot(toLight, norm) > 0.0)
			{ 
				vec3 reflected = reflect(toLight, norm);
				fragColor += smoothOccl * nDotL * max(pow(dot(reflected, view), exponent), 0.0) * 
					specular * lightSpecular[i] / denom;
			}
		}
	}
}
