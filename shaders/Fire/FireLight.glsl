/* FireLight
 * A shader intended to render fire to a cubemap using billboards,
 * for illuminating an object.
 */

-- Vertex
#version 330

in vec4 vPos;
in float vDecay;
in float vRandTex;

out VertexData{
	float decay;
	float randTex;
	} VertexOut;

uniform mat4 modelToWorld;
/* worldToObject transforms from world space to the model space of the
 *   illuminated object. It is the inverse of the object's own modelToWorld
 *   matrix.
 */
uniform mat4 worldToObject;

void main()
{
	VertexOut.decay = vDecay;
	VertexOut.randTex = vRandTex;
	gl_Position = worldToObject * modelToWorld * vPos;
}

-- Geometry
#version 330

layout(points) in;
layout(triangle_strip, max_vertices = 24) out; //6 triangle strips.

in VertexData {
	float decay;
	float randTex;
} VertexIn[];

out float decay;
out vec2 texCoord;
out vec2 bbPos;

uniform float bbWidth;
uniform float bbHeight;

// Camera matrices for the 6 cube faces.
const float zNear = 0.01;
const float zFar  = 50.0;

const float invRtTwo = 0.70710678118654; //\frac{1}{\sqrt{2}}

// A rotation of PI / 4 radians CCW about +ve z axis.
const mat4 lookRight = mat4(
	 invRtTwo, invRtTwo, 0.0, 0.0,
	-invRtTwo, invRtTwo, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.0, 0.0, 0.0, 1.0);
// A rotation of PI / 4 radians CCW about +ve x axis.
const mat4 lookDown = mat4(
	1.0, 0.0, 0.0, 0.0,
	0.0,  invRtTwo, invRtTwo, 0.0,
	0.0, -invRtTwo, invRtTwo, 0.0,
	0.0, 0.0, 0.0, 1.0);

const mat4 posZMat = mat4(
	1.0, 0.0, 0.0, 0.0, 
	0.0, 1.0, 0.0, 0.0,
	0.0, 0.0, -(zFar / (zFar - zNear)), -(zFar * zNear) / (zFar - zNear),
	0.0, 0.0, -1.0, 0.0);

const mat4 posXMat = posZMat * lookRight;
const mat4 negXMat = posZMat * lookRight * lookRight * lookRight;
const mat4 posYMat = posZMat * lookDown * lookDown * lookDown;
const mat4 negYMat = posZMat * lookDown;

const mat4 negZMat = posZMat * lookRight * lookRight;

void main()
{
	decay = VertexIn[0].decay;
	vec3 pointPos = gl_in[0].gl_Position.xyz;

	/* Find positions of billboard corners in model space */

	// Find vectors in plane of billboard.
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 across = normalize(cross(up, -pointPos));
	up = normalize(cross(-pointPos, across));
	

	float texLeft = VertexIn[0].randTex * 0.7;
	float texRight = texLeft + 0.3;
	float texBottom = VertexIn[0].decay * 0.85;
	float texTop = texBottom + 0.15;

	// Bottom left vertex
	vec4 blPos = vec4(pointPos - (0.5*bbWidth*across) - (0.5*bbHeight*up), 1.0);
	vec2 blTex = vec2(texLeft, texBottom);
	vec2 blBBPos = vec2(0, 0);

	// Top left vertex
	vec4 tlPos = vec4(pointPos - (0.5*bbWidth*across) + (0.5*bbHeight*up), 1.0);
	vec2 tlTex = vec2(texLeft, texTop);
	vec2 tlBBPos = vec2(0, 1);

	// Bottom right vertex
	vec4 brPos = vec4(pointPos + (0.5*bbWidth*across) - (0.5*bbHeight*up), 1.0);
	vec2 brTex = vec2(texRight, texBottom);
	vec2 brBBPos = vec2(1, 0);

	// Top right vertex
	vec4 trPos = vec4(pointPos + (0.5*bbWidth*across) + (0.5*bbHeight*up), 1.0);
	vec2 trTex = vec2(texRight, texTop);
	vec2 trBBPos = vec2(1, 1);

	/* Output a quad to each face of the cube. */

	//Positive x face (layer 0)
	gl_Layer = 0;

	gl_Position = posXMat * blPos;
	texCoord = blTex;
	bbPos = blBBPos;
	EmitVertex();

	gl_Position = posXMat * tlPos;
	texCoord = tlTex;
	bbPos = tlBBPos;
	EmitVertex();

	gl_Position = posXMat * brPos;
	texCoord = brTex;
	bbPos = brBBPos;
	EmitVertex();

	gl_Position = posXMat * trPos;
	texCoord = trTex;
	bbPos = trBBPos;
	EmitVertex();

	EndPrimitive();

	//Negative x face (layer 1)
	gl_Layer = 1;

	gl_Position = negXMat * blPos;
	texCoord = blTex;
	bbPos = blBBPos;
	EmitVertex();

	gl_Position = negXMat * tlPos;
	texCoord = tlTex;
	bbPos = tlBBPos;
	EmitVertex();

	gl_Position = negXMat * brPos;
	texCoord = brTex;
	bbPos = brBBPos;
	EmitVertex();

	gl_Position = negXMat * trPos;
	texCoord = trTex;
	bbPos = trBBPos;
	EmitVertex();

	EndPrimitive();

	//Positive y face (layer 2)
	gl_Layer = 2;

	gl_Position = posYMat * blPos;
	texCoord = blTex;
	bbPos = blBBPos;
	EmitVertex();

	gl_Position = posYMat * tlPos;
	texCoord = tlTex;
	bbPos = tlBBPos;
	EmitVertex();

	gl_Position = posYMat * brPos;
	texCoord = brTex;
	bbPos = brBBPos;
	EmitVertex();

	gl_Position = posYMat * trPos;
	texCoord = trTex;
	bbPos = trBBPos;
	EmitVertex();

	EndPrimitive();

	//Negative y face (layer 3)
	gl_Layer = 3;

	gl_Position = negYMat * blPos;
	texCoord = blTex;
	bbPos = blBBPos;
	EmitVertex();

	gl_Position = negYMat * tlPos;
	texCoord = tlTex;
	bbPos = tlBBPos;
	EmitVertex();

	gl_Position = negYMat * brPos;
	texCoord = brTex;
	bbPos = brBBPos;
	EmitVertex();

	gl_Position = negYMat * trPos;
	texCoord = trTex;
	bbPos = trBBPos;
	EmitVertex();

	EndPrimitive();

	//Positive z face (layer 4)
	gl_Layer = 4;

	gl_Position = posZMat * blPos;
	texCoord = blTex;
	bbPos = blBBPos;
	EmitVertex();

	gl_Position = posZMat * tlPos;
	texCoord = tlTex;
	bbPos = tlBBPos;
	EmitVertex();

	gl_Position = posZMat * brPos;
	texCoord = brTex;
	bbPos = brBBPos;
	EmitVertex();

	gl_Position = posZMat * trPos;
	texCoord = trTex;
	bbPos = trBBPos;
	EmitVertex();

	EndPrimitive();

	//Negative z face (layer 5)
	gl_Layer = 5;

	gl_Position = negZMat * blPos;
	texCoord = blTex;
	bbPos = blBBPos;
	EmitVertex();

	gl_Position = negZMat * tlPos;
	texCoord = tlTex;
	bbPos = tlBBPos;
	EmitVertex();

	gl_Position = negZMat * brPos;
	texCoord = brTex;
	bbPos = brBBPos;
	EmitVertex();

	gl_Position = negZMat * trPos;
	texCoord = trTex;
	bbPos = trBBPos;
	EmitVertex();

	EndPrimitive();
}

-- Fragment
#version 330

in vec2 texCoord;
in vec2 bbPos;

in float decay;

out vec4 outputColor;

uniform sampler2D decayTexture;
uniform float globalAlpha;

void main()
{
	vec2 fromCenter = (bbPos - vec2(0.5, 0.5)) * 2.0;
	float centerDistSq = clamp(dot(fromCenter, fromCenter), 0.0, 1.0);

	float fade = (1 - centerDistSq);

	float alpha = fade*fade*fade * ( decay < 0.3 ? decay : (1 - decay) );

	if (alpha < 0.05) discard;
	alpha *= globalAlpha;
	outputColor = vec4(texture2D(decayTexture, vec2(decay, 0.0)).xyz, alpha);
}
