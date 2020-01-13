#include <Windows.h>
#include <iostream>


int main()
{
	//For test includ local dll
	//char *path_dll2 = new char[MAX_PATH];
	//int ax = GetFullPathNameA("hook.dll", MAX_PATH, path_dll2, 0);

	//char path_dll[] = R"(d:\Projects\hook\Debug\hook.dll)";
	//HINSTANCE hinstLib = LoadLibrary(TEXT(path_dll));

	for (int i = 0;; i++)
	{
		Sleep(5000);
		// Expected to tell "Hooked!".
		char str[128] = "d:\\tmp\\file_";

		size_t sizebuff = 8;
		char *buff = (char *)malloc(sizebuff);
		_itoa_s(i, buff, sizebuff, 10);

		strcat_s(str, sizeof str, buff);
		std::cout << str << std::endl;
		HANDLE hFile = CreateFileA(str, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);

	}

	return 0;
}
