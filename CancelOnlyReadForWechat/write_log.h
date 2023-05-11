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


//������Ҫ��ʾ�ȼ����ƣ������Ǻ�ֵ��ʹ��#ʹ��#����׸���������Ϊһ�������ŵ��ַ������������ݵľ��Ǻ���
//__FILE__�ǵ�ǰ��������Դ�ļ�����__LINE__�ǵ�ǰ����Դ�ļ�������
#define LOG(level,message,path) Log(#level,message,__FILE__,__LINE__,path)

//[��־�ȼ�][ʱ���][��־��Ϣ][�����ļ���][����]
void Log(std::string level, std::string message, std::string fileName, int line,std::string& out_path) {
	//��ǰʱ��
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

