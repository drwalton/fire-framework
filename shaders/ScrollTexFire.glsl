-- Vertex
#version 330

uniform mat4 modelToWorld;

layout(location = 0)in vec4 vPos;
layout(location = 1)in float vDecay;
layout(location = 2)in float randTex;

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

uniform vec3 cameraDir;

layout(points) in;

layout(triangle_strip, max_vertices = 4) out;

in VertexData {
	float decay;
	float randTex;
} VertexIn[];

out float decay;

out float height;
out vec2 texCoord;
out vec2 bbPos;

void main()
{
	decay = VertexIn[0].decay;
	vec3 pointPos = gl_in[0].gl_Position.xyz;
	vec3 toCamera = normalize( cameraDir);

	// Find vectors in plane of billboard.
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 across = normalize(cross(up, toCamera));
	up = normalize(cross(toCamera, across));
	

	float texLeft = VertexIn[0].randTex * 0.7;
	float texRight = texLeft + 0.3;
	float texBottom = VertexIn[0].decay * 0.85;
	float texTop = texBottom + 0.15;

	vec3 corner;
	// Bottom left vertex
	corner = pointPos - (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(texLeft, texBottom);
	bbPos = vec2(0, 0);
	EmitVertex();

	// Top left vertex
	corner = pointPos - (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(texLeft, texTop);
	bbPos = vec2(0, 1);
	EmitVertex();

	// Bottom right vertex
	corner = pointPos + (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(texRight, texBottom);
	bbPos = vec2(1, 0);
	EmitVertex();

	// Top right vertex
	corner = pointPos + (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(texRight, texTop);
	bbPos = vec2(1, 1);
	EmitVertex();

	EndPrimitive();
}

-- Fragment
#version 330

in float height;
in vec2 texCoord;
in vec2 bbPos;

in float decay;

out vec4 outputColor;

uniform sampler2D bbTexture;
uniform sampler2D decayTexture;

void main()
{
	vec4 blue = vec4(0.0251, 0.2157, 1.0, 0.2);
	vec4 yellow = vec4(1.0, 0.8235, 0.2157, 1.0);
	vec4 red = vec4(1.0, 0.6784, 0.2157, 0.0);
	float t1 = 0.3;
	float t2 = 0.9;

	float fromEdge = bbPos.x;
	fromEdge = min(fromEdge, 1 - bbPos.x);
	fromEdge = min(fromEdge, bbPos.y);
	fromEdge = min(fromEdge, 1 - bbPos.y);

	float opacity = 2 * fromEdge * ( decay < 0.3 ? decay : (1 - decay) );

	float i = texture2D(bbTexture, texCoord).a * opacity;
	if (i < 0.05) discard;
	outputColor = vec4(texture2D(decayTexture, vec2(decay, 0.0)).xyz, i);
}

float min(float x, float y)
{
	return x <= y ? x : y;
}
