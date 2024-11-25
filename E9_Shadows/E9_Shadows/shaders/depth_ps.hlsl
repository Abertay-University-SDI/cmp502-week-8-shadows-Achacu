//cbuffer LightInfoBuffer : register(b0)
//{
//    float3 lightPos;
//    float range;
//};

struct InputType
{
    float4 position : SV_POSITION;
    float4 depthPosition : TEXCOORD0;
};

float4 main(InputType input) : SV_TARGET
{
    //float depthValue = distance(lightPos, input.depthPosition.xyz) / range;
	// Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
	float depthValue = input.depthPosition.z / input.depthPosition.w;
    return float4(depthValue, depthValue, depthValue, 1.0f);

}