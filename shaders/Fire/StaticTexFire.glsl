/* Sparks
 * Simple billboarding/texturing program intended to 
 * render billboards with a single texture and decaying
 * alpha (e.g. sparks/embers rising from a fire)
 */

-- Vertex
#version 430

uniform mat4 modelToWorld;

in vec4 vPos;
in float vDecay;

out VertexData{
	float decay;
	} VertexOut;

void main()
{
	VertexOut.decay = vDecay;
	gl_Position = modelToWorld * vPos;
}

-- Geometry
#version 430

uniform float bbWidth;
uniform float bbHeight;

layout(std140) uniform cameraBlock
{
	mat4 worldToCamera;
	vec4 cameraPos;
	vec4 cameraDir;
};

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
	vec3 toCamera = normalize(-vec3(cameraDir));

	// Find vectors in plane of billboard.
	vec3 up = vec3(0.0, 1.0, 0.0);
	vec3 across = normalize(cross(up, toCamera));
	up = normalize(cross(toCamera, across));

	vec3 corner;
	// Bottom left vertex
	corner = pointPos - (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(0, 0);
	EmitVertex();

	// Top left vertex
	corner = pointPos - (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(0, 1);
	EmitVertex();

	// Bottom right vertex
	corner = pointPos + (0.5*bbWidth*across) - (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(1, 0);
	EmitVertex();

	// Top right vertex
	corner = pointPos + (0.5*bbWidth*across) + (0.5*bbHeight*up);
	gl_Position = worldToCamera * vec4(corner, 1.0);
	height = gl_Position.y;
	texCoord = vec2(1, 1);
	EmitVertex();

	EndPrimitive();
}
-- Fragment
#version 430

in float height;
in vec2 texCoord;

in float decay;

out vec4 outputColor;

uniform sampler2D bbTexture;
uniform sampler2D decayTexture;
uniform float globalAlpha;

void main()
{
	vec2 fromCenter = (texCoord - vec2(0.5, 0.5)) * 2.0;
	float centerDistSq = clamp(dot(fromCenter, fromCenter), 0.0, 1.0);

	float fade = (1 - centerDistSq);

	float opacity = fade*fade*fade * ( decay < 0.3 ? decay : (1 - decay) );

	float alpha = texture(bbTexture, texCoord).a * opacity;
	if (alpha < 0.05) discard;
	alpha *= globalAlpha;
	outputColor = vec4(texture(decayTexture, vec2(decay, 0.0)).xyz, alpha);
}
