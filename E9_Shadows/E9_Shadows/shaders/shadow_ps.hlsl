#include "light_utils.hlsli" //covers t0-2, b1 and s0 registers

Texture2D shaderTexture : register(t3);
Texture2D normalTexture : register(t4);
SamplerState linearSampler  : register(s1);

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
	float3 worldPosition : POSITION;    
    float4 lightViewPos[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT] : TEXCOORD1; //vertex position in light view space (light2Vertex distance can be computed from this)
};

float3 ComputeNormalTangentToWorld(float3 normal, float3 tangent, float3 bitangent, float2 uv)
{
    float3 texNormal = normalTexture.Sample(linearSampler, uv).xyz * 2.0f - 1.0f; //(0,1) to (-1,1)

    float3 N = normal;
    float3 T = tangent; //normalize(tangent - dot(tangent, N) * N);
    float3 B = bitangent; //cross(N, T);
    
    float3x3 TBN = float3x3(T, B, N); //tangent to world space matrix (these 3 vectors are taken as columns (principal axes))
    
    return mul(texNormal, TBN);
}

float4 main(InputType input) : SV_TARGET
{    
    //input.normal = ComputeNormalTangentToWorld(normalize(input.normal), normalize(input.tangent), input.bitangent, input.tex);
    float3 bumpedNormal = ComputeNormalTangentToWorld(normalize(input.normal), normalize(input.tangent), normalize(input.bitangent), input.tex);
    
    //return float4(normalize(input.normal),1);
    //return float4(bumpedNormal, 1);
    
    float4 textureColor = shaderTexture.Sample(linearSampler, input.tex);
    float4 finalSpecularColor;
    float4 finalLightColor = applyLightingAndShadows(normalize(bumpedNormal), input.lightViewPos, input.worldPosition, camWorldPos, finalSpecularColor);
    
    float4 finalColor = textureColor * finalLightColor + finalSpecularColor;
    return finalColor;
}