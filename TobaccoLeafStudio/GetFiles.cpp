#include"Header.h"

void FileList::getList(const char *initDir, const char *specfile){
	char dir_Full[200];
	_fullpath(dir_Full, initDir, 200);//ת��Ϊ����·��
	_chdir(dir_Full);//changedir
	int len = strlen(dir_Full);
	if (dir_Full[len - 1] != '\\')
		strcat(dir_Full, "\\");//���ν�β
	long Hfile;//�������
	_finddata_t fileinfo;//������
	Hfile = _findfirst(specfile, &fileinfo);//�ڵ�ǰĿ¼���ҵ�һ�������ļ�����
	if (Hfile != -1){
		int flag;
		do{
			string buf = dir_Full;
			buf += fileinfo.name;
			names.push_back(buf);
			flag = _findnext(Hfile, &fileinfo);//������һ��

		} while (flag == 0);
	}
	else{//��ȡ��һ���ļ�ʧ��
		cout << "��ȡ��һ���ļ�ʧ��" << endl;
	}
	_findclose(Hfile);//���ҽ������رվ��	
	dir = dir_Full;
	spec = specfile;
	qty = names.size();
}
