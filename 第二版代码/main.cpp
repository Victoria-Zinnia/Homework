#include <QApplication>
#include "MainWindow.h"
#include "GameEngine.h"
#include "GameData.h"
#include "AudioManager.h"
#include "ResourceManager.h"

int main(int argc, char* argv[])
{
    QApplication a(argc, argv);

    GameData::instance();
    AudioManager::instance();
    ResourceManager::instance();

    MainWindow w;
    GameEngine::instance()->setMainWindow(&w);
    GameEngine::instance()->startGame();

    // 窗口模式运行（1280x720），中文正常后再改全屏
    w.resize(1280, 720);
    w.show();

    return a.exec();
}