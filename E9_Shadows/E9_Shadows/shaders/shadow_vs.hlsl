#include "light_utils.hlsli"

cbuffer MatrixBuffer : register(b0)
{
	matrix worldMatrix;
	matrix viewMatrix;
	matrix projectionMatrix;
    matrix lightViewMatrices[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT];
    matrix lightProjectionMatrices[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT];
};

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 worldPosition : POSITION;
    float4 lightViewPos[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT] : TEXCOORD1; //vertex position in light view space (light2Vertex distance can be computed from this)
};


OutputType main(InputType input)
{
    OutputType output;

	// Calculate the position of the vertex against the world, view, and projection matrices.
    float4 worldPosition = mul(input.position, worldMatrix);
    output.worldPosition = worldPosition.xyz;
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertice as viewed by the light source.
    float4 lightViewPos;
    for (int i = 0; i < DIR_LIGHT_COUNT; i++)
    {        
        lightViewPos = mul(worldPosition, lightViewMatrices[i]);
        output.lightViewPos[i] = mul(lightViewPos, lightProjectionMatrices[i]);
    }
    for (int j = 0; j < SPOT_LIGHT_COUNT; j++)
    {
        lightViewPos = mul(worldPosition, lightViewMatrices[i+j]);
        output.lightViewPos[i+j] = mul(lightViewPos, lightProjectionMatrices[i+j]);
    }

    output.tex = input.tex;
    output.normal = mul(input.normal, (float3x3)worldMatrix);
    output.normal = normalize(output.normal);

	return output;
}