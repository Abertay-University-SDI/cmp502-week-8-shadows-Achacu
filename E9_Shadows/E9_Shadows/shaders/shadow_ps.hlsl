#define DIR_LIGHT_COUNT 3


Texture2D shaderTexture : register(t0);
Texture2D depthMapTextures[DIR_LIGHT_COUNT] : register(t1);

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

// Calculate lighting intensity based on direction and normal. Combine with light colour.
float4 calculateLightingDirectional(float3 lightDir, float3 normal, float4 lightDiffuse)
{
    float intensity = saturate(dot(normal, lightDir));
    return saturate(lightDiffuse * intensity);
}
float4 calculateSpecular(float3 lightDir, float3 normal, float3 viewDir, float3 specularColor, float specularPower)
{
    float3 halfVector = normalize(lightDir + viewDir);
    float3 rawColor = max(dot(normal, halfVector), 0.0) * specularColor;
    return float4(saturate(pow(rawColor, specularPower)), 1.0f);
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
    return (lightDepthValue > depthValue);
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
    float4 finalSpecularColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 finalColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 textureColor = shaderTexture.Sample(diffuseSampler, input.tex);

	// Calculate the projected texture coordinates.
    float3 normal = normalize(input.normal);
    // Shadow test. Is or isn't in shadow
    
    float3 normalizedLightDir, viewDir;
    float2 pTexCoord;
    DirectionalLight dLight;
    for (int i = 0; i < DIR_LIGHT_COUNT; i++)
    {
        pTexCoord = getProjectiveCoords(input.lightViewPos[i]);
        dLight = dirLights[i];
        normalizedLightDir = normalize(-dLight.lightDir.xyz);

        if (hasDepthData(pTexCoord))
        {
            finalLightColor += dLight.ambient;
            if (!isInShadow(depthMapTextures[i], pTexCoord, input.lightViewPos[i], shadowMapBias))
            {
                finalLightColor += calculateLightingDirectional(normalizedLightDir, normal, dLight.diffuse);
                
                viewDir = normalize(camWorldPos - input.worldPosition);
                finalSpecularColor += calculateSpecular(normalizedLightDir, normal, viewDir, dLight.specular.rgb, dLight.specular.a);
            }
        }
    }
    finalColor = textureColor * finalLightColor + finalSpecularColor;
    return finalColor;
}