-- Vertex
#version 420

in vec4	vPosition;
in vec3 vNorm;
in vec2 vTexCoord;

out vec3 smoothNorm;
out vec4 worldPos;
out vec2 smoothTexCoord;

uniform mat4 modelToWorld;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec4 cameraPos;
	vec4 cameraDir;
};

void main()
{
	smoothNorm = mat3(modelToWorld) * vNorm; //!Not strictly accurate!
	smoothTexCoord = vTexCoord;
	worldPos = modelToWorld * vPosition;
	gl_Position = worldToCamera * worldPos;
}

-- Fragment
#version 420

in vec3 smoothNorm;
in vec4 worldPos;
in vec2 smoothTexCoord;

out vec4 fragColor;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec4 cameraPos;
	vec4 cameraDir;
};

layout(std140) uniform phongBlock
{
	vec4 lightPos[50];
	vec4 lightDiffuse[50];
	vec4 lightSpecular[50];
	float lightAttenuation[50];
	int nLights;
};

layout(std140) uniform ambBlock
{
	vec4 ambLight;
};

uniform sampler2D ambTex;
uniform sampler2D diffTex;
uniform sampler2D specTex;
uniform float specExp;

void main()
{
	fragColor = texture2D(ambTex, smoothTexCoord) * ambLight;

	vec3 norm = normalize(smoothNorm);

	vec3 view = normalize(-vec3(cameraPos) - vec3(worldPos));

	for(int i = 0; i < 50; ++i)
	{
		// Check if light is off.
		// Lights that are on must have diffuse.w and specular.w equal to 1.0
		if(lightDiffuse[i].w < 0.01 || lightSpecular[i].w < 0.01) continue;

		if(lightPos[i].w < 0.01)// Directional light source
		{
			// Diffuse lighting
			vec3 lightDir = normalize(vec3(lightPos[i]));
			float nDotL = max(dot(lightDir, norm), 0.0);
			fragColor += nDotL * texture2D(diffTex, smoothTexCoord) * lightDiffuse[i];

			// Specular (Phong) lighting
			if(dot(lightDir, norm) > 0.0)
			{
				vec3 reflected = reflect(lightDir, norm);
				fragColor += nDotL * pow(max(dot(reflected, view), 0.0), specExp) * 
					texture2D(specTex, smoothTexCoord) * lightSpecular[i];
			}
		}

		else // Point light source
		{
			vec3 toLight = vec3(lightPos[i] - worldPos);
			float denom = max(lightAttenuation[i] * length(toLight), 0.01);
			toLight = normalize(toLight);
			float nDotL = max(dot(toLight, norm), 0.0);
			// Diffuse lighting
			fragColor += nDotL * texture2D(diffTex, smoothTexCoord) * lightDiffuse[i] / denom;
			// Specular (Phong) lighting
			if(dot(toLight, norm) > 0.0)
			{ 
				vec3 reflected = reflect(toLight, norm);
				fragColor += nDotL * max(pow(dot(reflected, view), specExp), 0.0) * 
					texture2D(specTex, smoothTexCoord) * lightSpecular[i] / denom;
			}
		}
	}

	fragColor = texture2D(ambTex, smoothTexCoord);
}
