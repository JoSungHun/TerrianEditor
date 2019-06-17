#pragma once

class FileSystem final
{
public:
	static const char* EveryFilter;
	static const char* TextureFilter;
	static const char* ShaderFilter;

public:
	static void OpenFileDialog
	(
		std::function<void(std::string)>&& func,
		const char* filter = EveryFilter,
		const char* directory = ""
	);

	static void SaveFileDialog
	(
		std::function<void(std::string)>&& func,
		const char* filter = EveryFilter,
		const char* directory = ""
	);

public:
	static const bool Create_Directory(const std::string& path);
	static const bool Delete_Directory(const std::string& directory);
	static const bool Delete_File(const std::string& path);
	static const bool Copy_File(const std::string& src, const std::string& dst);
	static const bool IsDirectory(const std::string& path);
	static const bool ExistDirectory(const std::string& directory);
	static const bool ExistFile(const std::string& path);

	static const std::string GetFileNameFromPath(const std::string& path);
	static const std::string GetIntactFileNameFromPath(const std::string& path);
	static const std::string GetDirectoryFromPath(const std::string& path);
	static const std::string GetExtensionFromPath(const std::string& path);
	static const std::string GetPathWithoutExtension(const std::string& path);
	static const std::string GetRelativeFromPath(const std::string& absolutePath);
	static const std::string GetWorkingDirectory();
	static const std::string GetParentDirectory(const std::string& directory);

	static auto GetDirectoriesInDirectory(const std::string& directory) -> const std::vector<std::string>;
	static auto GetFilesInDirectory(const std::string& directory) -> const std::vector<std::string>;

	static auto GetSupportImageFormats() -> const std::vector<std::string>& { return supportImageFormat; }
	static auto GetSupportAudioFormats() -> const std::vector<std::string>& { return supportAudioFormat; }
	static auto GetSupportModelFormats() -> const std::vector<std::string>& { return supportModelFormat; }

	static auto IsSupportedImageFile(const std::string& path) -> const bool;
	static auto IsSupportedAudioFile(const std::string& path) -> const bool;
	static auto IsSupportedModelFile(const std::string& path) -> const bool;

	static const std::string ToUpper(const std::string& lower);
	static const std::string ToLower(const std::string& upper);
	static const std::string ToString(const std::wstring& str);
	static const std::wstring ToWstring(const std::string& str);

private:
	static std::vector<std::string> supportImageFormat;
	static std::vector<std::string> supportAudioFormat;
	static std::vector<std::string> supportModelFormat;
};