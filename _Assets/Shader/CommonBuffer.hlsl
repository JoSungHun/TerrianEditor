//================================================================
//Global Constant Buffer
//================================================================
cbuffer GlobalBuffer : register(b0)
{
    matrix global_wvp;
    matrix global_view;
    matrix global_proj;
    matrix global_proj_ortho;
    matrix global_vp;
    matrix global_vp_inverse;
    matrix global_vp_ortho;	
    float global_camera_near;
    float global_camera_far;
    float2 global_resolution;
    float3 global_camera_position;
    float global_bloom_intensity;
};

#define global_texel_size float2(1.0f / global_resolution.x, 1.0f / global_resolution.y)