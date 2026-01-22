#include "MouseAssistantStd.h"
#include <QApplication>
#include <QFile>
#include <QTextStream>
#include <QTextCodec>
#include <QDebug>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    
    // 设置全局编码为UTF-8
    QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF-8"));
    
    // 加载QSS样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QFile::ReadOnly | QFile::Text)) {
        QTextStream stream(&styleFile);
        app.setStyleSheet(stream.readAll());
        styleFile.close();
    }

    MouseAssistantStd keyPresser;
    keyPresser.setWindowIcon(QIcon(":/mouse.ico"));
    keyPresser.setWindowTitle(QStringLiteral("鼠标助手 标准版"));


    keyPresser.show();
    return app.exec();
}
