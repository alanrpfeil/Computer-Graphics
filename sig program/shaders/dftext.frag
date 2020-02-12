# version 330

uniform sampler2D TexId;

in  vec3 fColor;
in  vec2 fTexc;
out vec4 Color;

float contour(in float d, in float w) { return smoothstep(0.5-w,0.5+w,d); }
float samp(in vec2 uv, float w) { return contour ( texture2D(TexId,uv).a,w); }

void main() 
{
	float dist = texture2D ( TexId, fTexc ).a;
	float w = fwidth(dist) / 2.0;
	float alpha = contour ( dist, w );

	// Supersample 4 extra points
	vec2 uv = fTexc.xy;
	float dscale = 0.354; // half of 1/sqrt2
	vec2 duv = dscale * (dFdx(uv) + dFdy(uv));
	vec4 box = vec4(uv-duv, uv+duv);

	float sum = samp(box.xy,w) + samp(box.zw,w) + samp(box.xw,w) + samp(box.zy,w);
	alpha = (alpha + 0.5 * sum) / 3.0; // 1 + 0.5*4 = 3
	
	if ( alpha==0 ) discard;
	Color = vec4(fColor, alpha);
}
