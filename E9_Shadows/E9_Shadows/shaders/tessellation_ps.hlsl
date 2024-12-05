// Tessellation pixel shader
// Output colour passed to stage.
#include "heightmap.hlsli"

struct InputType
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float3 CalcNormal(float2 uv, out float uvStep)
{
    float texWidth, numOfLvls;
	
    heightTex.GetDimensions(0, texWidth, texWidth, numOfLvls); //square tex so width = height
    uvStep = 3.0f / texWidth; //uv (0 to 1): uvStep = 1.0 / texWidth; (we sample once per texture pixel=texel), uvStep = 1.0 / (texWidth/3) = 3.0 / texWidth; (we sample 3 times per texel)
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


// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLighting(float3 lightDirection, float3 normal, float4 diffuse)
{
    float intensity = saturate(dot(normal, lightDirection));
    float4 colour = saturate(diffuse * intensity);
    return colour;
}
float4 main(InputType input) : SV_TARGET
{
    float outUVStep;
    return calculateLighting(float3(0.7, 0.7, 0), CalcNormal(input.tex, outUVStep), input.colour);
}