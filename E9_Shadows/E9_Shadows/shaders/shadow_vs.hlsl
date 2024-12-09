#include "light_utils.hlsli" //matrix buffer in b0, light matrix buffer in b1

struct InputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 bitangent : BINORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
	float3 tangent : TANGENT;
	float3 bitangent : BINORMAL;
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
    //float3 viewPos = output.position.xyz;
    output.position = mul(output.position, projectionMatrix);
    
	// Calculate the position of the vertex as viewed and projected from each light source
    calculateLightViewPositions(worldPosition, output.lightViewPos);
    
    output.tex = input.tex;
    
    output.normal = normalize(mul(input.normal, (float3x3) worldMatrix));
    output.tangent = normalize(mul(input.tangent, (float3x3) worldMatrix));
    output.bitangent = normalize(mul(input.bitangent, (float3x3) worldMatrix));
    
    //TODO: remove
    //output.normal = input.normal;
    //output.tangent = input.tangent;
    //output.bitangent = input.bitangent;
	return output;
}