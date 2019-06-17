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

struct InputData
{
    float3 Position;
    float2 Uv;
    float3 Normal;
    float3 Tangent;
    float3 Binormal;
    float4 Color;
};

struct Triangle
{
    float3 pos1;
    float3 pos2;
    float3 pos3;
};

struct OutputData
{
    float3 pos;
    float distance;
    int picked;
};

StructuredBuffer<InputData> InputBuffer : register(t0);
RWStructuredBuffer<OutputData> OutputBuffer : register(u0);

Triangle GetTriangle(int index)
{
    //정점들이 겹치도록 인덱스를 구성 했을 때,
    Triangle tri;
    int x = (index) / 2 % Size.x;
    int z = (index) / 2 / Size.x;

    if (index % 2)
    {
        tri.pos1 = InputBuffer[z * Size.x + x].Position;
        tri.pos2 = InputBuffer[(z + 1) * Size.x + x].Position;
        tri.pos3 = InputBuffer[z * Size.x + (x + 1)].Position;
    }
    else
    {
        tri.pos1 = InputBuffer[z * Size.x + (x + 1)].Position;
        tri.pos2 = InputBuffer[(z + 1) * Size.x + x].Position;
        tri.pos3 = InputBuffer[(z + 1) * Size.x + (x + 1)].Position;
    }

    //정점들이 겹치지 않게 인덱스를 구성 했을 때,
    //Triangle tri;
    //tri.pos1 = InputBuffer[index * 3 + 0].Position;
    //tri.pos2 = InputBuffer[index * 3 + 1].Position;
    //tri.pos3 = InputBuffer[index * 3 + 2].Position;


    return tri;
}


[numthreads(1, 1, 1)]
void CS(uint3 GroupID : SV_GroupID)
{
    int index = GroupID.x;
// index == 0 0번째 삼각형 -> index = 0,1,2 
    Triangle tri = GetTriangle(index);

    
    float3 e1, e2, h, s, q;
    float a, f, u, v, t;
    
    e1 = tri.pos2 - tri.pos1;
    e2 = tri.pos3 - tri.pos1;

    h = cross(Direction, e2);
    a = dot(e1, h);

    if( a > -0.00001 && a < 0.00001)
        return;

    f = 1 / a;
    s = Origin - tri.pos1;
    u = f * dot(s, h);

    if( u < 0.0 || u > 1.0)
        return;

    q = cross(s, e1);
    v = f * dot(Direction, q);

    if( v < 0.0 || u+v > 1.0)
        return;

    t = f * dot(e2, q);

    if( t > 0.00001)
    {
        if(OutputBuffer[0].picked == 0)
        {
            OutputBuffer[0].picked = 1;
            OutputBuffer[0].distance = t;
            OutputBuffer[0].pos = Origin + t * Direction;
        }
        else
        {
            if(OutputBuffer[0].distance > t)
            {
                OutputBuffer[0].distance = t;
                OutputBuffer[0].pos = Origin + t * Direction;
            }
        }
    }


    ////Find vectors for two edges
    //float3 edge1 = tri.pos2 - tri.pos1;
    //float3 edge2 = tri.pos3 - tri.pos1;

    ////Begin calculating determinant - also used to calculate U parameter
    //float3 pvec = cross(Direction, edge2);

    //// if determinant is near zero, ray lies in plane of triangle
    //float det = dot(edge1, pvec);

    //float3 tvec;
    //if (det > 0)
    //{
    //    tvec = Origin - tri.pos1;
    //}
    //else
    //{
    //    tvec = tri.pos1 - Origin;
    //    det *= -1;
    //}

    //if (det < 0.0001f)
    //{
    //    return;
    //}

    ////Calculate U parameter and test bounds
    //float u = dot(tvec, pvec);
    //if (u < 0.0f || u > det)
    //{
    //    return;
    //}

    //float3 qvec = cross(tvec, edge1);

    ////Calculate V parameter and test bounds
    //float v = dot(Direction, qvec);
    //if (v < 0.0f || u + v > det)
    //{
    //    return;
    //}

    ////Calculate t, scale parameters, ray intersects tiangle
    //float t = dot(edge2, qvec);
    //if(t == 0)
    //    return;

    //float fInvDet = 1.0f / det;
    //t *= fInvDet;
    //u *= fInvDet;
    //v *= fInvDet;

   

    //OutputBuffer[0].picked = 1;
    //if (OutputBuffer[0].distance > t)
    //{
    //    OutputBuffer[0].distance = t;
    //    OutputBuffer[0].pos = Origin + t * Direction;
    //}
    
    //if (t < 0)
    //{
    //    OutputBuffer[0].distance = -t;
    //    OutputBuffer[0].pos = Origin + t * Direction;
    //    return;
    //}

}