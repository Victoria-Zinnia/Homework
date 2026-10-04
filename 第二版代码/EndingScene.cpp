#include "EndingScene.h"
#include "GameEngine.h"
#include "GameData.h"
#include "AudioManager.h"
#include "EffectManager.h"
#include "DialogBox.h"
#include "ResourceManager.h"
#include <QMessageBox>

EndingScene::EndingScene(QWidget* parent) : SceneBase(parent)
{
    m_finalEnding = FINAL_NONE;
    m_index = 0;
    m_effect = new EffectManager(this, this);
    m_dialog = new DialogBox(this);
    connect(m_dialog, &DialogBox::clicked, this, &EndingScene::onDialogClicked);
}

void EndingScene::setFinalEnding(FinalEnding ending) { m_finalEnding = ending; }

void EndingScene::onEnter()
{
    m_index = 0;
    setBackground(QString::fromUtf8(u8"结局页"));
    playBGM(QString::fromUtf8(u8"结局页.mp3"));

    m_dialog->setFixedHeight(200);
    updateDialogPosition();
    m_dialog->show();
    m_dialog->raise();

    advanceEnding();
}

void EndingScene::onExit()
{
    stopBGM();
    AudioManager::instance()->stopVoice();
    if (m_dialog) {
        m_dialog->clear();
    }
}

void EndingScene::onDialogClicked()
{
    if (m_dialog->isTyping()) {
        m_dialog->skipTyping();
        return;
    }
    AudioManager::instance()->stopVoice();
    advanceEnding();
}

void EndingScene::advanceEnding()
{
    switch (m_finalEnding) {
    case FINAL_BROKEN: runBroken(); break;
    case FINAL_DETAINED: runDetained(); break;
    case FINAL_RECONCILED: runReconciled(); break;
    case FINAL_TRUE: runTrueEnding(); break;
    default: GameEngine::instance()->changeScene(QString::fromUtf8(u8"hall")); break;
    }
}

void EndingScene::showLine(const QString& speaker, const QString& text, const QString& voice)
{
    m_dialog->setText(text);
    m_dialog->setVoice(voice);

    QPixmap bgPix = ResourceManager::instance()->getImage(QString::fromUtf8(u8"assets/images/ui/对话框底框.png"));
    if (!bgPix.isNull()) {
        m_dialog->setBackgroundImage(bgPix);
    }

    if (speaker == QString::fromUtf8(u8"玩家")) {
        m_dialog->setSpeaker(GameData::instance()->getSkinInfo(GameData::instance()->currentSkin()).name);
        m_dialog->setSpeakerSide(DialogBox::SideLeft);
        SkinInfo info = GameData::instance()->getSkinInfo(GameData::instance()->currentSkin());
        QPixmap pix = ResourceManager::instance()->getSkin(info.imageFile);
        showCharacterPixmap(pix, 50, -1);
    }
    else if (!speaker.isEmpty()) {
        m_dialog->setSpeaker(speaker);
        m_dialog->setSpeakerSide(DialogBox::SideRight);
        showCharacter(speaker, QString::fromUtf8(u8"normal"));
    }

    updateDialogPosition();
    m_dialog->startTyping();
    m_dialog->raise();
}

void EndingScene::showNarration(const QString& text, const QString& voice)
{
    hideCharacter();
    m_dialog->setSpeaker(QString());
    m_dialog->setText(text);
    m_dialog->setVoice(voice);

    QPixmap bgPix = ResourceManager::instance()->getImage(QString::fromUtf8(u8"assets/images/ui/对话框底框.png"));
    if (!bgPix.isNull()) {
        m_dialog->setBackgroundImage(bgPix);
    }

    m_dialog->setSpeakerSide(DialogBox::SideCenter);
    updateDialogPosition();
    m_dialog->startTyping();
    m_dialog->raise();
}

void EndingScene::updateDialogPosition()
{
    int dialogH = 200;
    int y = height() - dialogH - 20;
    m_dialog->setGeometry(50, y, width() - 100, dialogH);
}

void EndingScene::resizeEvent(QResizeEvent* event)
{
    SceneBase::resizeEvent(event);
    updateDialogPosition();
}

void EndingScene::runBroken()
{
    switch (m_index) {
    case 0:
        hideCharacter();
        showNarration(QString::fromUtf8(u8"轮回破碎，再无归途。一切归于虚无，下一周目即将开启……"),
            QString::fromUtf8(u8"narrator/narrator_018.mp3"));
        m_index = 1;
        break;
    case 1:
        QMessageBox::warning(this, QString::fromUtf8(u8"终章"), QString::fromUtf8(u8"【轮回破碎结局】\n强制开启下一周目，番外不解锁。"));
        GameData::instance()->resetCurrentRun();
        GameData::instance()->setNewGamePlus(GameData::instance()->getNewGamePlus() + 1);
        GameEngine::instance()->changeScene(QString::fromUtf8(u8"title"));
        break;
    default: break;
    }
}

void EndingScene::runDetained()
{
    switch (m_index) {
    case 0:
        hideCharacter();
        showNarration(QString::fromUtf8(u8"轮回羁留，此身难脱。四时流转，仍困于此……"),
            QString::fromUtf8(u8"narrator/narrator_019.mp3"));
        m_index = 1;
        break;
    case 1:
        QMessageBox::information(this, QString::fromUtf8(u8"终章"), QString::fromUtf8(u8"【轮回羁留结局】\n番外不解锁，轮回继续。"));
        GameEngine::instance()->changeScene(QString::fromUtf8(u8"hall"));
        break;
    default: break;
    }
}

void EndingScene::runReconciled()
{
    switch (m_index) {
    case 0:
        playEffect(QString::fromUtf8(u8"petal"));
        hideCharacter();
        showNarration(QString::fromUtf8(u8"四时和解，花开彼岸。然番外之扉，尚未开启……"),
            QString::fromUtf8(u8"narrator/narrator_020.mp3"));
        m_index = 1;
        break;
    case 1:
        QMessageBox::information(this, QString::fromUtf8(u8"终章"), QString::fromUtf8(u8"【四时和解结局】\n番外不解锁。"));
        GameEngine::instance()->changeScene(QString::fromUtf8(u8"hall"));
        break;
    default: break;
    }
}

void EndingScene::runTrueEnding()
{
    GameData* gd = GameData::instance();
    switch (m_index) {
    case 0:
        playBGM(QString::fromUtf8(u8"真结局.mp3"));
        playEffect(QString::fromUtf8(u8"petal"));
        hideCharacter();
        showNarration(QString::fromUtf8(u8"残书补全，轮回终章。四时的记忆汇聚于此，真相即将揭晓……"),
            QString::fromUtf8(u8"narrator/narrator_021.mp3"));
        m_index = 1;
        break;
    case 1:
        showLine(QString::fromUtf8(u8"苏堇纾"),
            QString::fromUtf8(u8"春之庭的花，终于等到了归人。"),
            QString::fromUtf8(u8"sujinshu/sujinshu_012.mp3"));
        m_index = 2;
        break;
    case 2:
        showLine(QString::fromUtf8(u8"顾知澜"),
            QString::fromUtf8(u8"夏之阁的江风，也吹向了远方。"),
            QString::fromUtf8(u8"guzhilan/guzhilan_011.mp3"));
        m_index = 3;
        break;
    case 3:
        showLine(QString::fromUtf8(u8"姜寄蘅"),
            QString::fromUtf8(u8"秋之祠的药香，渡过了最后一程。"),
            QString::fromUtf8(u8"jiangjiheng/jiangjiheng_010.mp3"));
        m_index = 4;
        break;
    case 4:
        showLine(QString::fromUtf8(u8"谢云绾"),
            QString::fromUtf8(u8"冬之坛的雪，化作了春泥。"),
            QString::fromUtf8(u8"xieyunwan/xieyunwan_010.mp3"));
        m_index = 5;
        break;
    case 5:
        hideCharacter();
        showNarration(QString::fromUtf8(u8"四时的执念皆已释然，轮回之庭的封印，正在缓缓打开……"),
            QString::fromUtf8(u8"narrator/narrator_021.mp3"));
        m_index = 6;
        break;
    case 6:
        if (gd->isRareSkin()) {
            showLine(QString::fromUtf8(u8"玩家"),
                QString::fromUtf8(u8"这一程，我终以完整的自己，走到了这里。"),
                QString::fromUtf8(u8"player/player_017.mp3"));
        }
        else {
            showLine(QString::fromUtf8(u8"玩家"),
                QString::fromUtf8(u8"这一程，我终以完整的自己，走到了这里。"),
                QString::fromUtf8(u8"player/player_018.mp3"));
        }
        m_index = 7;
        break;
    case 7:
        QMessageBox::information(this, QString::fromUtf8(u8"真结局"), QString::fromUtf8(u8"【真结局：残书补全】\n解锁番外入口，全部皮肤开放。"));
        gd->setExtraUnlocked(true);
        gd->setNewGamePlus(gd->getNewGamePlus() + 1);
        for (int i = 0; i < SKIN_COUNT; ++i)
            gd->unlockSkin(static_cast<SkinID>(i));
        m_index = 8;
        break;
    case 8:
        hideCharacter();
        showNarration(QString::fromUtf8(u8"轮回未竟，四时已全。新的篇章，等待书写……"),
            QString::fromUtf8(u8"narrator/narrator_022.mp3"));
        m_index = 9;
        break;
    case 9:
        GameEngine::instance()->changeScene(QString::fromUtf8(u8"title"));
        break;
    default: break;
    }
}