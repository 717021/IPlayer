// Test.cpp: 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include "..\IPlayerExport.h"
#include <Commdlg.h>  
#include <Shlobj.h>  
#include <locale>
#pragma comment(lib,"Shell32.lib")  
#pragma comment(lib,"iplayer.lib")  

CSoundPlayer*player;

int main()
{
	setlocale(LC_ALL, "chs");
	OPENFILENAME ofn = { 0 };
	TCHAR strFilename[MAX_PATH] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.hwndOwner = GetConsoleWindow();
	ofn.lpstrFilter = TEXT("音乐文件\0*.mp3;*.wav;*.mid;*.midi;*.ogg;*.pcm\0All(*.*)\0*.*\0\0\0");//设置过滤  
	ofn.nFilterIndex = 1;//过滤器索引  
	ofn.lpstrFile = strFilename;
	ofn.nMaxFile = sizeof(strFilename);
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = TEXT("打开音乐");
	ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
	if (!GetOpenFileName(&ofn))
	{
		wprintf(L"请选择一个文件\n");
		system("PAUSE");
		return 0;
	}
	player = CreateIPlayer(GetConsoleWindow());

	wprintf(L"音乐：%s\n", strFilename);
	if (player->Open(strFilename))
	{
		wprintf(L"音乐已打开\n");
		if (player->Play())
		{
			wprintf(L"音乐长度：%d 秒\n", (int)player->GetMusicLength());
			wprintf(L"音乐开始播放\n正在播放：000/%d", (int)player->GetMusicLength());
			for (int i = 0; i < 4; i++)
				putchar('\b');

			while (player->GetPlayerStatus() != TPlayerStatus::PlayEnd)
			{
				for (int i = 0; i < 3; i++)
					putchar('\b');
				wprintf(L"%03d", (int)player->GetMusicPos());
				Sleep(1000);
			}
		}
		else wprintf(L"播放失败：%s\n", player->GetLastErr());

		player->Stop();
		player->Close();
		wprintf(L"音乐已停止\n");
		system("PAUSE");
	}
	else {
		wprintf(L"打开文件失败：%s\n", player->GetLastErr());
		system("PAUSE");
	}
	DestroyIPlayer(player);
    return 0;
}

