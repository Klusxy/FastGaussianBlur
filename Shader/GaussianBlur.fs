#version 330 core

uniform sampler2D tex;
uniform int phase;
uniform int radius;
uniform float width;
uniform float height;

in vec2 uv;
out vec4 FragColor;

const float PI = 3.141592653589793;
const float sigma = 3.0f;
vec4 GaussianBlurV()
{
	float twoSigmaSqu = 2 * sigma * sigma;
	int r = radius; //int(ceil(sigma * 2.57));
	float allWeight = 0.0f;

	vec4 col = vec4(0.0f, 0.0f, 0.0f, 0.0f);
	for (int iy = -r; iy < (r + 1); iy++)
	{
		float weight = 1.0f / (PI * twoSigmaSqu) * exp(-(iy * iy) / twoSigmaSqu);
		vec2 offset = vec2(0.0f, 1.0f / height * iy);
		vec2 uv_offset = uv + offset;
		uv_offset.y = clamp(uv_offset.y, 0.0f, 1.0f);
		col += texture2D(tex, uv_offset) * weight;
		allWeight += weight;
	}
	col = col / allWeight;

	return col;
}

vec4 GaussianBlurH()
{
	float twoSigmaSqu = 2 * sigma * sigma;
	int r = radius; //int(ceil(sigma * 2.57));
	float allWeight = 0.0f;
	
	vec4 col = vec4(0.0f, 0.0f, 0.0f, 0.0f);//error X3014: incorrect number of arguments to numeric-type constructor
	for (int ix = -r; ix < (r + 1); ix++)
	{
		float weight = 1.0f / (PI * twoSigmaSqu) * exp(-(ix * ix) / twoSigmaSqu);
		vec2 offset = vec2(1.0f / width * ix, 0.0f);
		vec2 uv_offset = uv + offset;
		uv_offset.x = clamp(uv_offset.x, 0.0f, 1.0f);
		col += texture2D(tex, uv_offset) * weight;//get_texture0(uv_offset) * weight;// raw_tex.SampleLevel(SampleType, tc + offset , 0) * weight;
		allWeight += weight;
	}
	col = col / allWeight;
	
	return col;
}

void main()
{
	if(phase == 0)
	{
		FragColor = texture(tex, uv);
	}
	else if(phase == 1) 
	{
		FragColor = GaussianBlurH();
	}
	else if(phase == 2) 
	{
		FragColor = GaussianBlurV();
	}
	else
	{
		FragColor = vec4(1,0,0,1);
	}
}