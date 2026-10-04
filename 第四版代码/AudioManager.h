#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>
#include <QMap>

class AudioManager : public QObject
{
    Q_OBJECT
public:
    explicit AudioManager(QObject* parent = nullptr);
    static AudioManager* instance();

    // 播放配音（一句对话）
    void playVoice(const QString& voiceFile);
    void stopVoice();

    // 播放背景音乐（循环）
    void playBGM(const QString& bgmFile);
    void stopBGM();
    void pauseBGM();
    void resumeBGM();

    // 设置音量 0~100
    void setBGMVolume(int volume);
    void setVoiceVolume(int volume);

signals:
    void voiceFinished();

private slots:
    void onVoiceStateChanged(QMediaPlayer::State state);

private:
    static AudioManager* m_instance;
    QMediaPlayer* m_voicePlayer;
    QMediaPlayer* m_bgmPlayer;
    QMediaPlaylist* m_bgmPlaylist;
    int m_bgmVolume;
    int m_voiceVolume;
};

#endif // AUDIOMANAGER_H

