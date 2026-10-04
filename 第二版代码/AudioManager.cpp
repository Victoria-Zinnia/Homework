#include "AudioManager.h"
#include <QDebug>
#include <QUrl>

AudioManager* AudioManager::m_instance = nullptr;

AudioManager* AudioManager::instance()
{
    if (!m_instance)
        m_instance = new AudioManager();
    return m_instance;
}

AudioManager::AudioManager(QObject* parent) : QObject(parent)
{
    m_bgmVolume = 40;
    m_voiceVolume = 85;

    m_voicePlayer = new QMediaPlayer(this);
    m_bgmPlayer = new QMediaPlayer(this);
    m_bgmPlaylist = new QMediaPlaylist(this);

    m_bgmPlayer->setPlaylist(m_bgmPlaylist);
    m_bgmPlaylist->setPlaybackMode(QMediaPlaylist::Loop);

    m_voicePlayer->setVolume(m_voiceVolume);
    m_bgmPlayer->setVolume(m_bgmVolume);

    connect(m_voicePlayer, &QMediaPlayer::stateChanged,
        this, &AudioManager::onVoiceStateChanged);
}

void AudioManager::playVoice(const QString& voiceFile)
{
    if (voiceFile.isEmpty()) return;
    // 路径：assets/audio/voice/角色/文件名.mp3
    QString path = "assets/audio/voice/" + voiceFile;
    m_voicePlayer->setMedia(QUrl::fromLocalFile(path));
    m_voicePlayer->play();
}

void AudioManager::stopVoice()
{
    m_voicePlayer->stop();
}

void AudioManager::playBGM(const QString& bgmFile)
{
    if (bgmFile.isEmpty()) return;
    // 路径：assets/audio/bgm/文件名.mp3
    QString path = "assets/audio/bgm/" + bgmFile;
    m_bgmPlaylist->clear();
    m_bgmPlaylist->addMedia(QUrl::fromLocalFile(path));
    m_bgmPlayer->play();
}

void AudioManager::stopBGM()
{
    m_bgmPlayer->stop();
}

void AudioManager::pauseBGM()
{
    m_bgmPlayer->pause();
}

void AudioManager::resumeBGM()
{
    m_bgmPlayer->play();
}

void AudioManager::setBGMVolume(int volume)
{
    m_bgmVolume = qBound(0, volume, 100);
    m_bgmPlayer->setVolume(m_bgmVolume);
}

void AudioManager::setVoiceVolume(int volume)
{
    m_voiceVolume = qBound(0, volume, 100);
    m_voicePlayer->setVolume(m_voiceVolume);
}

void AudioManager::onVoiceStateChanged(QMediaPlayer::State state)
{
    if (state == QMediaPlayer::StoppedState) {
        emit voiceFinished();
    }
}