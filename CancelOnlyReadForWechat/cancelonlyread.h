#pragma once
#include <iostream>
#include "watchdir.h"
#include <string>
#include "write_log.h"

using std::string;

extern std::string log_out_path;
extern std::string app_watch_path;

namespace Tools {
	class CancelOnlyRead {
	public:
		CancelOnlyRead() {}
		CancelOnlyRead(string& path) :file_path(path) {}

		void Cancel(std::string file_name,WatchFolder& wf) {
			std::string path = file_path + file_name;

			int ret = SetFileAttributesA(path.c_str(), FILE_ATTRIBUTE_NORMAL);
			if (ret == 0) {
				std::cout << path << std::endl;
				wf.setflag();
				LOG(WARNING, "SetFileAttributes Faild.", log_out_path);
			}
			else {
				std::string info = "File:";
				int pos = file_name.rfind('\\');
				if ( pos != std::string::npos) {
					info += file_name.substr(pos + 1);
				}
				else {
					info += file_name;
				}
				
				info += " È¡ÏûÖ»¶Á";

				LOG(INFO, info, log_out_path);
			}

		}
		~CancelOnlyRead() {}
	private:
		std::string file_path = app_watch_path + "\\";
	};
}