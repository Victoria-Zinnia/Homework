#ifndef RESOURCEMANAGER_H
#define RESOURCEMANAGER_H

#include <QObject>
#include <QPixmap>
#include <QMap>
#include <QString>

class ResourceManager : public QObject
{
    Q_OBJECT
public:
    explicit ResourceManager(QObject* parent = nullptr);
    static ResourceManager* instance();

    QPixmap getImage(const QString& path, int width = 0, int height = 0);
    QPixmap getCharacter(const QString& name, const QString& state = "normal");
    QPixmap getBackground(const QString& name);
    QPixmap getCG(const QString& name);
    QPixmap getItem(const QString& name);
    QPixmap getSkin(const QString& fileName); // 直接按完整文件名加载
    QPixmap generatePlaceholder(const QString& text, int width, int height, QColor bgColor);
    void clearCache();

private:
    static ResourceManager* m_instance;
    QMap<QString, QPixmap> m_cache;
    bool loadFromFile(const QString& fullPath, QPixmap& out);
    bool loadFromQrc(const QString& qrcPath, QPixmap& out);
};

#endif // RESOURCEMANAGER_H