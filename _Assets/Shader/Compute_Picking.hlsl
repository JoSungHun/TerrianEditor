
#define EPSILON 0.00000001

cbuffer RayBuffer : register(b0)
{
    float3 ray_origin : packoffset(c0);
	float3 ray_direction : packoffset(c1);
    uint terrain_width : packoffset(c2.x);
    uint terrain_height : packoffset(c2.y);
};

struct InputData
{
    float3 Position;
    float2 Uv;
    float3 Normal;
    float3 Tangent;
    float4 blendIndices;
    float4 blendWeights;
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
    int x = (index) / 2 % terrain_width;
    int z = (index) / 2 / terrain_width;

    if (index % 2)
    {
        tri.pos1 = InputBuffer[z * terrain_width + x].Position;
        tri.pos2 = InputBuffer[(z + 1) * terrain_width + x].Position;
        tri.pos3 = InputBuffer[z * terrain_width + (x + 1)].Position;
    }
    else
    {
        tri.pos1 = InputBuffer[z * terrain_width + (x + 1)].Position;
        tri.pos2 = InputBuffer[(z + 1) * terrain_width + x].Position;
        tri.pos3 = InputBuffer[(z + 1) * terrain_width + (x + 1)].Position;
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

    h = cross(ray_direction, e2);
    a = dot(e1, h);

    if( a > -EPSILON && a < EPSILON)
        return;

    f = 1 / a;
    s = ray_origin - tri.pos1;
    u = f * dot(s, h);


    if( u < 0.0 || u > 1.0)
        return;

    q = cross(s, e1);
    v = f * dot(ray_direction, q);

    if( v < 0.0 || u+v > 1.0)
        return;

    t = f * dot(e2, q);


    if( t > EPSILON)
    {
        if(OutputBuffer[0].picked == 0)
        {
            OutputBuffer[0].picked = 1;
            OutputBuffer[0].distance = t;
            OutputBuffer[0].pos = ray_origin + t * ray_direction;
        }
        else
        {
            if(OutputBuffer[0].distance > t)
            {
                OutputBuffer[0].distance = t;
                OutputBuffer[0].pos = ray_origin + t * ray_direction;
            }
        }
    }
}