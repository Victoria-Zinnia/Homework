#include "ChoiceMenu.h"
#include "ResourceManager.h"
#include <QPushButton>
#include <QVBoxLayout>
#include <QPainter>
#include <QFile>

ChoiceMenu::ChoiceMenu(QWidget* parent) : QWidget(parent)
{
    setStyleSheet("background-color: transparent;");
    setGeometry(0, 0, 600, 300);
}

void ChoiceMenu::setChoices(const QList<QString>& choices)
{
    clear();

    bool hasChoiceBg = QFile::exists("assets/images/ui/选项框.png");
    QPixmap choiceBg;
    if (hasChoiceBg) {
        choiceBg = ResourceManager::instance()->getImage("assets/images/ui/选项框.png");
        if (choiceBg.isNull()) hasChoiceBg = false;
    }

    int btnHeight = hasChoiceBg ? qMax(75, choiceBg.height()) : 75;
    int btnWidth = hasChoiceBg ? choiceBg.width() : 600;
    int spacing = 18;
    int totalHeight = choices.size() * btnHeight + (choices.size() - 1) * spacing + 40;
    int yPos = (height() - totalHeight) / 2 + 20;
    if (yPos < 10) yPos = 10;

    if (totalHeight > 300) {
        setFixedHeight(totalHeight);
    }

    for (int i = 0; i < choices.size(); ++i) {
        QPushButton* btn = new QPushButton(this);
        btn->setText(choices[i]);
        int x = (width() - btnWidth) / 2;
        btn->setGeometry(x, yPos + i * (btnHeight + spacing), btnWidth, btnHeight);

        if (hasChoiceBg) {
            btn->setStyleSheet(QString(
                "QPushButton {"
                "  border-image: url(assets/images/ui/选项框.png) 0 0 0 0 stretch stretch;"
                "  color: #F0E8D8;"
                "  font-size: 28px;"
                "  font-family: 'Microsoft YaHei';"
                "  border: none;"
                "  padding: 4px;"
                "}"
                "QPushButton:hover {"
                "  color: #FFFFFF;"
                "}"
            ));
        }
        else {
            btn->setStyleSheet(
                "QPushButton {"
                "  background-color: rgba(40, 40, 60, 220);"
                "  color: #E0E0E0;"
                "  border: 2px solid #8888AA;"
                "  border-radius: 8px;"
                "  font-size: 28px;"
                "  font-family: 'Microsoft YaHei';"
                "  padding: 8px;"
                "}"
                "QPushButton:hover {"
                "  background-color: rgba(70, 70, 100, 240);"
                "  border: 2px solid #AAAAFF;"
                "  color: #FFFFFF;"
                "}"
            );
        }

        connect(btn, &QPushButton::clicked, this, [this, i]() {
            hideChoices();
            emit choiceSelected(i);
            });
        m_buttons.append(btn);
    }
}

void ChoiceMenu::clear()
{
    for (auto btn : m_buttons) {
        btn->deleteLater();
    }
    m_buttons.clear();
}

void ChoiceMenu::showChoices()
{
    for (auto btn : m_buttons) btn->show();
    show();
}

void ChoiceMenu::hideChoices()
{
    for (auto btn : m_buttons) btn->hide();
    hide();
}

void ChoiceMenu::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
}