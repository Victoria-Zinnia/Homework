#include "IntroScene.h"
#include "GameEngine.h"
#include "GameData.h"
#include "AudioManager.h"
#include <QPainter>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QPushButton>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>
#include <QTimer>

IntroScene::IntroScene(QWidget* parent) : SceneBase(parent),
m_state(Finished), m_currentPage(0),
m_pageWidget(nullptr), m_titleLabel(nullptr),
m_subtitleLabel(nullptr), m_bodyLabel(nullptr),
m_quoteLabel(nullptr), m_skipBtn(nullptr),
m_fadeAnim(nullptr), m_opacityEffect(nullptr)
{
    m_pages.append({
        QString::fromUtf8(u8"引 · 残书摄魂"),
        QString::fromUtf8(u8"—— 无名残书，青光摄魂 ——"),
        QString::fromUtf8(u8"你名陆令晞，本是现世一介凡人。\n"
            u8"那夜灯下翻阅一本无名残书，忽有青光摄魂，再睁眼时，"
            u8"已身处一座悬浮于虚空中的古老庭院。\n\n"
            u8"此处无日月轮转，却四时并存——春樱不谢，夏澜不息，"
            u8"秋蘅不枯，冬雪不融。一位无名之声告诉你："
            u8"这是轮回之庭，四位女子的执念锁住了出口。\n"
            u8"唯有解开她们的心结，方能补全残书，找到归途。"),
        QString::fromUtf8(u8"\"四时未竟，此身入局。\"")
        });

    m_pages.append({
        QString::fromUtf8(u8"春之庭 · 无根花"),
        QString::fromUtf8(u8"—— 花匠之女，幼年被拐，一生无归 ——"),
        QString::fromUtf8(u8"苏堇纾，本是江南花匠之女，幼年被拐，辗转流离。\n"
            u8"她被囚于一座永不凋零的春之庭中，年年花开，岁岁无归。\n"
            u8"世人只见她身处花海，却不知她一生漂泊，"
            u8"从未有过一寸属于自己的泥土。\n\n"
            u8"高墙锁死春光，她坐在花丛之间，手指抚过花瓣，"
            u8"眼神是长久的惘然。"),
        QString::fromUtf8(u8"\"我年年逢春，却年年都是过客。\"")
        });

    m_pages.append({
        QString::fromUtf8(u8"夏之阁 · 笼中雀"),
        QString::fromUtf8(u8"—— 世家嫡女，父兄强嫁，困于高楼 ——"),
        QString::fromUtf8(u8"顾知澜，世家贵族嫡女，才貌双全，心向远方。\n"
            u8"却被父兄视为攀附权贵的筹码，欲强嫁与垂暮老朽。\n"
            u8"她困于高楼之上，凭栏望江，江风浩荡，"
            u8"却吹不散她一身桎梏。\n\n"
            u8"闷热的江风穿堂而过，纱帘被风浪反复吹动。"
            u8"窗外大江奔涌，一望无际，却不允许居住者真正走向江水。"),
        QString::fromUtf8(u8"\"我知世间江河浩荡，奈何身有桎梏，只可坐看波澜远去。\"")
        });

    m_pages.append({
        QString::fromUtf8(u8"秋之祠 · 孤善寒"),
        QString::fromUtf8(u8"—— 济世医女，被诬不详，囚于祠堂 ——"),
        QString::fromUtf8(u8"姜寄蘅，一心济世，专医穷苦，不收分毫。\n"
            u8"却因女子行医触怒乡绅，被无知村民诬陷为不详妖女，囚于祠堂。\n"
            u8"半生善意，换得一身寒霜；满心期许，终究无人托稳。\n\n"
            u8"秋风卷着银杏落叶，源源不断吹入祠堂。肃穆牌位林立，"
            u8"她守着一筐草药，衣衫沾染药渍，神色苍凉疲惫。"),
        QString::fromUtf8(u8"\"我以草木渡人，把满心期许托付众生。到头来，却无人肯托我一寸安稳。\"")
        });

    m_pages.append({
        QString::fromUtf8(u8"冬之坛 · 活祭雪"),
        QString::fromUtf8(u8"—— 少数民族少女，愚昧祭祀，风雪埋骨 ——"),
        QString::fromUtf8(u8"谢云绾，边地少数民族少女，天真烂漫，能歌善舞。\n"
            u8"却因封建糟粕与愚昧迷信，被族人选为风雪祭坛的活祭。\n"
            u8"她站在高高的祭坛中央，任凭雪花落满衣袖，"
            u8"心底绾住一寸春念，却等不到风雪尽消。\n\n"
            u8"暴风雪呼啸不息，天地一片惨白。石阶覆着寒冰，"
            u8"四处散落祭祀之后残留的冷灰。"),
        QString::fromUtf8(u8"\"寒云覆满天地，我暗自绾住一寸春念，却始终等不到风雪尽消。\"")
        });

    m_pages.append({
        QString::fromUtf8(u8"四时未竟 · 此身入局"),
        QString::fromUtf8(u8"—— 你手持残书，便是这轮回中唯一的变数 ——"),
        QString::fromUtf8(u8"春樱、夏澜、秋蘅、冬雪——\n"
            u8"每一次选择，都将决定她们是执念消散、魂归虚无，"
            u8"还是心结得解、破茧重生。\n\n"
            u8"你能否为她们写完这最后一页？\n"
            u8"残书补全之日，便是轮回终章之时。"),
        QString::fromUtf8(u8"\"你，能否为她们写完这最后一页？\"")
        });
}

void IntroScene::onEnter()
{
    setBackground(QString::fromUtf8(u8"标题页"));
    playBGM(QString::fromUtf8(u8"标题页.mp3"));

    m_pageWidget = new QWidget(this);
    m_pageWidget->setGeometry(rect());
    m_pageWidget->show();

    m_opacityEffect = new QGraphicsOpacityEffect(m_pageWidget);
    m_pageWidget->setGraphicsEffect(m_opacityEffect);
    m_opacityEffect->setOpacity(0);

    // 大标题：深色，高度加大到80避免截断
    m_titleLabel = new QLabel(m_pageWidget);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    QFont titleFont("Microsoft YaHei", 32, QFont::Bold);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet("color: #1E1E2E; background: transparent;");
    m_titleLabel->setGeometry(0, height() / 5 - 10, width(), 80);

    // 副标题：深褐色
    m_subtitleLabel = new QLabel(m_pageWidget);
    m_subtitleLabel->setAlignment(Qt::AlignCenter);
    QFont subFont("Microsoft YaHei", 14);
    m_subtitleLabel->setFont(subFont);
    m_subtitleLabel->setStyleSheet("color: #4A3B2A; background: transparent;");
    m_subtitleLabel->setGeometry(0, height() / 5 + 72, width(), 30);

    // 正文：深灰黑，加行高
    m_bodyLabel = new QLabel(m_pageWidget);
    m_bodyLabel->setAlignment(Qt::AlignCenter);
    m_bodyLabel->setWordWrap(true);
    QFont bodyFont("Microsoft YaHei", 13);
    m_bodyLabel->setFont(bodyFont);
    m_bodyLabel->setStyleSheet(
        "color: #2D2D3D; background: transparent; line-height: 1.7;"
    );
    m_bodyLabel->setGeometry(width() / 6, height() / 5 + 115, width() * 2 / 3, height() / 2);

    // 引用句：深褐加粗
    m_quoteLabel = new QLabel(m_pageWidget);
    m_quoteLabel->setAlignment(Qt::AlignCenter);
    m_quoteLabel->setWordWrap(true);
    QFont quoteFont("Microsoft YaHei", 15, QFont::Bold);
    m_quoteLabel->setFont(quoteFont);
    m_quoteLabel->setStyleSheet("color: #3D3020; background: transparent;");
    m_quoteLabel->setGeometry(width() / 6, height() * 3 / 4 + 10, width() * 2 / 3, 55);

    // 跳过/进入按钮：深色底+白字，确保在标题页背景上可见
    m_skipBtn = new QPushButton(QString::fromUtf8(u8"跳过引子"), m_pageWidget);
    m_skipBtn->setGeometry(width() - 130, height() - 55, 110, 36);
    m_skipBtn->setStyleSheet(
        "QPushButton { background-color: rgba(30,30,46,180); color: #F0F0F0; "
        "border: 1px solid #555; border-radius: 6px; font-size: 12px; }"
        "QPushButton:hover { background-color: rgba(50,50,70,200); }"
    );
    connect(m_skipBtn, &QPushButton::clicked, this, &IntroScene::finishIntro);

    m_currentPage = 0;
    setupPage(0);
    startFadeIn();
}

void IntroScene::onExit()
{
    stopBGM();
    if (m_fadeAnim) {
        m_fadeAnim->stop();
        delete m_fadeAnim;
        m_fadeAnim = nullptr;
    }
    if (m_pageWidget) {
        m_pageWidget->deleteLater();
        m_pageWidget = nullptr;
    }
    m_titleLabel = nullptr;
    m_subtitleLabel = nullptr;
    m_bodyLabel = nullptr;
    m_quoteLabel = nullptr;
    m_skipBtn = nullptr;
    m_opacityEffect = nullptr;
}

void IntroScene::setupPage(int index)
{
    if (index < 0 || index >= m_pages.size()) return;

    const Page& pg = m_pages[index];
    m_titleLabel->setText(pg.title);
    m_subtitleLabel->setText(pg.subtitle);
    m_bodyLabel->setText(pg.body);
    m_quoteLabel->setText(pg.quote);

    if (index == m_pages.size() - 1) {
        m_skipBtn->setText(QString::fromUtf8(u8"进入轮回"));
        m_skipBtn->setStyleSheet(
            "QPushButton { background-color: rgba(180,140,80,220); color: #FFF8E0; "
            "border: 2px solid #8B6914; border-radius: 8px; font-size: 14px; "
            "font-family: 'Microsoft YaHei'; font-weight: bold; }"
            "QPushButton:hover { background-color: rgba(200,160,100,240); }"
        );
    }
    else {
        m_skipBtn->setText(QString::fromUtf8(u8"跳过引子"));
        m_skipBtn->setStyleSheet(
            "QPushButton { background-color: rgba(30,30,46,180); color: #F0F0F0; "
            "border: 1px solid #555; border-radius: 6px; font-size: 12px; }"
            "QPushButton:hover { background-color: rgba(50,50,70,200); }"
        );
    }
}

void IntroScene::startFadeIn()
{
    m_state = FadingIn;
    if (!m_opacityEffect) return;

    if (m_fadeAnim) {
        m_fadeAnim->stop();
        delete m_fadeAnim;
    }

    m_fadeAnim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeAnim->setDuration(FADE_DURATION);
    m_fadeAnim->setStartValue(0.0);
    m_fadeAnim->setEndValue(1.0);
    m_fadeAnim->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        m_fadeAnim = nullptr;
        m_state = Waiting;
        QTimer::singleShot(HOLD_DURATION, this, [this]() {
            if (m_state == Waiting) startFadeOut();
            });
        });

    m_fadeAnim->start();
}

void IntroScene::startFadeOut()
{
    m_state = FadingOut;
    if (!m_opacityEffect) return;

    if (m_fadeAnim) {
        m_fadeAnim->stop();
        delete m_fadeAnim;
    }

    m_fadeAnim = new QPropertyAnimation(m_opacityEffect, "opacity", this);
    m_fadeAnim->setDuration(FADE_DURATION);
    m_fadeAnim->setStartValue(1.0);
    m_fadeAnim->setEndValue(0.0);
    m_fadeAnim->setEasingCurve(QEasingCurve::InOutQuad);

    connect(m_fadeAnim, &QPropertyAnimation::finished, this, [this]() {
        m_fadeAnim = nullptr;
        advancePage();
        });

    m_fadeAnim->start();
}

void IntroScene::advancePage()
{
    m_currentPage++;
    if (m_currentPage >= m_pages.size()) {
        finishIntro();
        return;
    }
    setupPage(m_currentPage);
    startFadeIn();
}

void IntroScene::finishIntro()
{
    if (m_state == Finished) return;
    m_state = Finished;

    if (m_fadeAnim) {
        m_fadeAnim->stop();
        delete m_fadeAnim;
        m_fadeAnim = nullptr;
    }

    QVariantMap params;
    params["skipBoot"] = true;
    GameEngine::instance()->changeScene("title", params);
}

void IntroScene::mousePressEvent(QMouseEvent* event)
{
    Q_UNUSED(event)
        if (m_state == Waiting) {
            startFadeOut();
        }
        else if (m_state == Finished) {
            finishIntro();
        }
}

void IntroScene::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape || event->key() == Qt::Key_Space) {
        if (m_state == Waiting) {
            startFadeOut();
        }
        else {
            finishIntro();
        }
    }
}

void IntroScene::resizeEvent(QResizeEvent* event)
{
    SceneBase::resizeEvent(event);
    if (m_pageWidget) {
        m_pageWidget->setGeometry(rect());
        m_titleLabel->setGeometry(0, height() / 5 - 10, width(), 80);
        m_subtitleLabel->setGeometry(0, height() / 5 + 72, width(), 30);
        m_bodyLabel->setGeometry(width() / 6, height() / 5 + 115, width() * 2 / 3, height() / 2);
        m_quoteLabel->setGeometry(width() / 6, height() * 3 / 4 + 10, width() * 2 / 3, 55);
        m_skipBtn->setGeometry(width() - 130, height() - 55, 110, 36);
    }
}