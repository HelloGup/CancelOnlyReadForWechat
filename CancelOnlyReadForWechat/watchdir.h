#pragma once
#include<iostream>
#include<string>
#include<Windows.h>
#include<list>
#include<locale>

//�������淵�ؽ�����ļ���Ϣ
struct Ch_file
{
	std::wstring name; //�ļ���
	DWORD action;//�����ĸ�������

	Ch_file(const wchar_t* fileName, DWORD act) {
		name = fileName;
		action = act;
	}
};


class WatchFolder {
	HANDLE watch_file; //�����ӵ��ļ����
	char* file_buffer; //�����ļ���Ϣ���ĵĻ�����

	//�Ƿ���������������ʱ���µ��ļ�����֪ͨ
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



	//�򿪼��ӵ��ļ�
	bool Init(const std::string& path) {
		if (watch_file != nullptr) {
			CloseHandle(watch_file);
			watch_file = nullptr;
		}

		//string ת wchar_t*
		const char* pCStrKey = path.c_str();
		//��һ�ε��÷���ת������ַ������ȣ�����ȷ��Ϊwchar_t*���ٶ����ڴ�ռ�
		int pSize = MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, (int)strlen(pCStrKey) + 1, NULL, 0);
		//ʹ����Ҫ�ͷ�
		wchar_t* pWCStrKey = new wchar_t[pSize];
		//�ڶ��ε��ý����ֽ��ַ���ת����˫�ֽ��ַ���
		MultiByteToWideChar(CP_OEMCP, 0, pCStrKey, (int)strlen(pCStrKey) + 1, pWCStrKey, pSize);
		

		// ���ļ�
		// FILE_LIST_DIRECTORY������ʹ�ø÷���Ȩ�޴򿪴�Ŀ¼ �ſɱ�ReadDirectoryChangesW ��������
		// FILE_SHARE_READ: ֻ����ʽ
		// OPEN_EXISTING:�����ļ����豸����ʱ���Ŵ򿪸��ļ����豸
		// FILE_FLAG_BACKUP_SEMANTICS:�������ô˱�־���ܻ�ȡĿ¼�ľ��
		// ���һ�����������Ѵ����ļ�ʱ�� CreateFile �����Դ˲���������ΪNULL
		// ��������ʧ�ܷ���ֵΪ INVALID_HANDLE_VALUE���ɹ����ش򿪵��ļ����
		watch_file = CreateFileW(pWCStrKey, FILE_LIST_DIRECTORY, FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, 0, OPEN_EXISTING,
			FILE_FLAG_BACKUP_SEMANTICS, 0);
		//INVALID_HANDLE_VALUE

		delete[] pWCStrKey;

		return watch_file != nullptr;
	}


	/**
	 ��ʼ�����ļ���
	 Ch_file_list �����ı���ļ��б�
	 filter ��Ҫ��֪ͨ����Щ������Ϊ����ΪWatchFolder::NOTIFY_*����ȫ֪ͨ
	*/
	bool start_watch(std::list<Ch_file*>& Ch_file_list, DWORD filter) {

		Ch_file_list.clear(); //����б�

		//���ص��ֽ���
		DWORD lbyte;


		// �����ļ��ı�
		// ����1����һ���򿪵� �ļ��� ���
		// ����2�����ض�ȡ���ĸ��Ľ��������
		// ����3����������С
		// ����4���Ƿ������Ŀ¼��trueΪ����falseΪ������
		// ����5����Ҫ��֪ͨ����Щ������Ϊ
		// ����6�����صĽ�����ֽ���
		// ����7�����첽����ʹ�ã�����ΪNULL
		// ����8��������
		// ����ֵ��ʧ�ܷ���0���ɹ����ط���ֵ
		BOOL ret = ReadDirectoryChangesW(watch_file, file_buffer, 4096, true, filter, &lbyte, NULL, NULL); //��������
		if (!ret) {
			return false;
		}
		if (!own_set_flag) {
			FILE_NOTIFY_INFORMATION* ret_file = (FILE_NOTIFY_INFORMATION*)file_buffer; //�õ�FILE_NOTIFY_INFORMATION�ṹ�壬���н���
			//����ѭ��������������Ϣ
			do {
				//Ch_file* c_file = new Ch_file(ret_file->FileName, ret_file->Action);
				////���и��ĵ��ļ������ļ��б���
				//Ch_file_list.push_back(c_file);

				wchar_t* fileName = new wchar_t[ret_file->FileNameLength] {};
				memcpy(fileName, ret_file->FileName, ret_file->FileNameLength);
				Ch_file* file = new Ch_file(fileName, ret_file->Action);
				Ch_file_list.push_back(file);
				delete[] fileName;

				//ret_file���µ���һ���ļ�ָ�봦
				// ret_file�е�NextEntryOffset�������������ֽ������ܽ�����һ����¼��ֵΪ���ʾ�������һ����¼
				ret_file = (FILE_NOTIFY_INFORMATION*)((char*)ret_file + ret_file->NextEntryOffset);
			} while (ret_file->NextEntryOffset);

		}

		own_set_flag = !own_set_flag;
		
		//��ջ����� �ȴ��´η���
		memset(file_buffer, 0, lbyte);

		return !Ch_file_list.empty();
	}
};