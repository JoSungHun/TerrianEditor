#pragma once
#pragma warning(disable : 4819)
#pragma warning(disable : 4506)

#define OPTIMIZATION
#define NOMINMAX

#ifdef _DEBUG
#pragma comment(linker,"/entry:WinMainCRTStartup /subsystem:console")
#endif

//Window
#include <Windows.h>
#include <assert.h>
#include <time.h>

//STL
#include <string>
#include <iostream>
#include <fstream>
#include <array>
#include <vector>
#include <deque>
#include <queue>
#include <map>
#include <unordered_map>
#include <functional>
#include <chrono>
#include <limits>
#include <mutex>
#include <thread>

//DirectX
#include <dxgi1_2.h>
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include <d3d11shader.h>
#include <d3d11.h>
#include <D3DX10math.h>
#include <D3DX11async.h>

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "d3dx10.lib")
#pragma comment(lib, "d3dx11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dxguid.lib")

//DirectWrite
#include <d2d1_1.h>	
#include <dwrite.h>
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")


typedef unsigned int uint;
typedef unsigned long ulong;

//Macro Function
#define SAFE_RELEASE(p)			{ if(p) { (p)->Release(); (p) = nullptr; } }
#define SAFE_DELETE(p)			{ if(p) { delete (p); (p) = nullptr; } }
#define SAFE_DELETE_ARRAY(p)	{ if(p) { delete[] (p); (p) = nullptr; } }


//Math
#include "./Math/Math.h"
#include "./Math/Rect.h"

//Log
#include "./Log/Log.h"

//Utility
#include "./Utility/GeometryUtility.h"
#include "./Utility/FileStream.h"
#include "./Utility/FileSystem.h"
#include "./Utility/GUIDGenerator.h"

//Main System
#include "./Core/Settings.h"
#include "./Core/Context.h"
#include "./Core/Engine.h"

//EventSystem
#include "./Core/EventSystem/EventSystem.h"

//Basic Data
#include "./Core/D3D11/BasicData/Config.h"
#include "./Core/D3D11/BasicData/Vertex.h"
#include "./Core/D3D11/BasicData/Geometry.h"

//Subsystem
#include "./Core/Subsystem/Input.h"
#include "./Core/Subsystem/Graphics.h"
#include "./Core/Subsystem/SceneManager.h"
#include "./Core/Subsystem/DirectWrite.h"
#include "./Core/Subsystem/ResourceManager.h"
#include "./Core/Subsystem/Thread.h"
#include "./Core/Subsystem/Timer.h"
#include "./Core/Subsystem/Renderer.h"

//D3D11 Wrapper Class
#include "./Core/D3D11/IAStage/VertexBuffer.h"
#include "./Core/D3D11/IAStage/IndexBuffer.h"
#include "./Core/D3D11/IAStage/InstanceBuffer.h"
#include "./Core/D3D11/IAStage/InputLayout.h"
#include "./Core/D3D11/RSStage/RasterizerState.h"
#include "./Core/D3D11/OMStage/DepthStencilState.h"
#include "./Core/D3D11/OMStage/BlendState.h"
#include "./Core/D3D11/OMStage/RenderTexture.h"
#include "./Core/D3D11/ComputeShader/ComputeBuffer.h"
#include "./Core/D3D11/ComputeShader/ComputeShader.h"
#include "./Core/D3D11/ShaderStage/VertexShader.h"
#include "./Core/D3D11/ShaderStage/HullShader.h"
#include "./Core/D3D11/ShaderStage/DomainShader.h"
#include "./Core/D3D11/ShaderStage/PixelShader.h"
#include "./Core/D3D11/ShaderStage/ConstantBuffer.h"
#include "./Core/D3D11/ShaderStage/Sampler.h"

#include "./Scene/Component/Camera.h"
#include "./Scene/Component/Light/Light.h"

//Shader
#include "./Shader/Shader.h"

//Resources
#include "./Resource/Material.h"
#include "./Resource/Texture.h"
#include "./Resource/Mesh.h"
