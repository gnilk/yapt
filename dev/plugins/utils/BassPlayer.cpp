#include <math.h>
#include <string>
#include "yapt/ySystem.h"
#include "BassPlayer.h"
#include "bass.h"

static ILogger *pLogger;

BassPlayer::BassPlayer() {

}
void BassPlayer::Initialize(ISystem *ySys, IPluginObjectInstance *pInstance) {
	char *file = "music.mp3";
	pLogger = ySys->GetLogger("BassMP3Player");

	fileName = pInstance->CreateProperty("mp3file", kPropertyType_String, "music.mp3", "");
	playTime = pInstance->CreateOutputProperty("playtime", kPropertyType_Float, "0.0", "");


	if (HIWORD(BASS_GetVersion())!=BASSVERSION) {
		pLogger->Error("An incorrect version of BASS was loaded: %d",BASS_ErrorGetCode());
		exit(1);
	}

	// initialize default output device
	if (!BASS_Init(-1,44100,0,NULL,NULL)) {
		pLogger->Error("Can't initialize device: %d",BASS_ErrorGetCode());
		exit(1);
	}

	tLast = 0;
	bPlaying = false;

}

void BassPlayer::Render(double t, IPluginObjectInstance *pInstance) {

	if (!bPlaying) {
		chan=BASS_StreamCreateFile(FALSE,fileName->v->string,0,0,BASS_STREAM_PRESCAN);
		if (!chan) {
			pLogger->Error("Unable to open file: %d",BASS_ErrorGetCode());
			exit(1);
		}

		BASS_INFO info;
		if (BASS_GetInfo(&info)) {
			pLogger->Debug("BASS Info");
			pLogger->Debug("  Min buffer size: %d", info.minbuf);
			pLogger->Debug("  Latency: %d", info.latency);
		}

		if (!BASS_ChannelPlay(chan, true)) {
			//Error("BASS_ChannelPlay");
			exit(1);
		}
		bPlaying = true;
		tLast = t;
	} else {
		// somethine made time jump a second, let's adjust the audio
		if (fabs(t - tLast) > 1.0) {
			QWORD bassPos = BASS_ChannelSeconds2Bytes(chan, t);
			BASS_ChannelSetPosition(chan, bassPos, BASS_POS_BYTE);	
		}
		tLast = t;
	}
	QWORD pos = BASS_ChannelGetPosition(chan, BASS_POS_BYTE);
	double tChan = BASS_ChannelBytes2Seconds(chan, pos);
	playTime->v->float_val = tChan;
	//printf("%f\n",tChan);

}

void BassPlayer::PostInitialize(ISystem *ySys, IPluginObjectInstance *pInstance) {

}

void BassPlayer::PostRender(double t, IPluginObjectInstance *pInstance) {

}

void BassPlayer::Signal(int channelId, const char *channelName, int sigval, double sigtime) {

}
