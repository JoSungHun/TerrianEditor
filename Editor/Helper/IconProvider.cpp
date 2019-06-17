#include "stdafx.h"
#include "IconProvider.h"

void IconProvider::Initialize(Context * context)
{
	this->context = context;

	Load("../_Assets/Icon/folder.png", IconType::Thumbnail_Folder);
	Load("../_Assets/Icon/file.png", IconType::Thumbnail_File);
	Load("../_Assets/Icon/big_play_button.png", IconType::Button_Play_Big);
	Load("../_Assets/Icon/playButton.png", IconType::Button_Play);
	Load("../_Assets/Icon/pauseButton.png", IconType::Button_Pause);
	Load("../_Assets/Icon/stopButton.png", IconType::Button_Stop);
	Load("../_Assets/Icon/option_button.png", IconType::Button_Option);
	Load("../_Assets/Icon/log_info.png", IconType::Log_Info);
	Load("../_Assets/Icon/log_warning.png", IconType::Log_Warning);
	Load("../_Assets/Icon/log_error.png", IconType::Log_Error);
	Load("../_Assets/Icon/terrain_brush_flat.png", IconType::Terrain_Brush_Flat);
	Load("../_Assets/Icon/terrain_brush_height.png", IconType::Terrain_Brush_Height);
	Load("../_Assets/Icon/terrain_brush_paint.png", IconType::Terrain_Brush_Paint);
}

auto IconProvider::GetShaderResourceFromType(const IconType & type) -> ID3D11ShaderResourceView *
{
	return Load("", type)->texture->GetShaderResourceView();
}

auto IconProvider::GetShaderResourceFromPath(const std::string & path) -> ID3D11ShaderResourceView *
{
	return Load(path)->texture->GetShaderResourceView();
}

auto IconProvider::Load(const std::string & path, const IconType & type) -> Thumbnail *
{
	if (type != IconType::Custom)
	{
		for (auto& thumbnail : thumbnails)
		{
			if (thumbnail.type == type)
				return &thumbnail;
		}
	}
	else
	{
		for (auto& thumbnail : thumbnails)
		{
			if (thumbnail.path == path)
				return &thumbnail;
		}
	}

	if (FileSystem::IsDirectory(path))
		return GetThumbnailFromType(IconType::Thumbnail_Folder);
	if (FileSystem::IsSupportedImageFile(path))
	{
		auto texture = std::make_shared<Texture>(context);
		texture->SetIsNeedsMipChain(false);
		texture->SetWidth(100);
		texture->SetHeight(100);

		context->GetSubsystem<Thread>()->AddTask([texture, path]()
		{
			texture->LoadFromFile(path);
		});

		return &thumbnails.emplace_back(type, texture, path);
	}

	return GetThumbnailFromType(IconType::Thumbnail_File);
}

auto IconProvider::ImageButton(const Thumbnail & thumbnail, const ImVec2 & size) -> const bool
{
	bool bPressed = ImGui::ImageButton
	(
		thumbnail.texture->GetShaderResourceView(),
		size
	);
	return bPressed;
}

auto IconProvider::ImageButton(const IconType & type, const float & size) -> const bool
{
	bool bPressed = ImGui::ImageButton
	(
		GetShaderResourceFromType(type),
		ImVec2(size, size)
	);
	return bPressed;
}

auto IconProvider::ImageButton(const IconType & type, const float & width, const float & height) -> const bool
{
	bool bPressed = ImGui::ImageButton
	(
		GetShaderResourceFromType(type),
		ImVec2(width, height)
	);
	return bPressed;
}

auto IconProvider::ImageButton(const std::string & path, const float & size) -> const bool
{
	bool bPressed = ImGui::ImageButton
	(
		GetShaderResourceFromPath(path),
		ImVec2(size, size)
	);
	return bPressed;
}

auto IconProvider::GetThumbnailFromType(const IconType & type) -> Thumbnail *
{
	for (auto& thumbnail : thumbnails)
	{
		if (thumbnail.type == type)
			return &thumbnail;
	}
	return nullptr;
}
