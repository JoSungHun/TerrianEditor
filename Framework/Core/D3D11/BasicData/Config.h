#pragma once
#include "Framework.h"


//=========================================
// NEW
//========================================

static const std::string NOT_ASSIGNED_STR = "N/A";
static const int NOT_ASSIGNED_ID = -1;

enum class CullMode : uint
{
	NONE = 1U << 0,
	FRONT = 1U << 1,
	BACK = 1U << 2,
};

enum class FillMode : uint
{
	SOLID = 1U << 3,
	WIREFRAME = 1U << 4,
};

enum class Factor : uint
{
	ZERO = 1U << 2,
	ONE = 1U << 3,
	SRC_COLOR = 1U << 4,
	INV_SRC_COLOR = 1U << 5,
	SRC_ALPHA = 1U << 6,
	INV_SRC_ALPHA = 1U << 7,
	DEST_ALPHA = 1U << 8,
	INV_DEST_ALPHA = 1U << 9,
	DEST_COLOR = 1U << 10,
	INV_DEST_COLOR = 1U << 11,
};

enum class Operation : uint
{
	ADD = 1U << 12,
	SUBTRACT = 1U << 13,
	REV_SUBTRACT = 1U << 14,
	MIN = 1U << 15,
	MAX = 1U << 16,
};

enum class ColorMask : uint
{
	RED = 1U << 17,
	GREEN = 1U << 18,
	BLUE = 1U << 19,
	ALPHA = 1U << 20,
	ALL = RED | GREEN | BLUE | ALPHA,
};

enum class ComparisonFunc : uint
{
	NEVER = 1U << 2,
	LESS = 1U << 3,
	EQUAL = 1U << 4,
	LESS_EQUAL = 1U << 5,
	GREATER = 1U << 6,
	NOT_EQUAL = 1U << 7,
	GREATER_EQUAL = 1U << 8,
	ALWAYS = 1U << 9,
};

enum class ShaderStage : uint
{
	Unknown, VS, HS, DS, GS, PS, CS, Global,
};

struct GlobalData
{
	D3DXMATRIX WVP;
	D3DXMATRIX View;
	D3DXMATRIX Proj;
	D3DXMATRIX Proj_Ortho;
	D3DXMATRIX VP;
	D3DXMATRIX VP_Inverse;
	D3DXMATRIX VP_Ortho;
	float Camera_Near;
	float Camera_Far;
	D3DXVECTOR2 Resolution;
	D3DXVECTOR3 Camera_Position;
	float Bloom_Intensity;
};

struct TransformData
{
	D3DXMATRIX World;
	D3DXMATRIX WVP_Current;
	D3DXMATRIX WVP_Previous;
};

struct MaterialData
{
	D3DXCOLOR MaterialAlbedoColor;
	D3DXVECTOR2 MaterialTiling;
	D3DXVECTOR2 MaterialOffset;
	float MaterialRoughness;
	float MaterialMetallic;
	float MaterialNormalStrength;
	float MaterialHeight;
	float MaterialShadingMode;
	float Padding[3];
};

const static int MaxLights = 64;
struct LightData
{
	D3DXMATRIX WVP;
	D3DXMATRIX VP_Inv;
	D3DXCOLOR Dir_LightColor;
	D3DXVECTOR4 Dir_LightIntensity;
	D3DXVECTOR4 Dir_LightDirection;

	D3DXVECTOR4 Point_LightPosition[MaxLights];
	D3DXCOLOR Point_LightColor[MaxLights];
	D3DXVECTOR4 Point_LightIntensity_Range[MaxLights];

	D3DXCOLOR Spot_LightColor[MaxLights];
	D3DXVECTOR4 Spot_LightPosition[MaxLights];
	D3DXVECTOR4 Spot_LightDirection[MaxLights];
	D3DXVECTOR4 Spot_LightIntensity_Range_Angle[MaxLights];

	float Point_LightCount;
	float Spot_LightCount;
	int Screen_Space_Reflection;

	float Padding;
};

struct ShadowMapData
{
	D3DXMATRIX View;
	D3DXMATRIX View_Projection_Inv;
	D3DXMATRIX View_Projection[3];
	D3DXVECTOR3 Position;
	float Resolution;
	D3DXVECTOR3 Direction;
	float Range;
	D3DXVECTOR2 Biases;
	D3DXVECTOR2 Padding;
};

struct BlurData
{
	D3DXVECTOR2 Direction;
	float Sigma;
	float Padding;
};

struct LightCascadeData
{
	D3DXMATRIX WVP;
};

struct TransparentData
{
	D3DXMATRIX World;
	D3DXMATRIX WVP;
	D3DXCOLOR Color;
	D3DXVECTOR3 Camera_Pos;
	float Roughness;
	D3DXVECTOR3 Light_Dir;
	float Padding;
};

struct TerrainData
{
	D3DXVECTOR3 brush_position;
	float brush_range;
	float b_picked;
	float width;
	float height;
	float tess_factor;
	//flags

	//padding
};

struct OceanData
{
	float waterTranslation;
	float reflectRefractScale;
	int padding[2];
};



#pragma region oldVersion
//enum class ShaderType : uint
//{
//	VS, HS, DS, GS, PS, CS
//};
//
////========================================
////			Basics
////========================================
//#define MAX_SHADOW_LEVEL 4
//
////Camera Matrices
//struct CameraData
//{
//	D3DXMATRIX View;
//	D3DXMATRIX Proj;
//};
//
//// Camera RayCasting
//struct CameraRayData
//{
//	D3DXVECTOR3 origin;
//	int padding0;
//	D3DXVECTOR3 dir;
//	int padding1;
//};
//
//// World Matrix
//struct WorldData
//{
//	D3DXMATRIX World;
//};
//
//
//
////LightData
//struct LightData
//{
//	D3DXMATRIX lightView;
//	D3DXMATRIX lightProj[MAX_SHADOW_LEVEL];
//	D3DXVECTOR4 lightPos[MAX_SHADOW_LEVEL];
//	uint lightType;
//
//};
//
//struct LightColorData
//{
//	D3DXCOLOR ambient;
//	D3DXCOLOR diffuse;
//	D3DXVECTOR3 Direction;
//	uint lightType;
//};
//
//// Tessellation Factor
//struct TessData
//{
//	float tessFactor;
//	D3DXVECTOR3 CameraPos;
//};
//
//struct ClipPlaneData
//{
//	D3DXVECTOR4 clipPlane;
//
//	ClipPlaneData();
//	ClipPlaneData(D3DXVECTOR4 plane)
//		: clipPlane(plane){}
//};
//
////========================================
////			Water
////========================================
//
//struct WaterData
//{
//	float waterTranslation;
//	float reflectRefractScale;
//	int padding[2];
//};
//
//struct ReflectionData
//{
//	D3DXMATRIX reflectionMatrix;
//};
//
////========================================
////			Terrain 
////========================================
//
//// Terrain Painting(Edit Brush)
//struct PaintData
//{
//	D3DXCOLOR Color; // 0 - 4
//	D3DXVECTOR3 Position; //1 - 3
//	float Range;	// 1 - 4
//	float Intensity; // 2 - 1
//	int BrushType;// 2 - 2
//	uint trSize[2]; // 2 - 3,4
//	int ChannelNum; // 3- 1
//	int texRowPitch;// 3- 2
//	int padding[2];  // 3- 4
//
//
//	PaintData()
//		: Position(0, 0, 0)
//		, Range(0.f)
//		, Intensity(0.f)
//		, BrushType(0)
//		, ChannelNum(0)
//	{}
//};
//
//// Terrain Guide Rendering
//struct BrushData
//{
//	D3DXCOLOR Color;
//	D3DXVECTOR3 Position;
//	float Range;
//
//	BrushData()
//		: Position(0, 0, 0)
//		, Color(0,0,0,0)
//		, Range(0)
//	{}
//};
//
//// Terrain Size
//struct TerrainData
//{
//	D3DXVECTOR2 Size;
//	int padding[2]; 
//
//	TerrainData()
//		: Size(0,0){}
//	TerrainData(float width, float height)
//		: Size(width, height){}
//};
//
//
////========================================
////			Grass
////========================================
#pragma endregion