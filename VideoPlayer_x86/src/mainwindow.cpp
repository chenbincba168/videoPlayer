#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDateTime>
#include <QPainter>
#include <QFileDialog>
#include <QMessageBox>
#include <QSurfaceFormat>
#include <QOpenGLContext>
#include <QOpenGLShaderProgram>
#include <QLabel>
#include <QThread>


MainWindow::MainWindow(QWidget *parent):
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    openGL = new GLWidget(this);
//    int w = this->width();
//    int h = this->height();
    //openGL->setFixedSize(1366, 768);
    openGL->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    openGL->animate();
    ui->gridLayout->addWidget(openGL, 0, 1);

    openGL->installEventFilter(this);

    connect(openGL, SIGNAL(sizeChange(bool)), this, SLOT(set_window_active(bool)));

    QLabel *openGLLabel = new QLabel(tr("OpenGL"), this);
    openGLLabel->setAlignment(Qt::AlignHCenter);
    ui->gridLayout->addWidget(openGLLabel, 1, 1);

    //qDebug()<<"MainWindow: thread ID is "<<QThread::currentThreadId();//主线程ID
}

MainWindow::~MainWindow()
{
    delete ui;
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick)
    {
        if(obj == openGL)
        {
            qDebug()<<"mouse double click";
            if(openGL->isFullScreen())
            {
                openGL->setWindowFlags(Qt::SubWindow);
                openGL->showNormal();

                qDebug()<<"set_window_active 2";
                this->setWindowState(Qt::WindowActive);
                activateWindow();//设置成顶级城口
            }
            else
            {
                //openGL->setWindowFlags(Qt::Dialog);
                openGL->setWindowFlags(Qt::Window);
                openGL->showFullScreen();
            }
        }
    }
    return QObject::eventFilter(obj, event);
}


void MainWindow::keyPressEvent(QKeyEvent *event)
{
    qDebug()<<"keyPressEvent window";
    qDebug()<<event->key();
    if((event->key() == Qt::Key_Return) || (event->key() == Qt::Key_Enter))//qt回车键需要判断这两个值
    {
        openGL->setWindowFlags(Qt::Window);
        openGL->showFullScreen();
    }
}

void MainWindow::on_action_open_triggered()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Video Files(*.*)"));
    if(path.length() == 0)
    {
        //QMessageBox::information(NULL, tr("Path"), tr("You didn't select any files."));
    }
    else
    {
        //QMessageBox::information(NULL, tr("Path"), tr("You selected ") + path);
        ui->lineEdit->setText(path);
    }
}

void MainWindow::on_pushButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Open Image"), ".", tr("Video Files(*.*)"));
    if(path.length() == 0)
    {
        //QMessageBox::information(NULL, tr("Path"), tr("You didn't select any files."));
    }
    else
    {
        //QMessageBox::information(NULL, tr("Path"), tr("You selected ") + path);
        ui->lineEdit->setText(path);
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    QString path = ui->lineEdit->text();

    VPlayer = new VideoPlayer;

    VPlayer->setFileName(path);
    VPlayer->startPlay();

    APlayer = new AudioPlayer;
    APlayer->setFileName(path);
    APlayer->startPlay();

    connect(VPlayer, SIGNAL(sig_GetOneFrame(QImage)), openGL, SLOT(slotGetOneFrame(QImage)));
    connect(APlayer, SIGNAL(send_audio_clock(double)), VPlayer, SLOT(get_audio_clock(double)));
}

void MainWindow::set_window_active(bool flag)//true为窗口变小
{
    if(flag == true)
    {
        qDebug()<<"set_window_active";
        this->setWindowState(Qt::WindowActive);
        activateWindow();//设置成顶级城口
    }
}
