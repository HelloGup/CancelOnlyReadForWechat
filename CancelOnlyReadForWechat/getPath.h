#pragma once

#include <string>
#include <atlstr.h>
#include <shlobj_core.h>

#define Documents_PATH 1 
#define Desktop_PATH 2
#define Startup_PATH 3
#define Downloads_PATH 4
#define Favorites_PATH 5
#define LocalAppData_PATH 6
#define Pictures_PATH 7

std::string getWinPath(int option) {
	CString str_Path;
	WCHAR* strPath = nullptr;
	KNOWNFOLDERID rfid = FOLDERID_Documents;

	//FOLDERID_Documents:文档全路径
	//FOLDERID_Desktop:桌面全路径
	//FOLDERID_Startup:程序开机启动的文件夹，把程序放在这个文件夹，就可以开机自启
	//FOLDERID_Downloads:下载全路径
	//FOLDERID_Favorites:收藏夹全路径
	//FOLDERID_LocalAppData:appdata\\local全路径
	//FOLDERID_Pictures:图片全路径
	switch (option) {
	case Documents_PATH:
		rfid = FOLDERID_Documents;
		break;
	case Desktop_PATH:
		rfid = FOLDERID_Desktop;
		break;
	case Startup_PATH:
		rfid = FOLDERID_Startup;
		break;
	case Downloads_PATH:
		rfid = FOLDERID_Downloads;
		break;
	case Favorites_PATH:
		rfid = FOLDERID_Favorites;
		break;
	case LocalAppData_PATH:
		rfid = FOLDERID_LocalAppData;
		break;
	case Pictures_PATH:
		rfid = FOLDERID_Pictures;
		break;
	}

	//返回的路径没有末尾的"\"
	if (SUCCEEDED(::SHGetKnownFolderPath(rfid, 0, NULL, &strPath))) {
		str_Path = strPath;
		CoTaskMemFree(strPath);
	}

	std::string ret = CT2A(str_Path.GetBuffer());
 
	return ret;
}