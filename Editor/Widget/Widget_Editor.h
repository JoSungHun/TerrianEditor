#pragma once
#include "IWidget.h"

struct Item
{
	Item(const std::string& path, const Thumbnail& thumbnail)
		: path(path)
		, thumbnail(thumbnail)
	{
		label = FileSystem::GetFileNameFromPath(path);
	}

	Thumbnail thumbnail;
	std::string path;
	std::string label;
};

 
class Widget_Editor : public IWidget
{
public:
	static class Scene* scene;
	class ResourceManager* resourceManager;
//========================
// Member Func
//========================
public:
	Widget_Editor(class Context* context);
	~Widget_Editor();

	void Render() override;


private:
	void UpdateItems();
	void UpdateActorsID();

	
//Terrain
	//Create Terrain
	void ShowTerrainMenuBar();
	void ShowCreateTerrain();
	void ShowTerrainEditor();
	void ShowTerrainLoadFromFile();

	//Terrain Brush
	void ShowTerrainBrushHeight(class Terrain* terrain);
	void ShowTerrainBrushFlat(class Terrain* terrain);
	void ShowTerrainBrushTexture(class Terrain* terrain);
	
//SunLight
	void ShowLightEditor();

//Ocean
	void ShowOceanEditor();

//========================
// Member Var
//========================
private:
	std::vector<Item> brushes;
	std::vector<Item> painter_diffuses;
	std::vector<Item> painter_normals;

//Terrain
	uint terrain_id;
	bool bCreateTerrainPopUp;
	bool bLoadTerrain;


	bool bTerrainCollapse;
	bool bTerrainBrush;
	bool b_terrain_brush_height;
	bool b_terrain_brush_flat;
	bool b_terrain_brush_paint;

	uint selected_brush_ID; 
	//TextureBrush Icon
	uint channels_paint_ID[5];
	uint selected_paint_ID;

//Skybox
	uint skybox_id;

//Ocean
	uint ocean_id;
	bool bOceanCollapse;

//Light
	uint light_id;
	bool bLightCollapse;

};