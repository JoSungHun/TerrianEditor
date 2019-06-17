cbuffer PaintData : register(b0)
{
    float3 brush_position : packoffset(c0);
    float brush_range : packoffset(c0.w);
    float brush_intensity : packoffset(c1);
    float brush_type : packoffset(c1.y);
    float terrain_width : packoffset(c1.z);
    float terrain_height : packoffset(c1.w);
    float channel_index : packoffset(c2.x);
};



struct VertexData
{
    float3 Position;
    float2 Uv;
    float3 Normal;
	float3 Tangent;
	float4 BlendIndices;
    float4 BlendWeights;
};

struct PixelDATA
{
    float4 color;
};
 
StructuredBuffer<VertexData> InputBuffer : register(t0);
RWStructuredBuffer<VertexData> OutputBuffer : register(u0);
//StructuredBuffer<Pixel> InputTexture : register(t2);
RWStructuredBuffer<PixelDATA> OutputTexture : register(u1);


Texture2D BrushTexture : register(t1);
Texture2D SplatMap : register(t2);
SamplerState Samp : register(s0);

void EditHeight(inout VertexData vertex, in float2 uv)
{
    float4 brushTexColor = BrushTexture.SampleLevel(Samp, float2(uv.x, uv.y), 0);
    if(brushTexColor.g < 0.003f)
        return;

    vertex.Position += float3(0, brushTexColor.g, 0)*brush_intensity;
}

void EditSplatTex(int index, in float2 uv)
{
    uint indexX = index % (uint)terrain_width;
    uint indexY = index / (uint)terrain_height;

    /// 왜 0.5f 를 +해줘야되지??
    float2 indexUv = { (indexX+0.5f) / (float) terrain_width, (indexY+0.5f) / (float) terrain_height };
    //float2 indexUv = {(indexX/(float)terrain_width),(indexY)/(float)terrain_height};
    //indexUv = saturate(indexUv);
    float brushShapeValue = BrushTexture.SampleLevel(Samp, float2(uv.x, uv.y), 0).g;
    if (brushShapeValue < 0.003f)
        brushShapeValue = 0;
    float4 color = SplatMap.SampleLevel(Samp, indexUv, 0);
    
    switch (channel_index)
    {
        case 0:
            color.rgba -= brushShapeValue / 4.0f;
            break;
        case 1:
            color.r += brushShapeValue;
            break;
        case 2:
            color.g += brushShapeValue;
            break;
        case 3:
            color.b += brushShapeValue;
            break;
        case 4:
            color.a += brushShapeValue;
            break;
    }

    float sum = color.r + color.g + color.b + color.a;
    if(sum > 1.0f)
    {
        color /= sum;
    }

    int outIndex = indexX+ indexY*terrain_width;
    OutputTexture[outIndex].color = saturate(color);
}


[numthreads(1, 1, 1)]
void CS(uint3 GroupID : SV_GroupID)
{
    int index = GroupID.x;
   

    VertexData vertex = InputBuffer[index];
    OutputBuffer[index] = vertex;

    int type = (int)brush_type;
    int channel = (int)channel_index;
    //BrushType == None
    if (brush_type == 0)
    {
        return;
    }

    float range = floor(brush_range);
    float2 uvBrushTex = float2((vertex.Position.x - (brush_position.x - range)) / (2 * range), (vertex.Position.z - (brush_position.z - range)) / (2 * range));
    //Edit Height
    if (brush_type == 1)
    {
        EditHeight(vertex, uvBrushTex);
        OutputBuffer[index].Position = vertex.Position;
        OutputBuffer[index].Normal = float3(0, 0, 0);
    }

    //paint Texture
    if (brush_type == 3)
    {
        EditSplatTex(index, uvBrushTex);
        return;
    }
    



}