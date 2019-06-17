#include "Framework.h"
#include "Renderer.h"
#include "../D3D11/CommandList.h"
#include "../../Scene/Actor.h"
#include "../../Scene/Component/Camera.h"
#include "../../Scene/Component/Transform.h"
#include "../../Scene/Component/Renderable.h"
#include "../../Scene/Component/Terrain/Terrain.h"
#include "../../Scene/Component/Light/Light.h"
#include "../../Scene/Component/Skybox/Skybox.h"

static const float GIZMO_MAX_SIZE = 5.0f;
static const float GIZMO_MIN_SIZE = 0.1f;

void Renderer::PassMain()
{
	commandList->Begin("PassMain");

	PassLightDepth();
	PassGBuffer();
	PassPreLight
	(
		half_spare,
		half_shadows,
		half_ssao
	);
	PassLight
	(
		half_shadows,
		half_ssao,
		full_hdr_target1
	);
	PassTransparent(full_hdr_target1);
	PassPostLight
	(
		full_hdr_target1,
		full_hdr_target2
	);

	PassDebug(full_hdr_target2);

	commandList->End();
	commandList->Submit();
}

void Renderer::PassLightDepth()
{
	uint directional_count = 0;

	for (const auto& light_actor : renderables[RenderableType::Light])
	{
		auto light = light_actor->GetComponent<Light>();

		if (!light->IsCastShadow())
			continue;

		auto shadow_map = light->GetShadowMap();
		if (!shadow_map)
			continue;

		auto opaque_actors = renderables[RenderableType::Opaque];
		if (opaque_actors.empty())
			continue;

		commandList->Begin("PassLightDepth");

		commandList->SetBlendState(blend_disabled.get());
		commandList->SetDepthStencilState(depth_stencil_enabled.get());
		commandList->SetRasterizerState(cull_back_solid.get());
		commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		commandList->SetInputLayout(vs_depth->GetInputLayout());
		commandList->SetVertexShader(vs_depth->GetVertexShader());
		commandList->SetPixelShader(nullptr);
		commandList->SetViewport(shadow_map->GetViewport());

		uint current_mesh = 0;

		for (uint i = 0; i < light->GetShadowMap()->GetArraySize(); i++)
		{
			commandList->Begin("PassArray_" + std::to_string(i + 1));
			commandList->ClearDepthStencilTarget(shadow_map, D3D11_CLEAR_DEPTH, GetClearDepth());
			commandList->SetRenderTarget(nullptr, shadow_map);

			D3DXMATRIX light_vp = light->GetViewMatrix(i) * light->GetProjectionMatrix(i);

			for (const auto& opaque_actor : opaque_actors)
			{
				auto renderable = opaque_actor->GetComponent<Renderable>();
				auto material = renderable ? renderable->GetMaterial() : nullptr;
				auto mesh = renderable ? renderable->GetMesh() : nullptr;
				auto vertex_buffer = mesh ? mesh->GetVertexBuffer() : nullptr;
				auto index_buffer = mesh ? mesh->GetIndexBuffer() : nullptr;

				if (!renderable || !material || !mesh || !vertex_buffer || !index_buffer)
					continue;

				if (!renderable->IsCastShadow())
					continue;

				if (material->GetColorAlbedo().a < 1.0f)
					continue;

				if (current_mesh != mesh->GetResourceID())
				{
					commandList->SetVertexBuffer(vertex_buffer);
					commandList->SetIndexBuffer(index_buffer);
					current_mesh = mesh->GetResourceID();
				}

				if (light->GetLightType() == LightType::Directional)
				{
					directional_avg_dir += light->GetDirection();
					directional_count++;
				}

				auto transform = opaque_actor->GetTransform();
				UpdateLightCascadeBuffer(transform->GetWorldMatrix() * light_vp, i);

				commandList->SetConstantBuffer(1, ShaderStage::VS, cascade_buffer.get());
				commandList->DrawIndexed
				(
					index_buffer->GetIndexCount(),
					index_buffer->GetOffset(),
					vertex_buffer->GetOffset()
				);
			}
			commandList->End();
		}

		commandList->End();
		commandList->Submit();
	}

	directional_avg_dir /= static_cast<float>(directional_count);
}

void Renderer::PassGBuffer()
{


	commandList->Begin("PassGBuffer");

	if (renderables[RenderableType::Opaque].empty() && !terrain)
	{
		commandList->ClearRenderTarget(gbuffer_albedo.get());
		commandList->ClearRenderTarget(gbuffer_normal.get());
		commandList->ClearRenderTarget(gbuffer_material.get());
		commandList->ClearRenderTarget(gbuffer_velocity.get());
		commandList->ClearDepthStencilTarget(gbuffer_depth.get(), D3D11_CLEAR_DEPTH, GetClearDepth());
		commandList->End();
		commandList->Submit();
		return;
	}

	UpdateGlobalBuffer(static_cast<uint>(resolution.x), static_cast<uint>(resolution.y));

	std::vector<ID3D11RenderTargetView*> render_targets
	{
		gbuffer_albedo->GetRenderTargetView(),
		gbuffer_normal->GetRenderTargetView(),
		gbuffer_material->GetRenderTargetView(),
		gbuffer_velocity->GetRenderTargetView(),
	};

	commandList->SetRasterizerState(cull_back_solid.get());
	commandList->SetBlendState(blend_disabled.get());
	commandList->SetDepthStencilState(depth_stencil_enabled.get());
	commandList->SetRenderTargets(render_targets, gbuffer_depth->GetDepthStencilView());
	commandList->SetViewport(gbuffer_albedo->GetViewport());
	commandList->ClearRenderTargets(render_targets);
	commandList->ClearDepthStencilTarget(gbuffer_depth.get(), D3D11_CLEAR_DEPTH, GetClearDepth());

	commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetInputLayout(vs_gbuffer->GetInputLayout());
	commandList->SetVertexShader(vs_gbuffer->GetVertexShader());
	commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
	commandList->SetSamplerState(0, ShaderStage::PS, anisotropic_wrap.get());

	uint current_material = 0;
	uint current_mesh = 0;
	uint current_shader = 0;

	if (terrain)
	{
		commandList->Begin("Terrain");
		auto renderable = terrain->GetActor()->GetComponent<Renderable>();
		auto material = renderable ? renderable->GetMaterial() : nullptr;
		auto mesh = renderable ? renderable->GetMesh() : nullptr;
		auto shader = material ? material->GetShader() : nullptr;
		auto vertex_buffer = mesh ? mesh->GetVertexBuffer() : nullptr;
		auto index_buffer = mesh ? mesh->GetIndexBuffer() : nullptr;

		if (renderable && material && mesh && shader && vertex_buffer && index_buffer)
		{
			commandList->SetVertexBuffer(vertex_buffer);
			commandList->SetIndexBuffer(index_buffer);

			std::vector<ID3D11ShaderResourceView*> textures
			{
				terrain->GetSplatMapShaderResourceView(),
				terrain->GetBrushSharderResourceView(),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::Default,	ChannelType::Diffuse),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::Default,	ChannelType::Normal),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::R,			ChannelType::Diffuse),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::R,			ChannelType::Normal),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::G,			ChannelType::Diffuse),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::G,			ChannelType::Normal),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::B,			ChannelType::Diffuse),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::B,			ChannelType::Normal),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::A,			ChannelType::Diffuse),
				terrain->GetSplatChannelShaderResourceView(SplatChannel::A,			ChannelType::Normal),
			};
			material->UpdateConstantBuffer();
			terrain->UpdateTerrainBuffer();
			auto transform = terrain->GetTransform();
			transform->UpdateConstantBuffer(camera_vp);

			commandList->SetPixelShader(shader->GetPixelShader());
			commandList->SetRasterizerState(GetRasterizerState(material->GetCullMode()));
			commandList->SetShaderResources(0, ShaderStage::PS, textures);
			commandList->SetConstantBuffer(1, ShaderStage::PS, material->GetConstantBuffer());
			commandList->SetConstantBuffer(2, ShaderStage::Global, transform->GetConstantBuffer());
			commandList->SetConstantBuffer(3, ShaderStage::PS, terrain->GetTerrainBuffer());


			commandList->DrawIndexed
			(
				index_buffer->GetIndexCount(),
				index_buffer->GetOffset(),
				vertex_buffer->GetOffset()
			);
		}
		commandList->End();
	}

	

	for (const auto& actor : renderables[RenderableType::Opaque])
	{
		auto renderable = actor->GetComponent<Renderable>();
		auto material = renderable ? renderable->GetMaterial() : nullptr;
		auto mesh = renderable ? renderable->GetMesh() : nullptr;
		auto shader = material ? material->GetShader() : nullptr;
		auto vertex_buffer = mesh ? mesh->GetVertexBuffer() : nullptr;
		auto index_buffer = mesh ? mesh->GetIndexBuffer() : nullptr;

		if (!renderable || !material || !mesh || !shader || !vertex_buffer || !index_buffer)
			continue;

		if (current_mesh != mesh->GetResourceID())
		{
			commandList->SetVertexBuffer(mesh->GetVertexBuffer());
			commandList->SetIndexBuffer(mesh->GetIndexBuffer());
			current_mesh = mesh->GetResourceID();
		}

		if (current_shader != shader->GetShaderID())
		{
			commandList->SetPixelShader(shader->GetPixelShader());
			current_shader = shader->GetShaderID();
		}

		if (current_material != material->GetResourceID())
		{
			std::vector<ID3D11ShaderResourceView*> textures
			{
				material->GetTextureShaderResource(TextureType::Albedo),
				material->GetTextureShaderResource(TextureType::Roughness),
				material->GetTextureShaderResource(TextureType::Metallic),
				material->GetTextureShaderResource(TextureType::Normal),
				material->GetTextureShaderResource(TextureType::Height),
				material->GetTextureShaderResource(TextureType::Occlusion),
				material->GetTextureShaderResource(TextureType::Emissive),
				material->GetTextureShaderResource(TextureType::Mask),
			};
			material->UpdateConstantBuffer();

			commandList->SetRasterizerState(GetRasterizerState(material->GetCullMode()));
			commandList->SetShaderResources(0, ShaderStage::PS, textures);
			commandList->SetConstantBuffer(1, ShaderStage::PS, material->GetConstantBuffer());
			current_material = material->GetResourceID();
		}

		auto transform = actor->GetTransform();
		transform->UpdateConstantBuffer(camera_vp);
		commandList->SetConstantBuffer(2, ShaderStage::VS, transform->GetConstantBuffer());

		commandList->DrawIndexed
		(
			index_buffer->GetIndexCount(),
			index_buffer->GetOffset(),
			vertex_buffer->GetOffset()
		);
	}

	commandList->End();
	commandList->Submit();
}

void Renderer::PassPreLight(std::shared_ptr<class RenderTexture>& in, std::shared_ptr<class RenderTexture>& shadow_out, std::shared_ptr<class RenderTexture>& ssao_out)
{
	commandList->Begin("PassPreLight");
	commandList->SetDepthStencilState(depth_stencil_disabled.get());
	commandList->SetRasterizerState(cull_back_solid.get());
	commandList->SetBlendState(blend_disabled.get());
	commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetVertexBuffer(screen_vertex_buffer.get());
	commandList->SetIndexBuffer(screen_index_buffer.get());
	commandList->ClearRenderTarget(shadow_out.get(), D3DXCOLOR(1, 1, 1, 1));

	//Shadow mapping + blur
	auto bShadow_mapped = false;
	auto& lights = renderables[RenderableType::Light];
	for (uint i = 0; i < lights.size(); i++)
	{
		auto light = lights[i]->GetComponent<Light>();

		if (!light->IsCastShadow())
			continue;

		PassShadowMapping(shadow_out, light);
		bShadow_mapped = true;
	}

	if (!bShadow_mapped)
		commandList->ClearRenderTarget(shadow_out.get(), D3DXCOLOR(1, 1, 1, 1));

	//SSAO mapping + blur
	if (IsOnFlag(RendererOption::PostProcess_SSAO))
	{
		PassSSAO(in);
		const auto sigma = 1.0f;
		const auto pixel_stride = 1.0f;
		PassBilateralGaussianBlur(in, ssao_out, sigma, pixel_stride);
	}
	commandList->End();
}

void Renderer::PassLight(std::shared_ptr<class RenderTexture>& shadow_in, std::shared_ptr<class RenderTexture>& ssao_in, std::shared_ptr<class RenderTexture>& out)
{
	commandList->Begin("PassLight");

	UpdateGlobalBuffer(static_cast<uint>(resolution.x), static_cast<uint>(resolution.y));
	UpdateLightBuffer();

	std::vector<ID3D11SamplerState*> samplers
	{
		trilinear_clamp->GetState(),
		point_clamp->GetState(),
	};

	std::vector<ID3D11Buffer*> constant_buffers
	{
		global_buffer->GetBuffer(),
		light_buffer->GetBuffer(),
	};

	std::vector<ID3D11ShaderResourceView*> textures
	{
		gbuffer_albedo->GetShaderResourceView(),
		gbuffer_normal->GetShaderResourceView(),
		gbuffer_depth->GetShaderResourceView(),
		gbuffer_material->GetShaderResourceView(),
		shadow_in->GetShaderResourceView(),
		IsOnFlag(PostProcess_SSAO) ? ssao_in->GetShaderResourceView() : white_texture->GetShaderResourceView(),
		full_hdr_target2->GetShaderResourceView(),
		skybox ? skybox->GetTexture()->GetShaderResourceView() : white_texture->GetShaderResourceView(),
		lut_ibl_texture->GetShaderResourceView(),
	};

	commandList->SetDepthStencilState(depth_stencil_disabled.get());
	commandList->SetRasterizerState(cull_back_solid.get());
	commandList->SetBlendState(blend_disabled.get());
	commandList->SetRenderTarget(out.get());
	commandList->SetViewport(out->GetViewport());
	commandList->SetVertexBuffer(screen_vertex_buffer.get());
	commandList->SetIndexBuffer(screen_index_buffer.get());
	commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetInputLayout(vps_light->GetInputLayout());
	commandList->SetVertexShader(vps_light->GetVertexShader());
	commandList->SetPixelShader(vps_light->GetPixelShader());
	commandList->SetSamplerStates(0, ShaderStage::PS, samplers);
	commandList->SetConstantBuffers(0, ShaderStage::Global, constant_buffers);
	commandList->SetShaderResources(0, ShaderStage::PS, textures);
	commandList->DrawIndexed
	(
		screen_index_buffer->GetIndexCount(),
		screen_index_buffer->GetOffset(),
		screen_vertex_buffer->GetOffset()
	);

	commandList->End();
	commandList->Submit();
}

void Renderer::PassPostLight(std::shared_ptr<class RenderTexture>& in, std::shared_ptr<class RenderTexture>& out)
{
	commandList->Begin("PassPostLight");
	commandList->SetDepthStencilState(depth_stencil_disabled.get());
	commandList->SetRasterizerState(cull_back_solid.get());
	commandList->SetBlendState(blend_disabled.get());
	commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetVertexBuffer(screen_vertex_buffer.get());
	commandList->SetIndexBuffer(screen_index_buffer.get());
	commandList->SetInputLayout(vs_quad->GetInputLayout());
	commandList->SetVertexShader(vs_quad->GetVertexShader());

	const auto SwapTarget = [this, &in, &out]() {commandList->Submit(); out.swap(in); };

	if (IsOnFlag(RendererOption::PostProcess_TAA))
	{

	}

	if (IsOnFlag(RendererOption::PostProcess_Bloom))
	{
		PassBloom(in, out);
		SwapTarget();
	}


	if (IsOnFlag(RendererOption::PostProcess_FXAA))
	{

	}

	if (IsOnFlag(RendererOption::PostProcess_Sharpening))
	{

	}

	PassGammaCorrection(in, out);

	commandList->End();
	commandList->Submit();
}

void Renderer::PassTransparent(std::shared_ptr<class RenderTexture>& out)
{
	auto actors = renderables[RenderableType::Transparent];
	if (actors.empty())
		return;

	std::vector<ID3D11ShaderResourceView*> textures
	{
		gbuffer_depth->GetShaderResourceView(),
		skybox ? skybox->GetTexture()->GetShaderResourceView() : nullptr
	};

	commandList->Begin("PassTransparent");

	commandList->SetBlendState(blend_enabled.get());
	commandList->SetDepthStencilState(depth_stencil_enabled.get());
	commandList->SetRenderTarget(out.get(), gbuffer_depth.get());
	commandList->SetViewport(out->GetViewport());
	commandList->SetShaderResources(0, ShaderStage::PS, textures);
	commandList->SetSamplerState(0, ShaderStage::PS, bilinear_clamp.get());
	commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetInputLayout(vps_transparent->GetInputLayout());
	commandList->SetVertexShader(vps_transparent->GetVertexShader());
	commandList->SetPixelShader(vps_transparent->GetPixelShader());

	for (const auto& actor : actors)
	{
		auto renderable = actor->GetComponent<Renderable>();
		auto material = renderable ? renderable->GetMaterial() : nullptr;
		auto mesh = renderable ? renderable->GetMesh() : nullptr;
		auto vertex_buffer = mesh ? mesh->GetVertexBuffer() : nullptr;
		auto index_buffer = mesh ? mesh->GetIndexBuffer() : nullptr;

		if (!renderable || !material || !mesh || !vertex_buffer || !index_buffer)
			continue;

		commandList->SetRasterizerState(GetRasterizerState(material->GetCullMode()));
		commandList->SetVertexBuffer(vertex_buffer);
		commandList->SetIndexBuffer(index_buffer);

		auto transform = actor->GetTransform();
		UpdateTransparentBuffer
		(
			transform->GetWorldMatrix(),
			material->GetColorAlbedo(),
			material->GetRoughnessMultiplier()
		);

		commandList->SetConstantBuffer(1, ShaderStage::Global, transparent_buffer.get());
		commandList->DrawIndexed
		(
			index_buffer->GetIndexCount(),
			index_buffer->GetOffset(),
			vertex_buffer->GetOffset()
		);
	}

	commandList->End();
	commandList->Submit();
}

void Renderer::PassShadowMapping(std::shared_ptr<class RenderTexture>& out, Light * light)
{
	if (!light || !light->IsCastShadow())
		return;

	commandList->Begin("PassShadowMapping");

	std::shared_ptr<Shader> pixel_shader;
	switch (light->GetLightType())
	{
	case LightType::Directional:    pixel_shader = vps_shadow_mapping_directional;  break;
	case LightType::Point:          pixel_shader = ps_shadow_mapping_point;         break;
	case LightType::Spot:           pixel_shader = ps_shadow_mapping_spot;          break;
	}

	UpdateGlobalBuffer(out->GetWidth(), out->GetHeight());
	UpdateShadowMapBuffer(light);

	std::vector<ID3D11Buffer*> constant_buffers
	{
		global_buffer->GetBuffer(),
		shadow_map_buffer->GetBuffer()
	};

	std::vector<ID3D11SamplerState*> samplers
	{
		compare_depth->GetState(),
		bilinear_clamp->GetState()
	};

	std::vector<ID3D11ShaderResourceView*> textures
	{
		gbuffer_normal->GetShaderResourceView(),
		gbuffer_depth->GetShaderResourceView(),
		light->GetLightType() == LightType::Directional ? light->GetShadowMap()->GetShaderResourceView() : nullptr,
		light->GetLightType() == LightType::Point ? light->GetShadowMap()->GetShaderResourceView() : nullptr,
		light->GetLightType() == LightType::Spot ? light->GetShadowMap()->GetShaderResourceView() : nullptr,
	};

	commandList->SetRenderTarget(out.get());
	commandList->SetBlendState(blend_shadow_map.get());
	commandList->SetViewport(out->GetViewport());
	commandList->SetInputLayout(vps_shadow_mapping_directional->GetInputLayout());
	commandList->SetVertexShader(vps_shadow_mapping_directional->GetVertexShader());
	commandList->SetPixelShader(pixel_shader->GetPixelShader());
	commandList->SetShaderResources(0, ShaderStage::PS, textures);
	commandList->SetSamplerStates(0, ShaderStage::PS, samplers);
	commandList->SetConstantBuffers(0, ShaderStage::Global, constant_buffers);
	commandList->DrawIndexed
	(
		screen_index_buffer->GetIndexCount(),
		screen_index_buffer->GetOffset(),
		screen_vertex_buffer->GetOffset()
	);

	commandList->End();
	commandList->Submit();
}

void Renderer::PassBoxBlur(std::shared_ptr<class RenderTexture>& in, std::shared_ptr<class RenderTexture>& out, const float & sigma)
{
	commandList->Begin("PassBoxBlur");

	UpdateGlobalBuffer(out->GetWidth(), out->GetHeight());

	commandList->SetRenderTarget(out.get());
	commandList->SetViewport(out->GetViewport());
	commandList->SetPixelShader(ps_blur_box->GetPixelShader());
	commandList->SetShaderResource(0, ShaderStage::PS, in.get());
	commandList->SetSamplerState(0, ShaderStage::PS, trilinear_clamp.get());
	commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
	commandList->DrawIndexed
	(
		screen_index_buffer->GetIndexCount(),
		screen_index_buffer->GetOffset(),
		screen_vertex_buffer->GetOffset()
	);

	commandList->End();
	commandList->Submit();
}

void Renderer::PassGaussianBlur(std::shared_ptr<class RenderTexture>& in, std::shared_ptr<class RenderTexture>& out, const float & sigma, const float & pixel_stride)
{
	if (
		in->GetWidth() != out->GetWidth() ||
		in->GetHeight() != out->GetHeight() ||
		in->GetFormat() != out->GetFormat())
	{
		LOG_ERROR("Invalid parameter");
		return;
	}

	UpdateGlobalBuffer(in->GetWidth(), in->GetHeight());

	commandList->Begin("PassGaussianBlur");

	commandList->SetViewport(out->GetViewport());
	commandList->SetPixelShader(ps_blur_gaussian->GetPixelShader());
	commandList->SetSamplerState(0, ShaderStage::PS, bilinear_clamp.get());
	commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());

	commandList->Begin("PassGaussianBlur_Horizontal");
	{
		UpdateBlurBuffer(D3DXVECTOR2(pixel_stride, 0.0f), sigma);

		commandList->ClearShaderResources(ShaderStage::PS);
		commandList->SetRenderTarget(out.get());
		commandList->SetShaderResource(0, ShaderStage::PS, in.get());
		commandList->SetConstantBuffer(1, ShaderStage::PS, blur_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->Begin("PassGaussianBlur_Vertical");
	{
		UpdateBlurBuffer(D3DXVECTOR2(0.0f, pixel_stride), sigma);

		commandList->ClearShaderResources(ShaderStage::PS);
		commandList->SetRenderTarget(in.get());
		commandList->SetShaderResource(0, ShaderStage::PS, out.get());
		commandList->SetConstantBuffer(1, ShaderStage::PS, blur_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->End();
	commandList->Submit();

	in.swap(out);
}

void Renderer::PassBilateralGaussianBlur(std::shared_ptr<class RenderTexture>& in, std::shared_ptr<class RenderTexture>& out, const float & sigma, const float & pixel_stride)
{
	if (
		in->GetWidth() != out->GetWidth() ||
		in->GetHeight() != out->GetHeight() ||
		in->GetFormat() != out->GetFormat())
	{
		LOG_ERROR("Invalid parameter");
		return;
	}

	UpdateGlobalBuffer(in->GetWidth(), in->GetHeight());

	commandList->Begin("PassBilateralGaussianBlur");

	commandList->SetViewport(out->GetViewport());
	commandList->SetVertexShader(vs_quad->GetVertexShader());
	commandList->SetInputLayout(vs_quad->GetInputLayout());
	commandList->SetPixelShader(ps_blur_gaussian_bilateral->GetPixelShader());
	commandList->SetSamplerState(0, ShaderStage::PS, bilinear_clamp.get());
	commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());

	commandList->Begin("PassBilateralGaussianBlur_Horizontal");
	{
		UpdateBlurBuffer(D3DXVECTOR2(pixel_stride, 0.0f), sigma);

		std::vector<ID3D11ShaderResourceView*> textures
		{
			in->GetShaderResourceView(),
			gbuffer_depth->GetShaderResourceView(),
			gbuffer_normal->GetShaderResourceView()
		};

		commandList->ClearShaderResources(ShaderStage::PS);
		commandList->SetRenderTarget(out.get());
		commandList->SetShaderResources(0, ShaderStage::PS, textures);
		commandList->SetConstantBuffer(1, ShaderStage::PS, blur_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->Begin("PassBilateralGaussianBlur_Vertical");
	{
		UpdateBlurBuffer(D3DXVECTOR2(0.0f, pixel_stride), sigma);

		std::vector<ID3D11ShaderResourceView*> textures
		{
			out->GetShaderResourceView(),
			gbuffer_depth->GetShaderResourceView(),
			gbuffer_normal->GetShaderResourceView()
		};

		commandList->ClearShaderResources(ShaderStage::PS);
		commandList->SetRenderTarget(in.get());
		commandList->SetShaderResources(0, ShaderStage::PS, textures);
		commandList->SetConstantBuffer(1, ShaderStage::PS, blur_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->End();
	commandList->Submit();

	in.swap(out);
}

void Renderer::PassBloom(std::shared_ptr<class RenderTexture>& in, std::shared_ptr<class RenderTexture>& out)
{
	commandList->Begin("PassBloom");
	commandList->SetSamplerState(0, ShaderStage::PS, bilinear_clamp.get());

	commandList->Begin("Downsample");
	{
		UpdateGlobalBuffer(quarter_blur1->GetWidth(), quarter_blur1->GetHeight());

		commandList->SetRenderTarget(quarter_blur1.get());
		commandList->SetViewport(quarter_blur1->GetViewport());
		commandList->SetPixelShader(ps_downsample_box->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, in.get());
		commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->Begin("Bright");
	{
		UpdateGlobalBuffer(quarter_blur2->GetWidth(), quarter_blur2->GetHeight());

		commandList->SetRenderTarget(quarter_blur2.get());
		commandList->SetViewport(quarter_blur2->GetViewport());
		commandList->SetPixelShader(ps_bloom_bright->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, quarter_blur1.get());
		commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	//Gaussian Blur
	PassGaussianBlur(quarter_blur2, quarter_blur1, 2.0f);

	commandList->Begin("Upsample");
	{
		UpdateGlobalBuffer(half_spare2->GetWidth(), half_spare2->GetHeight());

		commandList->SetRenderTarget(half_spare2.get());
		commandList->SetViewport(half_spare2->GetViewport());
		commandList->SetPixelShader(ps_upsample_box->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, quarter_blur2.get());
		commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->Begin("Upsample");
	{
		UpdateGlobalBuffer(full_spare->GetWidth(), full_spare->GetHeight());

		commandList->SetRenderTarget(full_spare.get());
		commandList->SetViewport(full_spare->GetViewport());
		commandList->SetPixelShader(ps_upsample_box->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, half_spare2.get());
		commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->Begin("Blend");
	{
		UpdateGlobalBuffer(out->GetWidth(), out->GetHeight());

		std::vector<ID3D11ShaderResourceView*> textures
		{
			in->GetShaderResourceView(),
			full_spare->GetShaderResourceView(),
		};

		commandList->SetRenderTarget(out.get());
		commandList->SetViewport(out->GetViewport());
		commandList->SetPixelShader(ps_bloom_blend->GetPixelShader());
		commandList->SetShaderResources(0, ShaderStage::PS, textures);
		commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
		commandList->DrawIndexed
		(
			screen_index_buffer->GetIndexCount(),
			screen_index_buffer->GetOffset(),
			screen_vertex_buffer->GetOffset()
		);
	}
	commandList->End();

	commandList->End();
	commandList->Submit();
}

void Renderer::PassGammaCorrection(std::shared_ptr<class RenderTexture>& in, std::shared_ptr<class RenderTexture>& out)
{
	commandList->Begin("PassGammaCorrection");

	UpdateGlobalBuffer(out->GetWidth(), out->GetHeight());

	commandList->ClearShaderResources(ShaderStage::PS);
	commandList->SetRenderTarget(out.get());
	commandList->SetViewport(out->GetViewport());
	commandList->SetPixelShader(ps_gamma_correction->GetPixelShader());
	commandList->SetShaderResource(0, ShaderStage::PS, in.get());
	commandList->SetSamplerState(0, ShaderStage::PS, point_clamp.get());
	commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
	commandList->DrawIndexed(screen_index_buffer->GetIndexCount(), 0, 0);

	commandList->End();
	commandList->Submit();
}

void Renderer::PassSSAO(std::shared_ptr<class RenderTexture>& out)
{
	commandList->Begin("PassSSAO");

	std::vector<ID3D11ShaderResourceView*> textures
	{
		gbuffer_normal->GetShaderResourceView(),
		gbuffer_depth->GetShaderResourceView(),
		noise_normal_texture->GetShaderResourceView()
	};

	std::vector<ID3D11SamplerState*> samplers
	{
		bilinear_clamp->GetState(),
		bilinear_wrap->GetState()
	};

	UpdateGlobalBuffer(out->GetWidth(), out->GetHeight());

	commandList->ClearShaderResources(ShaderStage::PS);
	commandList->SetBlendState(blend_disabled.get());
	commandList->SetRenderTarget(out.get());
	commandList->SetViewport(out->GetViewport());
	commandList->SetInputLayout(vs_quad->GetInputLayout());
	commandList->SetVertexShader(vs_quad->GetVertexShader());
	commandList->SetPixelShader(ps_ssao->GetPixelShader());
	commandList->SetShaderResources(0, ShaderStage::PS, textures);
	commandList->SetSamplerStates(0, ShaderStage::PS, samplers);
	commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
	commandList->DrawIndexed
	(
		screen_index_buffer->GetIndexCount(),
		screen_index_buffer->GetOffset(),
		screen_vertex_buffer->GetOffset()
	);

	commandList->End();
	commandList->Submit();
}

void Renderer::PassDebug(std::shared_ptr<class RenderTexture>& out)
{
	if (debug_type == RendererDebugType::None)
		return;

	commandList->Begin("PassDebug");

	UpdateGlobalBuffer(out->GetWidth(), out->GetHeight());

	commandList->SetRasterizerState(cull_back_solid.get());
	commandList->SetBlendState(blend_disabled.get());
	commandList->SetDepthStencilState(depth_stencil_disabled.get());
	commandList->SetRenderTarget(out.get());
	commandList->SetViewport(out->GetViewport());

	switch (debug_type)
	{
	case RendererDebugType::Albedo:
		commandList->SetPixelShader(ps_texture->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, gbuffer_albedo->GetShaderResourceView());
		break;
	case RendererDebugType::Normal:
		commandList->SetPixelShader(ps_debug_normal->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, gbuffer_normal->GetShaderResourceView());
		break;
	case RendererDebugType::Material:
		commandList->SetPixelShader(ps_texture->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, gbuffer_material->GetShaderResourceView());
		break;
	case RendererDebugType::Velocity:
		commandList->SetPixelShader(ps_debug_velocity->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, gbuffer_velocity->GetShaderResourceView());
		break;
	case RendererDebugType::Depth:
		commandList->SetPixelShader(ps_debug_depth->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, gbuffer_depth->GetShaderResourceView());
		break;
	case RendererDebugType::SSAO:
		commandList->SetPixelShader(ps_debug_ssao->GetPixelShader());
		commandList->SetShaderResource(0, ShaderStage::PS, IsOnFlag(PostProcess_SSAO) ? half_ssao->GetShaderResourceView() : white_texture->GetShaderResourceView());
		break;
	}

	commandList->SetVertexBuffer(screen_vertex_buffer.get());
	commandList->SetIndexBuffer(screen_index_buffer.get());
	commandList->SetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->SetInputLayout(vs_quad->GetInputLayout());
	commandList->SetVertexShader(vs_quad->GetVertexShader());
	commandList->SetConstantBuffer(0, ShaderStage::Global, global_buffer.get());
	commandList->SetSamplerState(0, ShaderStage::PS, bilinear_clamp.get());
	commandList->DrawIndexed
	(
		screen_index_buffer->GetIndexCount(),
		screen_index_buffer->GetOffset(),
		screen_vertex_buffer->GetOffset()
	);
	commandList->End();
	commandList->Submit();
}
