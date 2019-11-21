#include <string>
#include <stdlib.h>
#include <iostream>

#include "utils.h"

using namespace std;
int main(int argc, char* argv[])
{
	if (argc != 3) {
		Utils::log_error("arguments miss, please run such as:[diff-rsync.exe src_dir dst_dir]");
		system("pause");
		exit(0);
	}

	//argv[1] = "E:/projects/reborn/shared/tools/table-check/table-check/table";
	//argv[2] = "E:/projects/reborn/shared/tools/table-check/table-check/zjw";

	string src_dir = argv[1];
	string dst_dir = argv[2];

	if (!Utils::checkDirExists(src_dir)) {
		Utils::log_error("%s not found", src_dir.c_str());
		system("pause");
		exit(0);
	}

	if (!Utils::checkDirExists(dst_dir)) {
		Utils::log_error("%s not found", dst_dir.c_str());
		system("pause");
		exit(0);
	}

	Utils::diffSync(src_dir, dst_dir);

	return 0;
}