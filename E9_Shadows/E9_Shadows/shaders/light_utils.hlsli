#define DIR_LIGHT_COUNT 3

Texture2DArray<float> depthMapTextures : register(t1);
//Texture2D depthMapTextures[DIR_LIGHT_COUNT] : register(t1);
SamplerState shadowSampler : register(s1);

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular; //(color.rgb, power)
    float4 position;
    float4 lightDir;
};

cbuffer LightBuffer : register(b0)
{
    DirectionalLight dirLights[DIR_LIGHT_COUNT];
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

bool isInShadow(int lightIndex, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    //float depthValue = depthMapTextures[lightIndex].Sample(shadowSampler, uv).r;
    float depthValue = depthMapTextures.Sample(shadowSampler, float3(uv, lightIndex)).r;
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