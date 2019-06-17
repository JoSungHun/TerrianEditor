#include "Common.hlsl"

PixelColor VS(VertexColor input)
{
    PixelColor output;
    output.position = mul(input.position, global_wvp);
    output.color = input.color;

    return output;
}

float4 PS(PixelColor input) : SV_Target
{
    return input.color;
}