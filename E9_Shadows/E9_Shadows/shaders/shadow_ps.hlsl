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
    float diffuseFactor;
    DirectionalLight dLight;
    viewDir = normalize(camWorldPos - input.worldPosition);  
    for (int i = 0; i < DIR_LIGHT_COUNT; i++)
    {
        pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
        dLight = dirLights[i];

        if (hasDepthData(pTexCoord))
        {
            finalLightColor += dLight.ambient;
            if (!isInShadow(i, pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
                diffuseFactor = calculateDiffuseFactor(dLight.lightDir.xyz, normal);
                [flatten]
                if (diffuseFactor > 0)
                {
                    finalLightColor += saturate(diffuseFactor * dLight.diffuse);
                    finalSpecularColor += calculateSpecular(dLight.lightDir.xyz, normal, viewDir, dLight.specular.rgb, dLight.specular.a);
                }                               
            }
        }
    }
    PointLight pLight;
    float3 lightVector;
    float attFactor;
    for (i = 0; i < POINT_LIGHT_COUNT; i++)
    {
        pLight = pLights[i];
        lightVector = pLight.position.xyz - input.worldPosition;
        attFactor = calculateAttenuation(length(lightVector), pLight.attenuation);
        
        finalLightColor += pLight.ambient;
        diffuseFactor = calculateDiffuseFactor(normalize(-lightVector), normal);
        [flatten]
        if (diffuseFactor > 0 && attFactor > 0)
        {
            finalLightColor += saturate(diffuseFactor * pLight.diffuse * attFactor);
            finalSpecularColor += calculateSpecular(normalize(lightVector), normal, viewDir, pLight.specular.rgb, pLight.specular.a) * attFactor;
        }
        
        
    }
    SpotLight sLight;
    float angleFalloffFactor;
    for (i = 0; i < SPOT_LIGHT_COUNT; i++)
    {
        sLight = sLights[i];
        lightVector = sLight.position.xyz - input.worldPosition;
        attFactor = calculateAttenuation(length(lightVector), sLight.attenuation);
        
        finalLightColor += sLight.ambient;
        angleFalloffFactor = calculateAngleFalloff(sLight.lightDir.xyz, lightVector, sLight.angleFalloff.x, sLight.angleFalloff.y);
        diffuseFactor = calculateDiffuseFactor(sLight.lightDir.xyz, normal);
        [flatten]
        if (diffuseFactor > 0 && attFactor > 0 && angleFalloffFactor > 0)
        {
            finalLightColor += saturate(diffuseFactor * pLight.diffuse * attFactor * angleFalloffFactor);
            finalSpecularColor += calculateSpecular(normalize(lightVector), normal, viewDir, sLight.specular.rgb, sLight.specular.a) * attFactor * angleFalloffFactor;
        }
    }
    finalColor = textureColor * finalLightColor + finalSpecularColor;
    return finalColor;
}