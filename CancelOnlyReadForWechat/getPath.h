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

	//FOLDERID_Documents:�ĵ�ȫ·��
	//FOLDERID_Desktop:����ȫ·��
	//FOLDERID_Startup:���򿪻��������ļ��У��ѳ����������ļ��У��Ϳ��Կ�������
	//FOLDERID_Downloads:����ȫ·��
	//FOLDERID_Favorites:�ղؼ�ȫ·��
	//FOLDERID_LocalAppData:appdata\\localȫ·��
	//FOLDERID_Pictures:ͼƬȫ·��
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

	//���ص�·��û��ĩβ��"\"
	if (SUCCEEDED(::SHGetKnownFolderPath(rfid, 0, NULL, &strPath))) {
		str_Path = strPath;
		CoTaskMemFree(strPath);
	}

	std::string ret = CT2A(str_Path.GetBuffer());
 
	return ret;
}