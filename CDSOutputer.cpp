#include "stdafx.h"
#include "CDSOutputer.h"
#include "CSoundPlayer.h"
#include "IPlayer.h"
#include <math.h>
#include "FastFourierTransform.h"

IDirectSound8 *m_pDS = NULL;
bool dsLoaded = false;
DirectSoundCreate8Fun f_ds;
HMODULE hDs;

//pre=10^(db/2000)*100
//log10(x)=y;
//db=2000.0 * log10(pre/100);
//pre=db/2000

#define PRE_TO_DB(pre) 2000.0 * log10(pre/100.0)
#define DB_TO_PRE(db) pow(10, db / 2000.0)*100.0

CDSOutputer::CDSOutputer(CSoundPlayer * instance)
{
	parent = instance;
	hThreadEventReset = CreateEvent(NULL, TRUE, FALSE, L"ResetEvent");
	m_event[4] = hThreadEventReset;

	m_FFT = new CSoundFFT();
	hBrushGray = CreateSolidBrush(RGB(255, 50, 50));
	hBrushBlack = CreateSolidBrush(RGB(0, 0, 0));
	hBrushGreen = CreateSolidBrush(RGB(0, 1, 0));
	hBrushRed = CreateSolidBrush(RGB(1, 0, 0));
}
CDSOutputer::~CDSOutputer()
{
	Destroy();
	CloseHandle(hThreadEventReset);
	DeleteObject(hBrushGray);
	DeleteObject(hBrushRed);
	DeleteObject(hBrushGreen);
	DeleteObject(hBrushBlack);

	if (m_FFT != NULL) {
		delete[] m_FFT;
		m_FFT = NULL;
	}
}

bool CDSOutputer::OnCopyData(CSoundPlayer * instance, LPVOID buf, DWORD buf_len)
{
	if (callback)return callback(parent,buf, buf_len);
	return false;
}

bool CDSOutputer::Create(HWND hWnd, int sample_rate, int channels,  int bits_per_sample)
{
	this->sample_rate = sample_rate;
	this->channels = channels;
	this->bits_per_sample = bits_per_sample;

	if (!dsLoaded) {
		hDs = LoadLibrary(L"dsound.dll");
		if (!hDs)
			return parent->err(L"Init DirectSound failed : Load dsound.dll failed!");
		f_ds = (DirectSoundCreate8Fun)GetProcAddress(hDs, "DirectSoundCreate8");
		if (FAILED(f_ds(NULL, &m_pDS, NULL)))
			return parent->err(L"Init DirectSound failed : DirectSoundCreate8 failed!");
		if (FAILED(m_pDS->SetCooperativeLevel(hWnd, DSSCL_NORMAL)))
			return parent->err(L"Init DirectSound failed : SetCooperativeLevel");
		dsLoaded = true;
	}

	memset(&dsbd, 0, sizeof(dsbd));
	dsbd.dwSize = sizeof(dsbd);
	dsbd.dwFlags = DSBCAPS_GLOBALFOCUS | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2| DSBCAPS_CTRLVOLUME;
	dsbd.dwBufferBytes = MAX_AUDIO_BUF * BUFFERNOTIFYSIZE;
	//WAVE Header  
	dsbd.lpwfxFormat = (WAVEFORMATEX*)malloc(sizeof(WAVEFORMATEX));
	dsbd.lpwfxFormat->wFormatTag = WAVE_FORMAT_PCM;
	/* format type */
	(dsbd.lpwfxFormat)->nChannels = channels;
	/* number of channels (i.e. mono, stereo...) */
	(dsbd.lpwfxFormat)->nSamplesPerSec = sample_rate;
	/* sample rate */
	(dsbd.lpwfxFormat)->nAvgBytesPerSec = sample_rate * (bits_per_sample / 8)*channels;
	/* for buffer estimation */
	(dsbd.lpwfxFormat)->nBlockAlign = (bits_per_sample / 8)*channels;
	/* block size of data */
	(dsbd.lpwfxFormat)->wBitsPerSample = bits_per_sample;
	/* number of bits per sample of mono data */
	(dsbd.lpwfxFormat)->cbSize = 0;

	//Creates a sound buffer object to manage audio samples.   
	if (FAILED(m_pDS->CreateSoundBuffer(&dsbd, &m_pDSBuffer, NULL))) {
		return parent->err(L"DirectSound error : CreateSoundBuffer failed!");
	}

	if (FAILED(m_pDSBuffer->QueryInterface(IID_IDirectSoundBuffer8, (LPVOID*)&m_pDSBuffer8))) {
		return parent->err(L"DirectSound error.");
	}
	else if (m_setedVol) {
		SetVol(m_setedVol);
		m_setedVol = 0;
	}
	//Get IDirectSoundNotify8  
	if (FAILED(m_pDSBuffer8->QueryInterface(IID_IDirectSoundNotify, (LPVOID*)&m_pDSNotify))) {
		return parent->err(L"DirectSound error.");
	}
	for (int i = 0; i<MAX_AUDIO_BUF; i++) {
		m_pDSPosNotify[i].dwOffset = i * BUFFERNOTIFYSIZE;
		m_event[i] = ::CreateEvent(NULL, false, false, NULL);
		m_pDSPosNotify[i].hEventNotify = m_event[i];
	}
	m_pDSNotify->SetNotificationPositions(MAX_AUDIO_BUF, m_pDSPosNotify);
	m_pDSNotify->Release();



	return true;
}
bool CDSOutputer::Destroy()
{
	if (m_pDSBuffer) {
		m_pDSBuffer->Stop();
		m_pDSBuffer->Release();
	}
	m_pDSBuffer = NULL;

	return false;
}

void CDSOutputer::SetFFTHDC(HDC hdc)
{
	if (hdc)
		drawFFT = true;
	else 
		drawFFT = false;
	ffthdc = hdc;
}
void CDSOutputer::DrawFFTOnHDC(HDC hdc)
{
	if (m_FFT != NULL && m_pDSBuffer8 != NULL)
	{
		HGDIOBJ old = SelectObject(hdc, hBrushBlack);
		DWORD pl, wr;
		if (SUCCEEDED(m_pDSBuffer8->GetCurrentPosition(&pl, &wr)))
		{
			pl %= BUFFERNOTIFYSIZE;
			if (pl + FFT_SIZE < sizeof(fft_datas))
				m_FFT->spectrogram(fft_datas, sample_rate, pl);
			else m_FFT->spectrogram(fft_datas, sample_rate, sizeof(fft_datas) - FFT_SIZE);

			int db = m_FFT->db(fft_datas, pl, FFT_SIZE);

			//wchar_t s[10];
			//wsprintf(s, L"%d\n",db); OutputDebugString(s);

			double *real = m_FFT->real;
			double *imag = m_FFT->imag;

			double model;
			//�������Ƶ�׸��ӵĿ��  
			float x_step = ((float)LineViewWidth / SPECTROGRAM_COUNT);
			//���ӵĸ߶�  
			float y_step = (float)LineViewHeight / ROW_LOCAL_COUNT;

			//SPECTROGRAM_COUNT = 31��ȡ����Ƶ�ʵ�����  
			for (int i = 0; i < SPECTROGRAM_COUNT; i++) {
				model = 2 * hypot(real[i], imag[i]) / FFT_SIZE;// �����Ƶ���ֵ��Math.hypot(x,y)����sqrt(x2+y2)����ߵ�Ƶ  
				for (int k = 1; k < ROW_LOCAL_COUNT; k++) {
					if (model >= row_local_table[k - 1]
						&& model < row_local_table[k]) {
						local[i] = k - 1;//����ȡ��ߵ�Ƶ����Ӧ�ķ�����  
						break;
					}
				}				
				if (db > 40)
					local[i] = (int)((float)local[i] * (float)(db - 40) / 40.0F);
				else local[i] = (int)((float)local[i] * 0.1F);
				//ROW_LOCAL_COUNT = 32�����ɻ��Ƶķ���������������õ�������˼ά���ƣ�y = 0��Ӧ��󷽸���32  
				local[i] = ROW_LOCAL_COUNT - local[i];

				// ����  
				if (Signaled) {
					//mPaint.setColor(sepColor);
					//mPaint.setAlpha(getSepAlpha());
				}
				else {

				}
				float x = XINTERVAL + i * x_step;
				//�������ϻ��Ʒ���  
				for (int j = ROW_LOCAL_COUNT; j > local[i]; j--) {
					float y = (j - 1) * y_step;
					Rectangle(hdc, (int)x, (int)y, (int)(x + x_step - XINTERVAL), (int)(y + y_step - YINTERVAL));// ���ƾ���  
				}
				// �̵�  
				if (Signaled) {

				}
				else {

				}
				//���沿����������ʾ���Ч���ģ�û��ǿ�������������ܻ����Σ�������Ҳ��������ע�ͣ��������츳��  
				//local[i] < top_local[i]˵����ߵ�ı䣨local[i]ԽС����Խ�ߣ�������Ҫע�⣩  
				if (local[i] < top_local[i]) {
					//�����뵽�����˵����Ҫ��������Ƶ��������ܷ�������һ������Ƶ�����½��Ĺ�����  
					top_local[i] = local[i];
					top_local_count[i] = 0;//���㣬���topֵδ���£���ѭ��10�ο�ʼ����  
				}
				else {
					top_local_count[i]++;
					//����top_local_count�����������¼�ﵽtopֵ�����壬Ȼ���ѭ��10�βſ�ʼ����  
					// top_local_count��С��DELAY��top_local�����ֲ���  
					if (top_local_count[i] >= DELAY) {
						top_local_count[i] = DELAY;
						//��������½����ٶ�  
						top_local[i] = local[i] > (top_local[i] + 1) ? (top_local[i] + 1) : local[i];
					}
				}
				//y���������λ�����½�  
				float y = top_local[i] * y_step;			
				SelectObject(hdc, hBrushGray);
				Rectangle(hdc, (int)x, (int)y, (int)(x + x_step - XINTERVAL), (int)(y + y_step - YINTERVAL));// ���λ�õķ���  
				SelectObject(hdc, hBrushBlack);
			}
		}
		SelectObject(hdc, old);
	}
}
void CDSOutputer::DrawFFTOnHDCSmall(HDC hdc)
{
	if (m_FFT != NULL && m_pDSBuffer8 != NULL)
	{
		HGDIOBJ old = SelectObject(hdc, hBrushBlack);
		DWORD pl, wr;
		if (SUCCEEDED(m_pDSBuffer8->GetCurrentPosition(&pl, &wr)))
		{
			pl %= BUFFERNOTIFYSIZE;
			if (pl + FFT_SIZE < sizeof(fft_datas))
				m_FFT->spectrogram_nostanderd(fft_datas, sample_rate, pl);
			else m_FFT->spectrogram_nostanderd(fft_datas, sample_rate, sizeof(fft_datas) - FFT_SIZE);

			int db = m_FFT->db(fft_datas, pl, FFT_SIZE);

			double *real = m_FFT->real;
			double *imag = m_FFT->imag;

			double model;
			//�������Ƶ�׸��ӵĿ��  
			float x_step = ((float)LineViewSmallWidth / SPECTROGRAM_COUNT_SMALL);
			float y_step = 1;

			//SPECTROGRAM_COUNT = 31��ȡ����Ƶ�ʵ�����  
			for (int i = 0; i < SPECTROGRAM_COUNT; i++) {
				model = 2 * hypot(real[i], imag[i]) / FFT_SIZE;// �����Ƶ���ֵ��Math.hypot(x,y)����sqrt(x2+y2)����ߵ�Ƶ  
				local[i] = 0;
				for (int k = 1; k < ROW_LOCAL_COUNT; k++) {
					if (model >= row_local_table_small[k - 1]
						&& model < row_local_table_small[k]) {
						local[i] = k - 1;//����ȡ��ߵ�Ƶ����Ӧ�ķ�����  
						break;
					}
				}
				if (db > 60)
					local[i] = (int)((float)local[i] * ((float)(db - 60) / 20.0F));
				else local[i] = (int)((float)local[i] * 0.1F);
				local[i] = ROW_LOCAL_COUNT - local[i];
				float x = XINTERVALSMALL + i * x_step;
				Rectangle(hdc, (int)x, LineViewSmallHeight, (int)(x + x_step - XINTERVALSMALL), (int)((local[i] - 1) * y_step + y_step - YINTERVALSMALL));
				//���沿����������ʾ���Ч���ģ�û��ǿ�������������ܻ����Σ�������Ҳ��������ע�ͣ��������츳��  
				//local[i] < top_local[i]˵����ߵ�ı䣨local[i]ԽС����Խ�ߣ�������Ҫע�⣩  
				if (local[i] < top_local[i]) {
					//�����뵽�����˵����Ҫ��������Ƶ��������ܷ�������һ������Ƶ�����½��Ĺ�����  
					top_local[i] = local[i];
					top_local_count[i] = 0;//���㣬���topֵδ���£���ѭ��10�ο�ʼ����  
				}
				else {
					top_local_count[i]++;
					//����top_local_count�����������¼�ﵽtopֵ�����壬Ȼ���ѭ��10�βſ�ʼ����  
					// top_local_count��С��DELAY��top_local�����ֲ���  
					if (top_local_count[i] >= DELAY) {
						top_local_count[i] = DELAY;
						//��������½����ٶ�  
						top_local[i] = local[i] > (top_local[i] + 1) ? (top_local[i] + 1) : local[i];
					}
				}
				//y���������λ�����½�  
				float y = top_local[i] * y_step;
				SelectObject(hdc, hBrushGray);
				Rectangle(hdc, (int)x, (int)y, (int)(x + x_step), (int)(y + y_step));// ���λ�õķ���
				SelectObject(hdc, hBrushBlack);
			}
		}
		SelectObject(hdc, old);
	}
}
bool CDSOutputer::IsOutPuting()
{
	return m_outputing;
}
bool CDSOutputer::StartOutPut()
{
	if (!m_outputing)
	{
		if (playThread) {
			DWORD exitcode;
			if (GetExitCodeThread(playThread, &exitcode) && exitcode == STILL_ACTIVE)
				TerminateThread(playThread, 0);
		}
		playThread = (HANDLE)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFuncPCM, (void*)this, 0, &hThreadID);
		if (playThread == NULL)
			return parent->err(L"Failed create player thread.");
		else m_outputing = true;
		return m_outputing;
	}
	return m_outputing;
}
bool CDSOutputer::EndOutPut()
{
	m_pDSBuffer->Stop();
	m_outputing = false;
	return true;
}
bool CDSOutputer::ResetOutPut()
{
	if (m_outputing)
	{
		m_pDSBuffer->Stop();
		SetEvent(hThreadEventReset);
		return true;
	}
	else return StartOutPut();
}

int CDSOutputer::GetVol()
{
	LONG lg = 0;
	if (m_pDSBuffer8 != NULL) {
		m_pDSBuffer8->GetVolume(&lg);
	}
	return static_cast<int>(DB_TO_PRE(lg));
}
void CDSOutputer::SetVol(int vol)
{
	if (m_pDSBuffer8 != NULL) {
		if (vol != 0) m_pDSBuffer8->SetVolume((LONG)(PRE_TO_DB(vol)));
		else m_pDSBuffer8->SetVolume((LONG)(-10000));
	}
	else {
		m_setedVol = vol;
	}
}

void CDSOutputer::CopyFFT(LPVOID buf, DWORD buf_len)
{
	//DWORD pl, wr;
	//if (SUCCEEDED(m_pDSBuffer8->GetCurrentPosition(&pl, &wr)))
	//{
		memcpy_s(fft_datas, sizeof(fft_datas), buf, buf_len);
	//}
}

int CDSOutputer::ThreadFuncPCM(void * lpdwParam)
{
	CDSOutputer*instance = static_cast<CDSOutputer*>(lpdwParam);
	LPVOID buf = NULL;
	DWORD  buf_len = 0;
RESET:
	instance->m_pDSBuffer8->SetCurrentPosition(0);
	instance->m_pDSBuffer8->Play(0, 0, DSBPLAY_LOOPING);
	while (instance->m_outputing) {
		if ((instance->res >= WAIT_OBJECT_0) && (instance->res <= WAIT_OBJECT_0 + 3)) {
			instance->m_pDSBuffer8->Lock(instance->offset, BUFFERNOTIFYSIZE, &buf, &buf_len, NULL, NULL, 0);
			instance->m_outputing = instance->OnCopyData(instance->parent, buf, buf_len);
			if (instance->drawFFT) 
				instance->CopyFFT(buf, buf_len);
			instance->m_pDSBuffer8->Unlock(buf, buf_len, NULL, 0);
			instance->offset += buf_len;
			instance->offset %= (BUFFERNOTIFYSIZE * MAX_AUDIO_BUF);
		}
		else if (instance->res == WAIT_OBJECT_0 + 4) {
			instance->res = WAIT_OBJECT_0;
			instance->offset = 0;
			ResetEvent(instance->m_event[4]);
			goto RESET;
		}
		instance->res = WaitForMultipleObjects(MAX_AUDIO_BUF+1, instance->m_event, FALSE, INFINITE);
	}
	instance->m_pDSBuffer->Stop();
	ExitThread(0);
	return 0;
}


