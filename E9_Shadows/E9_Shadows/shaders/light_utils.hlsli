#define DIR_LIGHT_COUNT 3
#define POINT_LIGHT_COUNT 1
#define SPOT_LIGHT_COUNT 2

Texture2DArray dirShadowMaps : register(t1);
Texture2DArray spotShadowMaps : register(t2);
TextureCube/*Array<float>*/ pointShadowMaps : register(t3);

//Texture2D dirShadowMaps[DIR_LIGHT_COUNT] : register(t1);
SamplerState shadowSampler : register(s1);

struct DirectionalLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular; //(color.rgb, power)
    float4 position;
    float4 lightDir;
};
struct PointLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular; //(color.rgb, power)
    float4 position;
    float4 attenuation;
};
struct SpotLight
{
    float4 ambient;
    float4 diffuse;
    float4 specular; //(color.rgb, power)
    float4 position;
    float4 lightDir;
    float4 attenuation;
    float4 angleFalloff; //(halfAngleDot, falloffExponent, -)
};

cbuffer LightBuffer : register(b0)
{
    DirectionalLight dirLights[DIR_LIGHT_COUNT];
    PointLight pLights[POINT_LIGHT_COUNT];
    SpotLight sLights[SPOT_LIGHT_COUNT];
};


float4 calculateDiffuseFactor(float3 lightDir, float3 normal) { return saturate(dot(normal, -lightDir)); }
float3 calculateAttenuation(float dist, float4 att) //(constAtt, linearAtt, quadraticAtt, range)
{
    return saturate(step(dist, att.w) * 1 / (att.x + att.y * dist + att.z * dist * dist));
}
float calculateAngleFalloff(float3 lightDir, float3 lightVector, float halfAngleDot, float fallOffExp)
{
    float dotForThisPixel = dot(normalize(lightDir), normalize(-lightVector));
    return min(max(dotForThisPixel - halfAngleDot, 0) / pow(1 - halfAngleDot, fallOffExp), 1);
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

bool isInShadow(Texture2DArray shadowMapArray, int lightIndex, float2 uv, float4 lightViewPosition, float bias)
{
    // Sample the shadow map (get depth of geometry)
    //float depthValue = dirShadowMaps[lightIndex].Sample(shadowSampler, uv).r;
    float depthValue = shadowMapArray.Sample(shadowSampler, float3(uv, lightIndex)).r;
	// Calculate the depth from the light.
    float lightDepthValue = lightViewPosition.z / lightViewPosition.w;
    lightDepthValue -= bias;
    
	// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
    return (lightDepthValue > depthValue);
}
bool isInShadow(TextureCube/*Array*/ shadowMapArray, int lightIndex, float3 lightVector, float bias, float farDst)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = shadowMapArray.Sample(shadowSampler, lightVector).r;
    //float depthValue = shadowMapArray.Sample(shadowSampler, float4(lightVector, lightIndex)).r;
	// Calculate the depth from the light.
    float lightDepthValue = length(lightVector) / farDst;
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