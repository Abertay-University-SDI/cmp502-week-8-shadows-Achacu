#include "light_utils.hlsli" //covers t0-2, b1 and s0 registers

Texture2D shaderTexture : register(t3);
SamplerState diffuseSampler  : register(s1);

cbuffer CameraBuffer : register(b1)
{
    float3 camWorldPos;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : POSITION;    
    float4 lightViewPos[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT] : TEXCOORD1; //vertex position in light view space (light2Vertex distance can be computed from this)
};

float4 main(InputType input) : SV_TARGET
{    
    float4 textureColor = shaderTexture.Sample(diffuseSampler, input.tex);
    float4 finalSpecularColor;
    float4 finalLightColor = applyLightingAndShadows(normalize(input.normal), input.lightViewPos, input.worldPosition, camWorldPos, finalSpecularColor);
    
    float4 finalColor = textureColor * finalLightColor + finalSpecularColor;
    return finalColor;
}