#pragma once
#include "Camera.h"

class SubCamera : public Camera
{
public:
	SubCamera(class Context* context);
	~SubCamera();

	void Update();
	void Render(ConstantBuffer* cameraBuffer);

private:
	/*class Context* context;
	class Input* input;

	ProjectionType projectionType;
	float fov;
	float nearPlane;
	float farPlane;

	D3DXVECTOR3 position;
	D3DXVECTOR3 rotation;
	D3DXVECTOR3 right;
	D3DXVECTOR3 up;
	D3DXVECTOR3 forward;

	D3DXMATRIX view;
	D3DXMATRIX proj;*/

	class Graphics* graphics;

	Geometry<VertexTexture> geometry;
	//IA
	VertexBuffer* vertexBuffer;
	IndexBuffer* indexBuffer;
	InputLayout* inputLayout;

	//VS
	VertexShader* vertexShader;
	ConstantBuffer* worldBuffer;
	D3DXMATRIX world;

	//RS
	RasterizerState* rsState;

	//PS
	PixelShader* pixelShader;


};