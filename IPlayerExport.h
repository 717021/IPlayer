#pragma once
#include "CSoundPlayer.h"

//���� ������ʵ��
//*��ĳ��������ھ��
CSoundPlayer * CreateIPlayer(HWND hWnd);
//�ͷ� ������ʵ��
// * ���ڳ����˳�ʱʹ�ô˷���
// * iplayer ��������ʵ��
void DestroyIPlayer(CSoundPlayer*iplayer);

//�ر��Ѵ򿪵�����
extern "C" BOOL __declspec(dllexport) CloseMusic();
/*������
* file ������·��
*/
extern "C" BOOL __declspec(dllexport) OpenMusic(LPWSTR file);
/*ֹͣ����*/
extern "C" BOOL __declspec(dllexport) StopMusic();
/*
���Ż���ͣ����
* ����������ڲ��ţ�����ͣ���֣����������ͣ����ʼ�������֣��൱�ڲ���/��ͣ��ť����
*/
extern "C" BOOL __declspec(dllexport) PausePlayMusic();
/*��ʼ��������
* ���ñ����C����apiʱ�����ȳ�ʼ��������
* hWnd �������ߴ��ھ��
*/
extern "C" BOOL __declspec(dllexport) IPlayerInit(HWND hWnd);
/*��ͷ��ʼ���ŵ�ǰ����*/
extern "C" BOOL __declspec(dllexport) ReplayMusic();
/*��ʼ��������*/
extern "C" BOOL __declspec(dllexport) PlayMusic();
/*��ͣ���ŵ�ǰ����*/
extern "C" BOOL __declspec(dllexport) PauseMusic();
//��ȡ��ǰ���ֵĳ���
// * �� �� Ϊ��λ
extern "C" double __declspec(dllexport) GetMusicLength();
/*���õ�ǰ���ֲ���λ��
* val����ǰ���ֲ���λ�ã���λΪ ��
*/
extern "C" BOOL __declspec(dllexport) SetMusicPos(double val);
/*��ȡ��ǰ���ֲ���λ��
* ���أ���ǰ���ֲ���λ�ã���λΪ ��
*/
extern "C" double __declspec(dllexport) GetMusicPos();
/*��ȡ�����Ƿ��Ѵ�*/
extern "C" BOOL __declspec(dllexport) GetOpened();
/*��ȡ�������ڲ��ŵ�ʱ���ַ���
* ��ʽ�ǣ�00:00/00:00 ���ڲ���ʱ��/����ʱ��
*/
extern "C" LPWSTR __declspec(dllexport) GetMusicTimeString();
/*��ȡ��������һ��������Ϣ*/
extern "C" LPWSTR __declspec(dllexport) GetPlayerError();
/*���ٲ�����
* ���ڳ����˳�ʱʹ�ô˷���
*/
extern "C" BOOL __declspec(dllexport) IPlayerDestroy();
/*���ֵ�����2s��
* currvol ����ǰ����
* �൱������������С
*/
extern "C" BOOL __declspec(dllexport) MusicTweedOut(int currvol);
/*���ֵ��루2s��
* currvol ����ǰ����
* �൱�������������
*/
extern "C" BOOL __declspec(dllexport) MusicTweedIn(int currvol);
/**/
extern "C" BOOL __declspec(dllexport) SetFFTHDC(HDC hdc);
/**/
extern "C" BOOL __declspec(dllexport) DrawFFTOnHDC(HDC hdc);
/**/
extern "C" BOOL __declspec(dllexport) DrawFFTOnHDCSmall(HDC hdc);
/*��ȡ������״̬
* TPlayerStatus::Unknow,
* TPlayerStatus::NotOpen, δ���ļ�
* TPlayerStatus::Opened, �ļ��Ѵ򿪾���
* TPlayerStatus::Playing, ���ڲ���
* TPlayerStatus::Paused, ��ͣ��
* TPlayerStatus::PlayEnd, ���ŵ����ֽ�β��
*/
extern "C" TPlayerStatus __declspec(dllexport) GetMusicPlayState();
/*��ȡ����������
* ����������ֵ ��1-100
*/
extern "C" int __declspec(dllexport) GetVolume();
/*���ò���������
* vol��������ֵ ��1-100��
*/
extern "C" BOOL __declspec(dllexport) SetVolume(int vol);
