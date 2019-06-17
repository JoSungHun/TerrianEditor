#pragma once
#include "ISubsystem.h"

class Frustum : public ISubsystem
{
public:
	Frustum(class Context* context);
	~Frustum();
	// ISubsystem을(를) 통해 상속됨
	virtual const bool Initialize() override;

	void ConstructFrustum(const float& screenDepth, const D3DXMATRIX& proj, const D3DXMATRIX& view);

	bool CheckPoint(const D3DXVECTOR3& position);
	bool CheckRay(const D3DXVECTOR3& org, const D3DXVECTOR3& dir);
	bool CheckCube(const D3DXVECTOR3& centerPos, const float& radius);
	bool CheckSphere(const D3DXVECTOR3& centerPos, const float& radius);
	bool CheckRectangle(const D3DXVECTOR3& centerPos, const float& xSize, const float& ySize, const float& zSize);
	bool CheckCube(const D3DXVECTOR3& min, const D3DXVECTOR3& max);

	void DrawCube(const D3DXVECTOR3& centerPos, const float& radius, const GlobalData& cameraBuffer);

	void DrawFrustum();

private:
	class Graphics* graphics;
	D3DXPLANE planes[6];

	class ConstantBuffer* cameraBuffer;
	class ConstantBuffer* cbuffer;
	class VertexBuffer* vertexBuffer;
	class IndexBuffer* indexBuffer;
	class VertexShader* vs;
	class PixelShader* ps;
	class InputLayout* inputLayout;
	class RasterizerState* rsState;
};

