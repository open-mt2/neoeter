#include <MilesLib/StdAfx.h>
#include <MilesLib/SoundManager.h>
#include <MilesLib/SoundData.h>
#include <MilesLib/Type.h>
#include <EterPack/EterPackManager.h>

#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <unistd.h>

namespace {

void test_nsound_type_and_serialization()
{
    NSound::SetResultString("MilesTestOK");
    assert(std::strcmp(NSound::GetResultString(), "MilesTestOK") == 0);

    NSound::TSoundDataVector dataVec(2);
    dataVec[0].fTime = 0.0f;
    dataVec[0].strSoundFileName = "sound/pc/warrior/swing1.wav";
    dataVec[1].fTime = 0.5f;
    dataVec[1].strSoundFileName = "sound/pc/warrior/swing2.wav";

    NSound::TSoundInstanceVector instVec;
    NSound::DataToInstance(dataVec, &instVec);
    assert(instVec.size() == 2);
    assert(instVec[0].dwFrame == 0);
    assert(instVec[1].dwFrame == 29 || instVec[1].dwFrame == 30);
    assert(instVec[1].strSoundFileName == "sound/pc/warrior/swing2.wav");

    const char* tmpPath = "/tmp/test_mileslib_soundinfo.mss";
    bool saved = NSound::SaveSoundInformationPiece(tmpPath, dataVec);
    assert(saved);
    unlink(tmpPath);

    std::printf("[PASS] test_nsound_type_and_serialization\n");
}

void test_sound_manager_lifecycle_and_playback()
{
    // Create a dummy WAV file on disk for 2D/3D sample loading
    const char* wavPath = "/tmp/test_mileslib_sample.wav";
    {
        FILE* fp = std::fopen(wavPath, "wb");
        assert(fp != nullptr);
        const char dummyWav[64] = {'R', 'I', 'F', 'F', 36, 0, 0, 0, 'W', 'A', 'V', 'E', 'f', 'm', 't', ' '};
        std::fwrite(dummyWav, 1, sizeof(dummyWav), fp);
        std::fclose(fp);
    }

    CEterPackManager packMgr;
    CSoundManager soundMgr;

    BOOL created = soundMgr.Create();
    assert(created == TRUE);

    // Volume & scale controls
    soundMgr.SetSoundScale(250.0f);
    assert(std::fabs(soundMgr.GetSoundScale() - 250.0f) < 1e-4f);

    soundMgr.SetSoundVolume(0.8f);
    assert(std::fabs(soundMgr.GetSoundVolume() - 0.8f) < 1e-4f);

    soundMgr.SetMusicVolume(0.6f);
    assert(std::fabs(soundMgr.GetMusicVolume() - 0.6f) < 1e-4f);

    // SaveVolume & RestoreVolume
    soundMgr.SaveVolume();
    assert(std::fabs(soundMgr.GetSoundVolume() - 0.0f) < 1e-4f);
    assert(std::fabs(soundMgr.GetMusicVolume() - 0.0f) < 1e-4f);

    soundMgr.RestoreVolume();
    assert(std::fabs(soundMgr.GetSoundVolume() - 0.8f) < 1e-4f);
    assert(std::fabs(soundMgr.GetMusicVolume() - 0.6f) < 1e-4f);

    // 2D & 3D sound playback
    soundMgr.SetPosition(100.0f, 200.0f, 0.0f);
    soundMgr.SetDirection(0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f);
    soundMgr.PlaySound2D(wavPath);
    soundMgr.PlaySound3D(120.0f, 220.0f, 0.0f, wavPath, 1);

    int ambIdx = soundMgr.PlayAmbienceSound3D(150.0f, 250.0f, 0.0f, wavPath, 1);
    assert(ambIdx >= 0);
    soundMgr.SetSoundVolume3D(ambIdx, 0.5f);
    soundMgr.StopSound3D(ambIdx);
    soundMgr.StopAllSound3D();

    // Music streaming & fade transitions
    soundMgr.FadeInMusic("bgm/login_window.mp3", 0.2f);
    for (int i = 0; i < 5; ++i)
        soundMgr.Update();

    soundMgr.FadeOutMusic("bgm/login_window.mp3", 0.3f);
    for (int i = 0; i < 5; ++i)
        soundMgr.Update();

    soundMgr.Destroy();
    unlink(wavPath);

    std::printf("[PASS] test_sound_manager_lifecycle_and_playback\n");
}

} // namespace

int main()
{
    test_nsound_type_and_serialization();
    test_sound_manager_lifecycle_and_playback();

    std::printf("All MilesLib smoke tests passed!\n");
    return 0;
}
