#include "EndingScene.h"
#include "GameEngine.h"
#include "GameData.h"
#include "AudioManager.h"
#include "EffectManager.h"
#include <QMessageBox>

EndingScene::EndingScene(QWidget* parent) : SceneBase(parent)
{
    m_finalEnding = FINAL_NONE;
    m_index = 0;
    m_effect = new EffectManager(this, this);
}

void EndingScene::setFinalEnding(FinalEnding ending) { m_finalEnding = ending; }

void EndingScene::onEnter()
{
    m_index = 0;
    setBackground("结局页");
    playBGM("结局页.mp3");

    switch (m_finalEnding) {
    case FINAL_BROKEN: runBroken(); break;
    case FINAL_DETAINED: runDetained(); break;
    case FINAL_RECONCILED: runReconciled(); break;
    case FINAL_TRUE: runTrueEnding(); break;
    default: GameEngine::instance()->changeScene("hall"); break;
    }
}

void EndingScene::onExit()
{
    stopBGM();
    AudioManager::instance()->stopVoice();
}

void EndingScene::runBroken()
{
    switch (m_index) {
    case 0:
        hideCharacter();
        playVoice("narrator/narrator_018.mp3");
        m_index = 1;
        break;
    case 1:
        QMessageBox::warning(this, "终章", "【轮回破碎结局】\n强制开启下一周目，番外不解锁。");
        GameData::instance()->resetCurrentRun();
        GameData::instance()->setNewGamePlus(GameData::instance()->getNewGamePlus() + 1);
        GameEngine::instance()->changeScene("title");
        break;
    default: break;
    }
}

void EndingScene::runDetained()
{
    switch (m_index) {
    case 0:
        hideCharacter();
        playVoice("narrator/narrator_019.mp3");
        m_index = 1;
        break;
    case 1:
        QMessageBox::information(this, "终章", "【轮回羁留结局】\n番外不解锁，轮回继续。");
        GameEngine::instance()->changeScene("hall");
        break;
    default: break;
    }
}

void EndingScene::runReconciled()
{
    switch (m_index) {
    case 0:
        playEffect("petal");
        hideCharacter();
        playVoice("narrator/narrator_020.mp3");
        m_index = 1;
        break;
    case 1:
        QMessageBox::information(this, "终章", "【四时和解结局】\n番外不解锁。");
        GameEngine::instance()->changeScene("hall");
        break;
    default: break;
    }
}

void EndingScene::runTrueEnding()
{
    GameData* gd = GameData::instance();
    switch (m_index) {
    case 0:
        playBGM("真结局.mp3");
        playEffect("petal");
        hideCharacter();
        playVoice("narrator/narrator_021.mp3");
        m_index = 1;
        break;
    case 1:
        showCharacter("苏堇纾", "normal");
        playVoice("sujinshu/sujinshu_012.mp3");
        m_index = 2;
        break;
    case 2:
        showCharacter("顾知澜", "normal");
        playVoice("guzhilan/guzhilan_011.mp3");
        m_index = 3;
        break;
    case 3:
        showCharacter("姜寄蘅", "normal");
        playVoice("jiangjiheng/jiangjiheng_010.mp3");
        m_index = 4;
        break;
    case 4:
        showCharacter("谢云绾", "normal");
        playVoice("xieyunwan/xieyunwan_010.mp3");
        m_index = 5;
        break;
    case 5:
        hideCharacter();
        playVoice("narrator/narrator_021.mp3");
        m_index = 6;
        break;
    case 6:
        if (gd->isRareSkin()) {
            playVoice("player/player_017.mp3");
        }
        else {
            playVoice("player/player_018.mp3");
        }
        m_index = 7;
        break;
    case 7:
        QMessageBox::information(this, "真结局", "【真结局：残书补全】\n解锁番外入口，全部皮肤开放。");
        gd->setExtraUnlocked(true);
        gd->setNewGamePlus(gd->getNewGamePlus() + 1);
        for (int i = 0; i < SKIN_COUNT; ++i)
            gd->unlockSkin(static_cast<SkinID>(i));
        m_index = 8;
        break;
    case 8:
        hideCharacter();
        playVoice("narrator/narrator_022.mp3");
        m_index = 9;
        break;
    case 9:
        GameEngine::instance()->changeScene("title");
        break;
    default: break;
    }
}