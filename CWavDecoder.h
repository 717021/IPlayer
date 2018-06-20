#pragma once
#include "CSoundDecoder.h"
#include <MMSystem.h>

class CWavDecoder :
	public CSoundDecoder
{
public:
	CWavDecoder();
	~CWavDecoder();

	bool Open(LPWSTR file) override;
	bool Close() override;

	int GetMusicChannelsCount()override;
	int GetMusicBitsPerSample()override;
	int GetMusicSampleRate()override;
	double GetMusicLength()override;

	double GetCurSample()override;
	double SeekToSample(double sec)override;
	double SeekToSec(double sec)override;
	size_t Read(void*  _Buffer, size_t _BufferSize, size_t _ElementSize, size_t _ElementCount) override;
	int Seek(long  _Offset, int  _Origin) override;

	double GetCurSec();
	bool IsOpened() override;

private:
	HMMIO hStream=NULL;
	MMIOINFO wavInfo;

	double _BitPerSample = 0;
	int _DataOffset = 0;
	int _FileSize = 0;
	double _FileSec = 0;
	double _CurSec = 0;
	long cur = 0;

	MMCKINFO ckIn;
	WAVEFORMATEX m_pwfx;
	MMCKINFO m_ckRiff;
};

