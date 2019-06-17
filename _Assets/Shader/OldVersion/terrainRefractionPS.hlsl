cbuffer LightColor : register(b0)
{
    float4 ambient;
    float4 diffuse;
    float3 lightDirection;
};
 
struct PS_INPUT
{
    float4 Position : SV_POSITION;
    float2 Uv : UV0;
    float2 bUv : UV1;
    float3 Normal : NORMAL;
    float clip : SV_ClipDistance0;
};


Texture2D SplatMap : register(t0); 
Texture2D DefaultTexture : register(t1);
Texture2D RChannelTexture : register(t2); 
Texture2D GChannelTexture : register(t3);
Texture2D BChannelTexture : register(t4);
Texture2D AChannelTexture : register(t5);

SamplerState Samp : register(s0);



float4 PS(PS_INPUT input) : SV_Target
{
    float4 channelValue = SplatMap.Sample(Samp, input.bUv);
    float defaultValue = 1.0f - channelValue.r - channelValue.g - channelValue.b - channelValue.a;

    //TEX COLOR
    float4 texColor = DefaultTexture.Sample(Samp, input.Uv) * defaultValue;
    texColor += RChannelTexture.Sample(Samp, input.Uv) * channelValue.r;
    texColor += GChannelTexture.Sample(Samp, input.Uv) * channelValue.g;
    texColor += BChannelTexture.Sample(Samp, input.Uv) * channelValue.b;
    texColor += AChannelTexture.Sample(Samp, input.Uv) * channelValue.a;
    
    
    float3 bumpNormal = input.Normal;  

    float3 lightDir;
    float lightIntensity;
    float4 color;

    color = ambient;

    lightDir = -lightDirection;

    lightIntensity = saturate(dot(input.Normal, lightDir));

    if(lightIntensity > 0.0f)
        color += (diffuse * lightIntensity);

    color = saturate(color);

    color = color * texColor;
     
    return color;
}
