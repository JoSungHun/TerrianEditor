#include "Framework.h"
#include "Ocean.h"

#include "./Scene/Actor.h"
#include "./Scene/Component/Renderable.h"

#pragma region OldVersion

//Ocean::Ocean(Context * context)
//	: context(context)
//	, pipeline(nullptr)
//	, vertexBuffer(nullptr)
//	, indexBuffer(nullptr)
//	, width(0), height(0), posY(0)
//	, waterTranslation(0)
//{
//	graphics = context->GetSubsystem<Graphics>();
//	D3DXMatrixIdentity(&world);
//}
//
//Ocean::~Ocean()
//{
//}
//
//void Ocean::ShaderReload()
//{
//
//	pipeline->SetShader(ShaderType::VS, "../_Assets/Shader/oceanVS.hlsl");
//	pipeline->SetShader(ShaderType::PS, "../_Assets/Shader/oceanPS.hlsl");
//
//}
//
//void Ocean::CreateOcean(uint width, uint height)
//{
//	this->width = width;
//	this->height = height;
//	
//	float w = static_cast<float>(width);
//	float h = static_cast<float>(height);
//	float vY = static_cast<float>(posY);
//
//	geometry.AddVertex
//	(
//		VertexTexture
//		(
//			{ 0,vY,0 },
//			{ 0,0 }
//		)
//	);
//	geometry.AddVertex
//	(
//		VertexTexture
//		(
//			{ w,vY,0 },
//			{ 1,0 }
//		)
//	);
//	geometry.AddVertex
//	(
//		VertexTexture
//		(
//			{ 0,vY,h },
//			{ 0,1 }
//		)
//	);
//	geometry.AddVertex
//	(
//		VertexTexture
//		(
//			{ w,vY,h },
//			{ 1,1 }
//		)
//	);
//
//	geometry.AddIndex(0);
//	geometry.AddIndex(2);
//	geometry.AddIndex(1);
//
//	geometry.AddIndex(1);
//	geometry.AddIndex(2);
//	geometry.AddIndex(3);
//
//	vertexBuffer = new VertexBuffer(context);
//	indexBuffer = new IndexBuffer(context);
//	vertexBuffer->Create<VertexTexture>(geometry.GetVertices());
//	indexBuffer->Create(geometry.GetIndices());
//
//	pipeline = new Pipeline(context);
//	pipeline->SetVertexBuffer(vertexBuffer);
//	pipeline->SetindexBuffer(indexBuffer);
//	pipeline->SetTopology(D3D11_PRIMITIVE_TOPOLOGY::D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	pipeline->SetShader(ShaderType::VS, "../_Assets/Shader/oceanVS.hlsl");
//	pipeline->SetConstantBuffer<CameraData>(ShaderType::VS);
//	pipeline->SetConstantBuffer<WorldData>(ShaderType::VS);
//	pipeline->SetConstantBuffer<ReflectionData>(ShaderType::VS);
//	pipeline->SetShader(ShaderType::PS, "../_Assets/Shader/oceanPS.hlsl");
//	pipeline->SetConstantBuffer<WaterData>(ShaderType::PS);
//	pipeline->SetSRVSlot(ShaderType::PS, 3);
//
//	D3DX11CreateShaderResourceViewFromFileA
//	(
//		graphics->GetDevice(),
//		"../_Assets/water01.dds",
//		nullptr,
//		nullptr,
//		&normalMap,
//		nullptr
//	);
//}
//
//void Ocean::Update()
//{
//	waterTranslation += 0.001f;
//	if (waterTranslation > 1.0f)
//	{
//		waterTranslation -= 1.0f;
//	}
//
//
//}
//
//void Ocean::Render(const CameraData & cameraData, const D3DXMATRIX& reflectionMat)
//{
//	pipeline->SetConstantBufferData(ShaderType::VS, cameraData);
//	WorldData wData;
//	D3DXMatrixTranspose(&wData.World, &world);
//	pipeline->SetConstantBufferData(ShaderType::VS, wData);
//
//	ReflectionData reflectionData;
//	D3DXMatrixTranspose(&reflectionData.reflectionMatrix, &reflectionMat);
//	pipeline->SetConstantBufferData(ShaderType::VS, reflectionData);
//
//	WaterData waterData;
//	waterData.reflectRefractScale = 0.01f;
//	waterData.waterTranslation = waterTranslation;
//	pipeline->SetConstantBufferData(ShaderType::PS, waterData);
//
//	pipeline->SetShaderResourceView(ShaderType::PS, 0, reflectionTexture->GetSrv()); 
//	pipeline->SetShaderResourceView(ShaderType::PS, 1, refractionTexture->GetSrv());
//	pipeline->SetShaderResourceView(ShaderType::PS, 2, normalMap);
//
//	pipeline->Bind();
//	graphics->GetDeviceContext()->DrawIndexed(geometry.GetIndexCount(), 0, 0);
//	pipeline->UnBind();
//
//}

#pragma endregion

Ocean::Ocean(Context* context, class Actor* actor, class Transform* transform)
	: IComponent(context, actor, transform)
	, ocean_translation(0.0f)
	, ocean_height(3.5f)
{
	graphics = context->GetSubsystem<Graphics>();
	renderer = context->GetSubsystem<Renderer>();

	// Add Renderable
	auto renderable = actor->AddComponent<Renderable>();
	renderable->CreateEmptyMesh();
	renderable->GetMesh()->SetResourceName("OceanMesh");
	renderable->SetRenderableType(RenderableType::Transparent);
	vertex_buffer = renderable->GetMesh()->GetVertexBuffer();
	index_buffer = renderable->GetMesh()->GetIndexBuffer();

	material = std::make_shared<Material>(context);
	material->SetCullMode(CullMode::BACK);
	material->SetColorAlbedo(D3DXCOLOR(0.0f, 0.0f, 0.0f, 173.f/255.f));
	material->SetShadingMode(ShadingMode::PBR);
	material->SetStandardShader();
	material->SetResourceName("Ocean_Mat");
	material->SetMetallicMultiplier(0.3f);
	material->SetRoughnessMultiplier(0.1f);

	// Shader : using default Shader 

	auto resource_manager = context->GetSubsystem<ResourceManager>();
	auto normal_texture = resource_manager->GetResourceFromName<Texture>("../_Assets/water01.dds");
	auto white_texture = resource_manager->GetResourceFromName<Texture>("../_Assets/white.bmp");
	material->SetTexture(TextureType::Albedo, white_texture);
	material->SetTexture(TextureType::Normal, normal_texture);

	renderable->SetMaterial(material.get());

	CreateOcean(100, 100);
}

void Ocean::Serialize(FileStream * stream)
{
}

void Ocean::Deserialize(FileStream * stream)
{
}

void Ocean::OnInitialize()
{
}

void Ocean::OnStart()
{
}

void Ocean::OnUpdate()
{
}

void Ocean::OnStop()
{
}

void Ocean::OnDestroy()
{
}

void Ocean::CreateOcean(uint width, uint height)
{
	this->width = width;
	this->height = height;

	geometry.Clear();
	float centerX = width / 2;
	float centerZ = height / 2;

	float radiusX = width * 2;
	float radiusZ = height * 2;

	float y = static_cast<float>(ocean_height);

	geometry.AddVertex
	(
		VertexModel
		(
			{ centerX - radiusX, y, centerZ - radiusZ },
			{ 0,0 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);
	geometry.AddVertex
	(
		VertexModel
		(
			//{ x / 2,vY,-z / 2 },
			{ centerX + radiusX, y, centerZ - radiusZ },
			{ 1,0 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);
	geometry.AddVertex
	(
		VertexModel
		(
			//{ -x / 2,vY,z / 2 },
			{ centerX - radiusX, y, centerZ + radiusZ },
			{ 0,1 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);
	geometry.AddVertex
	(
		VertexModel
		(
			//{ x / 2,vY,z / 2 },
			{ centerX + radiusX, y, centerZ + radiusZ },
			{ 1,1 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);

	geometry.AddIndex(0);
	geometry.AddIndex(2);
	geometry.AddIndex(1);

	geometry.AddIndex(1);
	geometry.AddIndex(2);
	geometry.AddIndex(3);

	vertex_buffer->Create<VertexModel>(geometry.GetVertices());
	index_buffer->Create(geometry.GetIndices());



}

void Ocean::EditOceanSize(uint width, uint height)
{
	this->width = width;
	this->height = height;

	geometry.Clear();
	float centerX = width / 2;
	float centerZ = height / 2;

	float radiusX = width * 2;
	float radiusZ = height * 2;

	float y = static_cast<float>(ocean_height);

	geometry.AddVertex
	(
		VertexModel
		(
			{ centerX- radiusX, y, centerZ - radiusZ },
			{ 0,0 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);
	geometry.AddVertex
	(
		VertexModel
		(
			//{ x / 2,vY,-z / 2 },
			{ centerX + radiusX, y, centerZ - radiusZ },
			{ 1,0 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);
	geometry.AddVertex
	(
		VertexModel
		(
			//{ -x / 2,vY,z / 2 },
			{ centerX - radiusX, y, centerZ + radiusZ },
			{ 0,1 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);
	geometry.AddVertex
	(
		VertexModel
		(
			//{ x / 2,vY,z / 2 },
			{ centerX + radiusX, y, centerZ + radiusZ },
			{ 1,1 },
			{ 0,0,0 },
			{ 0,0,0 },
			{ 0,0,0,0 },
			{ 0,0,0,0 }
		)
	);

	geometry.AddIndex(0);
	geometry.AddIndex(2);
	geometry.AddIndex(1);

	geometry.AddIndex(1);
	geometry.AddIndex(2);
	geometry.AddIndex(3);

	vertex_buffer->UpdateBuffer(geometry.GetVertexData(), geometry.GetVertexCount());

}

void Ocean::UpdateOceanBuffer()
{
	if (!gpu_ocean_buffer)
	{
		gpu_ocean_buffer = std::make_shared<ConstantBuffer>(context);
		gpu_ocean_buffer->Create<OceanData>();
	}

	auto data = gpu_ocean_buffer->Map<OceanData>();
	if (!data)
	{
		LOG_ERROR("ConstantBuffer data invalid (oceanData)");
		return;
	}

	data->reflectRefractScale = 0.1f;
	data->waterTranslation = ocean_translation;

	gpu_ocean_buffer->Unmap();
}

void Ocean::SetOceanHeight(const float & height)
{
	ocean_height = height;

	auto vertices = geometry.GetVertexData();
	for (int i = 0; i < geometry.GetVertexCount(); ++i)
	{
		vertices[i].Position.y = ocean_height;
	}

	vertex_buffer->UpdateBuffer(vertices, geometry.GetVertexCount());
}
