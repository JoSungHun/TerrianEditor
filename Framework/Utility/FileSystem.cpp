#include "Framework.h"
#include "FileSystem.h"
#include <filesystem>

using namespace std::filesystem;

//find() - 주어진 문자열이 존재하는 위치 -> 정방향
//rfind() - 주어진 문자열이 존재하는 위치 -> 역방향
//find_first_of() - 주어진 문자중에 하나라도 걸리는 첫번째 위치
//find_last_of() - 주어진 문자중에 하나라도 걸리는 마지막 위치
//find_first_not_of() - 주어진 문자가 아닌 문자가 걸리는 첫번째 위치
//find_last_not_of() - 주어진 문자가 아닌 문자가 걸릴는 마지막 위치

const char* FileSystem::EveryFilter = "Every File(*.*)\0*.*;\0";
const char* FileSystem::TextureFilter = "Texture File(*.png, *.bmp)\0*.png;*.bmp;\0";
const char* FileSystem::ShaderFilter = "Shader File(*.hlsl)\0*.hlsl;\0";

std::vector<std::string> FileSystem::supportImageFormat =
{
	".jpg",
	".png",
	".bmp",
	".tga",
	".dds",
	".exr",
	".raw",
	".gif",
	".hdr",
	".ico",
	".iff",
	".jng",
	".jpeg",
	".koala",
	".kodak",
	".mng",
	".pcx",
	".pbm",
	".pgm",
	".ppm",
	".pfm",
	".pict",
	".psd",
	".raw",
	".sgi",
	".targa",
	".tiff",
	".tif",
	".wbmp",
	".webp",
	".xbm",
	".xpm"
};

std::vector<std::string> FileSystem::supportAudioFormat =
{
	".aiff",
	".asf",
	".asx",
	".dls",
	".flac",
	".fsb",
	".it",
	".m3u",
	".midi",
	".mod",
	".mp2",
	".mp3",
	".ogg",
	".pls",
	".s3m",
	".vag", // PS2/PSP
	".wav",
	".wax",
	".wma",
	".xm",
	".xma" // XBOX 360
};

std::vector<std::string> FileSystem::supportModelFormat =
{
	".3ds",
	".obj",
	".fbx",
	".blend",
	".dae",
	".lwo",
	".c4d",
	".ase",
	".dxf",
	".hmp",
	".md2",
	".md3",
	".md5",
	".mdc",
	".mdl",
	".nff",
	".ply",
	".stl",
	".x",
	".smd",
	".lxo",
	".lws",
	".ter",
	".ac3d",
	".ms3d",
	".cob",
	".q3bsp",
	".xgl",
	".csm",
	".bvh",
	".b3d",
	".ndo"
};

void FileSystem::OpenFileDialog(std::function<void(std::string)>&& func, const char * filter, const char * directory)
{
	char buffer[255];
	ZeroMemory(buffer, 255);

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = Settings::Get().GetWindowHandle();
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = 255;
	ofn.lpstrInitialDir = directory;
	ofn.Flags = OFN_NOCHANGEDIR;

	if (GetOpenFileNameA(&ofn) == TRUE)
	{
		std::string path = GetRelativeFromPath(buffer);

		auto function = std::forward<std::function<void(std::string)>>(func);

		if (function != nullptr)
			function(path);
	}
}

void FileSystem::SaveFileDialog(std::function<void(std::string)>&& func, const char * filter, const char * directory)
{
	char buffer[255];
	ZeroMemory(buffer, 255);

	OPENFILENAMEA ofn;
	ZeroMemory(&ofn, sizeof(OPENFILENAMEA));
	ofn.lStructSize = sizeof(OPENFILENAMEA);
	ofn.hwndOwner = Settings::Get().GetWindowHandle();
	ofn.lpstrFilter = filter;
	ofn.lpstrFile = buffer;
	ofn.nMaxFile = 255;
	ofn.lpstrInitialDir = directory;
	ofn.Flags = OFN_NOCHANGEDIR;

	if (GetSaveFileNameA(&ofn) == TRUE)
	{
		std::string path = GetRelativeFromPath(buffer);

		auto function = std::forward<std::function<void(std::string)>>(func);

		if (function != nullptr)
			function(path);
	}
}

const bool FileSystem::Create_Directory(const std::string & path)
{
	bool bResult = false;

	try
	{
		bResult = create_directories(path);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return bResult;
}

const bool FileSystem::Delete_Directory(const std::string & directory)
{
	bool bResult = false;

	try
	{
		bResult = remove_all(directory) > 0;
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return bResult;
}

const bool FileSystem::Delete_File(const std::string & path)
{
	if (IsDirectory(path))
		return false;

	bool bResult = false;

	try
	{
		bResult = remove(path);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return bResult;
}

const bool FileSystem::Copy_File(const std::string & src, const std::string & dst)
{
	if (src == dst)
		return false;

	if (!ExistDirectory(GetDirectoryFromPath(dst)))
		Create_Directory(GetDirectoryFromPath(dst));

	bool bResult = false;

	try
	{
		bResult = copy_file(src, dst, copy_options::overwrite_existing);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return bResult;
}

const bool FileSystem::IsDirectory(const std::string & path)
{
	bool bResult = false;

	try
	{
		bResult = is_directory(path);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return bResult;
}

const bool FileSystem::ExistDirectory(const std::string & directory)
{
	bool bResult = false;

	try
	{
		bResult = exists(directory);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return bResult;
}

const bool FileSystem::ExistFile(const std::string & path)
{
	bool bResult = false;

	try
	{
		bResult = exists(path);
	}
	catch (filesystem_error error)
	{
		MessageBoxA(nullptr, error.what(), "ERROR!!!", MB_OK);
	}

	return bResult;
}

const std::string FileSystem::GetFileNameFromPath(const std::string & path)
{
	//../_Assets/Texture/Tree.png -----------------> Tree.png
	auto lastIndex = path.find_last_of("\\/");
	auto fileName = path.substr(lastIndex + 1, path.length());
	
	return fileName;
}

const std::string FileSystem::GetIntactFileNameFromPath(const std::string & path)
{
	//../_Assets/Texture/Tree.png -----------------> Tree
	auto fileName = GetFileNameFromPath(path);
	auto lastIndex = fileName.find_last_of('.');
	auto intactFileName = fileName.substr(0, lastIndex);

	return intactFileName;
}

const std::string FileSystem::GetDirectoryFromPath(const std::string & path)
{
	//../_Assets/Texture/Tree.png -----------------> ../_Assets/Texture/
	auto lastIndex = path.find_last_of("\\/");
	auto directory = path.substr(0, lastIndex + 1);

	return directory;
}

const std::string FileSystem::GetExtensionFromPath(const std::string & path)
{
	//../_Assets/Texture/Tree.png -----------------> .png
	auto lastIndex = path.find_last_of('.');

	if (lastIndex != std::string::npos)
		return path.substr(lastIndex, path.length());

	return std::string();
}

const std::string FileSystem::GetPathWithoutExtension(const std::string & path)
{
	auto directory = GetDirectoryFromPath(path);
	auto fileName = GetIntactFileNameFromPath(path);

	return directory + fileName;
}

const std::string FileSystem::GetRelativeFromPath(const std::string & absolutePath)
{
	//절대 경로를 만듬
	path p = absolute(absolutePath);
	path r = absolute(GetWorkingDirectory());

	//루트 경로가 다를경우 절대경로 반환
	if (p.root_path() != r.root_path())
		return p.generic_string();

	path result;

	//두 경로가 갈라지는 지점을 체크
	path::const_iterator iter_path = p.begin();
	path::const_iterator iter_relative = r.begin();

	while (
		*iter_path == *iter_relative &&
		iter_path != p.end() &&
		iter_relative != r.end())
	{
		iter_path++;
		iter_relative++;
	}

	//relative에 남은 각 토큰에대해 ..을 추가
	if (iter_relative != r.end())
	{
		iter_relative++;
		while (iter_relative != r.end())
		{
			result /= "..";
			iter_relative++;
		}
	}

	//남은 경로 추가
	while (iter_path != p.end())
	{
		result /= *iter_path;
		iter_path++;
	}

	return result.generic_string();
}

const std::string FileSystem::GetWorkingDirectory()
{
	return current_path().generic_string() + "/";
}

const std::string FileSystem::GetParentDirectory(const std::string & directory)
{
	auto found = directory.find_last_of("\\/");
	auto result = directory;

	if (found == std::string::npos)
		return directory;

	if (found == directory.length() - 1)
	{
		result = result.substr(0, found - 1);
		return GetParentDirectory(result);
	}

	return result.substr(0, found) + "/";
}

auto FileSystem::GetDirectoriesInDirectory(const std::string & directory) -> const std::vector<std::string>
{
	std::vector<std::string> subDirectories;

	directory_iterator endIter;
	for (directory_iterator iter(directory); iter != endIter; iter++)
	{
		if (!is_directory(iter->status()))
			continue;

		subDirectories.emplace_back(iter->path().generic_string());
	}
	return subDirectories;
}

auto FileSystem::GetFilesInDirectory(const std::string & directory) -> const std::vector<std::string>
{
	std::vector<std::string> files;

	directory_iterator endIter;
	for (directory_iterator iter(directory); iter != endIter; iter++)
	{
		if (!is_regular_file(iter->status()))
			continue;

		files.emplace_back(iter->path().generic_string());
	}
	return files;
}

auto FileSystem::IsSupportedImageFile(const std::string & path) -> const bool
{
	std::string fileExtension = GetExtensionFromPath(path);

	auto supportFormats = GetSupportImageFormats();
	for (const auto& format : supportFormats) //Range based for loop
	{
		if (fileExtension == format || fileExtension == ToUpper(format))
			return true;
	}
	return false;
}

auto FileSystem::IsSupportedAudioFile(const std::string & path) -> const bool
{
	std::string fileExtension = GetExtensionFromPath(path);

	auto supportFormats = GetSupportAudioFormats();
	for (const auto& format : supportFormats)
	{
		if (fileExtension == format || fileExtension == ToUpper(format))
			return true;
	}
	return false;
}

auto FileSystem::IsSupportedModelFile(const std::string & path) -> const bool
{
	std::string fileExtension = GetExtensionFromPath(path);

	auto supportFormats = GetSupportModelFormats();
	for (const auto& format : supportFormats)
	{
		if (fileExtension == format || fileExtension == ToUpper(format))
			return true;
	}
	return false;
}

const std::string FileSystem::ToUpper(const std::string & lower)
{
	std::string upper;
	for (const auto& character : lower)
		upper += toupper(character);

	return upper;
}

const std::string FileSystem::ToLower(const std::string & upper)
{
	std::string lower;
	for (const auto& character : upper)
		lower += tolower(character);

	return lower;
}

const std::string FileSystem::ToString(const std::wstring & str)
{
	//과제
	//std::string result;
	//result.assign(str.begin(), str.end());

	return std::string();
}

const std::wstring FileSystem::ToWstring(const std::string & str)
{
	std::wstring result;
	result.assign(str.begin(), str.end());

	return result;
}