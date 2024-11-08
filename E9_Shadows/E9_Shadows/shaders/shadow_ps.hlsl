#define DIR_LIGHT_COUNT 1


Texture2D shaderTexture : register(t0);
Texture2D depthMapTexture : register(t1);

SamplerState diffuseSampler  : register(s0);
SamplerState shadowSampler : register(s1);

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular; //(color.rgb, power)
    float4 position;
    float4 lightDir;
};

cbuffer DirectionalLightBuffer : register(b0)
{
    DirectionalLight dirLights[DIR_LIGHT_COUNT];
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
    float4 lightViewPos[DIR_LIGHT_COUNT] : TEXCOORD1;
};

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLightingDirectional(float3 lightDir, float3 normal, float4 lightDiffuse)
{
    float intensity = saturate(dot(normal, lightDir));
    return saturate(lightDiffuse * intensity);
}

// Is the gemoetry in our shadow map
bool hasDepthData(float2 uv)
{
    if (uv.x < 0.f || uv.x > 1.f || uv.y < 0.f || uv.y > 1.f)
    {
        return false;
    }
    return true;
}

bool isInShadow(Texture2D sMap, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = sMap.Sample(shadowSampler, uv).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;
    
	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    if (lightDepthValue < depthValue)
    {
        return false;
    }
    return true;
}

float2 getProjectiveCoords(float4 lightViewPosition)
{
    // Calculate the projected texture coordinates.
    float2 projTex = lightViewPosition.xy / lightViewPosition.w;
    projTex *= float2(0.5, -0.5);
    projTex += float2(0.5f, 0.5f);
    return projTex;
}

float4 main(InputType input) : SV_TARGET
{
    float shadowMapBias = 0.005f; //low value -> self-shadowing artifacts, high value -> some parts of shadow are lost
    float4 finalLightColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 finalSpecularColor;
    float4 finalColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColor = shaderTexture.Sample(diffuseSampler, input.tex);

	// Calculate the projected texture coordinates.
    float2 pTexCoord = getProjectiveCoords(input.lightViewPos[0]);
    float3 normal = normalize(input.normal);
    // Shadow test. Is or isn't in shadow
    
    float3 normalizedLightDir;
    DirectionalLight dLight;
    for (int i = 0; i < DIR_LIGHT_COUNT; i++)
    {
        dLight = dirLights[i];
        normalizedLightDir = normalize(-dLight.lightDir.xyz);
		
        if (hasDepthData(pTexCoord))
        {
            // Has depth map data
            if (!isInShadow(depthMapTexture, pTexCoord, input.lightViewPos[0], shadowMapBias))
            {
                // is NOT in shadow, therefore light
                finalLightColor += dLight.ambient + calculateLightingDirectional(normalizedLightDir, normal, dLight.diffuse);
            }
        }
        //finalSpecularColor += calculateSpecular(normalizedLightDir, normal, viewDir, specularD[i].rgb, specularD[i].a);
    }
    finalColor = textureColor * finalLightColor/*+ finalSpecularColor*/;
    return finalColor;
}