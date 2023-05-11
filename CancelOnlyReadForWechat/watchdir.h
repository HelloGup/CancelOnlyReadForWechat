#pragma once
#include<iostream>
#include<string>
#include<Windows.h>
#include<list>
#include<locale>

//用来保存返回结果的文件信息
struct Ch_file
{
	std::wstring name; //文件名
	DWORD action;//发生的更改类型

	Ch_file(const wchar_t* fileName, DWORD act) {
		name = fileName;
		action = act;
	}
};


class WatchFolder {
	HANDLE watch_file; //被监视的文件句柄
	char* file_buffer; //保存文件信息更改的缓存区

	//是否是自行设置属性时导致的文件更改通知
	bool own_set_flag = false;

public:

	void setflag() {
		own_set_flag = !own_set_flag;
	}

	WatchFolder(int bufSize = 4096) :watch_file(nullptr), file_buffer(new char[bufSize]) {}

	~WatchFolder()
	{
		if (watch_file) {
			CloseHandle(watch_file);
			watch_file = nullptr;
		}
		delete[] file_buffer;
	}



	//打开监视的文件
	bool Init(const std::string& path) {
		if (watch_file != nullptr) {
			CloseHandle(watch_file);
			watch_file = nullptr;
		}

		//string 转 wchar_t*
		const char* pCStrKey = path.c_str();
		//第一次调用返回转换后的字符串长度，用于确认为wchar_t*开辟多大的内存空间
		int pSize = MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, (int)strlen(pCStrKey) + 1, NULL, 0);
		//使用完要释放
		wchar_t* pWCStrKey = new wchar_t[pSize];
		//第二次调用将单字节字符串转换成双字节字符串
		MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, (int)strlen(pCStrKey) + 1, pWCStrKey, pSize);
		

		// 打开文件
		// FILE_LIST_DIRECTORY：必须使用该访问权限打开此目录 才可被ReadDirectoryChangesW 函数监视
		// FILE_SHARE_READ: 只读方式
		// OPEN_EXISTING:仅当文件或设备存在时，才打开该文件或设备
		// FILE_FLAG_BACKUP_SEMANTICS:必须设置此标志才能获取目录的句柄
		// 最后一个参数：打开已存在文件时， CreateFile 将忽略此参数，可以为NULL
		// 函数调用失败返回值为 INVALID_HANDLE_VALUE，成功返回打开的文件句柄
		watch_file = CreateFileW(pWCStrKey, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS, 0);
		//INVALID_HANDLE_VALUE

		delete[] pWCStrKey;

		return watch_file != nullptr;
	}


	/**
	 开始监视文件夹
	 Ch_file_list 发生改变的文件列表
	 filter 需要被通知的哪些更改行为，若为WatchFolder::NOTIFY_*，则全通知
	*/
	bool start_watch(std::list<Ch_file*>& Ch_file_list, DWORD filter) {

		Ch_file_list.clear(); //清空列表

		//返回的字节数
		DWORD lbyte;


		// 监视文件改变
		// 参数1：是一个打开的 文件夹 句柄
		// 参数2：返回读取到的更改结果缓冲区
		// 参数3：缓冲区大小
		// 参数4：是否监视子目录，true为监视false为不监视
		// 参数5：需要被通知的哪些更改行为
		// 参数6：返回的结果的字节数
		// 参数7：仅异步调用使用，可设为NULL
		// 参数8：不关心
		// 返回值：失败返回0，成功返回非零值
		BOOL ret = ReadDirectoryChangesW(watch_file, file_buffer, 4096, true, filter, &lbyte, NULL, NULL); //阻塞监视
		if (!ret) {
			return false;
		}
		if (!own_set_flag) {
			FILE_NOTIFY_INFORMATION* ret_file = (FILE_NOTIFY_INFORMATION*)file_buffer; //得到FILE_NOTIFY_INFORMATION结构体，进行解析
			//进入循环，解析所有信息
			do {
				//Ch_file* c_file = new Ch_file(ret_file->FileName, ret_file->Action);
				////将有更改的文件放入文件列表中
				//Ch_file_list.push_back(c_file);

				wchar_t* fileName = new wchar_t[ret_file->FileNameLength] {};
				memcpy(fileName, ret_file->FileName, ret_file->FileNameLength);
				Ch_file* file = new Ch_file(fileName, ret_file->Action);
				Ch_file_list.push_back(file);
				delete[] fileName;

				//ret_file更新到下一个文件指针处
				// ret_file中的NextEntryOffset：必须跳过的字节数才能进入下一条记录。值为零表示这是最后一条记录
				ret_file = (FILE_NOTIFY_INFORMATION*)((char*)ret_file + ret_file->NextEntryOffset);
			} while (ret_file->NextEntryOffset);

		}

		own_set_flag = !own_set_flag;
		
		//清空缓冲区 等待下次返回
		memset(file_buffer, 0, lbyte);

		return !Ch_file_list.empty();
	}
};