#include "light_utils.hlsli"

Texture2D shaderTexture : register(t0);
SamplerState diffuseSampler  : register(s0);

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
    float4 lightViewPos[DIR_LIGHT_COUNT] : TEXCOORD1; //vertex position in light view space (light2Vertex distance can be computed from this)
};

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f; //low value -> self-shadowing artifacts, high value -> some parts of shadow are lost
    float4 finalLightColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 finalSpecularColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 finalColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColor = shaderTexture.Sample(diffuseSampler, input.tex);

	// Calculate the projected texture coordinates.
    float3 normal = normalize(input.normal);
    // Shadow test. Is or isn't in shadow
    
    float3 normalizedLightDir, viewDir;
    float2 pTexCoord;
    DirectionalLight dLight;
    viewDir = normalize(camWorldPos - input.worldPosition);  
    for (int i = 0; i < DIR_LIGHT_COUNT; i++)
    {
        pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
        dLight = dirLights[i];
        normalizedLightDir = normalize(-dLight.lightDir.xyz);

        if (hasDepthData(pTexCoord))
        {
            finalLightColor += dLight.ambient;
            if (!isInShadow(i, pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
                finalLightColor += calculateLightingDirectional(normalizedLightDir, normal, dLight.diffuse);
                
                finalSpecularColor += calculateSpecular(normalizedLightDir, normal, viewDir, dLight.specular.rgb, dLight.specular.a);
            }
        }
    }
    PointLight pLight;
    float3 lightVector;
    float totalAtt;
    for (i = 0; i < POINT_LIGHT_COUNT; i++)
    {
        pLight = pLights[i];
        lightVector = pLight.position.xyz - input.worldPosition;
        totalAtt = calculateAttenuation(length(lightVector), pLight.attenuation);
        
        finalLightColor += pLight.ambient;
        finalLightColor += calculateLightingPoint(lightVector, normal, pLight.diffuse, totalAtt);                
        finalSpecularColor += calculateSpecular(normalize(lightVector), normal, viewDir, pLight.specular.rgb, pLight.specular.a, totalAtt);
    }
    SpotLight sLight;
    for (i = 0; i < SPOT_LIGHT_COUNT; i++)
    {
        sLight = sLights[i];
        lightVector = sLight.position.xyz - input.worldPosition;
        //totalAtt = calculateAttenuation(length(lightVector), sLight.attenuation);
        
        finalLightColor += sLight.ambient;
        finalLightColor += calculateLightingSpot(sLight.lightDir.xyz, lightVector, normal, sLight.diffuse, sLight.angleFalloff.xy);
        //finalSpecularColor += calculateSpecular(normalize(lightVector), normal, viewDir, sLight.specular.rgb, sLight.specular.a, totalAtt);
    }
    finalColor = textureColor * finalLightColor + finalSpecularColor;
    return finalColor;
}