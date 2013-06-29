-- Vertex
#version 330

uniform mat4 modelToWorld;

layout(location = 0)in vec4 vPos;
layout(location = 1)in float vDecay;

out VertexData{
	float decay;
	} VertexOut;

void main()
{
	VertexOut.decay = vDecay;
	gl_Position = modelToWorld * vPos;
}

-- Geometry
#version 330

uniform float bbWidth;
uniform float bbHeight;

uniform mat4 worldToCamera;

uniform vec3 cameraPos;

layout(points) in;

layout(triangle_strip, max_vertices = 4) out;

in VertexData {
	float decay;
} VertexIn[];

out float decay;

out float height;
out vec2 texCoord;

void main()
{
	decay = VertexIn[0].decay;
	vec3 pointPos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize(pointPos - cameraPos);

	// Find vectors in plane of billboard.
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 across = normalize(cross(up, toCamera));
	

	vec3 corner;
	// Bottom left vertex
	corner = pointPos - (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(0.0, 0.0);
	EmitVertex();

	// Top left vertex
	corner = pointPos - (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(0.0, 1.0);
	EmitVertex();

	// Bottom right vertex
	corner = pointPos + (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(1.0, 0.0);
	EmitVertex();

	// Top right vertex
	corner = pointPos + (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(1.0, 1.0);
	EmitVertex();

	EndPrimitive();
}

-- Fragment
#version 330

in float height;
in vec2 texCoord;

in float decay;

out vec4 outputColor;

uniform sampler2D bbTexture;
uniform sampler2D decayTexture;

void main()
{
	float opacity = decay < 0.7 ? 1.0 : (1 - decay);
	float i = texture2D(bbTexture, texCoord).a * opacity;
	if (i < 0.1) discard;
	outputColor = vec4(texture(decayTexture, vec2(decay, 0.0)).xyz, i);
}
