#ifndef GAMEENGINE_H
#define GAMEENGINE_H

#include <QObject>
#include <QString>
#include <QVariantMap>

class MainWindow;
class SceneBase;

class GameEngine : public QObject
{
    Q_OBJECT
public:
    explicit GameEngine(QObject* parent = nullptr);
    static GameEngine* instance();

    void setMainWindow(MainWindow* window);
    void startGame();

    void changeScene(const QString& sceneName, const QVariantMap& params = QVariantMap());

private:
    static GameEngine* m_instance;
    MainWindow* m_mainWindow;

    SceneBase* createScene(const QString& name, const QVariantMap& params);
};

#endif // GAMEENGINE_H

