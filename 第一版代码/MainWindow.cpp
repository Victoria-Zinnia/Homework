#include "MainWindow.h"
#include "SceneBase.h"
#include <QApplication>
#include <QKeyEvent>
#include <QTimer>

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent)
{
    m_currentScene = nullptr;
    setWindowTitle(QString::fromLocal8Bit("四时未竟书"));
    resize(1280, 720);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setScene(SceneBase* scene)
{
    // 关键：先强制处理完所有 pending 的动画/定时器事件，防止残留
    QApplication::processEvents(QEventLoop::ExcludeUserInputEvents);

    SceneBase* oldScene = m_currentScene;

    if (oldScene) {
        oldScene->onExit();
        oldScene->hide();
        takeCentralWidget();          // 移除旧场景，不删除
        oldScene->setParent(nullptr); // 断开 parent，防止重复删除
    }

    m_currentScene = scene;
    if (m_currentScene) {
        setCentralWidget(m_currentScene);
        m_currentScene->resize(size());
        m_currentScene->onEnter();
    }

    // 关键：延迟删除旧场景，确保 Qt 内部所有引用都释放
    if (oldScene) {
        QTimer::singleShot(0, oldScene, [oldScene]() {
            delete oldScene;
            });
    }
}

SceneBase* MainWindow::currentScene() const
{
    return m_currentScene;
}

void MainWindow::resizeEvent(QResizeEvent* event)
{
    QMainWindow::resizeEvent(event);
    if (m_currentScene) {
        m_currentScene->resize(size());
    }
}

void MainWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        close();
    }
    QMainWindow::keyPressEvent(event);
}