#include "ItemPopup.h"
#include "ResourceManager.h"
#include <QPainter>
#include <QVBoxLayout>

ItemPopup::ItemPopup(QWidget* parent) : QWidget(parent)
{
    setFixedSize(400, 180);
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignCenter);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(64, 64);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    layout->addWidget(m_iconLabel, 0, Qt::AlignCenter);

    m_nameLabel = new QLabel(this);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    QFont nameFont("Microsoft YaHei", 14, QFont::Bold);
    m_nameLabel->setFont(nameFont);
    m_nameLabel->setStyleSheet("color: #FFD700;");
    layout->addWidget(m_nameLabel);

    m_descLabel = new QLabel(this);
    m_descLabel->setAlignment(Qt::AlignCenter);
    QFont descFont("Microsoft YaHei", 11);
    m_descLabel->setFont(descFont);
    m_descLabel->setStyleSheet("color: #CCCCCC;");
    layout->addWidget(m_descLabel);

    m_hideTimer = new QTimer(this);
    m_hideTimer->setSingleShot(true);
    connect(m_hideTimer, &QTimer::timeout, this, [this]() {
        hide();
        emit popupFinished();
        });

    hide();
}

void ItemPopup::showItem(const QString& itemName, const QString& desc)
{
    QPixmap icon = ResourceManager::instance()->getItem(itemName);
    m_iconLabel->setPixmap(icon.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_nameLabel->setText("获得道具：【" + itemName + "】");
    m_descLabel->setText(desc.isEmpty() ? "已收入行囊" : desc);

    // 居中显示
    if (parentWidget()) {
        move((parentWidget()->width() - width()) / 2,
            (parentWidget()->height() - height()) / 2);
    }

    show();
    raise();
    m_hideTimer->start(2500); // 2.5秒后自动消失
}

void ItemPopup::paintEvent(QPaintEvent* event)
{
    Q_UNUSED(event)
        QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    QRect rect = this->rect().adjusted(1, 1, -1, -1);
    painter.fillRect(rect, QColor(20, 20, 40, 230));
    painter.setPen(QPen(QColor(150, 150, 200), 2));
    painter.drawRoundedRect(rect, 12, 12);
}