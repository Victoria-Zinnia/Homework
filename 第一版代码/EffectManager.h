#ifndef EFFECTMANAGER_H
#define EFFECTMANAGER_H

#include <QObject>
#include <QWidget>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>
#include <QList>
#include <QPointer>
#include <functional>
#include <QLabel>

class EffectManager : public QObject
{
    Q_OBJECT
public:
    explicit EffectManager(QWidget* parentWidget, QObject* parent = nullptr);
    ~EffectManager();

    void fadeIn(QWidget* widget, int duration = 1000);
    void fadeOut(QWidget* widget, int duration = 1000, std::function<void()> onFinished = nullptr);
    void flashback(QWidget* widget, int duration = 1500);
    void startParticleEffect(const QString& type);
    void stopParticleEffect();
    void blurBackground(QWidget* bgWidget, bool enable);
    void ghostFadeOut(QWidget* charWidget, int duration = 2000);
    void screenShake(QWidget* widget, int intensity = 10, int duration = 500);
    void blackFadeIn(QWidget* overlay, int duration = 2000);

signals:
    void effectFinished();

private:
    QWidget* m_parentWidget;
    QTimer* m_particleTimer;
    QList<QPointer<QLabel>> m_particles;
    void createParticle(const QString& type);
};

#endif // EFFECTMANAGER_H