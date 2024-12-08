Texture2D heightTex : register(t3);
SamplerState heightTexSampler : register(s1);

//TODO: add heightmap scale controls

float GetHeight(float2 uv)
{
    float height = heightTex.SampleLevel(heightTexSampler, uv, 0).r;
    return height * 200;
}