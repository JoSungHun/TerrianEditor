#include "Framework.h"
#include "Frustum.h"

Frustum::Frustum(Context * context)
	: ISubsystem(context)
{
}

Frustum::~Frustum()
{
	SAFE_DELETE(vertexBuffer);
	SAFE_DELETE(indexBuffer);
	SAFE_DELETE(inputLayout);
	SAFE_DELETE(vs);
	SAFE_DELETE(ps);
	SAFE_DELETE(rsState);
}

const bool Frustum::Initialize()
{
	vertexBuffer = new VertexBuffer(context);
	indexBuffer = new IndexBuffer(context);
	vs = new VertexShader(context);
	ps = new PixelShader(context);
	inputLayout = new InputLayout(context);
	rsState = new RasterizerState(context);
	cbuffer = new ConstantBuffer(context);
	cameraBuffer = new ConstantBuffer(context);
	cameraBuffer->Create<CameraData>();

	vs->Create("../_Assets/Shader/frustumGuide.hlsl");
	ps->Create("../_Assets/Shader/frustumGuide.hlsl");
	inputLayout->Create(vs->GetBlob());
	rsState->FillMode(D3D11_FILL_WIREFRAME);
	
	

	return true;
}

void Frustum::ConstructFrustum(const float & screenDepth, const D3DXMATRIX & proj, const D3DXMATRIX & view)
{
	float zMinimum, r;
	D3DXMATRIX mat;
	D3DXMATRIX matProj = proj;
	D3DXMATRIX matView = view;

	zMinimum = -proj._43 / proj._33;
	r = screenDepth / (screenDepth - zMinimum);
	matProj._33 = r;
	matProj._43 = -r * zMinimum;
	D3DXMatrixMultiply(&mat, &matView, &matProj);

	//Near
	float x = static_cast<float>(mat._14 + mat._13);
	float y = static_cast<float>(mat._24 + mat._23);
	float z = static_cast<float>(mat._34 + mat._33);
	float w = static_cast<float>(mat._44 + mat._43);
	planes[0] = { x,y,z,w };
	D3DXPlaneNormalize(&planes[0], &planes[0]);


	//Far
	x = static_cast<float>(mat._14 - mat._13);
	y = static_cast<float>(mat._24 - mat._23);
	z = static_cast<float>(mat._34 - mat._33);
	w = static_cast<float>(mat._44 - mat._43);
	planes[1] = { x,y,z,w };
	D3DXPlaneNormalize(&planes[1], &planes[1]);


	//Left
	x = static_cast<float>(mat._14 + mat._11);
	y = static_cast<float>(mat._24 + mat._21);
	z = static_cast<float>(mat._34 + mat._31);
	w = static_cast<float>(mat._44 + mat._41);
	planes[2] = { x,y,z,w };
	D3DXPlaneNormalize(&planes[2], &planes[2]);

	//Right
	x = static_cast<float>(mat._14 - mat._11);
	y = static_cast<float>(mat._24 - mat._21);
	z = static_cast<float>(mat._34 - mat._31);
	w = static_cast<float>(mat._44 - mat._41);
	planes[3] = { x,y,z,w };
	D3DXPlaneNormalize(&planes[3], &planes[3]);

	//Up
	x = static_cast<float>(mat._14 - mat._12);
	y = static_cast<float>(mat._24 - mat._22);
	z = static_cast<float>(mat._34 - mat._32);
	w = static_cast<float>(mat._44 - mat._42);
	planes[4] = { x,y,z,w };
	D3DXPlaneNormalize(&planes[4], &planes[4]);

	//Down
	x = static_cast<float>(mat._14 + mat._12);
	y = static_cast<float>(mat._24 + mat._22);
	z = static_cast<float>(mat._34 + mat._32);
	w = static_cast<float>(mat._44 + mat._42);
	planes[5] = { x,y,z,w };
	D3DXPlaneNormalize(&planes[5], &planes[5]);
}

bool Frustum::CheckPoint(const D3DXVECTOR3 & position)
{
	for (uint i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &position) < 0.0f)
			return false;
	}
	
	return true;
}

bool Frustum::CheckRay(const D3DXVECTOR3 & org, const D3DXVECTOR3 & dir)
{
	D3DXVECTOR3 pos = org + dir;

	for (uint i = 2; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &pos) < 0.0f)
			return false;
	}
	return true;
}

bool Frustum::CheckCube(const D3DXVECTOR3 & centerPos, const float & radius)
{
	for (uint i = 0; i < 6; i++)
	{
		D3DXVECTOR3 pos = { -radius,-radius,-radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		pos = { radius,-radius,-radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		pos = { -radius,+radius,-radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		pos = { radius,radius,-radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		pos = { -radius,-radius,radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		pos = { radius,-radius,radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		pos = { -radius,radius,radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		pos = { radius,radius,radius };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - pos)) >= 0.0)
			continue;

		return false;
	}


	return true;
}

bool Frustum::CheckSphere(const D3DXVECTOR3 & centerPos, const float & radius)
{
	for (uint i = 0; i < 6; i++)
	{
		if (D3DXPlaneDotCoord(&planes[i], &centerPos) < -radius)
			return false;
	}
	return true;
}

bool Frustum::CheckRectangle(const D3DXVECTOR3 & centerPos, const float & xSize, const float & ySize, const float & zSize)
{
	for (int i = 0; i < 6; i++)
	{
		D3DXVECTOR3 vSize = { xSize, ySize,zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;

		vSize = { xSize,-ySize,zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;

		vSize = { xSize, -ySize,-zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;

		vSize = { xSize, ySize,-zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;

		vSize = { -xSize, ySize,zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;

		vSize = { -xSize, -ySize,zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;

		vSize = { -xSize, -ySize,-zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;

		vSize = { -xSize, ySize,-zSize };
		if (D3DXPlaneDotCoord(&planes[i], &D3DXVECTOR3(centerPos - vSize)) >= 0.0f)
			continue;
	
		return false;
	}

	return true;
}

bool Frustum::CheckCube(const D3DXVECTOR3 & min, const D3DXVECTOR3 & max)
{
	for (uint i = 0; i < 6; i++)
	{
		D3DXVECTOR3 pos = { max.x,max.y,max.z };//+++
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		pos = { min.x,max.y,max.z }; // -++
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		pos = { max.x,min.y,max.z };//+-+
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		pos = { min.x,min.y,max.z };//--+
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		pos = { max.x,max.y,min.z };//++-
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		pos = { min.x,max.y,min.z };//-+-
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		pos = { max.x,min.y,min.z };//+--
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		pos = { min.x,min.y,min.z };//---
		if (D3DXPlaneDotCoord(&planes[i], &pos) >= 0.0)
			continue;

		return false;
	}


	return true;
}

void Frustum::DrawCube(const D3DXVECTOR3 & centerPos, const float & radius, const CameraData& cameraData)
{
	Geometry<VertexColor> geometry;
	//¾Õ 
	//  2  3
	//  0  1
	geometry.AddVertex(VertexColor({ centerPos.x - radius, - radius, centerPos.z + radius }, { 1,0,0,1 }));
	geometry.AddVertex(VertexColor({ centerPos.x + radius, - radius, centerPos.z + radius }, { 1,0,0,1 }));
	geometry.AddVertex(VertexColor({ centerPos.x - radius, + radius, centerPos.z + radius }, { 1,0,0,1 }));
	geometry.AddVertex(VertexColor({ centerPos.x + radius, + radius, centerPos.z + radius }, { 1,0,0,1 }));
	//µÚ
	//  6  7
	//  4  5
	geometry.AddVertex(VertexColor({ centerPos.x - radius, -radius, centerPos.z - radius }, { 1,0,0,1 }));
	geometry.AddVertex(VertexColor({ centerPos.x + radius, -radius, centerPos.z - radius }, { 1,0,0,1 }));
	geometry.AddVertex(VertexColor({ centerPos.x - radius, +radius, centerPos.z - radius }, { 1,0,0,1 }));
	geometry.AddVertex(VertexColor({ centerPos.x + radius, +radius, centerPos.z - radius }, { 1,0,0,1 }));

	
	geometry.AddIndex(0); geometry.AddIndex(1); 
	geometry.AddIndex(1); geometry.AddIndex(3);
	geometry.AddIndex(3); geometry.AddIndex(2);
	geometry.AddIndex(2); geometry.AddIndex(0);

	geometry.AddIndex(2); geometry.AddIndex(6);
	geometry.AddIndex(3); geometry.AddIndex(7);
	geometry.AddIndex(6); geometry.AddIndex(7);

	geometry.AddIndex(4); geometry.AddIndex(0);
	geometry.AddIndex(4); geometry.AddIndex(6); 

	geometry.AddIndex(1); geometry.AddIndex(5);
	geometry.AddIndex(5); geometry.AddIndex(7);

	geometry.AddIndex(4); geometry.AddIndex(5);

	//right
	
	//renderSetting
	Graphics* graphics;
	graphics = context->GetSubsystem<Graphics>();

	vertexBuffer->Create(geometry.GetVertices());
	indexBuffer->Create(geometry.GetIndices());

	cbuffer->Create<WorldData>();
	D3DXMATRIX world;
	D3DXMatrixIdentity(&world);
	auto worldData = static_cast<WorldData*>(cbuffer->Map());
	D3DXMatrixTranspose(&world, &world);
	worldData->World = world;
	cbuffer->Unmap();

	auto camera = static_cast<CameraData*>(cameraBuffer->Map());
	camera->Proj = cameraData.Proj;
	camera->View = cameraData.View;
	cameraBuffer->Unmap();


	rsState->BindPipeline();
	cameraBuffer->BindPipeline(ShaderType::VS, 0);
	cbuffer->BindPipeline(ShaderType::VS, 1);
	graphics->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_LINELIST);
	vertexBuffer->BindPipeline();
	indexBuffer->BindPipeline();
	inputLayout->BindPipeline();
	vs->BindPipeline();
	ps->BindPipeline();

	graphics->GetDeviceContext()->DrawIndexed(geometry.GetIndexCount(), 0, 0); 
	rsState->UnbindPipeline();



}

void Frustum::DrawFrustum()
{


}
