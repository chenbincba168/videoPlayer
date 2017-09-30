/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "glwidget.h"
#include <QDateTime>
#include <QKeyEvent>
#include <QImage>
#include <QTimer>
#include <QDebug>



GLWidget::GLWidget(QWidget *parent)
    : QGLWidget(QGLFormat(QGL::SampleBuffers), parent)
{
    //setFixedSize(800, 600);
    setAutoFillBackground(false);

    mPlayer = new VideoPlayer();
    connect(mPlayer, SIGNAL(sig_GetOneFrame(QImage)), this, SLOT(slotGetOneFrame(QImage)));

//    mPlayer2->setFileName("D:\\downloads\\test.mp4");
//    mPlayer->setFileName("F:\\movies\\H264-1Mbps.ts");
//    mPlayer->startPlay();
}

void GLWidget::animate()
{
    qDebug()<<"test1";
    update();
//    repaint();
}

void GLWidget::paintEvent(QPaintEvent *event)
{
    //qDebug()<<"start display picture"<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:zzz");

    QPainter painter(this);
    painter.setBrush(Qt::black);//设置画刷颜色
    painter.drawRect(0, 0, this->width(), this->height());//先画成黑色
    static int count = 0;
    count++;
    if (mImage.size().width() <= 0)
    {
        qDebug()<<"test"<<count;
        return;
    }

    ///将图像按比例缩放成和窗口一样大小
    QImage img = mImage.scaled(this->size(),Qt::KeepAspectRatio);

    int x = this->width() - img.width();
    int y = this->height() - img.height();

    x /= 2;
    y /= 2;

    painter.drawImage(QPoint(x,y),img); //画出图像
    //qDebug()<<"finish display picture"<<QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:zzz");
}

void GLWidget::keyPressEvent(QKeyEvent *event)
{
    qDebug()<<event->key();
    if(event->key() == Qt::Key_Escape)
    {
        qDebug()<<"keyPressEvent sub";
        this->setWindowFlags(Qt::SubWindow);
        this->showNormal();
        emit sizeChange(true);
    }
}
void GLWidget::slotGetOneFrame(QImage img)
{
#if 1
    mImage = img;
    update(); //调用update将执行 paintEvent函数
#else

    if (img.height()>0)
    {
        QPixmap pix = QPixmap::fromImage(img.scaled(this->width(),this->height()));
        ui->label->setPixmap(pix);
    }

#endif
}

void GLWidget::slotGetFilePath(QString path)
{
    //mPlayer2->setFileName("D:\\downloads\\test.mp4");
    mPlayer->setFileName(path.toLocal8Bit());
    mPlayer->startPlay();

}
