cbuffer LightColor : register(b0)
{
    float4 ambient;
    float4 diffuse;
    float3 lightDirection;
};



struct DS_VS_OUTPUT_PS_INPUT
{
    float4 Position : SV_POSITION;
    float4 DepthPosition : POSITION1;
    float2 Uv : UV0;
    float2 bUv : UV1;
    float3 Normal : NORMAL;
    float3 Tangent : TANGENT;
    float3 Binormal : BINORMAL;
    float4 BrushColor : COLOR;
    float4 lightViewPosition : TEXCOORD1;
    float3 lightPos : TEXCOORD2;
};


Texture2D DefaultTexture : register(t0);
Texture2D DefaultNormal : register(t1);
Texture2D DefaultSpecular : register(t2);

Texture2D SplatMap : register(t3);

Texture2D RChannelTexture : register(t4);
Texture2D RChannelNormal : register(t5);
Texture2D RChannelSpecular : register(t6);


Texture2D GChannelTexture : register(t7);
Texture2D GChannelNormal : register(t8);
Texture2D GChannelSpecular : register(t9);


Texture2D BChannelTexture : register(t10);
Texture2D BChannelNormal : register(t11);
Texture2D BChannelSpecular : register(t12);


Texture2D AChannelTexture : register(t13);
Texture2D AChannelNormal : register(t14);
Texture2D AChannelSpecular : register(t15);

Texture2D DepthMap : register(t16);

SamplerState Samp : register(s0);


float3 GetBumpNormal(in float2 inputUv, in float3 normal, in float3 tangent, in float3 binormal, in int channelNum)
{
    float4 bumpMap;
    float3 bumpNormal;
    switch (channelNum)
    {
        case 0:
            bumpMap = DefaultNormal.Sample(Samp, inputUv);
            break;
        case 1:
            bumpMap = RChannelNormal.Sample(Samp, inputUv);
            break;
        case 2:
            bumpMap = GChannelNormal.Sample(Samp, inputUv);
            break;
        case 3:
            bumpMap = BChannelNormal.Sample(Samp, inputUv);
            break;
        case 4:
            bumpMap = AChannelNormal.Sample(Samp, inputUv);
            break;
        
    }

    bumpMap = (bumpMap * 2.0f) - 1.0f;
    bumpNormal = normal + bumpMap.x * tangent + bumpMap.y * binormal;
    bumpNormal = normalize(bumpNormal);
    return bumpNormal;
}


float4 PS(DS_VS_OUTPUT_PS_INPUT input) : SV_Target
{
    float4 channelValue = SplatMap.Sample(Samp, input.bUv);
    float defaultValue = 1.0f - channelValue.r - channelValue.g - channelValue.b - channelValue.a;
    float depthValue = input.DepthPosition.z/input.DepthPosition.w;
    //TEX COLOR
    float4 texColor = DefaultTexture.Sample(Samp, input.Uv) * defaultValue;
    texColor += RChannelTexture.Sample(Samp, input.Uv) * channelValue.r;
    texColor += GChannelTexture.Sample(Samp, input.Uv) * channelValue.g;
    texColor += BChannelTexture.Sample(Samp, input.Uv) * channelValue.b;
    texColor += AChannelTexture.Sample(Samp, input.Uv) * channelValue.a;
    
    
    float3 bumpNormal = float3(0, 0, 0);
    if(depthValue < 0.9f)
    { 
        if (defaultValue > 0.008f)
            bumpNormal += GetBumpNormal(input.Uv, input.Normal, input.Tangent, input.Binormal, 0);
        if (channelValue.r > 0.008f)
            bumpNormal += GetBumpNormal(input.Uv, input.Normal, input.Tangent, input.Binormal, 1);
        if (channelValue.g > 0.008f)
            bumpNormal += GetBumpNormal(input.Uv, input.Normal, input.Tangent, input.Binormal, 2);
        if (channelValue.b > 0.008f)
            bumpNormal += GetBumpNormal(input.Uv, input.Normal, input.Tangent, input.Binormal, 3);
        if (channelValue.a > 0.008f)
            bumpNormal += GetBumpNormal(input.Uv, input.Normal, input.Tangent, input.Binormal, 4);
    }
    else
    {
        bumpNormal = input.Normal;
    }

    bumpNormal = normalize(bumpNormal);

	
    float bias;
    float4 color;
    float2 projectTexCoord;
    float depthMapValue;
    float lightDepthValue;
    float lightIntensity;
    float lightDir;

    color = ambient;

    lightDir = -lightDirection;
    
    bias = 0.0001f;
    input.lightViewPosition.xyz /= input.lightViewPosition.w;

    projectTexCoord.x = input.lightViewPosition.x  / 2.0f + 0.5f;
    projectTexCoord.y = input.lightViewPosition.y  / -2.0f + 0.5f;

    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {
        depthMapValue = DepthMap.Sample(Samp, projectTexCoord).r;

        lightDepthValue = input.lightViewPosition.z;

        lightDepthValue = lightDepthValue - bias;

        if (lightDepthValue < depthMapValue)
        {
            lightIntensity = saturate(dot(bumpNormal, lightDir));

            if (lightIntensity > 0.0f)
            {
                color += (diffuse * lightIntensity);
                color = saturate(color);
            }
        }
    }
    else
    {
        lightIntensity = saturate(dot(bumpNormal, lightDir));
        if (lightIntensity > 0.0f)
        {
            color += (diffuse * lightIntensity);
            color = saturate(color);
        }
    }
    

    color = color * texColor + input.BrushColor;
     
    return color;
}
