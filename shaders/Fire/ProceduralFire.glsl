/* ProceduralFire
 * A shader intended to render fire using billboards, 
 * which are textured procedurally using Perlin noise.
 * Makes use of code from the webgl-noise project:
 *   https://github.com/ashima/webgl-noise
 * Distributed under the MIT license 
 * See: ../licenses/webgl-noise.txt
 */

-- Vertex
#version 330

uniform mat4 modelToWorld;

in vec4 vPos;
in float vDecay;
in float vRandTex;

out VertexData{
	float decay;
	float randTex;
	} VertexOut;

void main()
{
	VertexOut.decay = vDecay;
	VertexOut.randTex = vRandTex;
	gl_Position = modelToWorld * vPos;
}

-- Geometry
#version 330

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
	vec3 toCamera = normalize(-vec3(cameraDir));

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

#define NOISE_DENSITY_X 16.0
#define NOISE_DENSITY_Y  8.0

in float height;
in vec2 texCoord;
in vec2 bbPos;

in float decay;

out vec4 outputColor;

uniform sampler2D decayTexture;

/* The following procedural noise generation code 
 *   was developed by Stefan Gustavson 
 *   (stefan.gustavson@liu.se)
 *   as part of the webgl-noise project:
 *   https://github.com/ashima/webgl-noise
 * ==============================================
 * Copyright (c) 2011 Stefan Gustavson. All rights reserved.
 * Distributed under the MIT license (see ../licenses/webgl-noise.txt).
 */

vec4 mod289(vec4 x)
{
  return x - floor(x * (1.0 / 289.0)) * 289.0;
}

vec4 permute(vec4 x)
{
  return mod289(((x*34.0)+1.0)*x);
}

vec4 taylorInvSqrt(vec4 r)
{
  return 1.79284291400159 - 0.85373472095314 * r;
}

vec2 fade(vec2 t) {
  return t*t*t*(t*(t*6.0-15.0)+10.0);
}

// Classic Perlin noise
float cnoise(vec2 P)
{
  vec4 Pi = floor(P.xyxy) + vec4(0.0, 0.0, 1.0, 1.0);
  vec4 Pf = fract(P.xyxy) - vec4(0.0, 0.0, 1.0, 1.0);
  Pi = mod289(Pi); // To avoid truncation effects in permutation
  vec4 ix = Pi.xzxz;
  vec4 iy = Pi.yyww;
  vec4 fx = Pf.xzxz;
  vec4 fy = Pf.yyww;

  vec4 i = permute(permute(ix) + iy);

  vec4 gx = fract(i * (1.0 / 41.0)) * 2.0 - 1.0 ;
  vec4 gy = abs(gx) - 0.5 ;
  vec4 tx = floor(gx + 0.5);
  gx = gx - tx;

  vec2 g00 = vec2(gx.x,gy.x);
  vec2 g10 = vec2(gx.y,gy.y);
  vec2 g01 = vec2(gx.z,gy.z);
  vec2 g11 = vec2(gx.w,gy.w);

  vec4 norm = taylorInvSqrt(vec4(dot(g00, g00), dot(g01, g01), dot(g10, g10), dot(g11, g11)));
  g00 *= norm.x;
  g01 *= norm.y;
  g10 *= norm.z;
  g11 *= norm.w;

  float n00 = dot(g00, vec2(fx.x, fy.x));
  float n10 = dot(g10, vec2(fx.y, fy.y));
  float n01 = dot(g01, vec2(fx.z, fy.z));
  float n11 = dot(g11, vec2(fx.w, fy.w));

  vec2 fade_xy = fade(Pf.xy);
  vec2 n_x = mix(vec2(n00, n01), vec2(n10, n11), fade_xy.x);
  float n_xy = mix(n_x.x, n_x.y, fade_xy.y);
  return 2.3 * n_xy;
}

/* END of code from webgl-noise project.
 * Subsequent code developed by the author.
 */

float min(float x, float y)
{
	return x <= y ? x : y;
}

void main()
{
	vec2 fromCenter = (bbPos - vec2(0.5, 0.5)) * 2.0;
	float centerDistSq = clamp(dot(fromCenter, fromCenter), 0.0, 1.0);

	float fade = (1 - centerDistSq);

	float opacity = fade*fade*fade * ( decay < 0.3 ? decay : (1 - decay) );

	float alpha = (1.0 + cnoise(vec2(texCoord.x * NOISE_DENSITY_X, texCoord.y * NOISE_DENSITY_Y))) * opacity / 2;
	if (alpha < 0.05) discard;
	outputColor = vec4(texture2D(decayTexture, vec2(decay, 0.0)).xyz, alpha);
}
