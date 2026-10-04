#ifndef SCENEBASE_H
#define SCENEBASE_H

#include <QWidget>
#include <QPixmap>
#include <QVariantMap>
#include <QLabel>

class AudioManager;
class EffectManager;
class ResourceManager;

class SceneBase : public QWidget
{
    Q_OBJECT
public:
    explicit SceneBase(QWidget* parent = nullptr);
    virtual ~SceneBase();

    virtual void onEnter() = 0;
    virtual void onExit() = 0;

    void setBackground(const QString& bgName);
    void setBackground(const QPixmap& pixmap);
    void showCharacter(const QString& name, const QString& state, int x = -1, int y = -1);
    void showCharacterPixmap(const QPixmap& pix, int x = -1, int y = -1);
    void hideCharacter();
    void playBGM(const QString& bgmName);
    void stopBGM();
    void playVoice(const QString& voiceFile);
    void playEffect(const QString& effectType);

signals:
    void changeScene(const QString& sceneName);
    void changeScene(const QString& sceneName, const QVariantMap& params);

protected:
    void paintEvent(QPaintEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;

    QPixmap m_bgPixmap;
    QLabel* m_charDisplay;
    int m_charX, m_charY;

    AudioManager* m_audio;
    EffectManager* m_effect;
    ResourceManager* m_res;
};

#endif // SCENEBASE_H

