#include "Framework.h"
#include "SubCamera.h"



SubCamera::SubCamera(Context * context) 
	: Camera(context)

{


	graphics = context->GetSubsystem<Graphics>();


	rotation = { 0.000000000f,0.749999881f,0.000000000f }; 
	position = { 44.8372269f,10.0000000f,10.4144621f };



	GeometryUtility::CreateCube(geometry);



	vertexBuffer = new VertexBuffer(context);

	vertexBuffer->Create<VertexTexture>(geometry.GetVertices());



	indexBuffer = new IndexBuffer(context);

	indexBuffer->Create(geometry.GetIndices());



	vertexShader = new VertexShader(context);

	vertexShader->Create("../_Assets/Shader/SubCamera.hlsl");



	pixelShader = new PixelShader(context);

	pixelShader->Create("../_Assets/Shader/SubCamera.hlsl");



	inputLayout = new InputLayout(context);

	inputLayout->Create(vertexShader->GetBlob());



	rsState = new RasterizerState(context);

	worldBuffer = new ConstantBuffer(context);

	worldBuffer->Create<WorldData>();



	D3DXMatrixIdentity(&world);

}



SubCamera::~SubCamera()

{

}



void SubCamera::Update()

{

	if (input->KeyPress('W'))

		position += forward * 0.5f;

	else if (input->KeyPress('S'))

		position -= forward * 0.5f;



	if (input->KeyPress('A'))

		rotation.y -= 0.05f;

	else if (input->KeyPress('D'))

		rotation.y += 0.05f;



	if (input->KeyPress('E'))

		position += up * 1;

	else if (input->KeyPress('Q'))

		position -= up * 1;







	UpdateViewMatrix();

	UpdateProjectionMatrix();



	//context->GetSubsystem<Frustum>()->ConstructFrustum(farPlane, proj, view);



	D3DXMATRIX S, R, T;

	D3DXMatrixScaling(&S, 5.0f, 5.0f, 5.0f);

	D3DXMatrixRotationY(&R, rotation.y);

	D3DXMatrixTranslation(&T, position.x, position.y, position.z);

	world = S * R * T;



	D3DXMatrixTranspose(&world, &world);



	auto worldData = static_cast<WorldData*>(worldBuffer->Map());

	worldData->World = world;

	worldBuffer->Unmap();



}



void SubCamera::Render(ConstantBuffer* cameraBuffer)

{

	vertexBuffer->BindPipeline();

	indexBuffer->BindPipeline();

	inputLayout->BindPipeline();

	graphics->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



	cameraBuffer->BindPipeline(ShaderType::VS, 0);

	worldBuffer->BindPipeline(ShaderType::VS, 1);

	vertexShader->BindPipeline();

	pixelShader->BindPipeline();

	rsState->BindPipeline();



	graphics->GetDeviceContext()->DrawIndexed(geometry.GetIndexCount(), 0, 0);

}