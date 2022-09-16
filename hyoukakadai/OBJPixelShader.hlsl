#include"OBJShaderHeader.hlsli"

Texture2D<float4>tex:register(t0);
SamplerState smp:register(s0);

float4 main(VSOutput input) : SV_TARGET
{
	float3 light = normalize(float3(1,-1,1));
	float light_diffuse = saturate(dot(-light, input.normal));
	float3 shade_color;
	shade_color = m_ambient;
	shade_color += m_diffuse * light_diffuse + 0.6;
	float4 texcolor = tex.Sample(smp, input.uv);
	//return color * float4(0.8 + brightness, 0.5 + brightness, 0.2 + brightness, 1);
	return float4 (texcolor.rgb * shade_color,texcolor.a * m_alpha)*color;
}