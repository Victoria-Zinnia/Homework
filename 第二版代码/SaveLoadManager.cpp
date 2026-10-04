#include "SaveLoadManager.h"
#include "GameData.h"
#include <QDialog>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QDir>
#include <QFile>

SaveLoadManager::SaveLoadManager(QWidget* parent) : QObject(parent), m_parent(parent)
{
}

bool SaveLoadManager::hasSaveFile() const
{
    return QFile::exists("save/savegame.ini");
}

void SaveLoadManager::showSaveDialog()
{
    QDialog dialog(m_parent);
    dialog.setWindowTitle("存档");
    dialog.setFixedSize(400, 300);
    dialog.setStyleSheet("background-color: #2a2a3a; color: white;");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* title = new QLabel("选择存档槽位", &dialog);
    title->setAlignment(Qt::AlignCenter);
    QFont font = title->font();
    font.setPointSize(14);
    title->setFont(font);
    layout->addWidget(title);

    QListWidget* list = new QListWidget(&dialog);
    list->setStyleSheet("QListWidget { background-color: #1a1a2e; color: white; border: 1px solid #555; }"
        "QListWidget::item { padding: 10px; }"
        "QListWidget::item:selected { background-color: #4a4a6a; }");
    refreshList(list);
    layout->addWidget(list);

    QPushButton* saveBtn = new QPushButton("保存", &dialog);
    saveBtn->setStyleSheet("QPushButton { background-color: #4a6fa5; color: white; padding: 8px; border: none; }"
        "QPushButton:hover { background-color: #5a8fc5; }");
    layout->addWidget(saveBtn);

    connect(saveBtn, &QPushButton::clicked, [&]() {
        GameData::instance()->saveToFile();
        QMessageBox::information(&dialog, "提示", "存档成功！");
        emit saveCompleted();
        dialog.accept();
        });

    dialog.exec();
}

void SaveLoadManager::showLoadDialog()
{
    if (!hasSaveFile()) {
        QMessageBox::warning(m_parent, "提示", "没有找到存档文件。");
        return;
    }

    QDialog dialog(m_parent);
    dialog.setWindowTitle("读档");
    dialog.setFixedSize(400, 300);
    dialog.setStyleSheet("background-color: #2a2a3a; color: white;");

    QVBoxLayout* layout = new QVBoxLayout(&dialog);
    QLabel* title = new QLabel("选择存档", &dialog);
    title->setAlignment(Qt::AlignCenter);
    QFont font = title->font();
    font.setPointSize(14);
    title->setFont(font);
    layout->addWidget(title);

    QListWidget* list = new QListWidget(&dialog);
    list->setStyleSheet("QListWidget { background-color: #1a1a2e; color: white; border: 1px solid #555; }"
        "QListWidget::item { padding: 10px; }"
        "QListWidget::item:selected { background-color: #4a4a6a; }");
    refreshList(list);
    layout->addWidget(list);

    QPushButton* loadBtn = new QPushButton("读取", &dialog);
    loadBtn->setStyleSheet("QPushButton { background-color: #4a6fa5; color: white; padding: 8px; border: none; }"
        "QPushButton:hover { background-color: #5a8fc5; }");
    layout->addWidget(loadBtn);

    connect(loadBtn, &QPushButton::clicked, [&]() {
        GameData::instance()->loadFromFile();
        QMessageBox::information(&dialog, "提示", "读档成功！");
        emit loadCompleted();
        dialog.accept();
        });

    dialog.exec();
}

void SaveLoadManager::refreshList(QListWidget* list)
{
    list->clear();
    if (QFile::exists("save/savegame.ini")) {
        list->addItem("存档一 [有数据]");
    }
    else {
        list->addItem("存档一 [空]");
    }
}