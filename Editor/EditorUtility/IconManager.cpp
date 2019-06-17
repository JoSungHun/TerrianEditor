#include "stdafx.h"
#include "IconManager.h"


bool IconManager::ImageButton(const Icon& icon, const float& width, const float& height)
{
	return ImGui::ImageButton(icon.texture->GetShaderResourceView(), ImVec2(width, height),
		ImVec2(0, 0), ImVec2(1, 1), 2, ImColor(0, 0, 0, 255));
}

void IconManager::Image(const Icon & icon, const float & width, const float & height)
{
	ImGui::Image(icon.texture->GetShaderResourceView(), ImVec2(width, height));
}



void IconManager::Initialize(class Context* context)
{
	this->context = context;


	std::vector<std::string> filePaths = FileSystem::GetFilesInDirectory("../_Assets/Widget_Icon/");
	

	for (auto filePath : filePaths)
	{
		Icon icon;
		icon.filePath = filePath;
		icon.name = FileSystem::GetFileNameFromPath(filePath);
		if (filePath.find("terrainBrush") != std::string::npos) icon.type = IconType::TerrainBrush;
		icon.texture = new Texture(context);
		icon.texture->LoadFromFile(filePath);
		icons.push_back(icon);
	}

	filePaths.shrink_to_fit();

	filePaths = FileSystem::GetFilesInDirectory("../_Assets/Brush/");
	for (auto filePath : filePaths)
	{
		Icon icon;
		icon.filePath = filePath;
		icon.name = FileSystem::GetFileNameFromPath(filePath);
		if (filePath.find("HeightBrush") != std::string::npos) icon.type = IconType::TerrainBrush_Height;
		icon.texture = new Texture(context);
		icon.texture->LoadFromFile(filePath);
		icons.push_back(icon);
	}

	filePaths.shrink_to_fit();

	filePaths = FileSystem::GetFilesInDirectory("../_Assets/terrainTexture/");
	for (auto filePath : filePaths)
	{
		Icon icon;
		icon.filePath = filePath;
		icon.name = FileSystem::GetFileNameFromPath(filePath);
		if(filePath.find("Diffuse") != std::string::npos) icon.type = IconType::TerrainBrush_Texture_D;
		else if (filePath.find("Normal") != std::string::npos)icon.type = IconType::TerrainBrush_Texture_N;
		else if (filePath.find("Specular") != std::string::npos)icon.type = IconType::TerrainBrush_Texture_S;
		icon.texture = new Texture(context);
		icon.texture->LoadFromFile(filePath);
		icons.push_back(icon);
	}

	filePaths.shrink_to_fit();
}

std::vector<ID3D11ShaderResourceView*> IconManager::GetSRVfromType(const IconType & type)
{
	std::vector<ID3D11ShaderResourceView*> vIcon;
	for (auto icon : icons)
	{
		if (icon.type == type)
			vIcon.push_back(icon.texture->GetShaderResourceView());
	}
	return vIcon;
}

ID3D11ShaderResourceView * IconManager::GetSRVfromName(const std::string & name)
{
	for (auto icon : icons)
	{
		if (icon.texture->GetResourceName() == name)
			return icon.texture->GetShaderResourceView();
	}


	return nullptr;
}

const std::vector<Icon> IconManager::GetIconFromType(const IconType & type)
{
	std::vector<Icon> vec;
	for (auto icon : icons)
	{
		if (icon.type == type)
			vec.push_back(icon);
	}

	return  vec;
}



const bool IconManager::ImageButton(const std::string & filePath, const float & width, const float& height)
{
	bool pressed = ImGui::ImageButton
	(
		GetSRVfromName(filePath),
		ImVec2(width,height)
	);
	return pressed;
}


IconManager::IconManager()
{
}

IconManager::~IconManager()
{
}
