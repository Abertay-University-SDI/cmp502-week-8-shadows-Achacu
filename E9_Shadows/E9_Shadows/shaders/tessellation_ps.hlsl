// Tessellation pixel shader
// Output colour passed to stage.
#include "light_utils.hlsli" //covers t0-2, b1 and s0 registers
#include "heightmap.hlsli" //covers t3 and s1

Texture2D diffuseTextures[4] : register(t4);
SamplerState diffuseSampler : register(s2);

struct InputType
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float4 lightViewPos[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT] : TEXCOORD1; //vertex position in light view space (light2Vertex distance can be computed from this)
};

float3 CalcNormal(float2 uv, out float uvStep)
{
    float texWidth, numOfLvls;
	
    heightTex.GetDimensions(0, texWidth, texWidth, numOfLvls); //square tex so width = height
    uvStep = 3.0f / texWidth; //uv (0 to 1): uvStep = 1.0 / texWidth; (we sample once per texture -> pixel=texel), uvStep = 1.0 / (texWidth/3) = 3.0 / texWidth; (we sample 3 times per texel)
    float vertexWorldStep = 100.0 * uvStep; //distance between this vertex and the next (100x100 m plane in this case)
	
	//get neighboring heights
    float heightU = GetHeight(float2(uv.x, uv.y + uvStep));
    float heightD = GetHeight(float2(uv.x, uv.y - uvStep));
    float heightR = GetHeight(float2(uv.x + uvStep, uv.y));
    float heightL = GetHeight(float2(uv.x - uvStep, uv.y));
    float height = GetHeight(float2(uv.x, uv.y));
	
	//get vectors from current elevated vertex to neighboring one
    float3 tanR = normalize(float3(vertexWorldStep, heightR - height, 0));
    float3 tanL = normalize(float3(-vertexWorldStep, heightL - height, 0));
    float3 biTanU = normalize(float3(0, heightU - height, vertexWorldStep));
    float3 biTanD = normalize(float3(0, heightD - height, -vertexWorldStep));

	//get normal by calculating cross products (keep LEFT-hand rule in mind for the sign)
    float3 normalRU = normalize(-cross(tanR, biTanU));
    float3 normalLU = normalize(cross(tanL, biTanU));
    float3 normalRD = normalize(cross(tanR, biTanD));
    float3 normalLD = normalize(-cross(tanL, biTanD));
		
    return (normalRU + normalLU + normalRD + normalLD) * 0.25f; //average of the 4 normals
	
	
	//alternative way to do it (not using current height and just the neighboring ones)
    float3 tanRL = normalize(float3(2 * vertexWorldStep, heightR - heightL, 0));
    float3 tanUD = normalize(float3(0, heightU - heightD, 2 * vertexWorldStep));
    return -cross(tanRL, tanUD);
}

float4 main(InputType input) : SV_TARGET
{
    float outUVStep;
    
    float4 finalTexColor;
    float4 textureColor = diffuseTextures[0].Sample(diffuseSampler, input.tex * 20);
    float4 textureColor2 = diffuseTextures[1].Sample(diffuseSampler, input.tex * 20);   
    
    float3 normal = CalcNormal(input.tex, outUVStep);
    
    finalTexColor = lerp(textureColor, textureColor2, normal.y);
    
    float4 finalSpecularColor;
    float4 finalLightColor = applyLightingAndShadows(normal, input.lightViewPos, input.worldPosition, camWorldPos, finalSpecularColor);
    
    float4 finalColor = finalTexColor * finalLightColor + finalSpecularColor;
    return finalColor;            
}