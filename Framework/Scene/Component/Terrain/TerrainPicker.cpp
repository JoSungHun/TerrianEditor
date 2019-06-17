#include "framework.h"
#include "TerrainPicker.h"
#include "Scene\Scene.h"
#include "Scene/Component/Camera.h"

TerrainPicker::TerrainPicker(Context * context)
	: context(context)
	, computeShader(nullptr)
	, inputBuffer(nullptr)
	, outputBuffer(nullptr)
	, rayBuffer(nullptr)
	, trInfoBuffer(nullptr)
{
	graphics = context->GetSubsystem<Graphics>();
	camera = context->GetSubsystem<SceneManager>()->GetCurrentScene()->GetCamera();

	inputBuffer = new ComputeBuffer(context);
	inputBuffer->SetBufferType(COMPUTE_BUFFER_TYPE::INPUT);
	outputBuffer = new ComputeBuffer(context);
	outputBuffer->SetBufferType(COMPUTE_BUFFER_TYPE::OUTPUT);

	rayBuffer = new ConstantBuffer(context);
	rayBuffer->Create<CameraRayData>();

	trInfoBuffer = new ConstantBuffer(context);
	trInfoBuffer->Create<TerrainData>();

	computeShader = new ComputeShader(context);
	computeShader->Create("../_Assets/Shader/pick.hlsl");


}

TerrainPicker::~TerrainPicker()
{
}

bool TerrainPicker::Picking(const D3DXMATRIX& world, D3DXVECTOR3& position, const D3DXVECTOR2& trSize)
{
	auto rayData = static_cast<CameraRayData*>(rayBuffer->Map()); 
	camera->GetLocalRay(rayData->origin, rayData->dir, world); 
	rayBuffer->Unmap();
	rayBuffer->BindPipeline(ShaderType::CS, 0);

	auto trInfoData = static_cast<TerrainData*>(trInfoBuffer->Map());
	trInfoData->Size = trSize;
	trInfoBuffer->Unmap();
	trInfoBuffer->BindPipeline(ShaderType::CS, 1);


	inputBuffer->BindPipeline(0);
	outputBuffer->BindPipeline(0);
	computeShader->BindPipeline();

	
	graphics->GetDeviceContext()->Dispatch(static_cast<uint>(trSize.x-1)*static_cast<uint>(trSize.y-1)*2, 1, 1);
	
	return GetResult(position); 

}


void TerrainPicker::SetOutput()
{
	std::vector<PickOutputData> outputDatas(1);
	outputBuffer->CreateBuffer(outputDatas.data(), 1);
}

bool TerrainPicker::GetResult(D3DXVECTOR3 & position)
{
	/*std::vector<PickOutputData> result(outputBuffer->GetSize());
	memcpy_s(result.data(), sizeof(PickOutputData)*outputBuffer->GetSize(), outputBuffer->GetResult(), sizeof(PickOutputData)*outputBuffer->GetSize());*/
	//static_cast<PickOutputData*>(outputBuffer->GetResult());

	
	memcpy_s(&pickedData, sizeof(PickOutputData), outputBuffer->GetResult(), sizeof(PickOutputData));

	/*float distance = 10000;
	D3DXVECTOR3 pos;
	bool isPicked = result.picked;*/


	if (pickedData.picked == 1)
	{
		position = pickedData.pos;
	}

	return pickedData.picked;
}


void TerrainPicker::DebugRender()
{
	auto dWrite = context->GetSubsystem<DirectWrite>();

	
	if (pickedData.picked == 1)
	{

		WCHAR t[128];
		swprintf_s(t, L"pick distance : %0.2f", pickedData.distance);
		dWrite->Text(t, D3DXVECTOR2(0, 70));

		swprintf_s(t, L"pick Pos : %0.2f, %0.2f, %0.2f", pickedData.pos.x, pickedData.pos.y, pickedData.pos.z);
		dWrite->Text(t, D3DXVECTOR2(0, 90));

	}
}