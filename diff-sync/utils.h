#ifndef DIFF_RSYNC_UTILS_H
#define DIFF_RSYNC_UTILS_H

#include <string>
#include <fstream>
#include <cstdarg>
#include <iostream>
#include <vector>
#include <map>
#include <io.h>
#include <windows.h>

#include "md5.h"

class Utils
{
public:
	static void log_error(const char *format, ...)
	{
		char buffer[655650];

		va_list args;
		va_start(args, format);
		::vsnprintf(buffer, sizeof(buffer), format, args);
		va_end(args);

		std::string str = std::string(buffer);
		std::cout << str << std::endl;
	}

	static bool getTableFileContent(const std::string &file_path,
		std::string *file_content)
	{
		std::ifstream fs(file_path.c_str(), std::ios::binary | std::ios::ate);
		if (fs.is_open() == false) {
			log_error("can not open table_file(%s)", file_path.c_str());
			return false;
		}

		std::vector<char> input_buffer(fs.tellg());
		fs.seekg(0);
		input_buffer.assign((std::istreambuf_iterator<char>(fs)),
			std::istreambuf_iterator<char>());

		std::vector<char> output_buffer(input_buffer.size() * 2);

		ucs2_to_utf8(&input_buffer[0], input_buffer.size(), &output_buffer[0], output_buffer.size());

		*file_content = std::string(&output_buffer[0]);

		fs.close();
		return true;
	}

	static int ucs2_to_utf8(char *in, int ilen, char *out, int olen)
	{
		int length = 0;
		if (!out) return length;
		char *start = NULL;
		char *pout = out;
		for (start = in; start != NULL && start < in + ilen - 1; start += 2)
		{
			unsigned short ucs2_code = *(unsigned short *)start;
			if (0x0080 > ucs2_code)
			{
				/* 1 byte UTF-8 Character.*/
				if (length + 1 > olen) return -1;

				*pout = (char)*start;
				length++;
				pout++;
			}
			else if (0x0800 > ucs2_code)
			{
				/*2 bytes UTF-8 Character.*/
				if (length + 2 > olen) return -1;
				*pout = ((char)(ucs2_code >> 6)) | 0xc0;
				*(pout + 1) = ((char)(ucs2_code & 0x003F)) | 0x80;
				length += 2;
				pout += 2;
			}
			else
			{
				/* 3 bytes UTF-8 Character .*/
				if (length + 3 > olen) return -1;

				*pout = ((char)(ucs2_code >> 12)) | 0xE0;
				*(pout + 1) = ((char)((ucs2_code & 0x0FC0) >> 6)) | 0x80;
				*(pout + 2) = ((char)(ucs2_code & 0x003F)) | 0x80;
				length += 3;
				pout += 3;
			}
		}

		return length;
	}

	static bool checkDirExists(const std::string &dir_path)
	{
		if (_access(dir_path.c_str(), 0) != -1) {
			return true;
		}
		return false;
	}

	static std::string md5(std::string &str)
	{
		MD5 md5;
		md5.update(str);
		return md5.toString();
	}

	static std::string fileMd5(std::string& file_path_name)
	{
		MD5 md5;
		md5.update(ifstream(file_path_name));
		return md5.toString();
	}

	static void fileSearch(std::string &dir, std::vector<std::string> &out)
	{
		long handler;
		struct _finddata_t file_info;

		handler = _findfirst(dir.c_str(), &file_info);
		if (handler == -1) {
			return;
		}
		do {
			out.push_back(file_info.name);
		} while (!_findnext(handler, &file_info));

		_findclose(handler);
	}

	static bool copyFile(std::string &src_file, std::string &dst_file)
	{
		/*	std::ofstream out(src_file);
			if (!out.is_open())
				return false;
			out.close();*/

		WCHAR buf[256];
		memset(buf, 0, sizeof(buf));
		MultiByteToWideChar(CP_ACP, 0, src_file.c_str(), src_file.length() + 1, buf, sizeof(buf) / sizeof(buf[0]));


		WCHAR buf2[256];
		memset(buf2, 0, sizeof(buf2));
		MultiByteToWideChar(CP_ACP, 0, dst_file.c_str(), dst_file.length() + 1, buf2, sizeof(buf2) / sizeof(buf2[0]));

		
		if (!::CopyFile(buf, buf2, FALSE)) {
			return false;
		}

		return true;
	}

	static void diffSync(std::string &src_dir, std::string &dst_dir)
	{
		std::vector<std::string> src_vec;
		std::vector<std::string> dst_vec;

		fileSearch(src_dir + "/*.*", src_vec);
		fileSearch(dst_dir + "/*.*", dst_vec);

		if (dst_vec.empty())
		{
			for (size_t i = 0; i < src_vec.size(); ++i)
			{
				std::string src_file = src_dir + "/" + src_vec[i];
				std::string dst_file = dst_dir + "/" + src_vec[i];
				if (!copyFile(src_file, dst_file)) {
					log_error("[1]copy file:%s failed", src_file.c_str());
					continue;
				}
			}
		}
		else {
			std::map<std::string, std::string> src_map;
			std::map<std::string, std::string> dst_map;

			for (size_t i = 0; i < src_vec.size(); ++i)
			{
				std::string src_file = src_dir + "/" + src_vec[i];
				std::string md5_str = fileMd5(src_file);
				src_map.insert(std::make_pair(src_vec[i], md5_str));
			}

			for (size_t i = 0; i < dst_vec.size(); ++i)
			{
				std::string dst_file = dst_dir + "/" + dst_vec[i];
				std::string md5_str = fileMd5(dst_file);
				dst_map.insert(std::make_pair(dst_vec[i], md5_str));
			}

			for (auto &iter : src_map) {
				auto iter_dst = dst_map.find(iter.first);
				if (iter_dst == dst_map.end()) {

					std::string src_file = src_dir + "/" + iter.first;
					std::string dst_file = dst_dir + "/" + iter.first;

					if (!copyFile(src_file, dst_file)) {
						log_error("[2]copy file:%s failed", src_file.c_str());
						continue;
					}
				}
				else {
					if (iter.second != iter_dst->second) {
						std::string src_file = src_dir + "/" + iter.first;
						std::string dst_file = dst_dir + "/" + iter.first;

						if (!copyFile(src_file, dst_file)) {
							log_error("[3]copy file:%s failed", src_file.c_str());
							continue;
						}
					}
				}
			}
		}
	}
};


#endif //DIFF_RSYNC_UTILS_H