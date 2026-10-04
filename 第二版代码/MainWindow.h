#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPointer>  // ← 新增

class SceneBase;

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

    void setScene(SceneBase* scene);
    SceneBase* currentScene() const;

protected:
    void resizeEvent(QResizeEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;

private:
    QPointer<SceneBase> m_currentScene;  // ← 改成 QPointer
};

#endif // MAINWINDOW_H

