#include "framework.h"
#include "Terrain.h"

#include "./Scene/Actor.h"
#include "./Scene/Component/Renderable.h"

Terrain::Terrain(Context * context, Actor * actor, Transform * transform)
	: IComponent(context, actor, transform)
	, gpu_terrain_buffer(nullptr)
	, gpu_picking_buffer(nullptr)
	, gpu_brush_buffer(nullptr)
	, brush_type(BrushType::BRUSH_NONE)
	, brush_texture(nullptr)
	, brush_range(10.0f), brush_intensity(0.5f)
	, width(0), height(0)
{
	graphics = context->GetSubsystem<Graphics>();
	renderer = context->GetSubsystem<Renderer>();
	
	// ADD Renderable
	auto renderable = actor->AddComponent<Renderable>();
	renderable->CreateEmptyMesh();
	renderable->GetMesh()->SetResourceName("TerrainMesh");
	vertex_buffer = renderable->GetMesh()->GetVertexBuffer();
	index_buffer = renderable->GetMesh()->GetIndexBuffer();

	material = std::make_shared<Material>(context);
	material->SetCullMode(CullMode::BACK);
	material->SetColorAlbedo(D3DXCOLOR(1.0f, 1.0f, 1.0f, 1.0f));
	material->SetShadingMode(ShadingMode::PBR);
	material->SetTiling(D3DXVECTOR2(20.0f, 20.0f));
	material->SetResourceName("Terrain_Mat");

	auto shader = new Shader(context);
	shader->AddDefine("ALBEDO_TEXTURE");
	shader->AddDefine("NORMAL_TEXTURE");
	shader->AddShader(ShaderStage::PS, "../_Assets/Shader/Terrain.hlsl");

	
	material->SetShader(shader);

	auto resourceManager = context->GetSubsystem<ResourceManager>();
	auto white_texture = resourceManager->GetResourceFromName<Texture>("HMCM");
	for (uint i = 0; i < 5; i++)
	{
		splat_channel_diffuse[i] = nullptr;
		splat_channel_normal[i] = nullptr;
	}
	splat_channel_diffuse[static_cast<uint>(SplatChannel::Default)] = white_texture;

	renderable->SetMaterial(material.get());

	//ADD Compute Picking
	picking_input_buffer = new ComputeBuffer(context);
	picking_input_buffer->SetBufferType(COMPUTE_BUFFER_TYPE::INPUT);
	picking_output_buffer = new ComputeBuffer(context);
	picking_output_buffer->SetBufferType(COMPUTE_BUFFER_TYPE::OUTPUT);

	compute_picking = new ComputeShader(context);
	compute_picking->Create("../_Assets/Shader/Compute_Picking.hlsl");

	//ADD Compute Brush
	brush_input_buffer = new ComputeBuffer(context);
	brush_input_buffer->SetBufferType(COMPUTE_BUFFER_TYPE::INPUT);
	brush_output_height_buffer = new ComputeBuffer(context);
	brush_output_height_buffer->SetBufferType(COMPUTE_BUFFER_TYPE::OUTPUT);
	brush_output_texture_buffer = new ComputeBuffer(context);
	brush_output_texture_buffer->SetBufferType(COMPUTE_BUFFER_TYPE::OUTPUT);

	compute_brush = new ComputeShader(context);
	compute_brush->Create("../_Assets/Shader/Compute_Brush.hlsl");

}

//=========================================================
// override
//=========================================================
#pragma region Overrides
void Terrain::Serialize(FileStream * stream)
{
}

void Terrain::Deserialize(FileStream * stream)
{
}

void Terrain::OnInitialize()
{
}

void Terrain::OnStart()
{
}

void Terrain::OnUpdate()
{

	auto input = context->GetSubsystem<Input>();

	if (bPicked = Picking(picked_pos))
	{
		if (input->BtnPress(KeyCode::CLICK_LEFT))
		{
			Painting(picked_pos);
		}
		LOG_INFO_F("PICKED : %0.2f %0.2f %0.2f", picked_pos.x, picked_pos.y, picked_pos.z);
	}
	else
		LOG_INFO("PICKED : X");

}

void Terrain::OnStop()
{
}

void Terrain::OnDestroy()
{
}
auto Terrain::GetSplatChannelShaderResourceView(const SplatChannel& channel, const ChannelType & type) -> ID3D11ShaderResourceView *
{ 
	switch (type)
	{
	case ChannelType::Diffuse:
	{
		auto texture = splat_channel_diffuse[static_cast<uint>(channel)];
		return texture ? texture->GetShaderResourceView() : nullptr;
	}
	break;
	case ChannelType::Normal: 
	{
		auto texture = splat_channel_normal[static_cast<uint>(channel)];
		return texture ? texture->GetShaderResourceView() : nullptr;
	}
	break;
	}
	
}

auto Terrain::GetChannelTexture(const SplatChannel& channel, const ChannelType& type) -> class Texture*
{
	switch (type)
	{
	case ChannelType::Diffuse: return splat_channel_diffuse[static_cast<uint>(channel)]; break;
	case ChannelType::Normal: return splat_channel_normal[static_cast<uint>(channel)]; break;
	}
}

void Terrain::SetChannelTexture(const SplatChannel& channel, const ChannelType & type, Texture * texture)
{
	switch (type)
	{
	case ChannelType::Diffuse: splat_channel_diffuse[static_cast<uint>(channel)] = texture; break;
	case ChannelType::Normal: splat_channel_normal[static_cast<uint>(channel)] = texture; break;
	}
}

void Terrain::UpdateTerrainBuffer()
{
	if (!gpu_terrain_buffer)
	{
		gpu_terrain_buffer = std::make_shared<ConstantBuffer>(context);
		gpu_terrain_buffer->Create<TerrainData>();
	}

	auto data = gpu_terrain_buffer->Map<TerrainData>();
	if (!data)
	{
		LOG_ERROR("ConstantBuffer data invalid (terrainData)");
		gpu_terrain_buffer->Unmap();
		return;
	}

	data->height = height;
	data->width = width;
	data->tess_factor = 0;
	data->b_picked = bPicked ? 1 : -1;
	data->brush_position = picked_pos;
	data->brush_range = brush_range;
	
	
	gpu_terrain_buffer->Unmap();
	
}

auto Terrain::Picking(D3DXVECTOR3 & position) -> bool
{
	//Set Buffers
	picking_input_buffer->UpdateBuffer(geometry.GetVertexData(), geometry.GetVertexCount());
	picking_output_buffer->UpdateBuffer(std::vector<PickOutputData>(1).data(), 1);

	if (!UpdatePickBuffer())
		return false;

	compute_picking->BindPipeline();
	picking_input_buffer->BindPipeline(0);
	picking_output_buffer->BindPipeline(0);
	gpu_picking_buffer->BindPipeline(ShaderStage::CS, 0);

	graphics->GetDeviceContext()->Dispatch((width - 1)*(height - 1) * 2, 1, 1);

	PickOutputData result_data;
	memcpy_s(&result_data, sizeof(PickOutputData), picking_output_buffer->GetResult(), sizeof(PickOutputData));

	bool bPicked = result_data.bPicked == 1 ? true : false;
	
	if (bPicked)
		position = result_data.position;
	
	return bPicked;
}

auto Terrain::UpdatePickBuffer() -> bool
{
	// Picker 버그 
	// ConstantBuffer Padding 문제

	D3DXVECTOR2 screen_pos;
	if (!renderer->GetCursorPos(screen_pos))
		return false;

	if (!gpu_picking_buffer)
	{
		gpu_picking_buffer = new ConstantBuffer(context);
		gpu_picking_buffer->Create<TerrainPickData>();
	}

	auto data = gpu_picking_buffer->Map<TerrainPickData>();
	if (!data)
	{
		LOG_ERROR("ConstantBuffer Data invalid (terrain_picking_data)");
		gpu_picking_buffer->Unmap();
	}

	auto camera = renderer->GetMainCamera();
	
	
	camera->GetLocalRay(data->Ray_origin, data->Ray_direction,transform->GetWorldMatrix());
	data->Terrain_height = height;
	data->Terrain_width = width;
	
	auto terrainPos = transform->GetTranslation();
	LOG_INFO_F("Terrain Pos : %0.2f %0.2f %0.2f", terrainPos.x, terrainPos.y, terrainPos.z);
	LOG_INFO_F("origin %0.2f %0.2f %0.2f, direction %0.2f %0.2f %0.2f",
		data->Ray_origin.x, data->Ray_origin.y, data->Ray_origin.z,
		data->Ray_direction.x, data->Ray_direction.y, data->Ray_direction.z);
	
	gpu_picking_buffer->Unmap();

	return true;
}


void Terrain::Painting(const D3DXVECTOR3& picked_position)
{ // Picking된 상태에서만 실행된다.
	if (!brush_texture || brush_type == BrushType::BRUSH_NONE)
		return;

	brush_input_buffer->UpdateBuffer(geometry.GetVertexData(), geometry.GetVertexCount());

	UpdateBrushBuffer(picked_position);

	compute_brush->BindPipeline();
	brush_input_buffer->BindPipeline(0);
	brush_output_height_buffer->BindPipeline(0);
	gpu_brush_buffer->BindPipeline(ShaderStage::CS, 0);

	auto brush_srv = brush_texture->GetShaderResourceView();
	graphics->GetDeviceContext()->CSSetShaderResources(1, 1, &brush_srv);

	if (brush_type == BrushType::BRUSH_PAINTING)
	{
		auto mipData = splat_map->GetMipLevelData(0);
		auto textureData = reinterpret_cast<const D3DXCOLOR*>(mipData->data());
		brush_output_texture_buffer->UpdateBuffer(textureData, width*height);
	}
	if (brush_type == BrushType::BRUSH_FLAT || brush_type == BrushType::BRUSH_HEIGHT)
	{
		brush_output_height_buffer->UpdateBuffer(geometry.GetVertexData(), geometry.GetVertexCount());
	}

	brush_input_buffer->BindPipeline(0);
	brush_output_height_buffer->BindPipeline(0);
	brush_output_texture_buffer->BindPipeline(1);
	auto splat_srv = splat_map->GetShaderResourceView();
	graphics->GetDeviceContext()->CSSetShaderResources(2, 1, &splat_srv);

	graphics->GetDeviceContext()->Dispatch(width*height, 1, 1);

	if (brush_type == BrushType::BRUSH_PAINTING)
	{
		auto result = static_cast<D3DXCOLOR*>(brush_output_texture_buffer->GetResult());
		auto mipData = splat_map->GetMipLevelData(0);
		memcpy(mipData->data(), result, sizeof(D3DXCOLOR)*width*height);
		SAFE_DELETE_ARRAY(result);

		splat_map->UpdateData(width, height, sizeof(D3DXCOLOR));
	}
	if (brush_type == BrushType::BRUSH_FLAT || brush_type == BrushType::BRUSH_HEIGHT)
	{
		auto result = static_cast<VertexModel*>(brush_output_height_buffer->GetResult());
		
		memcpy(geometry.GetVertexData(), result, sizeof(VertexModel)*geometry.GetVertexCount());
		UpdateNormal();

		vertex_buffer->UpdateBuffer(geometry.GetVertexData(), sizeof(VertexModel)*geometry.GetVertexCount());

		SAFE_DELETE_ARRAY(result);
	}

}

void Terrain::UpdateBrushBuffer(const D3DXVECTOR3& picked_position)
{
	if (!gpu_brush_buffer)
	{
		gpu_brush_buffer = new ConstantBuffer(context);
		gpu_brush_buffer->Create<TerrainBrushData>();
	}

	auto data = gpu_brush_buffer->Map<TerrainBrushData>();
	if (!data)
	{
		LOG_ERROR("ConstantBuffer Data invalid (terrain_brush_data)");
		gpu_brush_buffer->Unmap();
	}

	data->terrain_width = static_cast<float>(width);
	data->terrain_height = static_cast<float>(height);
	data->brush_position = picked_position;
	data->brush_type = static_cast<float>(static_cast<uint>(brush_type));
	data->channel_index = static_cast<float>(static_cast<uint>(brush_channel_type));
	data->brush_range = brush_range;
	data->brush_intensity = brush_intensity;
	
	gpu_brush_buffer->Unmap();
}

#pragma endregion



//=========================================================
// Create Terrain
//=========================================================
#pragma region Create_Terrain

void Terrain::CreateFromHeightMap(const std::string& paths)
{
	Clear();
	std::vector<D3DXCOLOR> pixels;
	uint width, height;

	ReadPixels(paths, pixels, width, height);
	SetupGeometry(pixels, width, height);	

	splat_map = std::make_shared<Texture>(context);
	splat_map->Create(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);

}

void Terrain::Create(const uint & width, const uint & height)
{
	Clear();
	std::vector<D3DXCOLOR> pixels(width*height);
	ZeroMemory(&pixels[0], sizeof(D3DXCOLOR)*width*height);
	 
	SetupGeometry(pixels, width, height);
	splat_map = std::make_shared<Texture>(context);
	splat_map->Create(width, height, DXGI_FORMAT_R32G32B32A32_FLOAT);
}

void Terrain::Clear()
{
	geometry.Clear();
	index_buffer->Clear();
	vertex_buffer->Clear();
}

void Terrain::ReadPixels(IN const std::string & filePath, OUT std::vector<D3DXCOLOR>& pixels, OUT uint & width, OUT uint & height)
{
	ID3D11Texture2D* srcTexture = nullptr;
	
	auto hr = D3DX11CreateTextureFromFileA
	(
		graphics->GetDevice(),
		filePath.c_str(),
		nullptr,
		nullptr,
		reinterpret_cast<ID3D11Resource**>(&srcTexture),
		nullptr
	);
	assert(SUCCEEDED(hr));
	
	D3D11_TEXTURE2D_DESC srcDesc;
	srcTexture->GetDesc(&srcDesc);
	
	width = srcDesc.Width;
	height = srcDesc.Height;
	
	D3D11_TEXTURE2D_DESC dstDesc;
	ZeroMemory(&dstDesc, sizeof(D3D11_TEXTURE2D_DESC));
	dstDesc.Width = srcDesc.Width;
	dstDesc.Height = srcDesc.Height;
	dstDesc.MipLevels = 1;
	dstDesc.ArraySize = 1;
	dstDesc.Format = srcDesc.Format;
	dstDesc.SampleDesc = srcDesc.SampleDesc;
	dstDesc.Usage = D3D11_USAGE_STAGING;
	dstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
	
	ID3D11Texture2D* dstTexture = nullptr;
	
	hr = graphics->GetDevice()->CreateTexture2D
	(
		&dstDesc,
		nullptr,
		&dstTexture
	);
	
	assert(SUCCEEDED(hr));
	
	hr = D3DX11LoadTextureFromTexture
	(
		graphics->GetDeviceContext(),
		srcTexture,
		nullptr,
		dstTexture
	);
	assert(SUCCEEDED(hr));
	
	
	std::vector<UINT> colors(srcDesc.Width * srcDesc.Height * 30, UINT());
	
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	graphics->GetDeviceContext()->Map
	(
		dstTexture,
		0,
		D3D11_MAP_READ,
		0,
		&mappedResource
	);
	
	unsigned char* pTexels = static_cast<unsigned char*>(mappedResource.pData);
	
	
	for (uint z = 0; z < srcDesc.Height; z++)
	{
		uint rowStart = (srcDesc.Height - (z + 1)) * mappedResource.RowPitch;
		for (uint x = 0; x < srcDesc.Width; x++)
		{
			uint colStart = x * 4;
	
			const float factor = 1.0f / 255.0f;
			float r = factor * static_cast<float>(pTexels[rowStart + colStart + 0]);
			float g = factor * static_cast<float>(pTexels[rowStart + colStart + 1]);
			float b = factor * static_cast<float>(pTexels[rowStart + colStart + 2]);
			float a = factor * static_cast<float>(pTexels[rowStart + colStart + 3]);
	
			pixels.emplace_back(r, g, b, a);
		}
	}
	
	graphics->GetDeviceContext()->Unmap(dstTexture, 0);
	
	colors.clear();
	colors.shrink_to_fit();
	
	SAFE_RELEASE(dstTexture);
	SAFE_RELEASE(srcTexture);
}
	
void Terrain::UpdateNormal()
{
	auto vertices = geometry.GetVertexData();
	auto indices = geometry.GetIndexData();

	for (uint i = 0; i < geometry.GetIndexCount()/3; i++)
	{
		uint index[3] = { indices[i * 3],indices[i * 3+1],indices[i * 3+2] };
		VertexModel vertex[3] = { vertices[index[0]],vertices[index[1]],vertices[index[2]] };

		D3DXVECTOR3 n;
		D3DXVec3Cross(&n, &(vertices[index[2]].Position - vertices[index[1]].Position), &(vertices[index[0]].Position - vertices[index[1]].Position));

		vertices[index[0]].Normal += n;
		vertices[index[1]].Normal += n;
		vertices[index[2]].Normal += n;
	}
	
	
	
	//auto vertices = geometry.GetVertexData();
	//auto indices = geometry.GetIndexData();
	//
	//inputVertices->UpdateBuffer(vertices, geometry.GetVertexCount());
	//inputIndices->UpdateBuffer(indices, geometry.GetIndexCount());
	//outputVertices->UpdateBuffer<VertexPTNTBC>(nullptr, geometry.GetVertexCount());
	//
	//inputVertices->BindPipeline(0);
	//inputIndices->BindPipeline(1);
	//outputVertices->BindPipeline(0);
	//trInfoBuffer->BindPipeline(ShaderType::CS, 0);
	//
	//calculateNormalShader->BindPipeline();
	//
	//graphics->GetDeviceContext()->Dispatch(geometry.GetIndexCount() / 3, 1, 1);
	//
	//VertexPTNTBC* result = static_cast<VertexPTNTBC*>(outputVertices->GetResult());
	///*std::vector<VertexPTNTBC> vTest(size);
	//memcpy(vTest.data(), result, sizeof(VertexPTNTBC)*size);*/
	//memcpy(vertices, result, sizeof(VertexPTNTBC)*geometry.GetVertexCount());
	//SAFE_DELETE_ARRAY(result);
}

void Terrain::SetupGeometry(IN std::vector<D3DXCOLOR>& heightMap, IN uint width, IN uint height)
{
	this->width = width;
	this->height = height;

	//Create Vertices
#pragma region Create 6Vertex
	//for (uint z = 0; z < height - 1; z++)
	//{

	//	for (uint x = 0; x < width - 1; x++)
	//	{
	//		//Bottom left
	//		positionX = static_cast<float>(x);
	//		positionZ = static_cast<float>(z);
	//		u = positionX / static_cast<float>(width);
	//		v = positionZ / static_cast<float>(height);
	//		geometry.AddVertex
	//		(
	//			VertexPTNTBC
	//			(
	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
	//				{ u,v },
	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
	//				{ 0,0,0,0 }
	//			)
	//		);
	//		geometry.AddIndex(index++);

	//		//upper left
	//		positionX = static_cast<float>(x);
	//		positionZ = static_cast<float>(z + 1);
	//		u = positionX / static_cast<float>(width);
	//		v = positionZ / static_cast<float>(height);
	//		geometry.AddVertex
	//		(
	//			VertexPTNTBC
	//			(
	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
	//				{ u,v },
	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
	//				{ 0,0,0,0 }
	//			)
	//		);
	//		geometry.AddIndex(index++);

	//		//upper right
	//		positionX = static_cast<float>(x + 1);
	//		positionZ = static_cast<float>(z + 1);
	//		u = positionX / static_cast<float>(width);
	//		v = positionZ / static_cast<float>(height);
	//		geometry.AddVertex
	//		(
	//			VertexPTNTBC
	//			(
	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
	//				{ u,v },
	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
	//				{ 0,0,0,0 }
	//			)
	//		);
	//		geometry.AddIndex(index++);

	//		// bottom left
	//		positionX = static_cast<float>(x);
	//		positionZ = static_cast<float>(z);
	//		u = positionX / static_cast<float>(width);
	//		v = positionZ / static_cast<float>(height);
	//		geometry.AddVertex
	//		(
	//			VertexPTNTBC
	//			(
	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
	//				{ u,v },
	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
	//				{ 0,0,0,0 }
	//			)
	//		);
	//		geometry.AddIndex(index++);

	//		// upper right
	//		positionX = static_cast<float>(x + 1);
	//		positionZ = static_cast<float>(z + 1);
	//		u = positionX / static_cast<float>(width);
	//		v = positionZ / static_cast<float>(height);
	//		geometry.AddVertex
	//		(
	//			VertexPTNTBC
	//			(
	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ *TERRAIN_INTERVEL },
	//				{ u,v },
	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
	//				{ 0,0,0,0 }
	//			)
	//		);
	//		geometry.AddIndex(index++);

	//		// bottom right
	//		positionX = static_cast<float>(x + 1);
	//		positionZ = static_cast<float>(z);
	//		u = positionX / static_cast<float>(width);
	//		v = positionZ / static_cast<float>(height);
	//		geometry.AddVertex
	//		(
	//			VertexPTNTBC
	//			(
	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
	//				{ u,v },
	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
	//				{ 0,0,0,0 }
	//			)
	//		);
	//		geometry.AddIndex(index++);

	//	}


	//}
#pragma endregion 

	for (uint z = 0; z < height; z++)
	{
		float fZ = static_cast<float>(z);

		for (uint x = 0; x < width; x++)
		{
			uint index = width * z + x;
			float fX = static_cast<float>(x);

			if (z < height - 1 && x < width - 1)
			{
				geometry.AddIndex(width*(z)+(x));
				geometry.AddIndex(width*(z + 1) + (x));
				geometry.AddIndex(width*(z)+(x + 1));
				geometry.AddIndex(width*(z)+(x + 1));
				geometry.AddIndex(width*(z + 1) + (x));
				geometry.AddIndex(width*(z + 1) + (x + 1));
			}

			geometry.AddVertex(
				VertexModel
				(
					{ fX, heightMap[index].g * 255 / 7.5f,fZ },
					{ fX/static_cast<float>(width),fZ/static_cast<float>(height) },
					{ 0,0,0 }, { 0,0,0 }, { 1.0f, 0, 0, 1.0f }, { 1.0f,1.0f,1.0f,1.0f }
				)
			);
		}
	}

	CalculateTangent();
	UpdateNormal();

	vertex_buffer->Create(geometry.GetVertices());
	index_buffer->Create(geometry.GetIndices());
}


void Terrain::CalculateTangent()
{
	auto vertices = geometry.GetVertexData();
	auto indices = geometry.GetIndexData();

	D3DXVECTOR3* tan1 = new D3DXVECTOR3[geometry.GetVertexCount()*2];
	D3DXVECTOR3* tan2 = tan1 + geometry.GetVertexCount();
	ZeroMemory(tan1, geometry.GetVertexCount() * sizeof(D3DXVECTOR3) * 2);


	// vertex, normal, texcoord
	{
		for (uint i = 0; i < geometry.GetIndexCount() / 3; i++)
		{
			uint index[3] = { indices[i * 3], indices[i * 3 + 1],indices[i * 3 + 2] };

			VertexModel vertex[3] =
			{
				vertices[index[0]], vertices[index[1]], vertices[index[2]]
			};

			float x1 = vertex[1].Position.x - vertex[0].Position.x;
			float x2 = vertex[2].Position.x - vertex[0].Position.x;
			float y1 = vertex[1].Position.y - vertex[0].Position.y;
			float y2 = vertex[2].Position.y - vertex[0].Position.y;
			float z1 = vertex[1].Position.z - vertex[0].Position.z;
			float z2 = vertex[2].Position.z - vertex[0].Position.z;

			float s1 = vertex[1].Uv.x - vertex[0].Uv.x;
			float s2 = vertex[2].Uv.x - vertex[0].Uv.x;
			float t1 = vertex[1].Uv.y - vertex[0].Uv.y;
			float t2 = vertex[2].Uv.y - vertex[0].Uv.y;

			float r = 1.0F / (s1 * t2 - s2 * t1);
			D3DXVECTOR3 sdir((t2 * x1 - t1 * x2) * r, (t2 * y1 - t1 * y2) * r, (t2 * z1 - t1 * z2) * r);
			D3DXVECTOR3 tdir((s1 * x2 - s2 * x1) * r, (s1 * y2 - s2 * y1) * r, (s1 * z2 - s2 * z1) * r);
			
			tan1[index[0]] += sdir;
			tan1[index[1]] += sdir;
			tan1[index[2]] += sdir;

			tan2[index[0]] += tdir;
			tan2[index[1]] += tdir;
			tan2[index[2]] += tdir;
			
		}
	}

	for (uint i = 0; i < geometry.GetVertexCount(); i++)
	{
		const D3DXVECTOR3& n = vertices[i].Normal;
		const D3DXVECTOR3& t = tan1[i];

		//Gram-Schmidt orthogonalize
		D3DXVec3Normalize(&vertices[i].Tangent, &(t - n * D3DXVec3Dot(&n, &t)));
	}

	SAFE_DELETE_ARRAY(tan1);
}
#pragma endregion







#pragma region OldVersion
//
//Terrain::Terrain(Context * context)
//	: context(context)
//	, vertexBuffer(nullptr)
//	, indexBuffer(nullptr)
//	, texSrv(nullptr)
//	, texNor(nullptr)
//	, texSpe(nullptr)
//	, picker(nullptr)
//	, parentCell(nullptr)
//	, frustumGuide(nullptr)
//{
//	graphics = context->GetSubsystem<Graphics>();
//
//	pipeline = new Pipeline(context);
//
//	trInfoBuffer = new ConstantBuffer(context);
//	trInfoBuffer->Create<TerrainData>();
//	wireFrame = false;
//	D3DXMatrixIdentity(&(world));
//
//	picker = new TerrainPicker(context);
//	brush = new TerrainBrush(context);
//
//	inputVertices = new ComputeBuffer(context, COMPUTE_BUFFER_TYPE::INPUT);
//	inputIndices = new ComputeBuffer(context, COMPUTE_BUFFER_TYPE::INPUT);
//	outputVertices = new ComputeBuffer(context, COMPUTE_BUFFER_TYPE::OUTPUT);
//
//	calculateNormalShader = new ComputeShader(context);
//	calculateNormalShader->Create("../_Assets/Shader/calculateNormal.hlsl");
//}
//
//Terrain::~Terrain()
//{
//	SAFE_DELETE(pipeline);
//	SAFE_DELETE(picker);
//	SAFE_DELETE(brush);
//}
//
//void Terrain::ShaderReload()
//{
//	pipeline->SetShader(ShaderType::VS, "../_Assets/Shader/terrainVS.hlsl");
//	pipeline->SetShader(ShaderType::PS, "../_Assets/Shader/terrainPS.hlsl");
//	pipeline->SetShader(ShaderType::HS, "../_Assets/Shader/terrainHS.hlsl");
//	pipeline->SetShader(ShaderType::DS, "../_Assets/Shader/terrainDS.hlsl");
//
//	shadowPipeline->SetShader(ShaderType::VS, "../_Assets/Shader/depthVS.hlsl");
//	shadowPipeline->SetShader(ShaderType::PS, "../_Assets/Shader/depthPS.hlsl");
//
//	refractionPipeline->SetShader(ShaderType::VS, "../_Assets/Shader/terrainRefractionVS.hlsl");
//	refractionPipeline->SetShader(ShaderType::PS, "../_Assets/Shader/terrainRefractionPS.hlsl");
//
//}
//
//
////  
//
//void Terrain::Update()
//{
//#ifdef NEWPIPELINE_TEST
//
//
//#else
//	auto worldData = static_cast<WorldData*>(worldBuffer->Map());
//	D3DXMatrixTranspose(&worldData->World, &world);
//	worldBuffer->Unmap();
//#endif
//
//
//	D3DXVECTOR3 pickedPos;
//	if (picker->Picking(world, pickedPos, D3DXVECTOR2(width, height))) // QuadTree Node Picking
//	{
//		picked = true;
//		brush->SetBrushPosition(pickedPos);
//		brush->SetTerrainSize(static_cast<uint>(width), static_cast<uint>(height));
//
//		auto input = context->GetSubsystem<Input>();
//		if (input->BtnDown(0)
//			|| (input->BtnPress(0)
//				&& (input->GetMouseMoveValue().x != 0 || input->GetMouseMoveValue().y != 0)))
//		{
//			auto vertices = geometry.GetVertexData();
//
//			brush->Update(vertices, geometry.GetVertexCount());
//			//TODO : node의 z값 변경
//
//			UpdateNormal();
//
//			graphics->GetDeviceContext()->UpdateSubresource
//			(
//				vertexBuffer->GetBuffer(),
//				0,
//				nullptr,
//				vertices,
//				sizeof(VertexPTNTBC)*geometry.GetVertexCount(),
//				0
//			);
//		}
//
//		picker->UpdateInput(geometry.GetVertices());
//	}
//	else picked = false;
//
//	auto input = context->GetSubsystem<Input>();
//	if (input->KeyDown(VK_F1))
//	{
//		brush->SaveSplatMap();
//	}
//
//}
//
//void Terrain::SetRSState(const D3D11_CULL_MODE & cullmode, const D3D11_FILL_MODE & fillmode)
//{
//	pipeline->SetRSCullMode(cullmode);
//	pipeline->SetRSFillMode(fillmode);
//}
//
//void Terrain::refractionRender(const CameraData & cameraData, const ClipPlaneData & clipPlaneData)
//{
//	refractionPipeline->SetVertexBuffer(vertexBuffer);
//	refractionPipeline->SetindexBuffer(indexBuffer);
//
//	// Set VS Constant
//	refractionPipeline->SetConstantBufferData(ShaderType::VS, cameraData);
//	refractionPipeline->SetConstantBufferData(ShaderType::VS, clipPlaneData);
//	refractionPipeline->SetConstantBufferData(ShaderType::VS, TerrainData(width, height));
//
//	WorldData wData;
//	D3DXMatrixTranspose(&wData.World, &world);
//	refractionPipeline->SetConstantBufferData(ShaderType::VS, wData);
//
//	//Set PS Constant
//	LightColorData lightColorData;
//	lightColorData.ambient = sunLight->GetAmbient();
//	lightColorData.diffuse = sunLight->GetDiffuse();
//	lightColorData.Direction = sunLight->GetDirection();
//	refractionPipeline->SetConstantBufferData(ShaderType::PS, lightColorData);
//
//	refractionPipeline->SetShaderResourceView(ShaderType::PS, 0, brush->GetSplatMap());
//	refractionPipeline->SetShaderResourceView(ShaderType::PS, 1, texSrv);
//	int startNum = 2;
//	for (int channel = 0; channel < 4; channel++)
//	{
//		refractionPipeline->SetShaderResourceView(ShaderType::PS, startNum + channel, brush->GetSplatChannel(channel, 0));
//	}
//
//
//	parentCell->Render(cameraData, refractionPipeline, frustumGuide);
//
//	if (picked) {
//		picker->DebugRender();
//		picked = false;
//	}
//}
//
//void Terrain::DepthRender(const CameraData& cameraData)
//{
//	shadowPipeline->SetVertexBuffer(vertexBuffer);
//
//	shadowPipeline->SetConstantBufferData(ShaderType::VS, cameraData);
//	WorldData wData;
//	D3DXMatrixTranspose(&wData.World, &world);
//	shadowPipeline->SetConstantBufferData(ShaderType::VS, wData);
//
//	parentCell->DepthRender(cameraData, shadowPipeline);
//}
//
//void Terrain::Render(const CameraData& cameraData)
//{
//	//Pipeline Setting
//	auto input = context->GetSubsystem<Input>();
//	if (input->KeyDown(VK_F2))
//	{
//		if (!wireFrame)
//		{
//			pipeline->SetRSFillMode(D3D11_FILL_WIREFRAME);
//			wireFrame = true;
//		}
//		else
//		{
//			wireFrame = false;
//			pipeline->SetRSFillMode(D3D11_FILL_SOLID);
//		}
//	}
//
//	if (input->KeyDown(VK_F4))
//	{
//		if (!wireFrame)
//		{
//			frustumGuide = true;
//		}
//		else
//		{
//			frustumGuide = false;
//		}
//	}
//
//	pipeline->SetVertexBuffer(vertexBuffer);
//	pipeline->SetindexBuffer(indexBuffer);
//
//	pipeline->SetConstantBufferData(ShaderType::DS, cameraData);
//
//	WorldData wData;
//	D3DXMatrixTranspose(&wData.World, &world);
//	pipeline->SetConstantBufferData(ShaderType::DS, wData);
//	LightData lightData;
//	sunLight->GetLightData(lightData);
//	pipeline->SetConstantBufferData(ShaderType::DS, lightData);
//
//	if (picked)
//	{
//		pipeline->SetConstantBufferData(ShaderType::DS, brush->GetBrushData());
//		pipeline->SetConstantBufferData(ShaderType::DS, TerrainData(width, height));
//		pipeline->SetShaderResourceView(ShaderType::DS, 0, brush->GetBrushSRV());
//	}
//
//	LightColorData lightColorData;
//	lightColorData.ambient = sunLight->GetAmbient();
//	lightColorData.diffuse = sunLight->GetDiffuse();
//	lightColorData.Direction = sunLight->GetDirection();
//	pipeline->SetConstantBufferData(ShaderType::PS, lightColorData);
//
//	pipeline->SetShaderResourceView(ShaderType::PS, 0, texSrv);
//	pipeline->SetShaderResourceView(ShaderType::PS, 1, texNor);
//	pipeline->SetShaderResourceView(ShaderType::PS, 2, texSpe);
//	pipeline->SetShaderResourceView(ShaderType::PS, 3, brush->GetSplatMap());
//	int startNum = 4;
//	for (int channel = 0; channel < 4; channel++)
//	{
//		for (int i = 0; i < 3; i++)
//			pipeline->SetShaderResourceView(ShaderType::PS, startNum + 3 * channel + i, brush->GetSplatChannel(channel, i));
//	}
//	pipeline->SetShaderResourceView(ShaderType::PS, 16, shadowTexture->GetSrv());
//
//
//	parentCell->Render(cameraData, pipeline, frustumGuide);
//
//	if (picked) {
//		picker->DebugRender();
//		picked = false;
//	}
//}
//
//
//
//
//#pragma region TerrainSetting
//
//
//void Terrain::CreateTerrain(const std::string & filePath)
//{
//	std::vector<D3DXCOLOR> pixels;
//	uint width, height;
//
//	SetUpPipeLine();
//	ReadPixels(filePath, pixels, width, height);
//
//	SetupGeometry(pixels, width, height);
//
//	SetTools();
//}
//
//void Terrain::CreateTerrain(const uint & width, const uint & height)
//{
	//std::vector<D3DXCOLOR> pixels(width*height);
	//ZeroMemory(&pixels[0], sizeof(D3DXCOLOR)*width*height);

	//SetUpPipeLine();

	//SetupGeometry(pixels, width, height);

//	SetTools();
//}
//
//void Terrain::SetUpPipeLine()
//{
//	vertexBuffer = new VertexBuffer(context);
//	indexBuffer = new IndexBuffer(context);
//
//	auto hr = D3DX11CreateShaderResourceViewFromFileA
//	(
//		graphics->GetDevice(),
//		"../_Assets/grey.png",
//		nullptr,
//		nullptr,
//		&texSrv,
//		nullptr
//	);
//
//
//
//	pipeline->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_3_CONTROL_POINT_PATCHLIST);
//	pipeline->SetShader(ShaderType::VS, "../_Assets/Shader/terrainVS.hlsl");
//
//	pipeline->SetShader(ShaderType::PS, "../_Assets/Shader/terrainPS.hlsl");
//	pipeline->SetSRVSlot(ShaderType::PS, 17);
//	pipeline->SetConstantBuffer<LightColorData>(ShaderType::PS);
//
//	pipeline->SetShader(ShaderType::HS, "../_Assets/Shader/terrainHS.hlsl");
//	pipeline->SetConstantBuffer<TessData>(ShaderType::HS);
//
//	pipeline->SetShader(ShaderType::DS, "../_Assets/Shader/terrainDS.hlsl");
//	pipeline->SetConstantBuffer<CameraData >(ShaderType::DS);
//	pipeline->SetConstantBuffer<WorldData >(ShaderType::DS);
//	pipeline->SetConstantBuffer<BrushData >(ShaderType::DS);
//	pipeline->SetConstantBuffer<TerrainData>(ShaderType::DS);
//	pipeline->SetConstantBuffer<LightData>(ShaderType::DS);
//	pipeline->SetSRVSlot(ShaderType::DS, 1);
//
//	shadowPipeline = new Pipeline(context);
//	shadowPipeline->SetShader(ShaderType::VS, "../_Assets/Shader/depthVS.hlsl");
//	shadowPipeline->SetConstantBuffer<CameraData>(ShaderType::VS);
//	shadowPipeline->SetConstantBuffer<WorldData>(ShaderType::VS);
//	shadowPipeline->SetShader(ShaderType::PS, "../_Assets/Shader/depthPS.hlsl");
//	shadowPipeline->SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	refractionPipeline = new Pipeline(context);
//	refractionPipeline->SetShader(ShaderType::VS, "../_Assets/Shader/terrainRefractionVS.hlsl");
//	refractionPipeline->SetConstantBuffer<CameraData>(ShaderType::VS);
//	refractionPipeline->SetConstantBuffer<WorldData>(ShaderType::VS);
//	refractionPipeline->SetConstantBuffer<TerrainData>(ShaderType::VS);
//	refractionPipeline->SetConstantBuffer<ClipPlaneData>(ShaderType::VS);
//
//	refractionPipeline->SetShader(ShaderType::PS, "../_Assets/Shader/terrainRefractionPS.hlsl");
//	refractionPipeline->SetSRVSlot(ShaderType::PS, 6);
//	refractionPipeline->SetConstantBuffer<LightColorData>(ShaderType::PS);
//	refractionPipeline->SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//
//
//
//}
//
//void Terrain::ReadPixels(IN const std::string& filePath, OUT std::vector<D3DXCOLOR>& pixels, OUT uint& width, OUT uint& height)
//{
//	ID3D11Texture2D* srcTexture = nullptr;
//
//	auto hr = D3DX11CreateTextureFromFileA
//	(
//		graphics->GetDevice(),
//		filePath.c_str(),
//		nullptr,
//		nullptr,
//		reinterpret_cast<ID3D11Resource**>(&srcTexture),
//		nullptr
//	);
//	assert(SUCCEEDED(hr));
//
//	D3D11_TEXTURE2D_DESC srcDesc;
//	srcTexture->GetDesc(&srcDesc);
//
//	width = srcDesc.Width;
//	height = srcDesc.Height;
//
//	D3D11_TEXTURE2D_DESC dstDesc;
//	ZeroMemory(&dstDesc, sizeof(D3D11_TEXTURE2D_DESC));
//	dstDesc.Width = srcDesc.Width;
//	dstDesc.Height = srcDesc.Height;
//	dstDesc.MipLevels = 1;
//	dstDesc.ArraySize = 1;
//	dstDesc.Format = srcDesc.Format;
//	dstDesc.SampleDesc = srcDesc.SampleDesc;
//	dstDesc.Usage = D3D11_USAGE_STAGING;
//	dstDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
//
//	ID3D11Texture2D* dstTexture = nullptr;
//
//	hr = graphics->GetDevice()->CreateTexture2D
//	(
//		&dstDesc,
//		nullptr,
//		&dstTexture
//	);
//
//	assert(SUCCEEDED(hr));
//
//	hr = D3DX11LoadTextureFromTexture
//	(
//		graphics->GetDeviceContext(),
//		srcTexture,
//		nullptr,
//		dstTexture
//	);
//	assert(SUCCEEDED(hr));
//
//
//	std::vector<UINT> colors(srcDesc.Width * srcDesc.Height * 30, UINT());
//
//	D3D11_MAPPED_SUBRESOURCE mappedResource;
//	graphics->GetDeviceContext()->Map
//	(
//		dstTexture,
//		0,
//		D3D11_MAP_READ,
//		0,
//		&mappedResource
//	);
//
//	unsigned char* pTexels = static_cast<unsigned char*>(mappedResource.pData);
//
//
//	for (uint z = 0; z < srcDesc.Height; z++)
//	{
//		uint rowStart = (srcDesc.Height - (z + 1)) * mappedResource.RowPitch;
//		for (uint x = 0; x < srcDesc.Width; x++)
//		{
//			uint colStart = x * 4;
//
//			const float factor = 1.0f / 255.0f;
//			float r = factor * static_cast<float>(pTexels[rowStart + colStart + 0]);
//			float g = factor * static_cast<float>(pTexels[rowStart + colStart + 1]);
//			float b = factor * static_cast<float>(pTexels[rowStart + colStart + 2]);
//			float a = factor * static_cast<float>(pTexels[rowStart + colStart + 3]);
//
//			pixels.emplace_back(r, g, b, a);
//		}
//	}
//
//	graphics->GetDeviceContext()->Unmap(dstTexture, 0);
//
//	colors.clear();
//	colors.shrink_to_fit();
//
//	SAFE_RELEASE(dstTexture);
//	SAFE_RELEASE(srcTexture);
//}
//
//void Terrain::UpdateNormal()
//{
//	auto vertices = geometry.GetVertexData();
//	auto indices = geometry.GetIndexData();
//
//	inputVertices->UpdateBuffer(vertices, geometry.GetVertexCount());
//	inputIndices->UpdateBuffer(indices, geometry.GetIndexCount());
//	outputVertices->UpdateBuffer<VertexPTNTBC>(nullptr, geometry.GetVertexCount());
//
//	inputVertices->BindPipeline(0);
//	inputIndices->BindPipeline(1);
//	outputVertices->BindPipeline(0);
//	trInfoBuffer->BindPipeline(ShaderType::CS, 0);
//
//	calculateNormalShader->BindPipeline();
//
//	graphics->GetDeviceContext()->Dispatch(geometry.GetIndexCount() / 3, 1, 1);
//
//	VertexPTNTBC* result = static_cast<VertexPTNTBC*>(outputVertices->GetResult());
//	/*std::vector<VertexPTNTBC> vTest(size);
//	memcpy(vTest.data(), result, sizeof(VertexPTNTBC)*size);*/
//	memcpy(vertices, result, sizeof(VertexPTNTBC)*geometry.GetVertexCount());
//	SAFE_DELETE_ARRAY(result);
//}
//
//#define TERRAIN_INTERVEL 1.F
//void Terrain::SetupGeometry(IN std::vector<D3DXCOLOR>& heightMap, IN uint width, IN uint height)
//{
//	this->width = static_cast<float>(width);
//	this->height = static_cast<float>(height);
//
//	// Texture Uv 설정
//
//	float incrementValue = static_cast<float>(TEXTURE_REPEAT) / static_cast<float>(width - 1);
//
//	int incrementCount = static_cast<int>(width - 1) / static_cast<int>(TEXTURE_REPEAT);
//
//	float uIncrement = incrementValue;
//	float vIncrement = incrementValue;
//
//	int tuCount = 0;
//	int tvCount = 0;
//
//	float tu = 0.0f;
//	float tv = 0.0f;
//
//	//float positionX, positionZ, u, v;
//	int index = 0;
//
//
//	//Create Vertices
//#pragma region Create 6Vertex
//	//for (uint z = 0; z < height - 1; z++)
//	//{
//
//	//	for (uint x = 0; x < width - 1; x++)
//	//	{
//	//		//Bottom left
//	//		positionX = static_cast<float>(x);
//	//		positionZ = static_cast<float>(z);
//	//		u = positionX / static_cast<float>(width);
//	//		v = positionZ / static_cast<float>(height);
//	//		geometry.AddVertex
//	//		(
//	//			VertexPTNTBC
//	//			(
//	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
//	//				{ u,v },
//	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
//	//				{ 0,0,0,0 }
//	//			)
//	//		);
//	//		geometry.AddIndex(index++);
//
//	//		//upper left
//	//		positionX = static_cast<float>(x);
//	//		positionZ = static_cast<float>(z + 1);
//	//		u = positionX / static_cast<float>(width);
//	//		v = positionZ / static_cast<float>(height);
//	//		geometry.AddVertex
//	//		(
//	//			VertexPTNTBC
//	//			(
//	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
//	//				{ u,v },
//	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
//	//				{ 0,0,0,0 }
//	//			)
//	//		);
//	//		geometry.AddIndex(index++);
//
//	//		//upper right
//	//		positionX = static_cast<float>(x + 1);
//	//		positionZ = static_cast<float>(z + 1);
//	//		u = positionX / static_cast<float>(width);
//	//		v = positionZ / static_cast<float>(height);
//	//		geometry.AddVertex
//	//		(
//	//			VertexPTNTBC
//	//			(
//	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
//	//				{ u,v },
//	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
//	//				{ 0,0,0,0 }
//	//			)
//	//		);
//	//		geometry.AddIndex(index++);
//
//	//		// bottom left
//	//		positionX = static_cast<float>(x);
//	//		positionZ = static_cast<float>(z);
//	//		u = positionX / static_cast<float>(width);
//	//		v = positionZ / static_cast<float>(height);
//	//		geometry.AddVertex
//	//		(
//	//			VertexPTNTBC
//	//			(
//	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
//	//				{ u,v },
//	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
//	//				{ 0,0,0,0 }
//	//			)
//	//		);
//	//		geometry.AddIndex(index++);
//
//	//		// upper right
//	//		positionX = static_cast<float>(x + 1);
//	//		positionZ = static_cast<float>(z + 1);
//	//		u = positionX / static_cast<float>(width);
//	//		v = positionZ / static_cast<float>(height);
//	//		geometry.AddVertex
//	//		(
//	//			VertexPTNTBC
//	//			(
//	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ *TERRAIN_INTERVEL },
//	//				{ u,v },
//	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
//	//				{ 0,0,0,0 }
//	//			)
//	//		);
//	//		geometry.AddIndex(index++);
//
//	//		// bottom right
//	//		positionX = static_cast<float>(x + 1);
//	//		positionZ = static_cast<float>(z);
//	//		u = positionX / static_cast<float>(width);
//	//		v = positionZ / static_cast<float>(height);
//	//		geometry.AddVertex
//	//		(
//	//			VertexPTNTBC
//	//			(
//	//				{ positionX*TERRAIN_INTERVEL, heightMap[static_cast<uint>(positionZ*width + positionX)].g * 255 / 7.5f,positionZ*TERRAIN_INTERVEL },
//	//				{ u,v },
//	//				{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
//	//				{ 0,0,0,0 }
//	//			)
//	//		);
//	//		geometry.AddIndex(index++);
//
//	//	}
//
//
//	//}
//#pragma endregion 
//#pragma region Create Vertex
//	for (uint z = 0; z < height; z++)
//	{
//
//
//		for (uint x = 0; x < width; x++)
//		{
//			uint index = width * z + x;
//
//			if (z < height - 1 && x < width - 1)
//			{
//				geometry.AddIndex(width*(z)+(x));
//				geometry.AddIndex(width*(z + 1) + (x));
//				geometry.AddIndex(width*(z)+(x + 1));
//				geometry.AddIndex(width*(z)+(x + 1));
//				geometry.AddIndex(width*(z + 1) + (x));
//				geometry.AddIndex(width*(z + 1) + (x + 1));
//			}
//
//			geometry.AddVertex(
//				VertexPTNTBC
//				(
//					{ static_cast<float>(x)*TERRAIN_INTERVEL, heightMap[index].g * 255 / 7.5f,static_cast<float>(z)*TERRAIN_INTERVEL },
//					{ tu,tv },
//					{ 0,0,0 }, { 0,0,0 }, { 0,0,0 },
//					{ 0,0,0,0 }
//				)
//			);
//
//			if (tuCount == incrementCount)
//			{
//				uIncrement *= -1.0f;
//				tuCount = 1;
//			}
//			else tuCount++;
//
//			tu += uIncrement;
//		}
//
//		if (tvCount == incrementCount)
//		{
//			vIncrement *= -1.0f;
//			tvCount = 1;
//		}
//		else tvCount++;
//
//		tu = 0.0f;
//		tuCount = 0;
//		uIncrement = incrementValue;
//		tv += vIncrement;
//
//		if (tv >= 1.0f || tv <= 0.0f)
//		{
//			tv <= 0.f ? tv = 0 : tv = 1;
//			tvCount = 0;
//			vIncrement = incrementValue;
//		}
//	}
//
//
//#pragma endregion
//
//
//
//	CalculateTangentBinormal();
//	UpdateNormal();
//	auto trData = static_cast<TerrainData*>(trInfoBuffer->Map());
//	trData->Size = { static_cast<float>(width), static_cast<float>(height) };
//	trInfoBuffer->Unmap();
//
//	if (parentCell)
//		SAFE_DELETE(parentCell);
//
//
//#pragma region Create Terrain Cell
//	parentCell = new TerrainCell(context);
//	parentCell->CreateTerrainCell(
//		D3DXVECTOR3((width* TERRAIN_INTERVEL) / 2.0f, 0, (height* TERRAIN_INTERVEL) / 2.0f),
//		width* TERRAIN_INTERVEL,
//		D3DXVECTOR2(width* TERRAIN_INTERVEL, height* TERRAIN_INTERVEL),
//		geometry
//	);
//#pragma endregion
//
//
//
//	vertexBuffer->Create(geometry.GetVertices());
//	indexBuffer->Create(geometry.GetIndices());
//
//}
//
//
//void Terrain::CalculateTangentBinormal()
//{
//	auto vertices = geometry.GetVertexData();
//	auto indices = geometry.GetIndexData();
//	for (uint i = 0; i < geometry.GetIndexCount() / 3; i++)
//	{
//		uint index[3] = { indices[i * 3],indices[i * 3 + 1],indices[i * 3 + 2] };
//
//		VertexPTNTBC vertex[3] =
//		{
//			vertices[index[0]], vertices[index[1]], vertices[index[2]]
//		};
//
//
//		float x1 = vertex[1].Position.x - vertex[0].Position.x;
//		float x2 = vertex[2].Position.x - vertex[0].Position.x;
//		float y1 = vertex[1].Position.y - vertex[0].Position.y;
//		float y2 = vertex[2].Position.y - vertex[0].Position.y;
//		float z1 = vertex[1].Position.z - vertex[0].Position.z;
//		float z2 = vertex[2].Position.z - vertex[0].Position.z;
//
//		float s1 = vertex[1].Uv.x - vertex[0].Uv.x;
//		float s2 = vertex[2].Uv.x - vertex[0].Uv.x;
//
//		float t1 = vertex[1].Uv.y - vertex[0].Uv.y;
//		float t2 = vertex[2].Uv.y - vertex[0].Uv.y;
//
//		float r = 1.0f / (s1*t2 - s2 * t1);
//		D3DXVECTOR3 sdir((t2*x1 - t1 * x2)*r, (t2*y1 - t1 * y2)*r, (t2*z1 - t1 * z2)*r);
//		D3DXVECTOR3 tdir((s1*x2 - s2 * x1)*r, (s1*y2 - s2 * y1)*r, (s1*z2 - s2 * z1)*r);
//
//		sdir = { abs(sdir.x),abs(sdir.y),abs(sdir.z) };
//		tdir = { abs(tdir.x),abs(tdir.y),abs(tdir.z) };
//
//		vertices[index[0]].Tangent += sdir;
//		vertices[index[1]].Tangent += sdir;
//		vertices[index[2]].Tangent += sdir;
//
//		vertices[index[0]].Binormal += tdir;
//		vertices[index[1]].Binormal += tdir;
//		vertices[index[2]].Binormal += tdir;
//
//	}
//}
//
//
//void Terrain::SetTools()
//{
//	picker->CreateInput(geometry.GetVertices());
//}
//#pragma endregion


#pragma endregion
