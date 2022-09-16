#include"Sprite.hlsli"
Texture2D<float4>tex:register(t0);

SamplerState smp:register(s0);
float4 main(Output input) : SV_TARGET
{
	//float3 light = normalize(float3(1,-1,1));
	//float diffuse = saturate(dot(-light, input.normal));
	//float brightness = diffuse + 1.0f;
	float4 texcolor = float4(tex.Sample(smp, input.uv));
	return float4(texcolor.rgb, 0) * color;
	//return tex.Sample(smp,input.uv) * color;
	//return float4(1.0f, 1.0f, 1.0f, 1.0f);
}