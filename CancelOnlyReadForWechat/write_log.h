#pragma once
#include <stdio.h>
#include <iostream>
#include <string>
#include <time.h>
#include "getPath.h"

#define INFO 1
#define WARNING 2
#define ERR 3
#define FATAL 4


//我们需要显示等级名称，而不是宏值，使用#使在#后的首个参数返回为一个带引号的字符串，这样传递的就是宏名
//__FILE__是当前行所属的源文件名，__LINE__是当前所在源文件的行数
#define LOG(level,message,path) Log(#level,message,__FILE__,__LINE__,path)

//[日志等级][时间戳][日志信息][错误文件名][行数]
void Log(std::string level, std::string message, std::string fileName, int line,std::string& out_path) {
	//当前时间
	time_t timer = time(nullptr);
	char* buf = ctime(&timer);
	buf[strlen(buf) - 1] = 0;

	std::string str = "[";
	str += level;
	str += "][";
	str += buf;
	str += "][";
	str += message;
	str += "][";
	str += std::to_string(line);
	str += "]";

	FILE* pfile = fopen(out_path.c_str(), "a+");
	if (pfile == NULL)
	{
		return;
	}
	fprintf(pfile, "%s\n", str.c_str());
	fclose(pfile);
}

