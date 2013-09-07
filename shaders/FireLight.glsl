/* FireLight
 * A shader intended to render fire to a cubemap using billboards,
 * for illuminating an object.
 */

-- Vertex
#version 330

in vec4 vPos;
in float vDecay;

out VertexData{
	float decay;
	} VertexOut;

uniform mat4 modelToWorld;
/* worldToObject transforms from world space to the model space of the
 *   illuminated object. It is the inverse of the object's own modelToWorld
 *   matrix.
 */
uniform mat4 worldToObject;
uniform mat4 rotation;

void main()
{
	VertexOut.decay = vDecay;
	gl_Position = rotation * worldToObject * modelToWorld * vPos;
}

-- Geometry
#version 330

uniform float bbWidth;
uniform float bbHeight;
uniform mat4 perspective;

layout(points) in;

layout(triangle_strip, max_vertices = 4) out;

in VertexData {
	float decay;
} VertexIn[];

out float decay;
out vec2 bbPos;

void main()
{
	decay = VertexIn[0].decay;
	vec3 pointPos = gl_in[0].gl_Position.xyz;

	// Find vectors in plane of billboard.
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 across = vec3(1.0, 0.0, 0.0);

	vec3 corner;
	// Bottom left vertex
	corner = pointPos - (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = perspective * vec4(corner, 1.0);
	bbPos = vec2(0, 0);
	EmitVertex();

	// Top left vertex
	corner = pointPos - (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = perspective * vec4(corner, 1.0);
	bbPos = vec2(0, 1);
	EmitVertex();

	// Bottom right vertex
	corner = pointPos + (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = perspective * vec4(corner, 1.0);
	bbPos = vec2(1, 0);
	EmitVertex();

	// Top right vertex
	corner = pointPos + (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = perspective * vec4(corner, 1.0);
	bbPos = vec2(1, 1);
	EmitVertex();

	EndPrimitive();
}

-- Fragment
#version 330

in float decay;
in vec2 bbPos;

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
	clamp(outputColor, vec4(0.9, 0.9, 0.9, 1.0), vec4(1.0, 1.0, 1.0, 1.0));
}
