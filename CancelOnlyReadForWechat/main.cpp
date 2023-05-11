//不显示cmd窗口
//#pragma comment( linker, "/subsystem:windows /entry:mainCRTStartup" )

#include "cancelonlyread.h"
#include "watchdir.h"
#include <comutil.h> 
#include <thread>
#include <set>
#include <winternl.h>
#include <winsvc.h>

#pragma comment(lib, "comsuppw.lib")

#define SLEEP_TIME 5000 //间隔时间

//要拦截的文档格式
std::set<string> format_map{ ".pdf",".txt",".docx",".doc",".dot",".rtf",".xltx",".xls",".xlsm",
"xlsb",".csv",".xml",".pptx",".ppt",".xps",".key",".pages",".psb",".eps",".ai",".excel",".md",".zip",".rar"};


//监视并拦截
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
		ReadDirectoryChangesW函数中可监视的属性

			FILE_NOTIFY_CHANGE_FILE_NAME, //文件名更改
			FILE_NOTIFY_CHANGE_DIR_NAME, //目录名更改
			FILE_NOTIFY_CHANGE_ATTRIBUTES, //文件属性更改
			FILE_NOTIFY_CHANGE_SIZE, //文件大小更改
			FILE_NOTIFY_CHANGE_LAST_WRITE, //文件最后写入时间更改
			FILE_NOTIFY_CHANGE_LAST_ACCESS, //文件最后访问时间更改
			FILE_NOTIFY_CHANGE_CREATION, //文件创建
			FILE_NOTIFY_CHANGE_SECURITY, //文件安全描述符更改

		FILE_NOTIFY_INFORMATION结构体中定义的返回的行为
			FILE_ACTION_ADDED, //文件添加
			FILE_ACTION_REMOVED, //文件删除
			FILE_ACTION_MODIFIED, //文件更改
			FILE_ACTION_RENAMED_OLD_NAME, //文件重命名 旧名字
			FILE_ACTION_RENAMED_NEW_NAME //文件重命名 新名字
		*/


		// 监视文件创建和属性被更改 被监视的属性和返回的行为是不同的
		while (true) {
			wf.start_watch(file_list, FILE_NOTIFY_CHANGE_FILE_NAME | FILE_NOTIFY_CHANGE_ATTRIBUTES);

			for (auto& file : file_list) {

				//判断文件被更改的行为
				/*switch (file->action)
				{
				case FILE_ACTION_ADDED:
					std::wcout << file->name << L":\t文件被添加" << std::endl;
					break;
				case FILE_ACTION_MODIFIED:
					std::wcout << file->name << L":\t文件被更改" << std::endl;
					break;
				default:
					std::wcout << "未知的变更" << std::endl;
					break;
				}*/

				//文件重命名、删除不拦截
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
					info += " 不需拦截";

					LOG(INFO, info);*/

					continue;
				}
				else {
					//取消只读属性
					//system("ATTRIB -R C:\\Users\\HelloGu\\Documents\\WeChat~1\\*");
					cr.Cancel(result,wf);
				}
			}
		}

}


//运行标志
bool brun = false;
std::string docu_path = getWinPath(Documents_PATH);
std::string log_out_path = docu_path + "\\CancelOnlyRead\\log.txt";
std::string app_watch_path = docu_path + "\\WeChat Files";

//包含服务的状态信息，服务使用 SetServiceStatus 函数中的此结构向服务控制管理器报告其当前状态。
SERVICE_STATUS servicestatus;
SERVICE_STATUS_HANDLE hstatus;

void WINAPI ServiceHandler(DWORD fdwControl)
{
	switch (fdwControl)
	{
	case SERVICE_CONTROL_STOP://服务可以停止
		brun = false;
		// 设置当前服务状态为服务未运行
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;
	case SERVICE_CONTROL_SHUTDOWN://关机 系统关闭时会通知服务
		brun = false;
		servicestatus.dwCurrentState = SERVICE_STOPPED;
		break;
	default:
		break;
	}

	//向服务控制管理器报告其当前状态
	SetServiceStatus(hstatus, &servicestatus);
}

//将需要执行的任务放到该函数中循环执行
void WINAPI service_main(int argc, char** argv)
{
	//服务类型：服务在其自己的进程中运行、该服务与其他服务共享进程
	servicestatus.dwServiceType = SERVICE_WIN32;
	//设置服务状态为服务正在启动
	servicestatus.dwCurrentState = SERVICE_START_PENDING;
	//服务接受的控制代码  在本例中只接受系统关机和停止服务两种控制命令
	servicestatus.dwControlsAccepted = SERVICE_ACCEPT_SHUTDOWN | SERVICE_ACCEPT_STOP;


	//dwWin32ExitCode 和 dwServiceSpecificExitCode ：这两个域在你终止服务并报告退出细节时很有用。
	//初始化服务时并不退出，因此，它们的值为 0
	
	//服务用于报告启动或停止时发生的错误的错误代码  应在运行和正常终止时将此值设置为 NO_ERROR 
	servicestatus.dwWin32ExitCode = 0;
	//服务在服务启动或停止时出错时返回的服务特定的错误代码 除非dwWin32ExitCode设置了，否则忽略此值
	servicestatus.dwServiceSpecificExitCode = 0;

	//dwCheckPoint 和 dwWaitHint ：这两个域表示初始化某个服务进程时要30 秒以上。
	//本例服务的初始化过程很短，所以这两个域的值都为 0

	//当服务没有启动、停止、暂停或继续操作挂起时，该值应为零
	servicestatus.dwCheckPoint = 0;
	//挂起开始、停止、暂停或继续操作所需的估计时间
	servicestatus.dwWaitHint = 0;

	// 1.服务名称 testservice 这是创建服务时CreateService 函数中指定的服务控制程序的服务名称
	// 2.指向要注册的处理程序函数的指针
	// 返回值为服务状态句柄 出错返回0
	hstatus = ::RegisterServiceCtrlHandlerA("cancelonlyread", ServiceHandler);
	if (hstatus == 0)
	{
		LOG(ERR,"RegisterServiceCtrlHandler failed", log_out_path);
		return;
	}


	LOG(INFO,"RegisterServiceCtrlHandler success", log_out_path);
	std::cout << log_out_path << std::endl;

	// 控制处理器函数必须报告服务状态，即便 SCM 每次发送控制请求的时候状态保持相同。
	// 因此，不管响应什么请求，都要调用 SetServiceStatus 。
	// 向SCM 报告运行状态 设置当前服务状态为服务正在运行
	servicestatus.dwCurrentState = SERVICE_RUNNING;
	SetServiceStatus(hstatus, &servicestatus);//向服务控制管理器报告其当前状态

	//开始任务循环，添加你自己希望服务做的工作
	brun = true;

	while (brun)
	{
		//创建文件夹
		std::string cmd = "mkdir -p ";
		cmd += docu_path;
		cmd += "\\CancelOnlyRead";
		system(cmd.c_str());

		watch_and_set();

		LOG(ERR, "线程异常退出，重新创建线程", log_out_path);

		//Sleep(SLEEP_TIME);
	}

	//服务停止
	LOG(ERR, "service stopped", log_out_path);
}

//像ServiceMain一样，CtrlHandler也是一个回调函数，用户必须为它的服务程序中每一个服务写一个单独的CtrlHandler函数，
//因此如果有一个程序含有两个服务，那么它至少要拥有5个不同的函数：作为入口点的main()或WinMain()，用于第一个服务的
//ServiceMain函数和CtrlHandler函数，以及用于第二个服务的ServiceMain函数和CtrlHandler函数。
//
//SCM调用一个服务的CtrlHandler函数去改变这个服务的状态。例如，当某个管理员用管理工具里的“服务”尝试停止你的服务的
//时候，你的服务的CtrlHandler函数将收到一个SERVICE_CONTROL_STOP通知。CtrlHandler函数负责执行停止服务所需的一切
//代码。由于是进程的主线程执行所有的CtrlHandler函数，因而必须尽量优化你的CtrlHandler函数的代码，使它运行起来足够
//快，以便相同进程中的其它服务的CtrlHandler函数能在适当的时间内收到属于它们的通知。而且基于上述原因，你的CtrlHandler
//函数必须要能够将想要传达的状态送到服务线程，这个传递过程没有固定的方法，完全取决于你的服务的用途。
//根据控制代码处理函数


int main()
{
	//指定可在调用过程中运行的服务的 ServiceMain 函数
	SERVICE_TABLE_ENTRYA entrytable[2];

	//要在此服务进程中运行的服务的名称
	entrytable[0].lpServiceName = "cancelonlyread";
	//指向 ServiceMain 函数的指针
	entrytable[0].lpServiceProc = (LPSERVICE_MAIN_FUNCTIONA)service_main;

	//表中最后一项的成员必须具有 NULL 值才能指定表的末尾 故不设置
	entrytable[1].lpServiceName = NULL;
	entrytable[1].lpServiceProc = NULL;

	//将服务进程的主线程连接到服务控制管理器，这会导致线程成为调用进程的服务控制调度程序线程。
	// 该函数成功，则返回值为非零值
	int ret = StartServiceCtrlDispatcherA(entrytable);
	ret = GetLastError();

	return 0;
}

