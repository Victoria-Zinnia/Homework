#include "ResourceManager.h"
#include <QPainter>
#include <QFile>
#include <QDebug>

ResourceManager* ResourceManager::m_instance = nullptr;

ResourceManager* ResourceManager::instance()
{
    if (!m_instance)
        m_instance = new ResourceManager();
    return m_instance;
}

ResourceManager::ResourceManager(QObject* parent) : QObject(parent)
{
}

bool ResourceManager::loadFromFile(const QString& fullPath, QPixmap& out)
{
    if (m_cache.contains(fullPath)) {
        out = m_cache[fullPath];
        return true;
    }

    if (QFile::exists(fullPath)) {
        if (out.load(fullPath)) {
            m_cache[fullPath] = out;
            return true;
        }
    }
    return false;
}

bool ResourceManager::loadFromQrc(const QString& qrcPath, QPixmap& out)
{
    if (m_cache.contains(qrcPath)) {
        out = m_cache[qrcPath];
        return true;
    }
    if (out.load(qrcPath)) {
        m_cache[qrcPath] = out;
        return true;
    }
    return false;
}

QPixmap ResourceManager::getImage(const QString& path, int width, int height)
{
    QPixmap pix;
    if (!loadFromFile(path, pix)) {
        QString qrcPath = ":" + path;
        if (!loadFromQrc(qrcPath, pix)) {
            int w = width > 0 ? width : 1920;
            int h = height > 0 ? height : 1080;
            pix = generatePlaceholder(path, w, h, QColor(40, 40, 60));
        }
    }
    if (width > 0 && height > 0 && !pix.isNull()) {
        pix = pix.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    }
    return pix;
}

QPixmap ResourceManager::getCharacter(const QString& name, const QString& state)
{
    QString filePath = QString("assets/images/char/%1_%2.png").arg(name).arg(state);
    QPixmap pix;
    if (!loadFromFile(filePath, pix)) {
        QString qrcPath = QString(":assets/images/char/%1_%2.png").arg(name).arg(state);
        if (!loadFromQrc(qrcPath, pix)) {
            pix = generatePlaceholder(name + "\n[" + state + "]", 900, 1400, QColor(60, 40, 40));
        }
    }
    return pix;
}

QPixmap ResourceManager::getBackground(const QString& name)
{
    QString filePath = QString("assets/images/bg/%1.jpg").arg(name);
    QPixmap pix;
    if (!loadFromFile(filePath, pix)) {
        filePath = QString("assets/images/bg/%1.png").arg(name);
        if (!loadFromFile(filePath, pix)) {
            QString qrcPath = QString(":assets/images/bg/%1.jpg").arg(name);
            if (!loadFromQrc(qrcPath, pix)) {
                pix = generatePlaceholder(name, 1920, 1080, QColor(20, 30, 50));
            }
        }
    }
    return pix;
}

QPixmap ResourceManager::getCG(const QString& name)
{
    QString filePath = QString("assets/images/cg/%1.jpg").arg(name);
    QPixmap pix;
    if (!loadFromFile(filePath, pix)) {
        filePath = QString("assets/images/cg/%1.png").arg(name);
        if (!loadFromFile(filePath, pix)) {
            QString qrcPath = QString(":assets/images/cg/%1.jpg").arg(name);
            if (!loadFromQrc(qrcPath, pix)) {
                pix = generatePlaceholder("CG\n" + name, 1920, 1080, QColor(30, 20, 40));
            }
        }
    }
    return pix;
}

QPixmap ResourceManager::getItem(const QString& name)
{
    QString filePath = QString("assets/images/items/%1.png").arg(name);
    QPixmap pix;
    if (!loadFromFile(filePath, pix)) {
        QString qrcPath = QString(":assets/images/items/%1.png").arg(name);
        if (!loadFromQrc(qrcPath, pix)) {
            pix = generatePlaceholder(name, 256, 256, QColor(80, 60, 40));
        }
    }
    return pix;
}

// 新增：直接按完整文件名加载皮肤
QPixmap ResourceManager::getSkin(const QString& fileName)
{
    QString filePath = QString("assets/images/char/%1").arg(fileName);
    QPixmap pix;
    if (!loadFromFile(filePath, pix)) {
        QString qrcPath = QString(":assets/images/char/%1").arg(fileName);
        if (!loadFromQrc(qrcPath, pix)) {
            pix = generatePlaceholder(fileName, 900, 1400, QColor(60, 40, 40));
        }
    }
    return pix;
}

QPixmap ResourceManager::generatePlaceholder(const QString& text, int width, int height, QColor bgColor)
{
    QPixmap pix(width, height);
    pix.fill(bgColor);

    QPainter painter(&pix);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.setPen(QPen(Qt::white, 3));
    painter.drawRect(2, 2, width - 4, height - 4);

    painter.setPen(Qt::white);
    QFont font = painter.font();
    font.setPointSize(qMax(12, width / 30));
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pix.rect(), Qt::AlignCenter, QString::fromUtf8(u8"[占位图]\n") + text + QString::fromUtf8(u8"\n(请替换为实际资源)"));

    painter.setPen(QPen(QColor(255, 255, 255, 80), 2));
    painter.drawLine(0, 0, width, height);
    painter.drawLine(width, 0, 0, height);

    painter.end();
    return pix;
}

void ResourceManager::clearCache()
{
    m_cache.clear();
}