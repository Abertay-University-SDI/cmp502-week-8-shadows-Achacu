// Tessellation domain shader
// After tessellation the domain shader processes the all the vertices
#include "light_utils.hlsli" //matrix buffer in b0, light matrix buffer in b1
#include "heightmap.hlsli"

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct InputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct OutputType
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float4 lightViewPos[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT] : TEXCOORD1; //vertex position in light view space (light2Vertex distance can be computed from this)
};

[domain("quad")]
OutputType main(ConstantOutputType input, float2 uvwCoord : SV_DomainLocation, const OutputPatch<InputType, 4> patch)
{
    float3 vertexPosition;
    OutputType output;
 
    // Determine the position of the new vertex.
    float3 v01 = lerp(patch[0].position, patch[1].position, uvwCoord.y); //top left to bottom left vertex
    float3 v32 = lerp(patch[3].position, patch[2].position, uvwCoord.y); //top right to bottom right vertex
    vertexPosition = lerp(v01, v32, uvwCoord.x); //final vertex position depends on which side (left vs right edge) is closer and by how much
        
    //vertexPosition.y += A * sin(vertexPosition.x * freq + time * speed) + A * sin(vertexPosition.z * freq + time * speed);

    //output.normal = float3(-A * freq * cos(freq * vertexPosition.x + speed * time), 1, -A * freq * cos(freq * vertexPosition.z + speed * time));    
    
    float2 t01 = lerp(patch[0].tex, patch[1].tex, uvwCoord.y); //top left to bottom left vertex
    float2 t32 = lerp(patch[3].tex, patch[2].tex, uvwCoord.y); //top right to bottom right vertex
    output.tex = lerp(t01, t32, uvwCoord.x); //final tex coord depends on which side (left vs right edge) is closer and by how much
    vertexPosition.y += GetHeight(output.tex);
    
    // Send the input color into the pixel shader.
    output.colour = patch[0].colour;
    
    // Calculate the position of the new vertex against the world, view, and projection matrices.
    float4 worldPosition = mul(float4(vertexPosition, 1.0f), worldMatrix);
    output.worldPosition = worldPosition.xyz;

   	// Calculate the position of the vertex as viewed and projected from each light source
    calculateLightViewPositions(worldPosition, output.lightViewPos);
    
    output.position = mul(worldPosition, viewMatrix);
    output.position = mul(output.position, projectionMatrix);
    return output;
}

