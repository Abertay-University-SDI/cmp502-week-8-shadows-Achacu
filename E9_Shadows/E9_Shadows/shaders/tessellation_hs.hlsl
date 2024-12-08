#include "heightmap.hlsli"

// Tessellation Hull Shader
// Prepares control points for tessellation
cbuffer TessellationBufferType : register(b0)
{
    float2 tesDstRange;
    float2 tesHeightRange;
    float3 camWorldPos;
    float maxTessellation;
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

//NOTE: edge factor calculations need to be done per-edge. Otherwise, neighboring edges in different patches will have different edge factors and the mesh will "break" 
float CalculateTesFactor(float4 A, float4 B) 
{
    float3 edgeMiddle = (A + B).xyz * 0.5f; 
    
    //adjust according to the height difference between the two vertices that form the edge. Steeper edges will be more tessellated
    float tesFactorHeight = lerp(1, maxTessellation, saturate((abs(A.y - B.y) - tesHeightRange.x) 
                                        / (tesHeightRange.y - tesHeightRange.x)));
    
    //adjust according to the distance to camera difference between the two vertices that form the edge. Closer edges will be more tessellated
    float tesFactorDstToCam = lerp(maxTessellation, 1, saturate((distance(edgeMiddle, camWorldPos) - tesDstRange.x)
                                        / (tesDstRange.y - tesDstRange.x)));
    
    return min(tesFactorDstToCam, tesFactorHeight);
}

ConstantOutputType PatchConstantFunction(InputPatch<InputType, 4> inputPatch, uint patchId : SV_PrimitiveID) //NOTE: patchID is not pointID
{    
    ConstantOutputType output;
    
    float4 vertexWorldPos0 = mul(float4(inputPatch[0].position.xyz,1), worldMatrix); //bottom left
    float4 vertexWorldPos1 = mul(float4(inputPatch[1].position.xyz,1), worldMatrix); //bottom right
    float4 vertexWorldPos2 = mul(float4(inputPatch[2].position.xyz,1), worldMatrix); //top right
    float4 vertexWorldPos3 = mul(float4(inputPatch[3].position.xyz,1), worldMatrix); //top left
    
    //Take into account vertex displacement
    vertexWorldPos0.y += GetHeight(inputPatch[0].tex);
    vertexWorldPos1.y += GetHeight(inputPatch[1].tex);
    vertexWorldPos2.y += GetHeight(inputPatch[2].tex);
    vertexWorldPos3.y += GetHeight(inputPatch[3].tex);
    
    //CAREFUL: edge indices are cw (starting from bottom) but vertices' are ccw (starting from bottom left)
    output.edges[0] = CalculateTesFactor(vertexWorldPos0, vertexWorldPos1); //bottom
    output.edges[1] = CalculateTesFactor(vertexWorldPos0, vertexWorldPos3); //left
    output.edges[2] = CalculateTesFactor(vertexWorldPos2, vertexWorldPos3); //top
    output.edges[3] = CalculateTesFactor(vertexWorldPos2, vertexWorldPos1); //right

    //Inner factors are calculated as an average of the surrounding edge factors to prevent tessellation discontinuities
    float innerFactor = (output.edges[0] + output.edges[1] + output.edges[2] + output.edges[3]) * 0.25f;
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