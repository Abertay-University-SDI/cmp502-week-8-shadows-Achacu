// Tessellation pixel shader
// Output colour passed to stage.
#include "light_utils.hlsli" //covers t0-2, b0-1 and s0 registers
#include "heightmap.hlsli" //covers t3 and s1

Texture2D paintTexturesMap : register(t4);
Texture2D diffuseTextures[4] : register(t5);
SamplerState linearSampler : register(s2);

cbuffer HeightmapPixelBuffer : register(b2)
{
    float4 diffuseTexScales;
    int samplesPerTexel;
    float3 padding3;
};

struct InputType
{
    float4 position : SV_POSITION;
    float4 colour : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
    float3 worldPosition : POSITION;
    float4 lightViewPos[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT] : TEXCOORD1; //vertex position in light view space (light2Vertex distance can be computed from this)
};

float3 CalcNormal(float2 uv)
{
    float texWidth, numOfLvls;
	
    heightTex.GetDimensions(0, texWidth, texWidth, numOfLvls); //square tex so width = height
    float uvStep = samplesPerTexel / texWidth; //uv (0 to 1): uvStep = 1.0 / texWidth; (we sample once per texture -> pixel=texel), uvStep = 1.0 / (texWidth/3) = 3.0 / texWidth; (we sample 3 times per texel)
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
    float3 normal = CalcNormal(input.tex);
    
    
    float4 tex0Color = diffuseTextures[0].Sample(linearSampler, input.tex * diffuseTexScales[0]);

        
    //tri-planar mapping: instead of sampling by uv it is done by world space coords in the principal planes
    float4 tex1ColorX = diffuseTextures[1].Sample(linearSampler, diffuseTexScales[1]*input.worldPosition.yz);
    float4 tex1ColorY = diffuseTextures[1].Sample(linearSampler, diffuseTexScales[1]*input.worldPosition.xz);
    float4 tex1ColorZ = diffuseTextures[1].Sample(linearSampler, diffuseTexScales[1]*input.worldPosition.xy);
    
    //the final tri-planar map color is a blend of all 3 weighted by the normal
    float3 blend = normalize(abs(normal)); //normalized so it amounts to 1 and abs since the sign is not important
    float4 tex1Color = blend.x * tex1ColorX + blend.y * tex1ColorY + blend.z * tex1ColorZ;

    
    float4 tex2Color = diffuseTextures[2].Sample(linearSampler, input.tex * diffuseTexScales[2]);
    
    
    float3 paintWeights = normalize(paintTexturesMap.Sample(linearSampler, input.tex));
    float4 finalTexColor = paintWeights.r * tex0Color + paintWeights.g * tex1Color + paintWeights.b * tex2Color;
    //finalTexColor = lerp(textureColor2, textureColor, normal.y);
    
    float4 finalSpecularColor;
    float4 finalLightColor = applyLightingAndShadows(normal, input.lightViewPos, input.worldPosition, camWorldPos, finalSpecularColor);
    
    float4 finalColor = finalTexColor * finalLightColor + finalSpecularColor;
    return finalColor;            
}