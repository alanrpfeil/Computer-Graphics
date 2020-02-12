# version 330

uniform int Mode;
uniform sampler2D TexId;

in  vec4 fColor;
in  vec2 fTexc;
out vec4 Color;

float contour (in float d, in float w) { return smoothstep(0.5-w, 0.5+w, d); }
float samp (in vec2 uv, float w) { return contour(texture2D(TexId, uv).r, w); }

void ProcessSDTexture() 
{
	float dist = texture2D ( TexId, fTexc ).r;
	if ( dist==0 ) discard;

	float w = fwidth(dist) / 2.0;
	float alpha = contour(dist,w);

	// Supersample 4 extra points
	vec2 uv = fTexc.xy;
    float dscale = 0.354; // half of 1/sqrt2
    vec2 duv = dscale * ( dFdx(uv)+dFdy(uv) );
    vec4 box = vec4(uv-duv,uv+duv);

    float sum = samp(box.xy,w) + samp(box.zw,w) + samp(box.xw,w) + samp(box.zy,w);
    alpha = (alpha + 0.5 * sum)/3.0; // 1 + 0.5*4 = 3
	
    Color = vec4(fColor.rgb,fColor.a*alpha);
}

void main() 
{
	// When condition is uniform (cte for the entire pass) branching is free
	// because specific versions of the shader should be compiled.

	if ( Mode==0 ) Color=fColor; // Colored
	else 
	if ( Mode==1 ) Color=texture2D(TexId,fTexc); // Textured
	else
	if ( Mode==2 ) Color=vec4 ( fColor.rgb*texture2D(TexId,fTexc).r, fColor.a ); // Bitmapped
	else
	if ( Mode==3 ) Color=vec4 ( fColor.rgb, fColor.a*texture2D(TexId,fTexc).r ); // Masked
	else
	if ( Mode==4 ) ProcessSDTexture(); // SDFiltered
} 
