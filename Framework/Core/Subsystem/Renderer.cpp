#include "Framework.h"
#include "Renderer.h"
#include "../D3D11/CommandList.h"
#include "../../Scene/Scene.h"
#include "../../Scene/Actor.h"
#include "../../Scene/Component/Camera.h"
#include "../../Scene/Component/Transform.h"
#include "../../Scene/Component/Renderable.h"
#include "../../Scene/Component/Terrain/Terrain.h"
#include "../../Scene/Component/Light/Light.h"
#include "../../Scene/Component/Skybox/Skybox.h"

Renderer::Renderer(Context * context)
	: ISubsystem(context)
	, skybox(nullptr)
	, terrain(nullptr)
	, render_flags(0)
	, bloomIntensity(0.4f)
	, blurSigma(1.0f)
	, debug_type(RendererDebugType::None)
{
	render_flags =
		PostProcess_Bloom |
		PostProcess_SSAO ;

	resolution = D3DXVECTOR2
	(
		Settings::Get().GetWidth(),
		Settings::Get().GetHeight()
	);

	EventSystem::Get().Subscribe(EventType::Render, EVENT_HANDLER(Render));
}

Renderer::~Renderer()
{
	Clear();
}

const bool Renderer::Initialize()
{
	D3DXMatrixIdentity(&camera_view);
	D3DXMatrixIdentity(&camera_proj);
	D3DXMatrixIdentity(&camera_vp);
	D3DXMatrixIdentity(&camera_vp_inv);
	D3DXMatrixIdentity(&postprocess_view);
	D3DXMatrixIdentity(&postprocess_ortho);
	D3DXMatrixIdentity(&postprocess_vp);

	line_vertex_buffer = std::make_shared<VertexBuffer>(context);
	editorCamera = std::make_shared<Camera>(context);
	commandList = std::make_shared<CommandList>(context);

	CreateRenderTextures();
	CreateTextures();
	CreateShaders();
	CreateConstantBuffers();
	CreateSamplers();
	CreateRasterizerStates();
	CreateBlendStates();
	CreateDepthStencilStates();

	return true;
}

auto Renderer::GetFrameResource() -> ID3D11ShaderResourceView *
{
	return full_hdr_target2 ? full_hdr_target2->GetShaderResourceView() : nullptr;
}

auto Renderer::GetMainCamera() const -> class Camera *
{
	return Engine::IsOnEngineFlag(EngineFlags_Game) ? sceneCamera : editorCamera.get();
}

void Renderer::SetResolution(const uint & width, const uint & height)
{
	if (width == 0 || height == 0)
	{
		LOG_ERROR_F("%dx%d is an invalid resolution", width, height);
		return;
	}

	if (resolution.x == width && resolution.y == height)
		return;

	resolution.x = static_cast<float>((width % 2 != 0) ? width - 1 : width);
	resolution.y = static_cast<float>((height % 2 != 0) ? height - 1 : height);

	CreateRenderTextures();

	LOG_INFO_F("Resolution set to %dx%d", width, height);
}

auto Renderer::GetCursorPos(D3DXVECTOR2 & pos) -> const bool
{
	if (mouse_pos.x < 0.0f || mouse_pos.x > resolution.x ||
		mouse_pos.y < 0.0f || mouse_pos.y > resolution.y)
		return false;

	pos = mouse_pos;
	return true;
}


void Renderer::AcquireRenderables(Scene * scene)
{
	Clear();

	auto actors = scene->GetActors();
	for (const auto& actor : actors)
	{
		auto renderable = actor->GetComponent<Renderable>();
		auto light = actor->GetComponent<Light>();
		auto skybox = actor->GetComponent<Skybox>();
		auto camera = actor->GetComponent<Camera>();
		auto terrain = actor->GetComponent<Terrain>();

		if (renderable)
		{
			if (!skybox && !terrain)
			{
				auto type = renderable->GetRenderableType();
				if(type == RenderableType::Opaque)
					renderables[RenderableType::Opaque].emplace_back(actor);
				if (type == RenderableType::Transparent)
					renderables[RenderableType::Transparent].emplace_back(actor);
				
			}
		}

		if (light)
			renderables[RenderableType::Light].emplace_back(actor);

		if (skybox)
			this->skybox = skybox;

		if (camera)
		{
			renderables[RenderableType::Camera].emplace_back(actor);
			sceneCamera = camera;
		}

		if (terrain)
			this->terrain = terrain;

		//TODO :
	}
}

void Renderer::Clear()
{
	renderables.clear();
}

void Renderer::DrawLine(const D3DXVECTOR3 & from, const D3DXVECTOR3 & to, const D3DXCOLOR & from_color, const D3DXCOLOR & to_color, const bool & bDepth)
{
	if (bDepth)
	{
		depth_enabled_line_vertices.emplace_back(from, from_color);
		depth_enabled_line_vertices.emplace_back(to, to_color);
	}
	else
	{
		depth_disabled_line_vertices.emplace_back(from, from_color);
		depth_disabled_line_vertices.emplace_back(to, to_color);
	}
}

void Renderer::Render()
{
	auto camera = GetMainCamera();
	if (!camera)
		return;

	if (renderables.empty())
		return;

	camera->UpdateEditorCamera();

	camera_view = camera->GetViewMatrix();
	camera_proj = camera->GetProjectionMatrix();
	camera_vp = camera->GetViewProjectionMatrix();
	camera_vp_inv = camera->GetViewProjectionInverseMatrix();
	camera_position = camera->GetTransform()->GetTranslation();
	camera_near = camera->GetNearPlane();
	camera_far = camera->GetFarPlane();

	D3DXMatrixOrthoLH
	(
		&postprocess_ortho,
		resolution.x,
		resolution.y,
		camera_near,
		camera_far
	);

	postprocess_view = camera->GetViewBaseMatrix();
	postprocess_vp = postprocess_view * postprocess_ortho;

	PassMain();
}

void Renderer::CreateRenderTextures()
{
	auto width = static_cast<uint>(resolution.x);
	auto height = static_cast<uint>(resolution.y);

	//Screen Quad
	Geometry<VertexTexture> geometry;
	GeometryUtility::CreateScreenQuad(geometry, width, height);

	screen_vertex_buffer = std::make_shared<VertexBuffer>(context);
	screen_vertex_buffer->Create(geometry.GetVertices());

	screen_index_buffer = std::make_shared<IndexBuffer>(context);
	screen_index_buffer->Create(geometry.GetIndices());

	//GBuffer
	gbuffer_albedo = std::make_shared<RenderTexture>(context);
	gbuffer_albedo->Create(width, height, false);

	gbuffer_normal = std::make_shared<RenderTexture>(context);
	gbuffer_normal->Create(width, height, false, DXGI_FORMAT_R16G16B16A16_FLOAT);

	gbuffer_material = std::make_shared<RenderTexture>(context);
	gbuffer_material->Create(width, height, false);

	gbuffer_velocity = std::make_shared<RenderTexture>(context);
	gbuffer_velocity->Create(width, height, false, DXGI_FORMAT_R16G16_FLOAT);

	gbuffer_depth = std::make_shared<RenderTexture>(context);
	gbuffer_depth->Create(width, height, true, DXGI_FORMAT_R32G32_FLOAT, DXGI_FORMAT_D32_FLOAT);

	full_hdr_target1 = std::make_unique<RenderTexture>(context);
	full_hdr_target1->Create(width, height, false, DXGI_FORMAT_R32G32B32A32_FLOAT);

	full_hdr_target2 = std::make_unique<RenderTexture>(context);
	full_hdr_target2->Create(width, height, false, DXGI_FORMAT_R32G32B32A32_FLOAT);

	full_spare = std::make_unique<RenderTexture>(context);
	full_spare->Create(width, height, false, DXGI_FORMAT_R16G16B16A16_FLOAT);

	half_shadows = std::make_unique<RenderTexture>(context);
	half_shadows->Create(width / 2, height / 2, false, DXGI_FORMAT_R8_UNORM);

	half_ssao = std::make_unique<RenderTexture>(context);
	half_ssao->Create(width / 2, height / 2, false, DXGI_FORMAT_R8_UNORM);

	half_spare = std::make_unique<RenderTexture>(context);
	half_spare->Create(width / 2, height / 2, false, DXGI_FORMAT_R8_UNORM);

	half_spare2 = std::make_unique<RenderTexture>(context);
	half_spare2->Create(width / 2, height / 2, false, DXGI_FORMAT_R16G16B16A16_FLOAT);

	quarter_blur1 = std::make_unique<RenderTexture>(context);
	quarter_blur1->Create(width / 4, height / 4, false, DXGI_FORMAT_R16G16B16A16_FLOAT);

	quarter_blur2 = std::make_unique<RenderTexture>(context);
	quarter_blur2->Create(width / 4, height / 4, false, DXGI_FORMAT_R16G16B16A16_FLOAT);
}

void Renderer::CreateTextures()
{
	noise_normal_texture = std::make_shared<Texture>(context);
	noise_normal_texture->LoadFromFile("../_Assets/Texture/noise.jpg");

	white_texture = std::make_shared<Texture>(context);
	white_texture->LoadFromFile("../_Assets/Texture/white.png");

	black_texture = std::make_shared<Texture>(context);
	black_texture->LoadFromFile("../_Assets/Texture/black.png");

	lut_ibl_texture = std::make_shared<Texture>(context);
	lut_ibl_texture->LoadFromFile("../_Assets/Texture/ibl_brdf_lut.png");

}

void Renderer::CreateShaders()
{
	vs_gbuffer = std::make_shared<Shader>(context);
	vs_gbuffer->AddShader(ShaderStage::VS, "../_Assets/Shader/GBuffer.hlsl");

	vs_quad = std::make_shared<Shader>(context);
	vs_quad->AddShader(ShaderStage::VS, "../_Assets/Shader/Postprocess.hlsl");

	vs_depth = std::make_shared<Shader>(context);
	vs_depth->AddShader(ShaderStage::VS, "../_Assets/Shader/Depth.hlsl");

	ps_texture = std::make_shared<Shader>(context);
	ps_texture->AddDefine("PASS_TEXTURE");
	ps_texture->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_debug_normal = std::make_shared<Shader>(context);
	ps_debug_normal->AddDefine("DEBUG_NORMAL");
	ps_debug_normal->AddShader(ShaderStage::PS, "../_Assets/Shader/Debug.hlsl");

	ps_debug_velocity = std::make_shared<Shader>(context);
	ps_debug_velocity->AddDefine("DEBUG_VELOCITY");
	ps_debug_velocity->AddShader(ShaderStage::PS, "../_Assets/Shader/Debug.hlsl");

	ps_debug_ssao = std::make_shared<Shader>(context);
	ps_debug_ssao->AddDefine("DEBUG_SSAO");
	ps_debug_ssao->AddShader(ShaderStage::PS, "../_Assets/Shader/Debug.hlsl");

	ps_debug_depth = std::make_shared<Shader>(context);
	ps_debug_depth->AddDefine("DEBUG_DEPTH");
	ps_debug_depth->AddShader(ShaderStage::PS, "../_Assets/Shader/Debug.hlsl");

	ps_bloom_bright = std::make_shared<Shader>(context);
	ps_bloom_bright->AddDefine("PASS_BRIGHT");
	ps_bloom_bright->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_bloom_blend = std::make_shared<Shader>(context);
	ps_bloom_blend->AddDefine("PASS_BLEND");
	ps_bloom_blend->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_downsample_box = std::make_shared<Shader>(context);
	ps_downsample_box->AddDefine("PASS_DOWNSAMPLE_BOX");
	ps_downsample_box->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_upsample_box = std::make_shared<Shader>(context);
	ps_upsample_box->AddDefine("PASS_UPSAMPLE_BOX");
	ps_upsample_box->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_blur_box = std::make_shared<Shader>(context);
	ps_blur_box->AddDefine("PASS_BOX_BLUR");
	ps_blur_box->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_blur_gaussian = std::make_shared<Shader>(context);
	ps_blur_gaussian->AddDefine("PASS_GAUSSIAN_BLUR");
	ps_blur_gaussian->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_blur_gaussian_bilateral = std::make_shared<Shader>(context);
	ps_blur_gaussian_bilateral->AddDefine("PASS_GAUSSIAN_BILATERAL_BLUR");
	ps_blur_gaussian_bilateral->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_gamma_correction = std::make_shared<Shader>(context);
	ps_gamma_correction->AddDefine("PASS_GAMMA_CORRECTION");
	ps_gamma_correction->AddShader(ShaderStage::PS, "../_Assets/Shader/Postprocess.hlsl");

	ps_ssao = std::make_shared<Shader>(context);
	ps_ssao->AddShader(ShaderStage::PS, "../_Assets/Shader/SSAO.hlsl");

	ps_shadow_mapping_point = std::make_shared<Shader>(context);
	ps_shadow_mapping_point->AddDefine("POINT");
	ps_shadow_mapping_point->AddShader(ShaderStage::PS, "../_Assets/Shader/ShadowMapping.hlsl");
	
	ps_shadow_mapping_spot = std::make_shared<Shader>(context);
	ps_shadow_mapping_spot->AddDefine("SPOT");
	ps_shadow_mapping_spot->AddShader(ShaderStage::PS, "../_Assets/Shader/ShadowMapping.hlsl");
	
	vps_shadow_mapping_directional = std::make_shared<Shader>(context);
	vps_shadow_mapping_directional->AddDefine("DIRECTIONAL");
	vps_shadow_mapping_directional->AddShader(ShaderStage::VS, "../_Assets/Shader/ShadowMapping.hlsl");
	vps_shadow_mapping_directional->AddShader(ShaderStage::PS, "../_Assets/Shader/ShadowMapping.hlsl");

	vps_light = std::make_shared<Shader>(context);
	vps_light->AddShader(ShaderStage::VS, "../_Assets/Shader/Light.hlsl");
	vps_light->AddShader(ShaderStage::PS, "../_Assets/Shader/Light.hlsl");

	vps_transparent = std::make_shared<Shader>(context);
	vps_transparent->AddShader(ShaderStage::VS, "../_Assets/Shader/Transparent.hlsl");
	vps_transparent->AddShader(ShaderStage::PS, "../_Assets/Shader/Transparent.hlsl");

	vps_color = std::make_shared<Shader>(context);
	vps_color->AddShader(ShaderStage::VS, "../_Assets/Shader/Color.hlsl");
	vps_color->AddShader(ShaderStage::PS, "../_Assets/Shader/Color.hlsl");
}

void Renderer::CreateConstantBuffers()
{
	global_buffer = std::make_shared<ConstantBuffer>(context);
	global_buffer->Create<GlobalData>();

	blur_buffer = std::make_shared<ConstantBuffer>(context);
	blur_buffer->Create<BlurData>();

	shadow_map_buffer = std::make_shared<ConstantBuffer>(context);
	shadow_map_buffer->Create<ShadowMapData>();

	light_buffer = std::make_shared<ConstantBuffer>(context);
	light_buffer->Create<LightData>();

	cascade_buffer = std::make_shared<ConstantBuffer>(context);
	cascade_buffer->Create<LightCascadeData>();

	transparent_buffer = std::make_shared<ConstantBuffer>(context);
	transparent_buffer->Create<TransparentData>();
}

void Renderer::CreateSamplers()
{
	compare_depth = std::make_shared<Sampler>(context);
	compare_depth->Create(D3D11_FILTER_COMPARISON_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_COMPARISON_GREATER);

	point_clamp = std::make_shared<Sampler>(context);
	point_clamp->Create(D3D11_FILTER_MIN_MAG_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_COMPARISON_ALWAYS);

	bilinear_clamp = std::make_shared<Sampler>(context);
	bilinear_clamp->Create(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_COMPARISON_ALWAYS);

	bilinear_wrap = std::make_shared<Sampler>(context);
	bilinear_wrap->Create(D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS);

	trilinear_clamp = std::make_shared<Sampler>(context);
	trilinear_clamp->Create(D3D11_FILTER_MIN_MAG_MIP_LINEAR, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS);

	anisotropic_wrap = std::make_shared<Sampler>(context);
	anisotropic_wrap->Create(D3D11_FILTER_ANISOTROPIC, D3D11_TEXTURE_ADDRESS_WRAP, D3D11_COMPARISON_ALWAYS);
}

void Renderer::CreateRasterizerStates()
{
	cull_none_solid = std::make_shared<RasterizerState>(context);
	cull_none_solid->Create(CullMode::NONE, FillMode::SOLID);

	cull_back_solid = std::make_shared<RasterizerState>(context);
	cull_back_solid->Create(CullMode::BACK, FillMode::SOLID);

	cull_front_solid = std::make_shared<RasterizerState>(context);
	cull_front_solid->Create(CullMode::FRONT, FillMode::SOLID);

	cull_none_wireframe = std::make_shared<RasterizerState>(context);
	cull_none_wireframe->Create(CullMode::NONE, FillMode::WIREFRAME);

	cull_back_wireframe = std::make_shared<RasterizerState>(context);
	cull_back_wireframe->Create(CullMode::BACK, FillMode::WIREFRAME);

	cull_front_wireframe = std::make_shared<RasterizerState>(context);
	cull_front_wireframe->Create(CullMode::FRONT, FillMode::WIREFRAME);
}

void Renderer::CreateBlendStates()
{
	blend_enabled = std::make_shared<BlendState>(context);
	blend_enabled->Create
	(
		true,
		Factor::SRC_ALPHA,
		Factor::INV_SRC_ALPHA,
		Operation::ADD,
		Factor::ONE,
		Factor::ONE,
		Operation::ADD
	);

	blend_disabled = std::make_shared<BlendState>(context);
	blend_disabled->Create
	(
		false,
		Factor::SRC_ALPHA,
		Factor::INV_SRC_ALPHA,
		Operation::ADD,
		Factor::ONE,
		Factor::ONE,
		Operation::ADD
	);

	blend_shadow_map = std::make_shared<BlendState>(context);
	blend_shadow_map->Create
	(
		false,
		Factor::SRC_COLOR,
		Factor::SRC_COLOR,
		Operation::MIN,
		Factor::ONE,
		Factor::ONE,
		Operation::ADD
	);
}

void Renderer::CreateDepthStencilStates()
{
	depth_stencil_enabled = std::make_shared<DepthStencilState>(context);
	depth_stencil_enabled->Create(true);

	depth_stencil_disabled = std::make_shared<DepthStencilState>(context);
	depth_stencil_disabled->Create(false);
}

void Renderer::UpdateGlobalBuffer(const uint & width, const uint & height, const D3DXMATRIX & wvp)
{
	auto data = global_buffer->Map<GlobalData>();
	if (!data)
	{
		LOG_ERROR("Invalid to buffer");
		return;
	}

	D3DXMatrixTranspose(&data->WVP, &wvp);
	D3DXMatrixTranspose(&data->View, &camera_view);
	D3DXMatrixTranspose(&data->Proj, &camera_proj);
	D3DXMatrixTranspose(&data->Proj_Ortho, &postprocess_ortho);
	D3DXMatrixTranspose(&data->VP, &camera_vp);
	D3DXMatrixTranspose(&data->VP_Inverse, &camera_vp_inv);
	D3DXMatrixTranspose(&data->VP_Ortho, &postprocess_vp);
	data->Resolution = D3DXVECTOR2(static_cast<float>(width), static_cast<float>(height));
	data->Camera_Near = camera_near;
	data->Camera_Far = camera_far;
	data->Camera_Position = camera_position;
	data->Bloom_Intensity = bloomIntensity;

	global_buffer->Unmap();
}

void Renderer::UpdateShadowMapBuffer(Light * light)
{
	auto data = shadow_map_buffer->Map<ShadowMapData>();
	if (!data)
	{
		LOG_ERROR("Failed to map buffer");
		return;
	}

	auto light_view = light->GetViewMatrix();
	D3DXMatrixTranspose(&data->View, &light_view);
	D3DXMatrixTranspose(&data->View_Projection_Inv, &camera_vp_inv);
	D3DXMatrixTranspose(&data->View_Projection[0], &(light_view * light->GetProjectionMatrix(0)));
	D3DXMatrixTranspose(&data->View_Projection[1], &(light_view * light->GetProjectionMatrix(1)));
	D3DXMatrixTranspose(&data->View_Projection[2], &(light_view * light->GetProjectionMatrix(2)));
	data->Biases = D3DXVECTOR2(light->GetBias(), light->GetNormalBias());
	data->Direction = light->GetDirection();
	data->Range = light->GetRange();
	data->Position = light->GetTransform()->GetTranslation();
	data->Resolution = static_cast<float>(light->GetShadowMap()->GetWidth());

	shadow_map_buffer->Unmap();
}

void Renderer::UpdateBlurBuffer(const D3DXVECTOR2 & direction, const float & sigma)
{
	auto data = blur_buffer->Map<BlurData>();
	if (!data)
	{
		LOG_ERROR("Failed to map buffer");
		return;
	}

	data->Direction = direction;
	data->Sigma = sigma;

	blur_buffer->Unmap();
}

void Renderer::UpdateLightBuffer()
{
	auto& lights = renderables[RenderableType::Light];

	if (lights.empty())
		return;

	auto data = light_buffer->Map<LightData>();
	if (!data)
	{
		LOG_ERROR("Failed to map buffer");
		return;
	}

	D3DXMatrixTranspose(&data->WVP, &postprocess_vp);
	D3DXMatrixTranspose(&data->VP_Inv, &camera_vp_inv);

	//Initialize
	data->Dir_LightColor = D3DXCOLOR(0, 0, 0, 0);
	data->Dir_LightDirection = D3DXVECTOR4(0, 0, 0, 0);
	data->Dir_LightIntensity = D3DXVECTOR4(0, 0, 0, 0);

	for (auto i = 0; i < MaxLights; i++)
	{
		data->Point_LightPosition[i] = D3DXVECTOR4(0, 0, 0, 0);
		data->Point_LightColor[i] = D3DXCOLOR(0, 0, 0, 0);
		data->Point_LightIntensity_Range[i] = D3DXVECTOR4(0, 0, 0, 0);
		data->Spot_LightPosition[i] = D3DXVECTOR4(0, 0, 0, 0);
		data->Spot_LightColor[i] = D3DXCOLOR(0, 0, 0, 0);
		data->Spot_LightDirection[i] = D3DXVECTOR4(0, 0, 0, 0);
		data->Spot_LightIntensity_Range_Angle[i] = D3DXVECTOR4(0, 0, 0, 0);
	}

	//Directional
	for (const auto& light : lights)
	{
		auto component = light->GetComponent<Light>();

		if (component->GetLightType() != LightType::Directional)
			continue;

		const auto direction = component->GetDirection();
		const auto intensity = component->GetIntensity();

		data->Dir_LightColor = component->GetColor();
		data->Dir_LightIntensity = D3DXVECTOR4(intensity, intensity, intensity, intensity);
		data->Dir_LightDirection = D3DXVECTOR4(direction.x, direction.y, direction.z, 0.0f);
	}

	//Point
	auto point_index = 0;
	for (const auto& light : lights)
	{
		auto component = light->GetComponent<Light>();

		if (component->GetLightType() != LightType::Point)
			continue;

		const auto position = light->GetTransform()->GetTranslation();

		data->Point_LightPosition[point_index] = D3DXVECTOR4(position.x, position.y, position.z, 1.0f);
		data->Point_LightColor[point_index] = component->GetColor();
		data->Point_LightIntensity_Range[point_index] = D3DXVECTOR4(component->GetIntensity(), component->GetRange(), 0.0f, 0.0f);

		point_index++;
	}

	//Spot
	auto spot_index = 0;
	for (const auto& light : lights)
	{
		auto component = light->GetComponent<Light>();

		if (component->GetLightType() != LightType::Spot)
			continue;

		const auto direction = component->GetDirection();
		const auto position = light->GetTransform()->GetTranslation();

		data->Spot_LightPosition[spot_index] = D3DXVECTOR4(position.x, position.y, position.z, 1.0f);
		data->Spot_LightColor[spot_index] = component->GetColor();
		data->Spot_LightDirection[spot_index] = D3DXVECTOR4(direction.x, direction.y, direction.z, 0.0f);
		data->Spot_LightIntensity_Range_Angle[spot_index] = D3DXVECTOR4(component->GetIntensity(), component->GetRange(), component->GetAngle(), 0.0f);

		spot_index++;
	}

	data->Point_LightCount = static_cast<float>(point_index);
	data->Spot_LightCount = static_cast<float>(spot_index);
	data->Screen_Space_Reflection = IsOnFlag(PostProcess_SSR) ? 1 : 0;

	light_buffer->Unmap();
}

void Renderer::UpdateLightCascadeBuffer(const D3DXMATRIX & wvp, const uint & index)
{
	auto data = cascade_buffer->Map<LightCascadeData>();
	if (!data)
	{
		LOG_ERROR("Failed to map buffer");
		return;
	}

	D3DXMatrixTranspose(&data->WVP, &wvp);

	cascade_buffer->Unmap();
}

void Renderer::UpdateTransparentBuffer(const D3DXMATRIX & world, const D3DXCOLOR & color, const float & roughness)
{
	auto data = transparent_buffer->Map<TransparentData>();
	if (!data)
	{
		LOG_ERROR("Failed to map buffer");
		return;
	}

	D3DXMatrixTranspose(&data->World, &world);
	D3DXMatrixTranspose(&data->WVP, &(world * camera_vp));
	data->Color = color;
	data->Camera_Pos = camera_position;
	data->Roughness = roughness;
	data->Light_Dir = directional_avg_dir;

	transparent_buffer->Unmap();
}

auto Renderer::GetDirectionalLight() -> class Light *
{
	for (const auto& actor : renderables[RenderableType::Light])
	{
		auto light = actor->GetComponent<Light>();
		if (light->GetLightType() == LightType::Directional)
			return light;
	}
	return nullptr;
}

auto Renderer::GetRasterizerState(const CullMode & cull_mode, const FillMode & fill_mode) -> RasterizerState *
{
	switch (cull_mode)
	{
	case CullMode::NONE:
	{
		switch (fill_mode)
		{
		case FillMode::SOLID:       return cull_none_solid.get();
		case FillMode::WIREFRAME:   return cull_none_wireframe.get();
		}
		break;
	}
	case CullMode::FRONT:
	{
		switch (fill_mode)
		{
		case FillMode::SOLID:       return cull_front_solid.get();
		case FillMode::WIREFRAME:   return cull_front_wireframe.get();
		}
		break;
	}
	case CullMode::BACK:
	{
		switch (fill_mode)
		{
		case FillMode::SOLID:       return cull_back_solid.get();
		case FillMode::WIREFRAME:   return cull_back_wireframe.get();
		}
		break;
	}
	}

	return cull_back_solid.get();
}
