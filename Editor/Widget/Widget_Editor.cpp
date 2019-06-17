#include "stdafx.h"
#include "Widget_Editor.h"
#include "../Framework/Scene/Scene.h"
#include "../Framework/Scene/Actor.h"
#include "../Framework/Scene/Component/Camera.h"
#include "../Framework/Scene/Component/Terrain/Terrain.h"
#include "../Framework/Scene/Component/Skybox/Skybox.h"
#include "../Framework/Scene/Component/Ocean/Ocean.h"
#include "../Framework/Scene/Component/Light/Light.h"
#include "../Framework/Scene/Component/Renderable.h"

#define ICON_TERRAIN_BRUSH_SIZE 30

Scene* Widget_Editor::scene = nullptr;

Widget_Editor::Widget_Editor(Context * context)
	: IWidget(context)
	, bCreateTerrainPopUp(false)
	, bLoadTerrain(false)
	, bTerrainCollapse(false)
	, bTerrainBrush(false)
	, b_terrain_brush_height(false)
	, b_terrain_brush_flat(false)
	, b_terrain_brush_paint(false)
	, bLightCollapse(false)
	, bOceanCollapse(false)
	, selected_brush_ID(0)
	, selected_paint_ID(0)
{
	title = "TerrainEditor";
	xMin = 250;
	yMin = 250;
	windowFlags |= ImGuiWindowFlags_MenuBar;


	scene = context->GetSubsystem<SceneManager>()->GetCurrentScene(); 
	resourceManager = context->GetSubsystem<ResourceManager>();

	UpdateItems();

	for (uint i = 0; i < 5; i++)
		for (uint j = 0; j < 2; j++)
			channels_paint_ID[i] = 0;


}

Widget_Editor::~Widget_Editor()
{
}

void Widget_Editor::Render()
{ 
	UpdateActorsID();

		//TerrainEditor Menu
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("Menu"))
		{
			ShowTerrainMenuBar();
			ImGui::EndMenu();
		}



		ImGui::EndMenuBar();
		if (bCreateTerrainPopUp) ShowCreateTerrain();
		if (bLoadTerrain) ShowTerrainLoadFromFile();
	}

	if (terrain_id)
	{
		if (ImGui::CollapsingHeader("Terrain"))
			bTerrainCollapse = true;
		else 
			bTerrainCollapse = false;

		if (bTerrainCollapse) ShowTerrainEditor();
	}
	
	{
		if (ImGui::CollapsingHeader("Light"))
			bLightCollapse = true;
		else
			bLightCollapse = false;

		if (bLightCollapse)
		{
			ShowLightEditor();
		}
	}

	{
		if (ImGui::CollapsingHeader("Ocean"))
			bOceanCollapse = true;
		else
			bOceanCollapse = false;

		if (bOceanCollapse) ShowOceanEditor();
	}
}

void Widget_Editor::UpdateItems()
{
	auto registerItems = [this](const std::string& directory, std::vector<Item>& items, const std::string& constraint = "")->void
	{
		auto files = FileSystem::GetFilesInDirectory(directory);
		for (const auto& file : files)
		{
			if (constraint.size() == 0)
			{
				items.emplace_back(file, *(IconProvider::Get().Load(file)));
			}
			else
			{
				if (file.find(constraint) != std::string::npos)
				{
					items.emplace_back(file, *(IconProvider::Get().Load(file)));
				}
			}
		}
	};

	registerItems("../_Assets/Texture/brush_shape/", brushes);
	registerItems("../_Assets/Texture/terrain_painter/", painter_diffuses, "Diffuse");
	registerItems("../_Assets/Texture/terrain_painter/", painter_normals, "Normal");
}

void Widget_Editor::UpdateActorsID()
{
	terrain_id = 0;
	skybox_id = 0;
	light_id = 0;
	for (const auto& actor : scene->GetActors())
	{
		if (actor->HasComponent<Terrain>())
		{
			terrain_id = actor->GetID();
			continue;
		}

		if (actor->HasComponent<Skybox>())
		{
			skybox_id = actor->GetID();
			continue;
		}

		if (actor->HasComponent<Light>())
		{
			light_id = actor->GetID();
			continue;
		}

		if (actor->HasComponent<Ocean>())
		{
			ocean_id = actor->GetID();
		}
	}
}




#pragma region TERRAIN_EDITOR

void Widget_Editor::ShowTerrainMenuBar()
{
	ImGui::MenuItem("(Create Terrain)", nullptr, false, false);
	if (ImGui::MenuItem("Create Terrain"))
	{
		bCreateTerrainPopUp = true;
	}
	if (ImGui::MenuItem("Load HeightMap"))
	{
		bLoadTerrain = true;
	}
	ImGui::Separator();
	ImGui::Spacing();
}


void Widget_Editor::ShowCreateTerrain()
{
	ImGui::OpenPopup("CreateTerrain");
	if (ImGui::BeginPopupModal("CreateTerrain", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
	{
		const ImU32 u_one = 1;

		static ImU32 width = 0;
		static ImU32 height = 0;


		ImGui::PushItemWidth(100.f);
		ImGui::InputScalar("Width", ImGuiDataType_U32, &width, nullptr, nullptr, "%d");
		ImGui::PushItemWidth(100.f);
		ImGui::InputScalar("height", ImGuiDataType_U32, &height, nullptr, nullptr, "%d");


		if (ImGui::Button("OK", ImVec2(80, 0)))
		{
			scene->GetActorFromID(terrain_id)->GetComponent<Terrain>()->Create(width, height);
			scene->GetActorFromID(ocean_id)->GetComponent<Ocean>()->EditOceanSize(width, height);
			ImGui::CloseCurrentPopup();
			bCreateTerrainPopUp = false;
		}
		ImGui::SetItemDefaultFocus();
		ImGui::SameLine();

		if (ImGui::Button("Cancel", ImVec2(80, 0)))
		{
			ImGui::CloseCurrentPopup();
			bCreateTerrainPopUp = false;
		}

		ImGui::EndPopup();
	}
}

/// "Terrain"Collapse - menu -> brush(height, texture)
void Widget_Editor::ShowTerrainEditor()
{
	auto terrain = scene->GetActorFromID(terrain_id)->GetComponent<Terrain>();


	auto ButtonChange = [&]()
	{
		b_terrain_brush_height = false;
		b_terrain_brush_flat = false;
		b_terrain_brush_paint = false;
		terrain->SetBrushType(BrushType::BRUSH_NONE);
		terrain->SetBrushTexture(nullptr);
	};

	auto DisplayButton = [this, &ButtonChange](const IconType& type, bool* toggle)
	{
		ImGui::PushStyleColor
		(
			ImGuiCol_Button,
			ImGui::GetStyle().Colors[*toggle ? ImGuiCol_ButtonActive : ImGuiCol_Button]
		);

		if (IconProvider::Get().ImageButton(type, ICON_TERRAIN_BRUSH_SIZE, ICON_TERRAIN_BRUSH_SIZE - 10))
		{
			ButtonChange();
			*toggle = !(*toggle);
		}
		ImGui::PopStyleColor();
		ImGui::SameLine();
	};

	DisplayButton(IconType::Terrain_Brush_Flat, &b_terrain_brush_flat);
	DisplayButton(IconType::Terrain_Brush_Height, &b_terrain_brush_height);
	DisplayButton(IconType::Terrain_Brush_Paint, &b_terrain_brush_paint);

	ImGui::NewLine();
	
		

	if (b_terrain_brush_height)
	{
		ShowTerrainBrushHeight(terrain);
		terrain->SetBrushType(BrushType::BRUSH_HEIGHT);
	}
	if (b_terrain_brush_flat)
	{
		ShowTerrainBrushFlat(terrain);
		terrain->SetBrushType(BrushType::BRUSH_FLAT);
	}
	if (b_terrain_brush_paint)
	{
		ShowTerrainBrushTexture(terrain);
		terrain->SetBrushType(BrushType::BRUSH_PAINTING);
	}


	//TEST
	//auto material = scene->GetActorFromID(terrain_id)->GetComponent<Renderable>()->GetMaterial();

	//if (!material)
	//	return;

	//if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen))
	//{
	//	auto roughness = material->GetRoughnessMultiplier();
	//	auto metallic = material->GetMetallicMultiplier();
	//	auto normal = material->GetNormalMultiplier();
	//	auto height = material->GetHeightMultiplier();
	//	auto tiling = material->GetTiling();
	//	auto offset = material->GetOffset();
	//	auto colorAlbedo = material->GetColorAlbedo();

	//	const auto albedoTexture = material->GetTexture(TextureType::Albedo);
	//	const auto roughnessTexture = material->GetTexture(TextureType::Roughness);
	//	const auto metallicTexture = material->GetTexture(TextureType::Metallic);
	//	const auto normalTexture = material->GetTexture(TextureType::Normal);
	//	const auto heightTexture = material->GetTexture(TextureType::Height);
	//	const auto occlusionTexture = material->GetTexture(TextureType::Occlusion);
	//	const auto emissiveTexture = material->GetTexture(TextureType::Emissive);
	//	const auto maskTexture = material->GetTexture(TextureType::Mask);

	//	const auto ShowTextureSlot = [&material](Texture* texture, const char* name, const TextureType& type)
	//	{
	//		ImGui::Text(name);
	//		ImGui::SameLine(70.0f);
	//		ImGui::Image
	//		(
	//			texture ? texture->GetShaderResourceView() : nullptr,
	//			ImVec2(80, 80),
	//			ImVec2(0, 0),
	//			ImVec2(1, 1),
	//			ImVec4(1, 1, 1, 1),
	//			ImColor(255, 255, 255, 128)
	//		);

	//		//DragDrop
	//		auto data = DragDrop::GetDragDrop(DragDropPayloadType::Texture);
	//		if (data != "")
	//			material->SetTexture(type, data);
	//	};

	//	//Albedo
	//	ShowTextureSlot(albedoTexture, "Albedo", TextureType::Albedo);
	//	ImGui::Text("Color");
	//	ImGui::SameLine(70.0f);
	//	ImGui::ColorEdit4("##ColorAlbedo", colorAlbedo);

	//	ImGui::Separator();

	//	//Roughness
	//	ShowTextureSlot(roughnessTexture, "Roughness", TextureType::Roughness);
	//	ImGui::Text("Factor");
	//	ImGui::SameLine(70.0f);
	//	ImGui::SliderFloat("##Roughness", &roughness, 0.0f, 1.0f);

	//	ImGui::Separator();

	//	//Metallic
	//	ShowTextureSlot(metallicTexture, "Metallic", TextureType::Metallic);
	//	ImGui::Text("Factor");
	//	ImGui::SameLine(70.0f);
	//	ImGui::SliderFloat("##Metallic", &metallic, 0.0f, 1.0f);

	//	ImGui::Separator();

	//	//Normal
	//	ShowTextureSlot(normalTexture, "Normal", TextureType::Normal);
	//	ImGui::Text("Factor");
	//	ImGui::SameLine(70.0f);
	//	ImGui::SliderFloat("##Normal", &normal, 0.0f, 1.0f);

	//	ImGui::Separator();

	//	//Height
	//	ShowTextureSlot(heightTexture, "Height", TextureType::Height);
	//	ImGui::Text("Factor");
	//	ImGui::SameLine(70.0f);
	//	ImGui::SliderFloat("##Height", &height, 0.0f, 1.0f);

	//	ImGui::Separator();

	//	//Occlusion
	//	ShowTextureSlot(occlusionTexture, "Occlusion", TextureType::Occlusion);

	//	ImGui::Separator();

	//	//Emissive
	//	ShowTextureSlot(emissiveTexture, "Emissive", TextureType::Emissive);

	//	ImGui::Separator();

	//	//Mask
	//	ShowTextureSlot(maskTexture, "Mask", TextureType::Mask);

	//	ImGui::Separator();

	//	//Tiling
	//	ImGui::PushItemWidth(100.0f);
	//	ImGui::TextUnformatted("Tilling");
	//	ImGui::SameLine(70.0f);
	//	ImGui::TextUnformatted("X");
	//	ImGui::SameLine();
	//	ImGui::InputFloat("##TillingX", &tiling.x, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	//	ImGui::SameLine();
	//	ImGui::TextUnformatted("Y");
	//	ImGui::SameLine();
	//	ImGui::InputFloat("##TillingY", &tiling.y, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	//	ImGui::PopItemWidth();

	//	//Offset
	//	ImGui::PushItemWidth(100.0f);
	//	ImGui::TextUnformatted("Offset");
	//	ImGui::SameLine(70.0f);
	//	ImGui::TextUnformatted("X");
	//	ImGui::SameLine();
	//	ImGui::InputFloat("##OffsetX", &offset.x, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	//	ImGui::SameLine();
	//	ImGui::TextUnformatted("Y");
	//	ImGui::SameLine();
	//	ImGui::InputFloat("##OffsetY", &offset.y, 0.0f, 0.0f, "%.3f", ImGuiInputTextFlags_CharsDecimal);
	//	ImGui::PopItemWidth();

	//	if (roughness != material->GetRoughnessMultiplier())    material->SetRoughnessMultiplier(roughness);
	//	if (metallic != material->GetMetallicMultiplier())      material->SetMetallicMultiplier(metallic);
	//	if (normal != material->GetNormalMultiplier())          material->SetNormalMultiplier(normal);
	//	if (height != material->GetHeightMultiplier())          material->SetHeightMultiplier(height);
	//	if (tiling != material->GetTiling())                    material->SetTiling(tiling);
	//	if (offset != material->GetOffset())                    material->SetOffset(offset);
	//	if (colorAlbedo != material->GetColorAlbedo())          material->SetColorAlbedo(colorAlbedo);
	//}
}

void Widget_Editor::ShowTerrainLoadFromFile()
{
	auto terrain = scene->GetActorFromID(terrain_id)->GetComponent<Terrain>();
	auto ocean = scene->GetActorFromID(ocean_id)->GetComponent<Ocean>();
	std::string path;
	FileSystem::OpenFileDialog
	(
		[&](std::string filepath)->void 
		{
			terrain->CreateFromHeightMap(filepath);
			ocean->EditOceanSize(terrain->GetTerrainWidth(), terrain->GetTerrainHeight());			
		},
		FileSystem::TextureFilter,
		"../_Assets/HeightMaps/"
	);

	bLoadTerrain = false;
}



#pragma region TERRAIN_BRUSH

void Widget_Editor::ShowTerrainBrushHeight(Terrain* terrain)
{


	ImGui::NewLine();
	ImGui::Text("BrushType : ");
	// Left region
	{
		float iconSize = 60;
		float childWidth = 70;
		ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));
		ImGui::BeginChild("Brush Texture", ImVec2(childWidth, childWidth), true, ImGuiWindowFlags_NoScrollbar);

		ImGui::Image
		(
			brushes[selected_brush_ID].thumbnail.texture->GetShaderResourceView(),
			ImVec2(iconSize, iconSize)
		);

		ImGui::EndChild();
		ImGui::PopStyleColor(1);
	}
	ImGui::SameLine();

	// Right region
	{
		float iconSize = 40;
		float childWidth = ImGui::GetWindowWidth() - 100;
		float childHeight = 100;
		int columns = static_cast<int>(childWidth / (iconSize + 10));
		columns = columns < 1 ? 1 : columns;

		ImGui::BeginChild("Choose Texture", ImVec2(childWidth, childHeight), true, ImGuiWindowFlags_HorizontalScrollbar);

		ImGui::Columns(columns, nullptr, false);

		for (uint i = 0; i < brushes.size(); i++)
		{
			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));


			if (ImGui::ImageButton(brushes[i].thumbnail.texture->GetShaderResourceView(),ImVec2(iconSize, iconSize)))
			{
				terrain->SetBrushTexture(brushes[i].thumbnail.texture.get());
				selected_brush_ID = i;
			}
			ImGui::PopStyleColor(1);

			ImGui::NextColumn();
		}
		ImGui::EndChild();
	}

	float dragSpeed = 0.01f;
	ImGui::NewLine();
	ImGui::Text("Brush width : ");
	// edit Brush Width
	{
		float val = terrain->GetBrushRange();
		float min = 5.0f; float max = 30.0f;
		ImGui::DragFloat("##range", &val, dragSpeed, min, max);
		terrain->SetBrushRange(val);
	}

	ImGui::NewLine();
	ImGui::Text("Brush Intensity : ");
	// edit Brush Intensity
	{
		float val = terrain->GetBrushIntensity();
		val *= 100;
		float min = -100.0f; float max = 100.0f;
		ImGui::DragFloat("##intensity", &val, dragSpeed, min, max);
		val *= 0.01f;
		terrain->SetBrushIntensity(val);
	}

}

void Widget_Editor::ShowTerrainBrushFlat(Terrain* terrain)
{


}

void Widget_Editor::ShowTerrainBrushTexture(Terrain* terrain)
{
	auto Image = [this](const Item& item, float width, float height)->void
	{
		ImGui::Image(item.thumbnail.texture->GetShaderResourceView(), ImVec2(width, height));
	};

	auto ImageButton = [this](const Item& item, float width, float height) -> bool
	{
		return ImGui::ImageButton(item.thumbnail.texture->GetShaderResourceView(), ImVec2(width, height));
	};

	//Show & Choose Brush Shape
	{
		ImGui::NewLine();
		ImGui::Text("Brush Shape : ");
		 //Left region
		{
			float iconSize = 60;
			float childWidth = 70;
			ImGuiWindowFlags flag = ImGuiWindowFlags_NoScrollbar;
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));
			ImGui::BeginChild("Brush Texture", ImVec2(childWidth, childWidth), true, flag);

			Image(brushes[selected_brush_ID], childWidth, childWidth);

			ImGui::EndChild();
			ImGui::PopStyleColor();
		}
		ImGui::SameLine();


		 //Right region
		{
			float iconSize = 40;
			float childWidth = ImGui::GetWindowWidth() - 100;
			float childHeight = 100;
			int columns = static_cast<int>(childWidth / (iconSize + 10));
			ImGuiWindowFlags flag = ImGuiWindowFlags_HorizontalScrollbar;
			columns = columns < 1 ? 1 : columns;					
			ImGui::BeginChild("Choose Texture", ImVec2(childWidth, childHeight), true, ImGuiWindowFlags_HorizontalScrollbar);						
			ImGui::Columns(columns, nullptr, false);

			for (uint i = 0; i < brushes.size(); i++)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				if(ImageButton(brushes[i],iconSize,iconSize))
				{
					terrain->SetBrushTexture(brushes[i].thumbnail.texture.get());
					selected_brush_ID = i;
				}

				ImGui::PopStyleColor(1);
				ImGui::NextColumn();
			}
			ImGui::EndChild();
		}

	}

	
	//Show & Choose Painter
	bool smallButtonSelected = false;
	{
		ImGui::NewLine();
		ImGui::Text("BrushTexture : ");
		 //Left region
		{
			float iconSize = 60;
			float childWidth = 70;
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));
			ImGui::BeginChild("Brush Texture1", ImVec2(childWidth, childWidth), true, ImGuiWindowFlags_NoScrollbar);

			Image(painter_diffuses[selected_paint_ID], iconSize, iconSize);

			ImGui::EndChild();
			ImGui::PopStyleColor(1);
		}
		ImGui::SameLine();

		 //Right region
		{
			float iconSize = 40;
			float childWidth = ImGui::GetWindowWidth() - 100;
			float childHeight = 100;
			int columns = static_cast<int>(childWidth / (iconSize + 10));
			columns = columns < 1 ? 1 : columns;
			ImGui::BeginChild("Choose Texture1", ImVec2(childWidth, childHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Columns(columns, nullptr, false);

			for (uint i = 0; i < painter_diffuses.size(); i++)
			{
				ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0, 0, 0, 0));
				if(ImageButton(painter_diffuses[i],iconSize,iconSize))
				{
					selected_paint_ID = i;
				}
				ImGui::PopStyleColor(1);
				ImGui::NextColumn();
			}

			ImGui::EndChild();
		}
		ImGui::NewLine();
		if (ImGui::SmallButton("SetBrushTexture"))
		{
			smallButtonSelected = true;
		}
	}


	//Brush Texture Intensity
	{
		float dragSpeed = 0.01f;
		ImGui::NewLine();
		ImGui::Text("Brush width : ");
		// edit Brush Width
		{
			float val = terrain->GetBrushRange();
			float min = 5.0f; float max = 30.0f;
			ImGui::DragFloat("##range", &val, dragSpeed, min, max);
			terrain->SetBrushRange(val);
		}

		ImGui::NewLine();
		ImGui::Text("Brush Intensity : ");
		// edit Brush Intensity
		{
			float val = terrain->GetBrushIntensity();
			val *= 100;
			float min = -100.0f; float max = 100.0f;
			ImGui::DragFloat("##intensity", &val, dragSpeed, min, max);
			val *= 0.01f;
			terrain->SetBrushIntensity(val);
		}
	}
	
	///Select Channel
	{
		auto ShowChannelImages = [&](uint channelNum)
		{
			if (channelNum == 0)
				ImGui::Text("Default");
			else ImGui::Text("Channel%d", channelNum);
			float iconSize = 60;
			float childWidth = 60;
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));
			char name[128];
			sprintf_s(name, "Channel%dDiffuse", channelNum);
			ImGui::BeginChild(name, ImVec2(childWidth, childWidth), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

			Image(painter_diffuses[channels_paint_ID[channelNum]], iconSize, iconSize);

			ImGui::EndChild();
			ImGui::PopStyleColor(1);

			// : Normal
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));
			sprintf_s(name, "Channel%dNormal", channelNum);
			ImGui::BeginChild(name, ImVec2(childWidth, childWidth), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

			Image(painter_normals[channels_paint_ID[channelNum]], iconSize, iconSize);

			ImGui::EndChild();
			ImGui::PopStyleColor(1);

			// : Specular
			/*ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.1f, 0.1f, 0.1f, 0.1f));
			sprintf_s(name, "Channel%dSpecular", channelNum);
			ImGui::BeginChild(name, ImVec2(childWidth, childWidth), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

			Image(painter_speculars[channels_paint_ID[channelNum]], iconSize, iconSize);

			ImGui::EndChild();
			ImGui::PopStyleColor(1);*/
		};


		uint selectedTab = 0;

		ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
		if (ImGui::BeginTabBar("Channels", tab_bar_flags))
		{
			for (uint i = 0; i < 5; i++)
			{
				char label[128];
				if (i == 0) sprintf_s(label, "Default");
				else sprintf_s(label, "%d", i);
				if (ImGui::BeginTabItem(label))
				{
					terrain->SetBrushChannel(static_cast<SplatChannel>(i));
					ShowChannelImages(i);
					selectedTab = i;
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}

		if (smallButtonSelected)
		{
			channels_paint_ID[selectedTab] = selected_paint_ID;
			terrain->SetChannelTexture
			(
				static_cast<SplatChannel>(selectedTab),
				ChannelType::Diffuse,
				painter_diffuses[selected_paint_ID].thumbnail.texture.get()
			);
			terrain->SetChannelTexture
			(
				static_cast<SplatChannel>(selectedTab),
				ChannelType::Normal,
				painter_normals[selected_paint_ID].thumbnail.texture.get()
			);
		}
	}
}



#pragma endregion



#pragma endregion

#pragma region LIGHT_EDITOR

void Widget_Editor::ShowLightEditor()
{
	auto light = scene->GetActorFromID(light_id)->GetComponent<Light>();
	auto transform = light->GetTransform();
	if (!transform)
		return;

	auto position = transform->GetTranslation();
	auto rotation = transform->GetEulerAngle();
	auto scale = transform->GetScale();

	auto ShowFloat = [](const char* label, float* value)
	{
		float step = 1.0f;
		float step_fast = 1.0f;
		char* format = const_cast<char*>("%.3f");
		auto flags = ImGuiInputTextFlags_CharsDecimal;

		ImGui::PushItemWidth(100.0f);
		ImGui::InputFloat(label, value, step, step_fast, format, flags);
		ImGui::PopItemWidth();
	};

	if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
	{
		ImGui::Text("Position");
		ImGui::SameLine(70.0f); ShowFloat("##PositionX", &position.x);
		ImGui::SameLine();      ShowFloat("##PositionY", &position.y);
		ImGui::SameLine();      ShowFloat("##PositionZ", &position.z);

		ImGui::Text("Rotation");
		ImGui::SameLine(70.0f); ShowFloat("##RotationX", &rotation.x);
		ImGui::SameLine();      ShowFloat("##RotationY", &rotation.y);
		ImGui::SameLine();      ShowFloat("##RotationZ", &rotation.z);

		ImGui::Text("Scale");
		ImGui::SameLine(70.0f); ShowFloat("##ScaleX", &scale.x);
		ImGui::SameLine();      ShowFloat("##ScaleY", &scale.y);
		ImGui::SameLine();      ShowFloat("##ScaleZ", &scale.z);
	}

	transform->SetTranslation(position);
	transform->SetRotationFromEulerAngle(rotation);
	transform->SetScale(scale);

	if (!light)
		return;

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
	{
		static std::vector<std::string> light_types{ "Directional", "Point", "Spot" };
		std::string light_type = light_types[static_cast<uint>(light->GetLightType())];
		auto intensity = light->GetIntensity();
		auto angle = Math::ToDegree(light->GetAngle());
		auto cast_shadow = light->IsCastShadow();
		auto bias = light->GetBias();
		auto normal_bias = light->GetNormalBias();
		auto range = light->GetRange();
		auto color = light->GetColor();

		//Type
		ImGui::TextUnformatted("Type");
		ImGui::PushItemWidth(110.0f);
		ImGui::SameLine(70.0f);
		if (ImGui::BeginCombo("##LightType", light_type.c_str()))
		{
			for (uint i = 0; i < light_types.size(); i++)
			{
				const auto bSelected = (light_type == light_types[i]);
				if (ImGui::Selectable(light_types[i].c_str(), bSelected))
				{
					light_type = light_types[i];
					light->SetLightType(static_cast<LightType>(i));
				}

				if (bSelected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		ImGui::PopItemWidth();

		//Color
		ImGui::TextUnformatted("Color");
		ImGui::SameLine(70.0f);
		ImGui::ColorEdit4("##LightColor", color);

		//Intensity
		ImGui::Text("Intensity");
		ImGui::SameLine(70.0f);
		ImGui::PushItemWidth(300.0f);
		ImGui::DragFloat("##LightIntensity", &intensity, 0.01f, 0.0f, 100.0f);
		ImGui::PopItemWidth();

		//Cast Shadow
		ImGui::TextUnformatted("Shadow");
		ImGui::SameLine(70.0f);
		ImGui::Checkbox("##LightShadow", &cast_shadow);

		//Cascade
		if (light->GetLightType() == LightType::Directional)
		{
			//Bias
			ImGui::TextUnformatted("Bias");
			ImGui::SameLine(70.0f);
			ImGui::PushItemWidth(300.0f);
			ImGui::InputFloat("##LightBias", &bias, 0.0001f, 0.0001f, "%.4f");
			ImGui::PopItemWidth();

			//Normal Bias
			ImGui::TextUnformatted("Normal Bias");
			ImGui::SameLine(70.0f);
			ImGui::PushItemWidth(300.0f);
			ImGui::InputFloat("##LightNormalBias", &normal_bias, 1.0f, 1.0f, "%.0f");
			ImGui::PopItemWidth();
		}

		//Angle
		if (light->GetLightType() == LightType::Spot)
		{
			ImGui::TextUnformatted("Angle");
			ImGui::SameLine(70.0f);
			ImGui::PushItemWidth(300.0f);
			ImGui::DragFloat("##LightAngle", &angle, 0.01f, 1.0f, 179.0f);
			ImGui::PopItemWidth();
		}

		//Range
		if (light->GetLightType() != LightType::Directional)
		{
			ImGui::TextUnformatted("Range");
			ImGui::SameLine(70.0f);
			ImGui::PushItemWidth(300.0f);
			ImGui::DragFloat("##LightRange", &range, 0.01f, 0.0f, 100.0f);
			ImGui::PopItemWidth();
		}

		if (intensity != light->GetIntensity())         light->SetIntensity(intensity);
		if (cast_shadow != light->IsCastShadow())       light->SetIsCastShadow(cast_shadow);
		if (Math::ToRadian(angle) != light->GetAngle()) light->SetAngle(Math::ToRadian(angle));
		if (range != light->GetRange())                 light->SetRange(range);
		if (bias != light->GetBias())                   light->SetBias(bias);
		if (normal_bias != light->GetNormalBias())      light->SetNormalBias(normal_bias);
		if (color != light->GetColor())                 light->SetColor(color);
	}
}

#pragma endregion


#pragma region OCEAN_EDITOR
void Widget_Editor::ShowOceanEditor()
{
	auto ocean = scene->GetActorFromID(ocean_id)->GetComponent<Ocean>();


	auto material = ocean->GetMaterial();

	float dragSpeed = 0.01f;
	ImGui::NewLine();
	ImGui::Text("Ocean Height : ");
	// edit Brush Width
	{
		float val = ocean->GetOceanHeight();
		float min = 0.0f; float max = 100.0f;
		ImGui::DragFloat("##oceanHeight", &val, dragSpeed, min, max);
		ocean->SetOceanHeight(val);
	}

	ImGui::NewLine();
	ImGui::Text("Ocean Roughness : ");
	// edit Brush Width
	{
		float val = material->GetRoughnessMultiplier();
		float min = 0.0f; float max = 100.0f;
		ImGui::DragFloat("##roughness", &val, dragSpeed, min, max);
		material->SetRoughnessMultiplier(val);
	}

	D3DXCOLOR albedo_color = material->GetColorAlbedo();
	ImGui::ColorEdit4("color 2", reinterpret_cast<float*>(&albedo_color));
	material->SetColorAlbedo(albedo_color);
}
#pragma endregion