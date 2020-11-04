#include <Windows.h>
#include <stdio.h>


int main()
{
	//For test include local dll
	//char *path_dll2 = new char[MAX_PATH];
	//int ax = GetFullPathNameA("hook.dll", MAX_PATH, path_dll2, 0);

	//char path_dll[] = R"(d:\Projects\hook\Debug\hook.dll)";
	//HINSTANCE hinstLib = LoadLibrary(TEXT(path_dll));
	int i;
	char buff[128];
	char str[] = "d:\\tmp\\file_";

	for (i = 0;; i++)
	{
		Sleep(5000);
		// Expected to tell "Hooked!".	
		snprintf(buff, strlen(buff)+strlen(str), "%s%d", str, i);
		printf("%s\n", buff);
		HANDLE hFile = CreateFileA(str, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	}

	return 0;
}
