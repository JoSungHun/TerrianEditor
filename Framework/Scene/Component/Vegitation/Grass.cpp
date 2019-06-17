#include "Framework.h"
#include "Grass.h"
#include "Scene\Scene.h"
#include "Scene\Component\Camera.h"

#define INSTANCE_NUM 25
#define RAND_MAX 100

Grass::Grass(Context * context)
	: context(context)
	, m_windRotation(0)
	, m_windDirection(0)
{
	graphics = context->GetSubsystem<Graphics>();
	pipeline = new Pipeline(context);

	auto hr = D3DX11CreateShaderResourceViewFromFileA
	(
		graphics->GetDevice(),
		"../_Assets/Tree.png",
		nullptr,
		nullptr,
		&grassTex,
		nullptr
	);

	//정점 버퍼
	int index = 0;
	//왼쪽 아래
	geometry.AddVertex(VertexTexture({ 0.0f,0.0f,0.0f }, { 0.0f,1.0f }));
	geometry.AddIndex(index++);
	//왼쪽 위
	geometry.AddVertex(VertexTexture({ 0.0f,1.0f,0.0f }, { 0.0f,0.0f }));
	geometry.AddIndex(index++);
	//오른쪽 아래
	geometry.AddVertex(VertexTexture({ 1.0f,0.0f,0.0f }, { 1.0f,1.0f }));
	geometry.AddIndex(index++);
	//오른쪽 아래
	geometry.AddVertex(VertexTexture({ 1.0f,0.0f,0.0f }, { 1.0f,1.0f }));
	geometry.AddIndex(index++);
	//왼쪽 위
	geometry.AddVertex(VertexTexture({ 0.0f,1.0f,0.0f }, { 0.0f,0.0f }));
	geometry.AddIndex(index++);
	//오른쪽 위
	geometry.AddVertex(VertexTexture({ 1.0f,1.0f,0.0f }, { 1.0f,0.0f }));
	geometry.AddIndex(index);

	vertexBuffer = new VertexBuffer(context);
	vertexBuffer->Create<VertexTexture>(geometry.GetVertices());
	indexBuffer = new IndexBuffer(context);
	indexBuffer->Create(geometry.GetIndices());
	 
	//초기 인스턴스 버퍼 생성
	srand((int)time(nullptr));
	for (int i = 0; i < INSTANCE_NUM; ++i)
	{
		for (int j = 0; j < INSTANCE_NUM; ++j)
		{
			GrassTest gt;
			/*float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX)*9.0f - 4.5f;
			float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX)*9.0f - 4.5f;
			gt.pos = D3DXVECTOR3(x, 0, z);

			float red = static_cast<float>(rand()) / static_cast<float>(RAND_MAX)*1.0f;
			float green = static_cast<float>(rand()) / static_cast<float>(RAND_MAX)*1.0f;
			gt.color = D3DXCOLOR(red+1.0f, green+0.5f, 0, 1);*/

			gt.pos = D3DXVECTOR3(static_cast<float>(i),0.f, static_cast<float>(j) );
			gt.color = D3DXCOLOR(1, 1, 0, 1);
			instanceDatas.push_back(gt);
			instances.push_back(InstanceGrass());
		}
		
	}

	instanceBuffer = new InstanceBuffer(context);
	instanceBuffer->Create<InstanceGrass>(instances);

	pipeline->SetVertexBuffer(vertexBuffer);
	pipeline->SetindexBuffer(indexBuffer);
	pipeline->SetInstanceBuffer(instanceBuffer);

	pipeline->SetShader(ShaderType::VS, "../_Assets/Shader/GrassVS.hlsl");
	pipeline->SetConstantBuffer<CameraData>(ShaderType::VS);
	pipeline->SetConstantBuffer<LightData>(ShaderType::VS);

	pipeline->SetShader(ShaderType::PS, "../_Assets/Shader/GrassPS.hlsl");
	pipeline->SetConstantBuffer<LightColorData>(ShaderType::PS);
	pipeline->SetSRVSlot(ShaderType::PS, 2);
	pipeline->SetShaderResourceView(ShaderType::PS, 0, grassTex);
	

	pipeline->SetInputLayout(InstanceGrass::Desc, InstanceGrass::Count); 
	pipeline->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	depth_pipeline = new Pipeline(context);
	depth_pipeline->SetVertexBuffer(vertexBuffer);
	depth_pipeline->SetindexBuffer(indexBuffer);
	depth_pipeline->SetInstanceBuffer(instanceBuffer);
	depth_pipeline->SetShader(ShaderType::VS, "../_Assets/Shader/GrassDepthVS.hlsl");
	depth_pipeline->SetConstantBuffer<CameraData>(ShaderType::VS);
	depth_pipeline->SetShader(ShaderType::PS, "../_Assets/Shader/depthPS.hlsl");
	depth_pipeline->SetInputLayout(InstanceGrass::Desc, InstanceGrass::Count);
	depth_pipeline->SetTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

}

Grass::~Grass()
{
}

void Grass::Update()
{
	D3DXMATRIX matRot, matTrans, matRot2, matFinal;
	D3DXVECTOR3 pos = { 0,0,0 };
	auto camera = context->GetSubsystem<SceneManager>()->GetCurrentScene()->GetCamera();
	D3DXVECTOR3 cameraPos = camera->GetPosition();
	
	float angle = 0.0f;
	float rotation = 0.0f;
	float windRotation = 0.0f;

	if (m_windDirection == 1)
	{
		m_windRotation += 0.1f;
		if (m_windRotation > 10.0f)
			m_windDirection = 2;
	}
	else
	{
		m_windRotation -= 0.1f;
		if (m_windRotation < -10.0f)
			m_windDirection = 1;
	}


	////인스턴스 버퍼 업데이트
	for (int i = 0; i < INSTANCE_NUM*INSTANCE_NUM; ++i)
	{
		pos.x = instanceDatas[i].pos.x;
		pos.y = -0.1f;
		pos.z = instanceDatas[i].pos.z;

		float angle = atan2(pos.x - cameraPos.x, pos.z - cameraPos.z)*(180.0f / static_cast<float>(D3DX_PI));

		rotation = static_cast<float>(angle)*0.01745352925f;

		D3DXMatrixRotationY(&matRot, rotation);

		windRotation = m_windRotation * 0.0174532925f;

		D3DXMatrixRotationX(&matRot2,windRotation);

		D3DXMatrixTranslation(&matTrans, pos.x, pos.y, pos.z);

		matFinal = matRot * matRot2*matTrans;

		D3DXMatrixTranspose(&instances[i].InstanceWorld, &matFinal);
		instances[i].Color = instanceDatas[i].color;
	}
	 
	instanceBuffer->Update(instances.data(), instances.size());
}

void Grass::Render(CameraData cameraData)
{
	pipeline->SetConstantBufferData(ShaderType::VS, cameraData);
	
	LightData lightData;
	D3DXMatrixTranspose(&lightData.lightView, &sunLight->GetViewMatrix());
	D3DXMatrixTranspose(&lightData.lightProj, &sunLight->GetPerspectiveProjMatrix());
	lightData.lightPos = sunLight->GetPosition();
	pipeline->SetConstantBufferData(ShaderType::VS, lightData); 
	pipeline->SetShaderResourceView(ShaderType::PS, 1, renderTexture->GetSrv());

	LightColorData lightColorData;
	lightColorData.ambient = sunLight->GetAmbient();
	lightColorData.diffuse = sunLight->GetDiffuse();
	lightColorData.Direction = sunLight->GetDirection();
	pipeline->SetConstantBufferData(ShaderType::PS, lightColorData);

	pipeline->Bind();
	graphics->GetDeviceContext()->DrawInstanced(geometry.GetIndexCount(), instances.size(), 0, 0);
}

void Grass::DepthRender(CameraData cameraData)
{
	depth_pipeline->SetConstantBufferData(ShaderType::VS, cameraData);
	depth_pipeline->Bind();
	graphics->GetDeviceContext()->DrawInstanced(geometry.GetIndexCount(), instances.size(), 0, 0);
}
