struct VertexData
{
    float3 Position;
    float2 Uv;
    float3 Normal;
    float3 Tangent;
    float3 Binormal;
    float4 Color;
};
 

cbuffer TerrainData : register(b0)
{
    float2 TerrainSize : packoffset(c0);
}

StructuredBuffer<VertexData> InputBuffer : register(t0);
StructuredBuffer<uint> InputIndices : register(t1);
RWStructuredBuffer<VertexData> OutputBuffer : register(u0);

[numthreads(1, 1, 1)]
void CS(uint3 GroupID : SV_GroupID)
{
    int index = GroupID.x;
   
    int vertexIndex[3] = { InputIndices[index * 3], InputIndices[index * 3 + 1], InputIndices[index * 3 + 2] };
    
    float3 p[3];
    p[0] = InputBuffer[vertexIndex[0]].Position;
    p[1] = InputBuffer[vertexIndex[1]].Position;
    p[2] = InputBuffer[vertexIndex[2]].Position;

    VertexData v[3];
    
    v[0] = InputBuffer[vertexIndex[0]];
    v[1] = InputBuffer[vertexIndex[1]];
    v[2] = InputBuffer[vertexIndex[2]];

    OutputBuffer[vertexIndex[0]].Position = InputBuffer[vertexIndex[0]].Position;
    OutputBuffer[vertexIndex[1]].Position = InputBuffer[vertexIndex[1]].Position;
    OutputBuffer[vertexIndex[2]].Position = InputBuffer[vertexIndex[2]].Position;

    OutputBuffer[vertexIndex[0]].Uv = InputBuffer[vertexIndex[0]].Uv;
    OutputBuffer[vertexIndex[1]].Uv = InputBuffer[vertexIndex[1]].Uv;
    OutputBuffer[vertexIndex[2]].Uv = InputBuffer[vertexIndex[2]].Uv;

    OutputBuffer[vertexIndex[0]].Tangent = InputBuffer[vertexIndex[0]].Tangent;
    OutputBuffer[vertexIndex[1]].Tangent = InputBuffer[vertexIndex[1]].Tangent;
    OutputBuffer[vertexIndex[2]].Tangent = InputBuffer[vertexIndex[2]].Tangent;

    OutputBuffer[vertexIndex[0]].Binormal = InputBuffer[vertexIndex[0]].Binormal;
    OutputBuffer[vertexIndex[1]].Binormal = InputBuffer[vertexIndex[1]].Binormal;
    OutputBuffer[vertexIndex[2]].Binormal = InputBuffer[vertexIndex[2]].Binormal;

    OutputBuffer[vertexIndex[0]].Color = InputBuffer[vertexIndex[0]].Color;
    OutputBuffer[vertexIndex[1]].Color = InputBuffer[vertexIndex[1]].Color;
    OutputBuffer[vertexIndex[2]].Color = InputBuffer[vertexIndex[2]].Color;

    float3 d0, d1;
    d0 = p[1] - p[0];
    d1 = p[2] - p[0];
    float3 n = cross(d0, d1);
    
    OutputBuffer[vertexIndex[0]].Normal += n;
    OutputBuffer[vertexIndex[1]].Normal += n;
    OutputBuffer[vertexIndex[2]].Normal += n;
   

}