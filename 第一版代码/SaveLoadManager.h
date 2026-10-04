#ifndef SAVELOADMANAGER_H
#define SAVELOADMANAGER_H

#include <QObject>
#include <QWidget>
#include <QListWidget>

class SaveLoadManager : public QObject
{
    Q_OBJECT
public:
    explicit SaveLoadManager(QWidget* parent = nullptr);

    void showSaveDialog();
    void showLoadDialog();
    bool hasSaveFile() const;

signals:
    void loadCompleted();
    void saveCompleted();

private:
    QWidget* m_parent;
    void refreshList(QListWidget* list);
};

#endif // SAVELOADMANAGER_H