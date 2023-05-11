//����ʾcmd����
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

#include "cancelonlyread.h"
#include "watchdir.h"
#include <comutil.h> 
#include <thread>
#include <set>
#include <winternl.h>
#include <winsvc.h>

#pragma comment(lib, "comsuppw.lib")

#define SLEEP_TIME 5000 //���ʱ��

//Ҫ���ص��ĵ���ʽ
std::set<string> format_map{ ".pdf",".txt",".docx",".doc",".dot",".rtf",".xltx",".xls",".xlsm",
"xlsb",".csv",".xml",".pptx",".ppt",".xps",".key",".pages",".psb",".eps",".ai",".excel",".md",".zip",".rar"};


//���Ӳ�����
void watch_and_set() {
		setlocale(LC_ALL, "");

		Tools::CancelOnlyRead cr;

		WatchFolder wf;

		bool ret = wf.Init(app_watch_path);

		if (!ret) {
			LOG(ERR,"WatchFolder Init Error.", log_out_path);
			exit(1);
		}

		std::list<Ch_file*> file_list;

		/*
		ReadDirectoryChangesW�����пɼ��ӵ�����

			FILE_NOTIFY_CHANGE_FILE_NAME, //�ļ�������
			FILE_NOTIFY_CHANGE_DIR_NAME, //Ŀ¼������
			FILE_NOTIFY_CHANGE_ATTRIBUTES, //�ļ����Ը���
			FILE_NOTIFY_CHANGE_SIZE, //�ļ���С����
			FILE_NOTIFY_CHANGE_LAST_WRITE, //�ļ����д��ʱ�����
			FILE_NOTIFY_CHANGE_LAST_ACCESS, //�ļ�������ʱ�����
			FILE_NOTIFY_CHANGE_CREATION, //�ļ�����
			FILE_NOTIFY_CHANGE_SECURITY, //�ļ���ȫ����������

		FILE_NOTIFY_INFORMATION�ṹ���ж���ķ��ص���Ϊ
			FILE_ACTION_ADDED, //�ļ����
			FILE_ACTION_REMOVED, //�ļ�ɾ��
			FILE_ACTION_MODIFIED, //�ļ�����
			FILE_ACTION_RENAMED_OLD_NAME, //�ļ������� ������
			FILE_ACTION_RENAMED_NEW_NAME //�ļ������� ������
		*/


		// �����ļ����������Ա����� �����ӵ����Ժͷ��ص���Ϊ�ǲ�ͬ��
		while (true) {
			wf.start_watch(file_list, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES);

			for (auto& file : file_list) {

				//�ж��ļ������ĵ���Ϊ
				/*switch (file->action)
				{
				case FILE_ACTION_ADDED:
					std::wcout << file->name << L":\t�ļ������" << std::endl;
					break;
				case FILE_ACTION_MODIFIED:
					std::wcout << file->name << L":\t�ļ�������" << std::endl;
					break;
				default:
					std::wcout << "δ֪�ı��" << std::endl;
					break;
				}*/

				//�ļ���������ɾ��������
				if (file->action == FILE_ACTION_RENAMED_OLD_NAME || /*file->action == FILE_ACTION_RENAMED_NEW_NAME ||*/
					file->action == FILE_ACTION_REMOVED ) {
					continue;
				}

				//wstring 2 string
				_bstr_t t = file->name.c_str();
				char* pchar = (char*)t;
				std::string result = pchar;


				std::string str_front;

				size_t front_ret = result.rfind('\\');
				if (front_ret != std::string::npos) {
					str_front = result.substr(front_ret + 1,1);
					if (str_front == "~") {
						continue;
					}
				}

				size_t end_ret = result.rfind('.');
				int format_num = 0;

				if (end_ret != std::string::npos) {
					str_front = result.substr(end_ret);
					format_num = (int)str_front.size() - 1;
				}


				if (format_num <= 0 || format_num > 5 || format_map.find(str_front) == format_map.end()) {
					
					/*std::string info = "File:";
					info += result;
					info += " ��������";

					LOG(INFO, info);*/

					continue;
				}
				else {
					//ȡ��ֻ������
					//system("ATTRIB -R C:\\Users\\HelloGu\\Documents\\WeChat~1\\*");
					cr.Cancel(result,wf);
				}
			}
		}

}


//���б�־
bool brun = false;
std::string docu_path = getWinPath(Documents_PATH);
std::string log_out_path = docu_path + "\\CancelOnlyRead\\log.txt";
std::string app_watch_path = docu_path + "\\WeChat Files";

//���������״̬��Ϣ������ʹ�� SetServiceStatus �����еĴ˽ṹ�������ƹ����������䵱ǰ״̬��
SERVICE_STATUS servicestatus;
SERVICE_STATUS_HANDLE hstatus;

void WINAPI ServiceHandler(DWORD fdwControl)
{
	switch (fdwControl)
	{
	case SERVICE_CONTROL_STOP://�������ֹͣ
		brun = false;
		// ���õ�ǰ����״̬Ϊ����δ����
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_SHUTDOWN://�ػ� ϵͳ�ر�ʱ��֪ͨ����
		brun = false;
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;
	default:
		break;
	}

	//�������ƹ����������䵱ǰ״̬
	SetServiceStatus(hstatus, &servicestatus);
}

//����Ҫִ�е�����ŵ��ú�����ѭ��ִ��
void WINAPI service_main(int argc, char** argv)
{
	//�������ͣ����������Լ��Ľ��������С��÷������������������
	servicestatus.dwServiceType = SERVICE_WIN32;
	//���÷���״̬Ϊ������������
	servicestatus.dwCurrentState = SERVICE_START_PENDING;
	//������ܵĿ��ƴ���  �ڱ�����ֻ����ϵͳ�ػ���ֹͣ�������ֿ�������
	servicestatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;


	//dwWin32ExitCode �� dwServiceSpecificExitCode ����������������ֹ���񲢱����˳�ϸ��ʱ�����á�
	//��ʼ������ʱ�����˳�����ˣ����ǵ�ֵΪ 0
	
	//�������ڱ���������ֹͣʱ�����Ĵ���Ĵ������  Ӧ�����к�������ֹʱ����ֵ����Ϊ NO_ERROR 
	servicestatus.dwWin32ExitCode = 0;
	//�����ڷ���������ֹͣʱ����ʱ���صķ����ض��Ĵ������ ����dwWin32ExitCode�����ˣ�������Դ�ֵ
	servicestatus.dwServiceSpecificExitCode = 0;

	//dwCheckPoint �� dwWaitHint �����������ʾ��ʼ��ĳ���������ʱҪ30 �����ϡ�
	//��������ĳ�ʼ�����̺̣ܶ��������������ֵ��Ϊ 0

	//������û��������ֹͣ����ͣ�������������ʱ����ֵӦΪ��
	servicestatus.dwCheckPoint = 0;
	//����ʼ��ֹͣ����ͣ�������������Ĺ���ʱ��
	servicestatus.dwWaitHint = 0;

	// 1.�������� testservice ���Ǵ�������ʱCreateService ������ָ���ķ�����Ƴ���ķ�������
	// 2.ָ��Ҫע��Ĵ����������ָ��
	// ����ֵΪ����״̬��� ������0
	hstatus = ::RegisterServiceCtrlHandlerA("cancelonlyread", ServiceHandler);
	if (hstatus == 0)
	{
		LOG(ERR,"RegisterServiceCtrlHandler failed", log_out_path);
		return;
	}


	LOG(INFO,"RegisterServiceCtrlHandler success", log_out_path);
	std::cout << log_out_path << std::endl;

	// ���ƴ������������뱨�����״̬������ SCM ÿ�η��Ϳ��������ʱ��״̬������ͬ��
	// ��ˣ�������Ӧʲô���󣬶�Ҫ���� SetServiceStatus ��
	// ��SCM ��������״̬ ���õ�ǰ����״̬Ϊ������������
	servicestatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hstatus, &servicestatus);//�������ƹ����������䵱ǰ״̬

	//��ʼ����ѭ����������Լ�ϣ���������Ĺ���
	brun = true;

	while (brun)
	{
		//�����ļ���
		std::string cmd = "mkdir -p ";
		cmd += docu_path;
		cmd += "\\CancelOnlyRead";
		system(cmd.c_str());

		watch_and_set();

		LOG(ERR, "�߳��쳣�˳������´����߳�", log_out_path);

		//Sleep(SLEEP_TIME);
	}

	//����ֹͣ
	LOG(ERR, "service stopped", log_out_path);
}

//��ServiceMainһ����CtrlHandlerҲ��һ���ص��������û�����Ϊ���ķ��������ÿһ������дһ��������CtrlHandler������
//��������һ������������������ô������Ҫӵ��5����ͬ�ĺ�������Ϊ��ڵ��main()��WinMain()�����ڵ�һ�������
//ServiceMain������CtrlHandler�������Լ����ڵڶ��������ServiceMain������CtrlHandler������
//
//SCM����һ�������CtrlHandler����ȥ�ı���������״̬�����磬��ĳ������Ա�ù�������ġ����񡱳���ֹͣ��ķ����
//ʱ����ķ����CtrlHandler�������յ�һ��SERVICE_CONTROL_STOP֪ͨ��CtrlHandler��������ִ��ֹͣ���������һ��
//���롣�����ǽ��̵����߳�ִ�����е�CtrlHandler������������뾡���Ż����CtrlHandler�����Ĵ��룬ʹ�����������㹻
//�죬�Ա���ͬ�����е����������CtrlHandler���������ʵ���ʱ�����յ��������ǵ�֪ͨ�����һ�������ԭ�����CtrlHandler
//��������Ҫ�ܹ�����Ҫ�����״̬�͵������̣߳�������ݹ���û�й̶��ķ�������ȫȡ������ķ������;��
//���ݿ��ƴ��봦����


int main()
{
	//ָ�����ڵ��ù��������еķ���� ServiceMain ����
	SERVICE_TABLE_ENTRYA entrytable[2];

	//Ҫ�ڴ˷�����������еķ��������
	entrytable[0].lpServiceName = "cancelonlyread";
	//ָ�� ServiceMain ������ָ��
	entrytable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)service_main;

	//�������һ��ĳ�Ա������� NULL ֵ����ָ�����ĩβ �ʲ�����
	entrytable[1].lpServiceName = NULL;
	entrytable[1].lpServiceProc = NULL;

	//��������̵����߳����ӵ�������ƹ���������ᵼ���̳߳�Ϊ���ý��̵ķ�����Ƶ��ȳ����̡߳�
	// �ú����ɹ����򷵻�ֵΪ����ֵ
	int ret = StartServiceCtrlDispatcherA(entrytable);
	ret = GetLastError();

	return 0;
}

