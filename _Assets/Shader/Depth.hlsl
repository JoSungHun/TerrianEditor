#include "Common.hlsl"

cbuffer CascadeBuffer : register(b1)
{
    matrix wvp;
};

Pixel VS(Vertex input)
{
    Pixel ouput;

    ouput.position = mul(input.position, wvp);

    return ouput;
}