#include "GameEngine.h"
#include "MainWindow.h"
#include "SceneBase.h"
#include "TitleScene.h"
#include "WardrobeScene.h"
#include "HallScene.h"
#include "LevelScene.h"
#include "EndingScene.h"
#include "GameData.h"
#include "AudioManager.h"

GameEngine* GameEngine::m_instance = nullptr;

GameEngine* GameEngine::instance()
{
    if (!m_instance)
        m_instance = new GameEngine();
    return m_instance;
}

GameEngine::GameEngine(QObject* parent) : QObject(parent)
{
    m_mainWindow = nullptr;
}

void GameEngine::setMainWindow(MainWindow* window)
{
    m_mainWindow = window;
}

void GameEngine::startGame()
{
    // 不再自动读档，由标题界面选择
    changeScene("title");
}

void GameEngine::changeScene(const QString& sceneName, const QVariantMap& params)
{
    if (!m_mainWindow) return;

    SceneBase* scene = createScene(sceneName, params);
    if (scene) {
        m_mainWindow->setScene(scene);
    }
}

SceneBase* GameEngine::createScene(const QString& name, const QVariantMap& params)
{
    if (name == "title") return new TitleScene(m_mainWindow);
    if (name == "wardrobe") return new WardrobeScene(m_mainWindow);
    if (name == "hall") return new HallScene(m_mainWindow);
    if (name == "level") {
        LevelScene* ls = new LevelScene(m_mainWindow);
        if (params.contains("level"))
            ls->setLevel(static_cast<LevelID>(params["level"].toInt()));
        return ls;
    }
    if (name == "ending") {
        EndingScene* es = new EndingScene(m_mainWindow);
        if (params.contains("finalEnding"))
            es->setFinalEnding(static_cast<FinalEnding>(params["finalEnding"].toInt()));
        return es;
    }
    return nullptr;
}