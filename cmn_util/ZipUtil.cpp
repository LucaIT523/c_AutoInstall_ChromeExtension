
#include "ZipUtil.h"
#include "zip_file.hpp"


std::string ConvertWStringToString(const std::wstring& wstr) {
	std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> converter;
	return converter.to_bytes(wstr);
}

void ListFilesAndFolders(const std::wstring& directory, std::vector<std::wstring>& files, std::vector<std::wstring>& folders) {
	WIN32_FIND_DATA findFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	std::wstring searchPath = directory + L"\\*";  // Add wildcard to the directory path

	hFind = FindFirstFile(searchPath.c_str(), &findFileData);

	if (hFind == INVALID_HANDLE_VALUE) {
		std::cerr << "FindFirstFile failed (" << GetLastError() << ")\n";
		return;
	}

	do {
		const std::wstring fileOrFolderName = findFileData.cFileName;

		if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			// Skip "." and ".." directories
			if (fileOrFolderName != L"." && fileOrFolderName != L"..") {
				std::wstring fullFolderPath = directory + L"\\" + fileOrFolderName;
				folders.push_back(fullFolderPath);
				// Recursively list files and folders in the subdirectory
				ListFilesAndFolders(fullFolderPath, files, folders);
			}
		}
		else {
			std::wstring fullFilePath = directory + L"\\" + fileOrFolderName;
			files.push_back(fullFilePath);
		}
	} while (FindNextFile(hFind, &findFileData) != 0);

	FindClose(hFind);
}
// Function to extract the last folder name from a given path
std::wstring GetLastFolderName(const std::wstring& path) {
	// Find the last backslash in the path
	size_t lastBackslashPos = path.find_last_of(L"\\/");
	if (lastBackslashPos == std::wstring::npos) {
		// No backslash found, the entire path is the folder name
		return path;
	}
	// Extract the substring after the last backslash
	return path.substr(lastBackslashPos + 1);
}


void	MyZipCRXData(std::wstring	p_CRXDataPath, std::wstring	p_CRXZipFilePath)
{
	int				folderAttr = 0x41ed << 16;
	int				normalAttr = 0x81a4 << 16;

	std::wstring	directory = p_CRXDataPath;
	std::vector<std::wstring> files;
	std::vector<std::wstring> folders;
	miniz_cpp::zip_file newZip;

	std::wstring lastFolderName = GetLastFolderName(directory);
	ListFilesAndFolders(directory, files, folders);
	newZip.write(ConvertWStringToString(directory), "ext/", folderAttr);

	//. Folder
	for (const auto& folder : folders) {
		std::wstring	w_subFolderName = L"ext/";
		std::wstring	w_spliteStr = folder.substr(directory.length() + 1, folder.length());
		w_subFolderName += w_spliteStr + L"/";
		newZip.write(ConvertWStringToString(folder + L"/"), ConvertWStringToString(w_subFolderName), folderAttr);
	}
	//. File	
	for (const auto& file : files) {
		std::wstring	w_subFileName = L"ext/";
		std::wstring	w_spliteStr = file.substr(directory.length() + 1, file.length());
		w_subFileName += w_spliteStr;
		newZip.write(ConvertWStringToString(file), ConvertWStringToString(w_subFileName), normalAttr);
	}

	newZip.save(ConvertWStringToString(p_CRXZipFilePath));
}
void createDirectories(const std::string& path) {
	std::istringstream pathStream(path);
	std::string segment;
	std::string currentPath;

	// Split the path using '/' and create folders recursively
	while (std::getline(pathStream, segment, '/')) {
		if (!segment.empty()) {
			currentPath += segment + "/"; // Append the current segment to the path

			// Create directory if it doesn't exist
			if (CreateDirectoryA(currentPath.c_str(), NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
//				std::cout << "Directory created or already exists: " << currentPath << std::endl;
			}
			else {
//				std::cerr << "Failed to create directory: " << currentPath << std::endl;
				return;
			}
		}
	}
}
void MyUnzipFile(const std::string& zipPath, const std::string& extractPath) {

	int normalAttr = 0x81a4 << 16;
	int std_normalAttr = 0x20;
	int oth_normalAttr = 0x01a4 << 16;
	std::string UnzipPath = extractPath;

	miniz_cpp::zip_file zipFile(zipPath);
	//zipFile.printdir();
	std::vector<miniz_cpp::zip_info> zipInfos = zipFile.infolist();
	for (auto zi : zipInfos) {

		if (!(zi.filename.empty()) && zi.filename.back() == '/') {
			std::string w_Dir = UnzipPath + zi.filename;
			CreateDirectoryA(w_Dir.c_str(), NULL);
			continue;
		}
		else {
			if (zi.filename.find('/') != -1) {
				std::string directoryPath = zi.filename.substr(0, zi.filename.find_last_of('/'));
				if (directoryPath.size() > 0) {
					createDirectories(UnzipPath + directoryPath);
				}
			}

			zipFile.extractExternal(zi, UnzipPath + zi.filename);

		}

		//if (zi.external_attr == normalAttr || zi.external_attr == std_normalAttr || zi.external_attr == oth_normalAttr) {
		//	if (zi.filename.find('/') != -1) {
		//		std::string directoryPath = zi.filename.substr(0, zi.filename.find_last_of('/'));
		//		if (directoryPath.size() > 0) {
		//			createDirectories(UnzipPath + directoryPath);
		//		}
		//	}

		//	zipFile.extractExternal(zi, UnzipPath + zi.filename);
		//}
		//else {
		//	std::string w_Dir = UnzipPath + zi.filename;
		//	CreateDirectoryA(w_Dir.c_str(), NULL);
		//	continue;

		//}
	}
}

void Test_MyUnzipFile(const std::string& zipPath, const std::string& extractPath) {

	int normalAttr = 0x20;
	std::string UnzipPath = extractPath;

	miniz_cpp::zip_file zipFile(zipPath);
	//zipFile.printdir();
	std::vector<miniz_cpp::zip_info> zipInfos = zipFile.infolist();
	for (auto zi : zipInfos) {
		if (zi.external_attr == normalAttr) {
			zipFile.extractExternal(zi, UnzipPath + zi.filename);
		}
		else {
			std::string w_Dir = UnzipPath + zi.filename;
			CreateDirectoryA(w_Dir.c_str(), NULL);
			continue;

		}
	}
}