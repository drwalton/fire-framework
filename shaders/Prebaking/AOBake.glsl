/* AOBake
 * Shader intended for rendering per-vertex AO coefficients
 * to a framebuffer.
 */

--Vertex
#version 430

in vec2 vTexCoord;
in float vOccl;

out float smoothOccl;

void main()
{
	smoothOccl = vOccl;
	//Transform tex coords to Normalized Device Coordinates.
	vec2 NDC = (vTexCoord * 2.0) - vec2(1.0, 1.0);
	NDC.y = -NDC.y;
	gl_Position = vec4(NDC, 0.0, 1.0);
}

--Fragment
#version 430

in float smoothOccl;

out vec4 fragColor;

void main()
{
	float bright = smoothOccl;
	fragColor = vec4(bright, bright, bright, 1.0);
}
