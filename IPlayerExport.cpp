// BellsPlayer.cpp: 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "IPlayerExport.h"
#include "IPlayer.h"

CSoundPlayer*player = NULL;

CSoundPlayer * CreateIPlayer(HWND hWnd)
{
	return new IPlayer(hWnd);
}
void DestroyIPlayer(CSoundPlayer*iplayer)
{
	delete iplayer;
}

extern "C" BOOL __declspec(dllexport) OpenMusic(LPWSTR file)
{
	if (player)return player->Open(file);
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) CloseMusic()
{
	if (player)return player->Close();
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) StopMusic()
{
	if (player)return player->Stop();
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) PausePlayMusic()
{
	if (player)return player->PausePlay();
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) IPlayerInit(HWND hWnd)
{
	BOOL rs = false;
	if (!player) {
		player = CreateIPlayer(hWnd);
		if (player) rs = true;
	}
	return rs;
}
extern "C" BOOL __declspec(dllexport) ReplayMusic()
{
	if (player) return player->Restart();
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) PlayMusic()
{
	if (player) return player->Play();
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) PauseMusic()
{
	if (player) return player->Pause();
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) SetMusicPos(double val)
{
	if (player) return player->SetMusicPos(val);
	else return FALSE;
}
extern "C" double __declspec(dllexport) GetMusicLength()
{
	if (player) return player->GetMusicLength();
	else return FALSE;
}
extern "C" double __declspec(dllexport) GetMusicPos()
{
	if (player) return player->GetMusicPos();
	else return FALSE;
}
extern "C" BOOL __declspec(dllexport) GetOpened()
{
	if (player) return player->IsOpened();
	else return FALSE;
}
extern "C" LPWSTR __declspec(dllexport) GetMusicTimeString()
{
	if (player)
		return player->GetMusicTimeString();
	else return L"Not ceate.";
}
extern "C" LPWSTR __declspec(dllexport) GetPlayerError()
{
	if (player)
		return player->GetLastErr();
	else return L"Not ceate.";
}
extern "C" BOOL __declspec(dllexport) IPlayerDestroy()
{
	if (player) {
		DestroyIPlayer(player);
		return 1;
	}
	player = NULL;
	return FALSE;
}
extern "C" BOOL __declspec(dllexport) MusicTweedOut(int currvol)
{
	BOOL rs = false;
	if (player&&player->IsOpened()) {
		//TStreamTime pTimeEnd;
		//pTimeEnd.sec = playerPos.sec + 2;
		//player->GetPlayerVolume(&ol, &or );
		//rs = player->SlideVolume(tfSecond, &playerPos, ol, or , tfSecond, &pTimeEnd, 0, 0);
		player->FadeOut(2, currvol, 0);
	}
	return rs;
}
extern "C" BOOL __declspec(dllexport) MusicTweedIn(int currvol)
{
	BOOL rs = false;
	if (player&&player->IsOpened()) {
		//TStreamTime pTimeEnd;
		//pTimeEnd.sec = playerPos.sec + 2;
		//rs = player->SlideVolume(tfSecond, &playerPos, 0, 0, tfSecond, &pTimeEnd, ol, or);
		player->FadeIn(2, 0, currvol);
	}
	return rs;
}
extern "C" BOOL __declspec(dllexport) SetFFTHDC(HDC hdc) {
	BOOL rs = false;
	if (player)
	{
		player->SetFFTHDC(hdc);
	}
	return rs;
}
extern "C" BOOL __declspec(dllexport) DrawFFTOnHDC(HDC hdc)
{
	BOOL rs = false;
	if (player)
	{
		player->DrawFFTOnHDC(hdc);
	}
	return rs;
}
extern "C" BOOL __declspec(dllexport) DrawFFTOnHDCSmall(HDC hdc)
{
	BOOL rs = false;
	if (player)
	{
		player->DrawFFTOnHDCSmall(hdc);
	}
	return rs;
}
extern "C" TPlayerStatus __declspec(dllexport) GetMusicPlayState()
{
	if (player)
		return player->GetPlayerStatus();
	else return TPlayerStatus::Unknow;
}
extern "C" int __declspec(dllexport) GetVolume()
{
	if (player)
		return player->GetPlayerVolume();
	else return -1;
}
extern "C" BOOL __declspec(dllexport) SetVolume(int vol)
{
	if (player) {
		player->SetPlayerVolume(vol);
		return TRUE;
	}
	else return FALSE;
}
