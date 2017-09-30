#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QImage>
#include <QDebug>
#include <QPaintEvent>
#include <QTimer>
#include "videoplayer.h"
#include "audioplayer.h"
#include "glwidget.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
protected:
    bool eventFilter(QObject *obj, QEvent *event);
    void keyPressEvent(QKeyEvent *event);

private:
    Ui::MainWindow *ui;
    VideoPlayer *VPlayer; //视频播放线程
    AudioPlayer *APlayer; //音频播放线程
    GLWidget *openGL; //
    QImage mImage; //记录当前的图像
    QString FileName;
    QTimer *decode_timer;

private slots:
    void on_action_open_triggered();
    void on_pushButton_clicked();
    void on_pushButton_2_clicked();
    void set_window_active(bool flag);

};

#endif // MAINWINDOW_H
