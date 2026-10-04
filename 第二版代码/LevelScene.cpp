#include "LevelScene.h"
#include "GameEngine.h"
#include "GameData.h"
#include "AudioManager.h"
#include "EffectManager.h"
#include "ResourceManager.h"
#include <QMouseEvent>
#include <QMessageBox>
#include <QEventLoop>
#include <QApplication>

LevelScene::LevelScene(QWidget* parent) : SceneBase(parent)
{
    m_level = LEVEL_NONE;
    m_index = 0;
    m_inChoice = false;
    m_inCG = false;
    m_aPre = m_bPre = m_dPre = false;
    m_hide = false;
    m_pendingEnding = ENDING_NONE;
    m_npcState = "corrupt";

    m_dialog = new DialogBox(this);
    m_choice = new ChoiceMenu(this);
    m_popup = new ItemPopup(this);
    m_cg = new CGPlayer(this);

    connect(m_dialog, &DialogBox::clicked, this, &LevelScene::onDialogClicked);
    connect(m_choice, &ChoiceMenu::choiceSelected, this, &LevelScene::onChoiceMade);
    connect(m_cg, &CGPlayer::cgFinished, this, &LevelScene::onCGFinished);

    m_effect = new EffectManager(this, this);
    m_itemGame = new ItemMiniGame(this);
    connect(m_itemGame, &ItemMiniGame::gameFinished, this, &LevelScene::onMiniGameFinished);
    connect(m_itemGame, &ItemMiniGame::gameSkipped, this, &LevelScene::onMiniGameSkipped);
    m_inMiniGame = false;
    m_postMiniGameIndex = 0;
}

void LevelScene::setLevel(LevelID level) { m_level = level; }

void LevelScene::onEnter()
{
    setGraphicsEffect(nullptr);
    m_effect->stopParticleEffect();

    m_inMiniGame = false;
    m_index = 0;
    m_inChoice = false;
    m_inCG = false;
    m_aPre = m_bPre = m_dPre = false;
    m_hide = false;
    m_pendingEnding = ENDING_NONE;
    m_items.clear();
    m_npcState = "corrupt";

    // 新增：进入关卡时锁定当前皮肤，防止中途被其他逻辑覆盖
    m_playerSkinId = GameData::instance()->currentSkin();

    GameData* gd = GameData::instance();
    gd->setHideFlag(m_level, false);
    gd->setDPreFlag(m_level, false);

    m_dialog->setFixedHeight(280);
    updateDialogPosition();

    m_choice->setGeometry((width() - 600) / 2, height() / 2 + 50, 600, 300);
    m_cg->setGeometry(0, 0, width(), height());

    advanceScript();
}

void LevelScene::onExit()
{
    stopBGM();
    if (m_effect) m_effect->stopParticleEffect();
    AudioManager::instance()->stopVoice();

    // 关键：强制停止所有可能残留的动画和定时器
    for (QObject* child : findChildren<QObject*>()) {
        QAbstractAnimation* anim = qobject_cast<QAbstractAnimation*>(child);
        if (anim) anim->stop();
        QTimer* timer = qobject_cast<QTimer*>(child);
        if (timer) timer->stop();
    }

    // 关键：强制移除所有 GraphicsEffect，防止删除时访问无效 effect
    for (QWidget* w : findChildren<QWidget*>()) {
        w->setGraphicsEffect(nullptr);
    }
}

void LevelScene::mousePressEvent(QMouseEvent* event)
{
    if (m_inChoice || m_inCG) return;
    QWidget::mousePressEvent(event);
    if (m_inChoice || m_inCG || m_inMiniGame) return;
}

void LevelScene::onDialogClicked()
{
    if (m_inChoice || m_inCG || m_inMiniGame) return;
    if (m_dialog->isTyping()) {
        m_dialog->skipTyping();
        return;
    }
    AudioManager::instance()->stopVoice();
    advanceScript();
    if (m_inChoice || m_inCG || m_inMiniGame) return;
}

void LevelScene::onChoiceMade(int index)
{
    m_inChoice = false;
    m_choice->hideChoices();

    switch (m_level) {
    case LEVEL_SPRING:
        if (m_index == 101) {
            if (index == 0) { m_aPre = true; m_index = 200; }
            else if (index == 1) { m_bPre = true; m_index = 210; }
            else if (index == 2) { m_index = 220; }
            else if (index == 3) { m_index = 230; }
        }
        break;
    case LEVEL_SUMMER:
        if (m_index == 101) {
            if (index == 0) { m_aPre = true; m_index = 200; }
            else if (index == 1) { m_bPre = true; m_index = 210; }
            else if (index == 2) { m_index = 220; }
            else if (index == 3) { m_index = 230; }
        }
        break;
    case LEVEL_AUTUMN:
        if (m_index == 101) {
            if (index == 0) { m_aPre = true; m_index = 200; }
            else if (index == 1) { m_bPre = true; m_index = 210; }
            else if (index == 2) { m_index = 220; }
            else if (index == 3) { m_index = 230; }
        }
        break;
    case LEVEL_WINTER:
        if (m_index == 101) {
            if (index == 0) { m_aPre = true; m_index = 200; }
            else if (index == 1) { m_bPre = true; m_index = 210; }
            else if (index == 2) { m_index = 220; }
            else if (index == 3) { m_index = 230; }
        }
        break;
    default: break;
    }
    advanceScript();
}

void LevelScene::onCGFinished()
{
    m_inCG = false;
    advanceScript();
}

void LevelScene::advanceScript()
{
    if (m_inChoice || m_inCG) return;
    switch (m_level) {
    case LEVEL_SPRING: runSpring(); break;
    case LEVEL_SUMMER: runSummer(); break;
    case LEVEL_AUTUMN: runAutumn(); break;
    case LEVEL_WINTER: runWinter(); break;
    default: break;
    }
}

void LevelScene::showLine(const QString& speaker, const QString& text, const QString& voice)
{
    QString displayName = speaker;

    if (speaker == QString::fromUtf8(u8"玩家")) {
        displayName = GameData::instance()->getSkinInfo(m_playerSkinId).name;
        m_dialog->setSpeakerSide(DialogBox::SideLeft);
        showPlayer();
    }
    else {
        m_dialog->setSpeakerSide(DialogBox::SideRight);
        showCharacter(speaker, m_npcState);
    }

    m_dialog->setSpeaker(displayName);
    m_dialog->setText(text);
    m_dialog->setVoice(voice);

    QPixmap bgPix = ResourceManager::instance()->getImage(QString::fromUtf8(u8"assets/images/ui/对话框底框.png"));
    if (!bgPix.isNull()) {
        m_dialog->setBackgroundImage(bgPix);
    }

    updateDialogPosition();
    m_dialog->startTyping();
    m_dialog->raise();
}

void LevelScene::showNarration(const QString& text, const QString& voice)
{
    hideCharacter();
    m_dialog->setSpeaker("");
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

void LevelScene::showPlayer()
{
    SkinInfo info = GameData::instance()->getSkinInfo(m_playerSkinId);
    QPixmap pix = ResourceManager::instance()->getSkin(info.imageFile);
    showCharacterPixmap(pix, 50, -1);
}

void LevelScene::showOption(const QStringList& options)
{
    AudioManager::instance()->stopVoice();
    m_inChoice = true;
    m_choice->setChoices(options);
    int ch = m_choice->height();
    int y = qMax(80, (height() - ch) / 2);
    m_choice->setGeometry((width() - m_choice->width()) / 2, y, m_choice->width(), ch);

    m_choice->showChoices();
    m_choice->raise();
}

void LevelScene::obtainItem(const QString& itemName)
{
    m_items.insert(itemName);
    m_popup->showItem(itemName);
}

void LevelScene::showCG(const QString& cgName)
{
    AudioManager::instance()->stopVoice();
    m_inCG = true;
    m_cg->playCG(cgName);
}

void LevelScene::backToHall()
{
    // 关键：确保所有异步事件处理完再切换场景
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    GameEngine::instance()->changeScene("hall");
}

void LevelScene::triggerEnding(EndingType type)
{
    m_pendingEnding = type;
    GameData::instance()->setLevelEnding(m_level, type);
    if (type == ENDING_C_CORRUPT)
        GameData::instance()->setAnyCorrupt(true);
}

void LevelScene::updateDialogPosition()
{
    int dialogH = 280;
    int y = height() - dialogH;
    int dialogW = 0;
    int x = 0;
    int charReserve = 420;

    m_dialog->setFixedHeight(dialogH);

    switch (m_dialog->speakerSide()) {
    case DialogBox::SideLeft:
        x = charReserve;
        dialogW = width() - x - 15;
        break;
    case DialogBox::SideRight:
        x = 15;
        dialogW = width() - charReserve - x;
        break;
    case DialogBox::SideCenter:
    default:
        x = 15;
        dialogW = width() - 30;
        break;
    }

    if (dialogW < 300) dialogW = 300;
    m_dialog->setGeometry(x, y, dialogW, dialogH);
    m_dialog->raise();
}

void LevelScene::resizeEvent(QResizeEvent* event)
{
    SceneBase::resizeEvent(event);

    if (m_dialog) {
        updateDialogPosition();
    }
    if (m_choice) {
        m_choice->move((width() - m_choice->width()) / 2, height() / 2 + 50);
    }
    if (m_cg) {
        m_cg->setGeometry(0, 0, width(), height());
    }
}

void LevelScene::startItemMiniGame(const QStringList& items, int nextIndex, const QString& bgName)
{
    AudioManager::instance()->stopVoice();
    m_inMiniGame = true;
    m_postMiniGameIndex = nextIndex;
    m_itemGame->startGame(items, bgName);
}

void LevelScene::onMiniGameFinished(const QStringList& items)
{
    for (const QString& item : items) {
        obtainItem(item);
    }
    m_inMiniGame = false;
    m_index = m_postMiniGameIndex;
    advanceScript();
}

void LevelScene::onMiniGameSkipped(const QStringList& items)
{
    for (const QString& item : items) {
        obtainItem(item);
    }
    m_inMiniGame = false;
    m_index = m_postMiniGameIndex;
    advanceScript();
}

// ==================== S1 春之庭 ====================
void LevelScene::runSpring()
{
    GameData* gd = GameData::instance();
    switch (m_index) {
    case 0: {
        setBackground(QString::fromUtf8(u8"春之庭_黑化"));
        playBGM(QString::fromUtf8(u8"春之庭.mp3"));
        showCharacter(QString::fromUtf8(u8"苏堇纾"), "corrupt");
        if (gd->isDefaultSkin()) m_index = 10;
        else if (gd->isSpringSkin()) m_index = 20;
        else m_index = 30;
        showNarration(QString::fromUtf8(u8"春之庭被不祥的花影笼罩，空气中弥漫着凋零的气息……"), "");
        break;
    }
    case 10:
        showNarration(
            QString::fromUtf8(u8"你踏入春之庭。四丈高墙将天地切割成一方小院。\n"
                u8"繁花肆意盛放，春风不断吹动花枝，可是这里没有出口。\n"
                u8"此地春光永不凋零，居住在这里的人，却永远回不到故土。\n"
                u8"花海中央，一名素衣女子静静坐在花丛之间，手指轻轻抚过花瓣，眼神是长久的惘然。"),
            QString::fromUtf8(u8"narrator/narrator_003.mp3"));
        m_index = 11;
        break;
    case 11:
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"同是庭中花草，你得朝阳滋养。我一心盼一份宽解，却辗转无处安栖。"), QString::fromUtf8(u8"sujinshu/sujinshu_001.mp3"));
        m_index = 100;
        break;
    case 20:
        playEffect("flashback");
        showNarration(QString::fromUtf8(u8"你踏入春之庭。高墙依旧禁锢这片花海。当你脚步落下，周遭的春花微微向内收拢，仿佛在回应你的到来。花海中央，苏堇纾猛地抬眼望向你。"), QString::fromUtf8(u8"narrator/narrator_004.mp3"));
        m_index = 21;
        break;
    case 21:
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"你身上有和我同源的春息……你不是过客，是我遗失许久的本心。我能感觉到，你懂我的无根、懂我的漂泊、懂我年年春开、年年无归。"), QString::fromUtf8(u8"sujinshu/sujinshu_002.mp3"));
        m_hide = true;
        gd->setHideFlag(LEVEL_SPRING, true);
        m_index = 100;
        break;
    case 30:
        showNarration(QString::fromUtf8(u8"你踏入春之庭。高墙锁死花海。春风吹拂，但周遭花朵对你没有任何呼应。苏堇纾望向你，神色疏离淡漠。"), QString::fromUtf8(u8"narrator/narrator_005.mp3"));
        m_index = 31;
        break;
    case 31:
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"你身上不是春风气息。你见过别的时节，不会懂我被困一春、一生无栖的苦。"), QString::fromUtf8(u8"sujinshu/sujinshu_003.mp3"));
        m_dPre = false;
        m_index = 100;
        break;

    case 100:
        showNarration(QString::fromUtf8(u8"你打算如何回应苏堇纾？"), "");
        m_index = 101;
        break;
    case 101:
        showOption(QStringList{
            QString::fromUtf8(u8"共情体恤"),
            QString::fromUtf8(u8"平淡客观"),
            QString::fromUtf8(u8"破局强攻"),
            QString::fromUtf8(u8"命格深问")
            });
        break;

    case 200:
        setBackground(QString::fromUtf8(u8"春之庭"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"你盛放于春，却从未拥有属于自己的春。"), QString::fromUtf8(u8"player/player_001.mp3"));
        m_index = 201;
        break;
    case 201:
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"难得有人看见我的飘零，而不是只看见我的花。我年年开花、年年守候，原来终有人懂我不是贪恋繁花，只是贪恋一寸安稳。"), QString::fromUtf8(u8"sujinshu/sujinshu_004.mp3"));
        m_aPre = true;
        m_index = 300;
        break;

    case 210:
        setBackground(QString::fromUtf8(u8"春之庭"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"这座庭院困住你很久了。"), QString::fromUtf8(u8"player/player_002.mp3"));
        m_index = 211;
        break;
    case 211:
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"人人知我被困，无人问我苦久。世人只叹庭院幽深，从不在意我一生无处落脚。"), QString::fromUtf8(u8"sujinshu/sujinshu_005.mp3"));
        m_bPre = true;
        m_index = 300;
        break;

    case 220:
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"拆掉高墙，直接离开即可。"), QString::fromUtf8(u8"player/player_003.mp3"));
        m_index = 221;
        break;
    case 221:
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"不要毁掉这里……这是我唯一的方寸栖身之地。若连花海也碎，我便真的一无所有了。"), QString::fromUtf8(u8"sujinshu/sujinshu_006.mp3"));
        m_index = 222;
        break;
    case 222:
        triggerEnding(ENDING_C_CORRUPT);
        m_index = 500;
        runSpring();
        break;

    case 230:
        setBackground(QString::fromUtf8(u8"春之庭"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"你本是春草，为何偏偏不得春风？"), QString::fromUtf8(u8"player/player_004.mp3"));
        m_index = 231;
        break;
    case 231:
        if (gd->isSpringSkin() || gd->isDefaultSkin()) {
            showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"原来真的有人敢问我心底最深的执念。我生来如春草温柔，命却无根。岁岁逢春，岁岁落空。"), QString::fromUtf8(u8"sujinshu/sujinshu_008.mp3"));
            m_dPre = true;
        }
        else {
            showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"四时殊途，说了你也不能体会。"), QString::fromUtf8(u8"sujinshu/sujinshu_007.mp3"));
            m_dPre = false;
        }
        m_index = 300;
        break;

    case 300:
        showNarration(QString::fromUtf8(u8"【交互任务】\n记忆碎片散落在虚空中，请靠近它们以完成共鸣……"), "");
        m_index = 301;
        break;
    case 301:
        startItemMiniGame(QStringList{
            QString::fromUtf8(u8"花残瓣"),
            QString::fromUtf8(u8"青芜绳"),
            QString::fromUtf8(u8"春时旧簪")
            }, 304, QString::fromUtf8(u8"春之庭"));
        break;
    case 304:
        showNarration(QString::fromUtf8(u8"三件道具共鸣完成，主线达成。"), "");
        m_index = 400;
        break;

    case 400:
        if (m_pendingEnding == ENDING_C_CORRUPT) { m_index = 500; runSpring(); }
        else if (m_dPre) { m_index = 430; runSpring(); }      // D 路线独立
        else if (m_aPre) { m_index = 410; runSpring(); }      // A 路线
        else if (m_bPre) { m_index = 420; runSpring(); }      // B 路线
        else { m_index = 420; runSpring(); }                  // 保底
        break;

        // ---- A 结局路线（与夏秋冬统一）----
    case 410:
        playEffect("petal");
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"原来我的存在，终被人完整看见。漂泊无根的苦到此为止，我可以安心归于这片春光。"), QString::fromUtf8(u8"sujinshu/sujinshu_011.mp3"));
        m_index = 411;
        break;

    case 411:
        // 结局演出前强制清理所有后台特效，避免与 ghostFadeOut 冲突
        m_effect->stopParticleEffect();
        setGraphicsEffect(nullptr);

        m_effect->ghostFadeOut(m_dialog, 2000);
        m_dialog->setEnabled(false);

        QTimer::singleShot(2100, this, [this]() {
            // 安全检查：如果玩家极速点击导致场景已切换，直接返回
            if (!m_dialog) return;

            AudioManager::instance()->stopVoice();
            triggerEnding(ENDING_A_PERFECT);
            QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得春记忆碎片、称号【沐春知栖】；解锁普通皮陆昭芃·春栖"));
            GameData::instance()->unlockSkin(SKIN_SPRING_NORMAL);

            // 恢复对话框位置（防止 ghostFadeOut 的 rise 动画把它移出屏幕）
            updateDialogPosition();
            m_dialog->setEnabled(true);
            m_index = 412;
            });
        break;

    case 412:
        backToHall();
        break;

    case 430:
        showNarration(QString::fromUtf8(u8"不止看见她被困的现在，你看见了她未曾被囚禁的从前。"), QString::fromUtf8(u8"narrator/narrator_007.mp3"));
        m_index = 431;
        break;
    case 431:
        showCG(QString::fromUtf8(u8"CG_S1回忆"));
        m_index = 432;
        break;
    case 432:
        gd->setDFlag(LEVEL_SPRING, true);
        QMessageBox::information(this, QString::fromUtf8(u8"彩蛋"), QString::fromUtf8(u8"解锁典藏皮陆昭芃·无根"));
        gd->unlockSkin(SKIN_SPRING_RARE);
        backToHall();
        break;

    case 420:
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"记忆寻回了，可属于我的安稳依旧不曾到来。"), QString::fromUtf8(u8"sujinshu/sujinshu_010.mp3"));
        m_index = 421;
        break;
    case 421:
        triggerEnding(ENDING_B_NEUTRAL);
        QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得春记忆碎片，不解锁皮肤。"));
        backToHall();
        break;

    case 500:
        setBackground(QString::fromUtf8(u8"春之庭_黑化"));
        playBGM(QString::fromUtf8(u8"春之庭_黑化.mp3"));
        showLine(QString::fromUtf8(u8"苏堇纾"), QString::fromUtf8(u8"连仅存的春光，也尽数消散了。"), QString::fromUtf8(u8"sujinshu/sujinshu_009.mp3"));
        m_index = 501;
        break;
    case 501:
        QMessageBox::warning(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"崩坏失败·庭毁花零，无奖励。"));
        backToHall();
        break;

    default: break;
    }
}

// ==================== S2 夏之阁 ====================
void LevelScene::runSummer()
{
    GameData* gd = GameData::instance();
    switch (m_index) {
    case 0: {
        setBackground(QString::fromUtf8(u8"夏之阁_黑化"));
        playBGM(QString::fromUtf8(u8"夏之阁.mp3"));
        showCharacter(QString::fromUtf8(u8"顾知澜"), "corrupt");
        if (gd->isDefaultSkin()) m_index = 10;
        else if (gd->isSummerSkin()) m_index = 20;
        else m_index = 30;
        showNarration(QString::fromUtf8(u8"夏之阁被狂乱的江风撕扯，书稿散落一地，江水倒灌……"), "");
        break;
    }
    case 10:
        showNarration(QString::fromUtf8(u8"你踏入夏之阁。闷热的江风穿堂而过。纱帘被风浪反复吹动。窗外大江奔涌，一望无际。这座楼阁可以看见世间壮阔水景，却不允许居住者真正走向江水。顾知澜凭栏而立，默默望向滚滚流水。"), QString::fromUtf8(u8"narrator/narrator_008.mp3"));
        m_index = 11;
        break;
    case 11:
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"我知世间江河浩荡，奈何身有桎梏，只可坐看波澜远去。"), QString::fromUtf8(u8"guzhilan/guzhilan_001.mp3"));
        m_index = 100;
        break;
    case 20:
        playEffect("ripple");
        showNarration(QString::fromUtf8(u8"你踏入夏之阁。江风骤然柔和几分。顾知澜猛地转头看向你，眼中有惊讶，还有一种相逢知己的震动。"), QString::fromUtf8(u8"narrator/narrator_009.mp3"));
        m_index = 21;
        break;
    case 21:
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"你眼底有和我相同的流水远志。你也见过波澜、心向四方，你懂我不是不甘阁楼，是不甘此生受限。"), QString::fromUtf8(u8"guzhilan/guzhilan_002.mp3"));
        m_hide = true;
        gd->setHideFlag(LEVEL_SUMMER, true);
        m_index = 100;
        break;
    case 30:
        showNarration(QString::fromUtf8(u8"你踏入夏之阁。江水滔滔，楼阁寂静。顾知澜看了你一眼，随即重新望向江面，态度疏远。"), QString::fromUtf8(u8"narrator/narrator_010.mp3"));
        m_index = 31;
        break;
    case 31:
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"你未见我所见江河，便不懂我半生困惘。"), QString::fromUtf8(u8"guzhilan/guzhilan_003.mp3"));
        m_dPre = false;
        m_index = 100;
        break;

    case 100:
        showNarration(QString::fromUtf8(u8"你准备如何回应顾知澜？"), "");
        m_index = 101;
        break;
    case 101:
        showOption(QStringList{
            QString::fromUtf8(u8"共情知心"),
            QString::fromUtf8(u8"客观平淡"),
            QString::fromUtf8(u8"指责软弱"),
            QString::fromUtf8(u8"深念追问")
            });
        break;

    case 200:
        setBackground(QString::fromUtf8(u8"夏之阁"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"你的向往，本就值得被尊重。"), QString::fromUtf8(u8"player/player_005.mp3"));
        m_index = 201;
        break;
    case 201:
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"世人皆笑我空想，难得有人认可我心中江河。"), QString::fromUtf8(u8"guzhilan/guzhilan_004.mp3"));
        m_aPre = true;
        m_index = 300;
        break;

    case 210:
        setBackground(QString::fromUtf8(u8"夏之阁"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"楼阁困住你的脚步，却困不住你的心念。"), QString::fromUtf8(u8"player/player_006.mp3"));
        m_index = 211;
        break;
    case 211:
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"道理我早已知晓，只是心中终究留有遗憾。"), QString::fromUtf8(u8"guzhilan/guzhilan_005.mp3"));
        m_bPre = true;
        m_index = 300;
        break;

    case 220:
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"想走就走，是你执念太重。"), QString::fromUtf8(u8"player/player_007.mp3"));
        m_index = 221;
        break;
    case 221:
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"原来连你也觉得，是我不够勇敢……"), QString::fromUtf8(u8"guzhilan/guzhilan_006.mp3"));
        triggerEnding(ENDING_C_CORRUPT);
        m_index = 500;
        runSummer();
        break;

    case 230:
        setBackground(QString::fromUtf8(u8"夏之阁"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"你向往江海，究竟向往的是什么？"), QString::fromUtf8(u8"player/player_008.mp3"));
        m_index = 231;
        break;
    case 231:
        if (gd->isSummerSkin() || gd->isDefaultSkin()) {
            showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"我向往从不属于我的自由。"), QString::fromUtf8(u8"guzhilan/guzhilan_008.mp3"));
            m_dPre = true;
        }
        else {
            showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"未曾亲历，多说无益。"), QString::fromUtf8(u8"guzhilan/guzhilan_007.mp3"));
            m_dPre = false;
        }
        m_index = 300;
        break;

    case 300:
        showNarration(QString::fromUtf8(u8"【交互任务】\n观澜手记残页散落在阁楼各处，请靠近拾取……"), "");
        m_index = 301;
        break;
    case 301:
        startItemMiniGame(QStringList{
            QString::fromUtf8(u8"观澜手记残页")
            }, 302, QString::fromUtf8(u8"夏之阁"));
        break;
    case 302:
        showNarration(QString::fromUtf8(u8"手记已在临江书案拼合完成，主线达成。"), "");
        m_index = 400;
        break;

    case 400:
        if (m_pendingEnding == ENDING_C_CORRUPT) { m_index = 500; runSummer(); }
        else if (m_dPre) { m_index = 430; runSummer(); }
        else if (m_aPre) { m_index = 410; runSummer(); }
        else if (m_bPre) { m_index = 420; runSummer(); }
        else { m_index = 420; runSummer(); }
        break;

    case 410:
        playEffect("ripple");
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"纵然肉身不曾出发，我的向往已经顺着江河奔赴山海。"), QString::fromUtf8(u8"guzhilan/guzhilan_010.mp3"));
        m_index = 411;
        break;
    case 411:
        // 结局演出前强制清理所有后台特效，避免与 ghostFadeOut 冲突
        m_effect->stopParticleEffect();
        setGraphicsEffect(nullptr);

        m_effect->ghostFadeOut(m_dialog, 2000);
        m_dialog->setEnabled(false);
        QTimer::singleShot(2100, this, [this]() {
            if (!m_dialog) return;

            AudioManager::instance()->stopVoice();
            triggerEnding(ENDING_A_PERFECT);
            QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得夏记忆碎片，称号【观澜知远】；解锁普通皮陆安泫·观澜"));
            GameData::instance()->unlockSkin(SKIN_SUMMER_NORMAL);

            updateDialogPosition();
            m_dialog->setEnabled(true);
            m_index = 412;
            });
        break;
    case 412:
        backToHall();
        break;
    case 430:
        showCG(QString::fromUtf8(u8"CG_S2回忆"));
        m_index = 431;
        break;
    case 431:
        gd->setDFlag(LEVEL_SUMMER, true);
        QMessageBox::information(this, QString::fromUtf8(u8"彩蛋"), QString::fromUtf8(u8"解锁典藏皮陆安泫·羁澜"));
        gd->unlockSkin(SKIN_SUMMER_RARE);
        backToHall();
        break;

    case 420:
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"文字得以留存，但心中向往依旧被囚于此地。"), QString::fromUtf8(u8"guzhilan/guzhilan_009.mp3"));
        triggerEnding(ENDING_B_NEUTRAL);
        QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得夏记忆碎片，不解锁皮肤。"));
        backToHall();
        break;

    case 500:
        setBackground(QString::fromUtf8(u8"夏之阁_黑化"));
        playBGM(QString::fromUtf8(u8"夏之阁_黑化.mp3"));
        showLine(QString::fromUtf8(u8"顾知澜"), QString::fromUtf8(u8"原来连你也觉得，是我不够勇敢……"), QString::fromUtf8(u8"guzhilan/guzhilan_006.mp3"));
        QMessageBox::warning(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"崩坏失败，无奖励。"));
        backToHall();
        break;

    default: break;
    }
}

// ==================== S3 秋之祠 ====================
void LevelScene::runAutumn()
{
    GameData* gd = GameData::instance();
    switch (m_index) {
    case 0: {
        setBackground(QString::fromUtf8(u8"秋之祠_黑化"));
        playBGM(QString::fromUtf8(u8"秋之祠.mp3"));
        showCharacter(QString::fromUtf8(u8"姜寄蘅"), "corrupt");
        if (gd->isDefaultSkin()) m_index = 10;
        else if (gd->isAutumnSkin()) m_index = 20;
        else m_index = 30;
        showNarration(QString::fromUtf8(u8"秋之祠笼罩在肃杀之气中，药草尽数枯萎，秋风如刀……"), "");
        break;
    }
    case 10:
        showNarration(QString::fromUtf8(u8"秋风卷着银杏落叶，源源不断吹入祠堂。肃穆牌位林立。侧廊之中，姜寄蘅守着一筐草药，衣衫沾染药渍，神色苍凉疲惫。"), QString::fromUtf8(u8"narrator/narrator_012.mp3"));
        m_index = 11;
        break;
    case 11:
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"我以草木渡人，把满心期许托付众生。到头来，却无人肯托我一寸安稳。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_001.mp3"));
        m_index = 100;
        break;
    case 20:
        playEffect("ginkgo");
        showNarration(QString::fromUtf8(u8"银杏落叶在你周身盘旋一圈。姜寄蘅抬头看向你，眼中是难得的一丝暖意。"), QString::fromUtf8(u8"narrator/narrator_013.mp3"));
        m_index = 21;
        break;
    case 21:
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"你身带秋禾温善之气，你懂济世寒凉、善意空落的滋味。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_002.mp3"));
        m_hide = true;
        gd->setHideFlag(LEVEL_AUTUMN, true);
        m_index = 100;
        break;
    case 30:
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"四时心性不同，你难共情我半生孤善。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_003.mp3"));
        m_dPre = false;
        m_index = 100;
        break;

    case 100:
        showNarration(QString::fromUtf8(u8"你准备如何回应姜寄蘅？"), "");
        m_index = 101;
        break;
    case 101:
        showOption(QStringList{
            QString::fromUtf8(u8"共情渡心"),
            QString::fromUtf8(u8"劝慰释怀"),
            QString::fromUtf8(u8"否定善良"),
            QString::fromUtf8(u8"执念深问")
            });
        break;

    case 200:
        setBackground(QString::fromUtf8(u8"秋之祠"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"你的善意本身就有重量，不必依靠他人回馈。"), QString::fromUtf8(u8"player/player_009.mp3"));
        m_index = 201;
        break;
    case 201:
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"从来没有人这样告诉我。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_004.mp3"));
        m_aPre = true;
        m_index = 300;
        break;

    case 210:
        setBackground(QString::fromUtf8(u8"秋之祠"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"施恩不图回报，本就是世间难事。"), QString::fromUtf8(u8"player/player_010.mp3"));
        m_index = 211;
        break;
    case 211:
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"道理我懂，只是寒心难以消解。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_005.mp3"));
        m_bPre = true;
        m_index = 300;
        break;

    case 220:
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"善良是软肋，不值得遗憾。"), QString::fromUtf8(u8"player/player_011.mp3"));
        m_index = 221;
        break;
    case 221:
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"原来连善意，也成为一桩过错吗。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_006.mp3"));
        triggerEnding(ENDING_C_CORRUPT);
        m_index = 500;
        runAutumn();
        break;

    case 230:
        setBackground(QString::fromUtf8(u8"秋之祠"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"救人济世，你究竟在渴求什么？"), QString::fromUtf8(u8"player/player_012.mp3"));
        m_index = 231;
        break;
    case 231:
        if (gd->isAutumnSkin() || gd->isDefaultSkin()) {
            showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"渴求一份对等的人心。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_007.mp3"));
            m_dPre = true;
        }
        else {
            showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"四时心性不同，你难共情我半生孤善。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_003.mp3"));
            m_dPre = false;
        }
        m_index = 300;
        break;

    case 300:
        showNarration(QString::fromUtf8(u8"【交互任务】\n济世药草残株散落于祠堂角落，请靠近拾取……"), "");
        m_index = 301;
        break;
    case 301:
        startItemMiniGame(QStringList{
            QString::fromUtf8(u8"济世药草残株")
            }, 302, QString::fromUtf8(u8"秋之祠"));
        break;
    case 302:
        showNarration(QString::fromUtf8(u8"药草已放回旧药筐，主线达成。"), "");
        m_index = 400;
        break;

    case 400:
        if (m_pendingEnding == ENDING_C_CORRUPT) { m_index = 500; runAutumn(); }
        else if (m_dPre) { m_index = 430; runAutumn(); }
        else if (m_aPre) { m_index = 410; runAutumn(); }
        else if (m_bPre) { m_index = 420; runAutumn(); }
        else { m_index = 420; runAutumn(); }
        break;

    case 410:
        playEffect("ginkgo");
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"我不必依靠他人的善待，来证明我善意的价值。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_009.mp3"));
        m_index = 411;
        break;
    case 411:
        // 结局演出前强制清理所有后台特效，避免与 ghostFadeOut 冲突
        m_effect->stopParticleEffect();
        setGraphicsEffect(nullptr);

        m_effect->ghostFadeOut(m_dialog, 2000);
        m_dialog->setEnabled(false);
        QTimer::singleShot(2100, this, [this]() {
            if (!m_dialog) return;

            AudioManager::instance()->stopVoice();
            triggerEnding(ENDING_A_PERFECT);
            QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得秋记忆碎片，称号【怀蘅渡世】；解锁普通皮陆祐禾·秋衡"));
            GameData::instance()->unlockSkin(SKIN_AUTUMN_NORMAL);

            updateDialogPosition();
            m_dialog->setEnabled(true);
            m_index = 412;
            });
        break;
    case 412:
        backToHall();
        break;
    case 430:
        showCG(QString::fromUtf8(u8"CG_S3回忆"));
        m_index = 431;
        break;
    case 431:
        gd->setDFlag(LEVEL_AUTUMN, true);
        QMessageBox::information(this, QString::fromUtf8(u8"彩蛋"), QString::fromUtf8(u8"解锁典藏皮陆祐禾·空蘅"));
        gd->unlockSkin(SKIN_AUTUMN_RARE);
        backToHall();
        break;

    case 420:
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"道理我都懂，只是受过的寒，难以轻易抹平。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_008.mp3"));
        triggerEnding(ENDING_B_NEUTRAL);
        QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得秋记忆碎片，不解锁皮肤。"));
        backToHall();
        break;

    case 500:
        setBackground(QString::fromUtf8(u8"秋之祠_黑化"));
        playBGM(QString::fromUtf8(u8"秋之祠_黑化.mp3"));
        showLine(QString::fromUtf8(u8"姜寄蘅"), QString::fromUtf8(u8"原来连善意，也成为一桩过错吗。"), QString::fromUtf8(u8"jiangjiheng/jiangjiheng_006.mp3"));
        QMessageBox::warning(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"崩坏失败，无奖励。"));
        backToHall();
        break;

    default: break;
    }
}

// ==================== S4 冬之坛 ====================
void LevelScene::runWinter()
{
    GameData* gd = GameData::instance();
    switch (m_index) {
    case 0: {
        setBackground(QString::fromUtf8(u8"冬之坛_黑化"));
        playBGM(QString::fromUtf8(u8"冬之坛.mp3"));
        showCharacter(QString::fromUtf8(u8"谢云绾"), "corrupt");
        if (gd->isDefaultSkin()) m_index = 10;
        else if (gd->isWinterSkin()) m_index = 20;
        else m_index = 30;
        showNarration(QString::fromUtf8(u8"冬之坛被狂暴的暴风雪吞噬，怨魂呼啸，祭祀的冷灰被卷上天空……"), "");
        break;
    }
    case 10:
        showNarration(QString::fromUtf8(u8"暴风雪呼啸不息。天地是一片惨白。高高的祭祀祭坛立在风雪中央。石阶覆着寒冰。四处散落祭祀之后残留冷灰。白衣谢云绾站在祭坛中央，任凭雪花落满衣袖。"), QString::fromUtf8(u8"narrator/narrator_015.mp3"));
        m_index = 11;
        break;
    case 11:
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"寒云覆满天地，我暗自绾住一寸春念，却始终等不到风雪尽消。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_001.mp3"));
        m_index = 100;
        break;
    case 20:
        playEffect("snow");
        showNarration(QString::fromUtf8(u8"风雪一瞬间减弱片刻。谢云绾猛然转头望向你，眼中是难以置信的微光。"), QString::fromUtf8(u8"narrator/narrator_016.mp3"));
        m_index = 21;
        break;
    case 21:
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"你携风雪晨光而来……原来这漫长寒冬里，不止我一人在等春。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_002.mp3"));
        m_hide = true;
        gd->setHideFlag(LEVEL_WINTER, true);
        m_index = 100;
        break;
    case 30:
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"你未经永冬，不懂岁岁空等的寒凉。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_003.mp3"));
        m_dPre = false;
        m_index = 100;
        break;

    case 100:
        showNarration(QString::fromUtf8(u8"你准备如何回应谢云绾？"), "");
        m_index = 101;
        break;
    case 101:
        showOption(QStringList{
            QString::fromUtf8(u8"共情怜盼"),
            QString::fromUtf8(u8"清淡劝慰"),
            QString::fromUtf8(u8"否定等待"),
            QString::fromUtf8(u8"岁月深问")
            });
        break;

    case 200:
        setBackground(QString::fromUtf8(u8"冬之坛"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"你心底那一份对春天的期盼，本就无比珍贵。"), QString::fromUtf8(u8"player/player_013.mp3"));
        m_index = 201;
        break;
    case 201:
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"世人皆劝我放弃，唯有你看见我心底的盼望。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_004.mp3"));
        m_aPre = true;
        m_index = 300;
        break;

    case 210:
        setBackground(QString::fromUtf8(u8"冬之坛"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"等待本就煎熬，只是春未必会如期而至。"), QString::fromUtf8(u8"player/player_014.mp3"));
        m_index = 211;
        break;
    case 211:
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"我知晓，却无法放下。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_005.mp3"));
        m_bPre = true;
        m_index = 300;
        break;

    case 220:
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"明知无果，何必死守。"), QString::fromUtf8(u8"player/player_015.mp3"));
        m_index = 221;
        break;
    case 221:
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"连心底这一寸春念，也不配留存于世间吗。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_006.mp3"));
        triggerEnding(ENDING_C_CORRUPT);
        m_index = 500;
        runWinter();
        break;

    case 230:
        setBackground(QString::fromUtf8(u8"冬之坛"));
        m_npcState = "normal";
        showLine(QString::fromUtf8(u8"玩家"), QString::fromUtf8(u8"你苦苦等候的，究竟是春天，还是一份解脱？"), QString::fromUtf8(u8"player/player_016.mp3"));
        m_index = 231;
        break;
    case 231:
        if (gd->isWinterSkin() || gd->isDefaultSkin()) {
            showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"是我不甘被命运摆布。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_007.mp3"));
            m_dPre = true;
        }
        else {
            showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"你未经永冬，不懂岁岁空等的寒凉。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_003.mp3"));
            m_dPre = false;
        }
        m_index = 300;
        break;

    case 300:
        showNarration(QString::fromUtf8(u8"【交互任务】\n⚠️警告：信物禁止放到主高台！\n嵌玉发梳、梅纹围脖、木雕小牌散落于祭坛四周，请靠近拾取……"), "");
        m_index = 301;
        break;
    case 301:
        startItemMiniGame(QStringList{
            QString::fromUtf8(u8"嵌玉发梳"),
            QString::fromUtf8(u8"梅纹围脖"),
            QString::fromUtf8(u8"木雕小牌")
            }, 304, QString::fromUtf8(u8"冬之坛"));
        break;
    case 304:
        showNarration(QString::fromUtf8(u8"三件信物已摆放到外围纪念石台，主线达成。"), "");
        m_index = 400;
        break;

    case 400:
        if (m_pendingEnding == ENDING_C_CORRUPT) { m_index = 500; runWinter(); }
        else if (m_dPre) { m_index = 430; runWinter(); }
        else if (m_aPre) { m_index = 410; runWinter(); }
        else if (m_bPre) { m_index = 420; runWinter(); }
        else { m_index = 420; runWinter(); }
        break;

    case 410:
        playEffect("snow");
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"不必以鲜血换取春天。只要我的期盼被记住，寒冬就已经结束。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_009.mp3"));
        m_index = 411;
        break;
    case 411:
        // 结局演出前强制清理所有后台特效，避免与 ghostFadeOut 冲突
        m_effect->stopParticleEffect();
        setGraphicsEffect(nullptr);

        m_effect->ghostFadeOut(m_dialog, 2000);
        m_dialog->setEnabled(false);
        QTimer::singleShot(2100, this, [this]() {
            if (!m_dialog) return;

            AudioManager::instance()->stopVoice();
            triggerEnding(ENDING_A_PERFECT);
            QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得冬记忆碎片，称号【绾雪待春】；解锁普通皮陆融霏·归晞"));
            GameData::instance()->unlockSkin(SKIN_WINTER_NORMAL);

            updateDialogPosition();
            m_dialog->setEnabled(true);
            m_index = 412;
            });
        break;
    case 412:
        backToHall();
        break;
    case 430:
        showCG(QString::fromUtf8(u8"CG_S4回忆"));
        m_index = 431;
        break;
    case 431:
        gd->setDFlag(LEVEL_WINTER, true);
        QMessageBox::information(this, QString::fromUtf8(u8"彩蛋"), QString::fromUtf8(u8"解锁典藏皮陆融霏·寒绾"));
        gd->unlockSkin(SKIN_WINTER_RARE);
        backToHall();
        break;

    case 420:
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"可我这一生，耗不起漫长等待。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_008.mp3"));
        triggerEnding(ENDING_B_NEUTRAL);
        QMessageBox::information(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"获得冬记忆碎片，不解锁皮肤。"));
        backToHall();
        break;

    case 500:
        setBackground(QString::fromUtf8(u8"冬之坛_黑化"));
        playBGM(QString::fromUtf8(u8"冬之坛_黑化.mp3"));
        showLine(QString::fromUtf8(u8"谢云绾"), QString::fromUtf8(u8"连心底这一寸春念，也不配留存于世间吗。"), QString::fromUtf8(u8"xieyunwan/xieyunwan_006.mp3"));
        QMessageBox::warning(this, QString::fromUtf8(u8"结局"), QString::fromUtf8(u8"崩坏失败，无奖励。"));
        backToHall();
        break;

    default: break;
    }
}