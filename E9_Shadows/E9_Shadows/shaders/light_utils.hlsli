#define DIR_LIGHT_COUNT 3
#define POINT_LIGHT_COUNT 2
#define SPOT_LIGHT_COUNT 2

//vertex shader
cbuffer MatrixBuffer : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
    matrix lightViewMatrices[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT];
    matrix lightProjectionMatrices[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT];
};


Texture2DArray dirShadowMaps : register(t0);
Texture2DArray spotShadowMaps : register(t1);
TextureCubeArray pointShadowMaps : register(t2);

//Texture2D dirShadowMaps[DIR_LIGHT_COUNT] : register(t1);
SamplerState shadowSampler : register(s0);

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
static const float3 pointLightDirections[6] =
{
    float3(1.0f, 0.0f, 0.0f),			
    float3(-1.0f, 0.0f, 0.0f),			
    float3(0.0f, 1.0f, 0.0f),			
    float3(0.0f, -1.0f, 0.0f),			
    float3(0.0f, 0.0f, 1.0f),			
    float3(0.0f, 0.0f, -1.0f)
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
    matrix pLightViews[6 * POINT_LIGHT_COUNT];
    matrix pLightProjections[POINT_LIGHT_COUNT];
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
bool isInShadow(TextureCubeArray shadowMapArray, int lightIndex, float3 pixelWorldPos, float3 lightVector, float bias)
{
    // Sample the shadow map (get depth of geometry)
    float depthValue = shadowMapArray.Sample(shadowSampler, float4(lightVector,lightIndex)).r;
    
    int maxDotIndex = 0;
    float currentDot, maxDot = 0;
    [unroll]
    for (int i = 0; i < 6; i++)
    {
        currentDot = dot(lightVector, pointLightDirections[i]);
        [flatten]
        if (currentDot > maxDot)
        {
            maxDot = currentDot;
            maxDotIndex = i;
        }
    }
    float4 pixelSeenFromLight = mul(mul(float4(pixelWorldPos, 1), pLightViews[6 * lightIndex + maxDotIndex]), pLightProjections[lightIndex]);
    
    //float depthValue = shadowMapArray.Sample(shadowSampler, float4(lightVector, lightIndex)).r;
	// Calculate the depth from the light.
    float lightDepthValue = pixelSeenFromLight.z / pixelSeenFromLight.w;
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
void calculateLightViewPositions(float4 worldPos, out float4 lightViewPositions[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT])
{
    float4 lightViewPos;
    for (int i = 0; i < DIR_LIGHT_COUNT; i++)
    {
        lightViewPos = mul(worldPos, lightViewMatrices[i]);
        lightViewPositions[i] = mul(lightViewPos, lightProjectionMatrices[i]);
    }
    for (int j = 0; j < SPOT_LIGHT_COUNT; j++)
    {
        lightViewPos = mul(worldPos, lightViewMatrices[i + j]);
        lightViewPositions[i + j] = mul(lightViewPos, lightProjectionMatrices[i + j]);
    }
}

float4 applyLightingAndShadows(float3 normal, float4 lightViewPos[DIR_LIGHT_COUNT + SPOT_LIGHT_COUNT], float3 worldPos, float3 camWorldPos, out float4 finalSpecularColor)
{
    finalSpecularColor = float4(0, 0, 0, 0);
    
    float shadowMapBias = 0.005f; //low value -> self-shadowing artifacts, high value -> some parts of shadow are lost
    float4 finalLightColor = float4(0.f, 0.f, 0.f, 1.f);
    float4 finalColor = float4(0.f, 0.f, 0.f, 1.f);
    
    float3 normalizedLightDir, viewDir;
    float2 pTexCoord;
    float diffuseFactor;
    DirectionalLight dLight;
    viewDir = normalize(camWorldPos - worldPos);
    [unroll]
    for (int i = 0; i < DIR_LIGHT_COUNT; i++)
    {
        pTexCoord = getProjectiveCoords(lightViewPos[i]);
        dLight = dirLights[i];

        [flatten]
        if (hasDepthData(pTexCoord))
        {
            finalLightColor += dLight.ambient;
            [flatten]
            if (!isInShadow(dirShadowMaps, i, pTexCoord, lightViewPos[i], shadowMapBias))
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
    SpotLight sLight;
    float3 lightVector;
    float attFactor, angleFalloffFactor;
    [unroll]
    for (int j = 0; j < SPOT_LIGHT_COUNT; j++)
    {
        sLight = sLights[j];
        lightVector = sLight.position.xyz - worldPos;
        pTexCoord = getProjectiveCoords(lightViewPos[i + j]);
        
        [flatten]
        if (hasDepthData(pTexCoord))
        {
            finalLightColor += sLight.ambient;
            [flatten]
            if (!isInShadow(spotShadowMaps, j, pTexCoord, lightViewPos[i + j], shadowMapBias))
            {
                attFactor = calculateAttenuation(length(lightVector), sLight.attenuation);
        
                angleFalloffFactor = calculateAngleFalloff(sLight.lightDir.xyz, lightVector, sLight.angleFalloff.x, sLight.angleFalloff.y);
                diffuseFactor = calculateDiffuseFactor(sLight.lightDir.xyz, normal);
                [flatten]
                if (diffuseFactor > 0 && attFactor > 0 && angleFalloffFactor > 0)
                {
                    finalLightColor += saturate(diffuseFactor * sLight.diffuse * attFactor * angleFalloffFactor);
                    finalSpecularColor += calculateSpecular(normalize(lightVector), normal, viewDir, sLight.specular.rgb, sLight.specular.a) * attFactor * angleFalloffFactor;
                }
            }
        }
    }
    PointLight pLight;
    [unroll]
    for (int k = 0; k < POINT_LIGHT_COUNT; k++)
    {
        pLight = pLights[k];
        lightVector = pLight.position.xyz - worldPos;
        
        [flatten]
        if (!isInShadow(pointShadowMaps, k, worldPos, -lightVector, shadowMapBias))
        {
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
    }
    return finalLightColor;
}