#include"Header.h"

void FileList::getList(const char *initDir, const char *specfile){
	char dir_Full[200];
	_fullpath(dir_Full, initDir, 200);//转换为绝对路径
	_chdir(dir_Full);//changedir
	int len = strlen(dir_Full);
	if (dir_Full[len - 1] != '\\')
		strcat(dir_Full, "\\");//修饰结尾
	long Hfile;//搜索句柄
	_finddata_t fileinfo;//数据体
	Hfile = _findfirst(specfile, &fileinfo);//在当前目录查找第一个具体文件类型
	if (Hfile != -1){
		int flag;
		do{
			string buf = dir_Full;
			buf += fileinfo.name;
			names.push_back(buf);
			flag = _findnext(Hfile, &fileinfo);//查找下一个

		} while (flag == 0);
	}
	else{//获取第一个文件失败
		cout << "获取第一个文件失败" << endl;
	}
	_findclose(Hfile);//查找结束，关闭句柄	
	dir = dir_Full;
	spec = specfile;
	qty = names.size();
}
