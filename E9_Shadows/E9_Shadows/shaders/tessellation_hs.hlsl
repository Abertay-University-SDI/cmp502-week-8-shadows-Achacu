#include "heightmap.hlsli"

// Tessellation Hull Shader
// Prepares control points for tessellation
cbuffer TessellationBufferType : register(b0)
{
    float4 tessellationFactors;
    float3 camWorldPos;
    float maxTesDst;
};
cbuffer MatrixBuffer : register(b1)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
};

struct InputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

struct ConstantOutputType
{
    float edges[4] : SV_TessFactor;
    float inside[2] : SV_InsideTessFactor;
};

struct OutputType
{
    float3 position : POSITION;
    float4 colour : COLOR;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
};

float CalculateTesFactor(float4 A, float4 B)
{
    float3 edgeMiddle = (A + B) / 2; 
    float minTesDst = tessellationFactors.r;

    float tesFactorHeight = lerp(1, 32, saturate(abs(A.y - B.y) / tessellationFactors.y));
    float tesFactorDstToCam = lerp(32, 1, saturate((distance(edgeMiddle, camWorldPos) - minTesDst)
                                        / (maxTesDst - minTesDst)));
    return min(tesFactorDstToCam, tesFactorHeight);
}

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID) //NOTE: patchID is not pointID
{    
    ConstantOutputType output;
    
    float4 vertexWorldPos0 = mul(float4(inputPatch[0].position.xyz,1), worldMatrix);
    float4 vertexWorldPos1 = mul(float4(inputPatch[1].position.xyz,1), worldMatrix);
    float4 vertexWorldPos2 = mul(float4(inputPatch[2].position.xyz,1), worldMatrix);
    float4 vertexWorldPos3 = mul(float4(inputPatch[3].position.xyz,1), worldMatrix);
    
    //Take into account vertex displacement
    vertexWorldPos0 += GetHeight(inputPatch[0].tex);
    vertexWorldPos1 += GetHeight(inputPatch[1].tex);
    vertexWorldPos2 += GetHeight(inputPatch[2].tex);
    vertexWorldPos3 += GetHeight(inputPatch[3].tex);
    
    //CAREFUL: edges are cw but vertices are ccw (both starting from bottom left)
    output.edges[0] = CalculateTesFactor(vertexWorldPos0, vertexWorldPos1);
    output.edges[1] = CalculateTesFactor(vertexWorldPos0, vertexWorldPos3);
    output.edges[2] = CalculateTesFactor(vertexWorldPos2, vertexWorldPos3);
    output.edges[3] = CalculateTesFactor(vertexWorldPos2, vertexWorldPos1);

    // Set the tessellation factor for tessallating inside the triangle.
    float innerFactor = (output.edges[0] + output.edges[1] + output.edges[2] + output.edges[3]) / 4;
    output.inside[0] = innerFactor;
    output.inside[1] = innerFactor;

    return output;
}


[domain("quad")] //tri -> controlPoint = vertex, triangle = batch, quad -> controlPoint = vertex, quad = batch
[partitioning("fractional_odd")] //pow2, integer, fractional_odd and fractional_even
[outputtopology("triangle_ccw")] //ccw = counterclockwise cw = clockwise
[outputcontrolpoints(4)] //3 for triangle, 4 for quad
[patchconstantfunc("PatchConstantFunction")] //declared in line 21
OutputType main(InputPatch<InputType, 4> patch, uint pointId : SV_OutputControlPointID, uint patchId : SV_PrimitiveID)
{
    OutputType output;

    // Set the position for this control point as the output position.
    output.position = patch[pointId].position;

    // Set the input colour as the output colour.
    output.colour = patch[pointId].colour;
    output.tex = patch[pointId].tex;
    output.normal = patch[pointId].normal;

    return output;
}