cbuffer PaintData : register(b0)
{
    float4 Color : packoffset(c0);
    float3 Position : packoffset(c1);
    float Range : packoffset(c1.w);
    float Intensity : packoffset(c2);
    uint BrushType : packoffset(c2.y);
    uint2 TrSize : packoffset(c2.z);
    uint ChannelNum : packoffset(c3.x);
    uint RowPitch : packoffset(c3.y);
};

struct VertexData
{
    float3 Position;
    float2 Uv;
    float3 Normal;
	float3 Tangent;
	float3 Binormal;
    float4 Color;
};
struct Pixel
{
    float4 color;
};
 
StructuredBuffer<VertexData> InputBuffer : register(t0);
RWStructuredBuffer<VertexData> OutputBuffer : register(u0);
//StructuredBuffer<Pixel> InputTexture : register(t2);
RWStructuredBuffer<Pixel> OutputTexture : register(u1);


Texture2D BrushTexture : register(t1);
Texture2D SplatMap : register(t2);
SamplerState Samp : register(s0);


float GuideColor(in VertexData vertex, in float2 uv)
{
    
    float4 brushTexColor = BrushTexture.SampleLevel(Samp, float2(uv.x, uv.y), 0);

    return brushTexColor.g;
}

void EditHeight(inout VertexData vertex, in float2 uv)
{
    float4 brushTexColor = BrushTexture.SampleLevel(Samp, float2(uv.x, uv.y), 0);
    if(brushTexColor.g < 0.03f)
        return;

    vertex.Position += float3(0, brushTexColor.g, 0);
}

void EditSplatTex(int index, in float2 uv)
{
    uint indexX = index % TrSize[0];
    uint indexY = index / TrSize[1];

    /// 왜 0.5f 를 +해줘야되지??
    float2 indexUv = { (indexX+0.5f) / (float) TrSize[0], (indexY+0.5f) / (float) TrSize[1] };
    indexUv = saturate(indexUv);
    float brushShapeValue = BrushTexture.SampleLevel(Samp, float2(uv.x, uv.y), 0).g;
    if (brushShapeValue < 0.05f)
        brushShapeValue = 0;
    float4 color = SplatMap.SampleLevel(Samp, indexUv, 0);
    
    switch (ChannelNum)
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

    int outIndex = indexX+ indexY*TrSize[0];
    OutputTexture[outIndex].color = saturate(color);
}


//void UpdateNormal(int index)
//{
//    //TrSize = {width,height}
//    int trWidth = (int) TrSize.x;
//    int trHeight = (int) TrSize.y;
//    if (index > (trWidth - 1) * (trHeight - 1)) // 정점 개수 = width*height, 삼각형 개수  (width-1)(height-1)*2
//        return;

//    float3 p[6];
//    int indices[6];

//    int x = (index) % (TrSize.x - 1);
//    int z = (index) / (TrSize.x - 1);

//    indices[0] = z * trWidth + x;
//    indices[1] = (z + 1) * trWidth + x;
//    indices[2] = z * trWidth + x + 1;

//    indices[3] = z * trWidth + x + 1;
//    indices[4] = (z + 1) * trWidth + x;
//    indices[5] = (z + 1) * trWidth + x + 1;
    

//    /*
//    geometry.AddIndex(width*z + x);
//			geometry.AddIndex(width*(z + 1) + x);
//			geometry.AddIndex(width*z + (x + 1));

//			geometry.AddIndex(width*z + (x + 1));
//			geometry.AddIndex(width*(z + 1) + x);
//			geometry.AddIndex(width*(z + 1) + (x + 1));
//    */


//    p[0] = InputBuffer[indices[0]].Position;
//    p[1] = InputBuffer[indices[1]].Position;
//    p[2] = InputBuffer[indices[2]].Position;
//    p[3] = InputBuffer[indices[3]].Position;
//    p[4] = InputBuffer[indices[4]].Position;
//    p[5] = InputBuffer[indices[5]].Position;

//    float3 d0, d1;
//    d0 = p[1] - p[0];
//    d1 = p[2] - p[0];
//    float3 n = cross(d0, d1);

//    OutputBuffer[indices[0]].Normal += n;
//    OutputBuffer[indices[1]].Normal += n;
//    OutputBuffer[indices[2]].Normal += n;

   
//    d0 = p[4] - p[3];
//    d1 = p[5] - p[3];
//    n = cross(d0, d1);

//    OutputBuffer[indices[3]].Normal += n;
//    OutputBuffer[indices[4]].Normal += n;
//    OutputBuffer[indices[5]].Normal += n;
//}





[numthreads(1, 1, 1)]
void CS(uint3 GroupID : SV_GroupID)
{
    int index = GroupID.x;
   

    VertexData vertex = InputBuffer[index];
    vertex.Normal = float3(1, 2, 3 );
    OutputBuffer[index] = vertex;

    //BrushType == None
    if (BrushType == 0)
    {
        return;
    }

    float range = floor(Range);
    float2 uvBrushTex = float2((vertex.Position.x - (Position.x - range)) / (2 * range), (vertex.Position.z - (Position.z - range)) / (2 * range));
    //Edit Height
    if (BrushType == 1)
    {
        EditHeight(vertex, uvBrushTex);
        OutputBuffer[index].Position = vertex.Position;
        OutputBuffer[index].Normal = float3(0, 0, 0);
    }

    //paint Texture
    if (BrushType == 3)
    {
        EditSplatTex(index, uvBrushTex);
        return;
    }
    



}