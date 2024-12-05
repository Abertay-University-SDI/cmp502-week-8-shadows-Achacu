Texture2D heightTex : register(t0);
SamplerState heightTexSampler : register(s0);

float GetHeight(float2 uv)
{
    float height = heightTex.SampleLevel(heightTexSampler, uv, 0).r;
    return height * 5;
}