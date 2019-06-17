#include "Framework.h"
#include "TerrainCell.h"
#include "./Scene/Component/Terrain/TerrainPicker.h"
#include "./Scene/Component/Terrain/TerrainBrush.h"
#include "./Scene/Scene.h"

//TODO:	Create ȿ�������� ó��
//		�� ��忡�� Indices�� ���� (IndexBuffer)�� ��������
//		Picking�� triangleCount = 0 -> Pass,
//				  Node ������ü ��ŷ ���� �׽�Ʈ -> ����ϸ� Pick�˻�, �ƴϸ� return
//		Brush->Height�� ��ȭ�� �ش� ����� ���̰� ����


TerrainCell::TerrainCell(Context * context)
	: context(context)
	, tessBuffer(nullptr)
{
	graphics = context->GetSubsystem<Graphics>();
	frustum = context->GetSubsystem<Frustum>();
	numTriangles = 0;

	for (uint i = 0; i < 4; i++)
		childCell[i] = nullptr;

	indexBuffer = new IndexBuffer(context);


}

TerrainCell::~TerrainCell()
{

}


void TerrainCell::RefractionRender(const CameraData& cameraData, Pipeline* pipeline)
{
	if (!frustum->CheckCube(centerPos, width / 2.0f))
		return;

	if (numTriangles == 0)
	{
		for (uint i = 0; i < 4; i++)
		{
			if (childCell[i])
				childCell[i]->DepthRender(cameraData, pipeline);
		}
		return;
	}

	pipeline->SetindexBuffer(indexBuffer);

	pipeline->Bind();
	graphics->GetDeviceContext()->DrawIndexed(indices.size(), 0, 0);
	pipeline->UnBind();
}


void TerrainCell::DepthRender(const CameraData& cameraData, Pipeline* pipeline)
{
	if (!frustum->CheckCube(centerPos, width / 2.0f))
		return;

	if (numTriangles == 0)
	{
		for (uint i = 0; i < 4; i++)
		{
			if (childCell[i])
				childCell[i]->DepthRender(cameraData, pipeline);
		}
		return;
	}

	pipeline->SetindexBuffer(indexBuffer);

	pipeline->Bind();
	graphics->GetDeviceContext()->DrawIndexed(indices.size(), 0, 0);
	pipeline->UnBind();
}

void TerrainCell::Render(const CameraData& cameraData, Pipeline* pipeline, bool frustumGuide)
{

	if (!frustum->CheckCube(centerPos,width/2.0f))
		return;

	if (numTriangles == 0)
	{
		for (uint i = 0; i < 4; i++)
		{
			if (childCell[i])
				childCell[i]->Render(cameraData, pipeline,frustumGuide);
		}
		return;
	}

	if(frustumGuide)
		frustum->DrawCube(centerPos, width / 2, cameraData);

	
	pipeline->SetindexBuffer(indexBuffer);

	//TEST
	D3DXVECTOR3 cameraPos = context->GetSubsystem<SceneManager>()->GetCurrentScene()->GetCamera()->GetPosition();

	float d = D3DXVec3Length(&D3DXVECTOR3(centerPos - cameraPos));
	const float d0 = 10.0f;
	const float d1 = 130.0f;
	float tess = (d1 - d) / (d1 - d0);
	if (tess < 0) tess = 0.05f;
	else if (tess > 1) tess = 1.0f;
	tess *= 20.0f;

	TessData tessdata;
	tessdata.CameraPos = cameraPos;
	tessdata.tessFactor = 1;
	pipeline->SetConstantBufferData(ShaderType::HS, tessdata);
	

	pipeline->Bind();
	graphics->GetDeviceContext()->DrawIndexed(indices.size(), 0, 0);
	pipeline->UnBind();

}


void TerrainCell::CreateTerrainCell(const D3DXVECTOR3 & centerPosition, const float& size, const D3DXVECTOR2& trSize, Geometry<VertexPTNTBC>& geometry)
{
	this->centerPos = centerPosition;
	width = size;
	uint numTriangles = CountTriangles(centerPosition.x, centerPosition.z, size, geometry);

	if (numTriangles == 0)
		return;

	if (numTriangles > MAX_TRIANGLES)
	{
		numTriangles = 0;
		for (uint i = 0; i < 4; i++)
		{ /// i = 0 ���ʾƷ�, 1 �����ʾƷ�, 2 ������, 3 ������ ��
			float offsetX = (((i % 2) < 1) ? -1.0f : 1.0f)* (size / 4.0f);
			float offsetZ = (((i % 4) < 2) ? -1.0f : 1.0f)*(size / 4.0f);

			uint count = CountTriangles(centerPosition.x + offsetX, centerPosition.z + offsetZ, size/2,geometry);
			if (count > 0)
			{
				childCell[i] = new TerrainCell(context);
				//��������� ��带 �����ϵ��� �Ѵ�.
				childCell[i]->CreateTerrainCell({ centerPosition.x + offsetX,0, centerPosition.z + offsetZ }, size / 2.0f,trSize,geometry);
			}
		}
		return;
	}

	this->numTriangles = numTriangles;

	int vertexCount = numTriangles * 3;

	if (tessBuffer)
		SAFE_DELETE(tessBuffer);
	tessBuffer = new ConstantBuffer(context);
	tessBuffer->Create<TessData>();

	auto* vertices = geometry.GetVertexData();
	auto* indices = geometry.GetIndexData();
	for (uint i = 0; i < geometry.GetIndexCount()/3; i++)
	{
		if (IsTriangleContained(geometry, i, centerPosition.x, centerPosition.z, size))
		{ 
			this->indices.push_back(indices[i * 3]); 
			this->indices.push_back(indices[i * 3 + 1]); 
			this->indices.push_back(indices[i * 3 + 2]);
		}
	}


	if(!this->indices.empty())
		indexBuffer->Create(this->indices);
}


int TerrainCell::CountTriangles(float centerX, float centerZ, const float& size, Geometry<VertexPTNTBC>& geometry)
{
	uint count = 0;

	for (uint i = 0; i < geometry.GetIndexCount()/3; ++i)
	{
		uint index = i;
		if (IsTriangleContained(geometry,index, centerX, centerZ, size))
			count++; // IsTriangleContained() �Լ��� �̿��� �ﰢ���� ���ԵǾ������� ī��Ʈ�� �ø���.
	}

	return count;

}

bool TerrainCell::IsTriangleContained(Geometry<VertexPTNTBC>& geometry, int index, float centerX, float centerZ, const float& size)
{
	float radius = size / 2.0f;

	uint vertexIndex = index * 3;

	auto vertices = geometry.GetVertexData();
	uint* indices = geometry.GetIndexData();

	D3DXVECTOR2 point[3];
	for(uint i = 0; i< 3; i++,vertexIndex++)
		point[i] = { vertices[indices[vertexIndex]].Position.x, vertices[indices[vertexIndex]].Position.z };

	float minX = min(point[0].x, min(point[1].x, point[2].x));
	float maxX = max(point[0].x, min(point[1].x, point[2].x));
	float minZ = min(point[0].y, min(point[1].y, point[2].y));
	float maxZ = max(point[0].y, max(point[1].y, point[2].y));

	if (minX > centerX + radius)
		return false;
	if (maxX < centerX - radius)
		return false;
	if (minZ > centerZ + radius)
		return false;
	if (maxZ < centerZ - radius)
		return false;

	return true;

}
