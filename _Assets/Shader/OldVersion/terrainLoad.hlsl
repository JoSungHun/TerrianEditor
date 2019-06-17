cbuffer RayBuffer : register(b0)
{
    float3 Origin : POSITION0;
    int padding;
	float3 Direction : POSITION1;
};

cbuffer TerrainInfoBuffer : register(b1)
{
    float2 Size : packoffset(c0);
}

struct VertexData
{
    float3 Position;
    float2 Uv;
    float3 Normal;
    float4 Color;
};

struct IndexData
{
    uint index;
};

StructuredBuffer<VertexData> InputBuffer : register(t0);
StructuredBuffer<

RWStructuredBuffer<VertexData> OutputBuffer : register(u0);



[numthreads(1, 1, 1)]
void CS(uint3 GroupID : SV_GroupID)
{
    int index = GroupID.x;


}