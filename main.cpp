#include <QApplication>
#include <QString>
#include "sensorworker.h"
#include "mainwindow.h"

/* 深色主题样式(全局 QSS) */
static const char *kStyle = R"(
QWidget {
    background-color: #0e1116;
    color: #dfe6ee;
    font-family: "WenQuanYi Micro Hei","Droid Sans Fallback","Noto Sans CJK SC",sans-serif;
}
QWidget#card {
    background-color: #171d26;
    border: 1px solid #263041;
    border-radius: 8px;
}
QWidget#panel {
    background-color: #171d26;
    border: 1px solid #263041;
    border-radius: 8px;
}
QLabel#caption { color: #7f8ea3; font-size: 14px; }
QLabel#value   { color: #e8f1fa; font-size: 30px; font-weight: bold; }
QLabel#unit    { color: #7f8ea3; font-size: 14px; }
QLabel#title   { color: #4dd0e1; font-size: 22px; font-weight: bold; }
QLabel#clock   { color: #b9c6d6; font-size: 14px; }
QLabel#dutyBig { color: #69f0ae; font-size: 26px; font-weight: bold; }
QLabel#modeTag { color: #171d26; font-size: 13px; font-weight: bold;
                 border-radius: 10px; padding: 2px 10px; }
QLabel#stBar   { color: #ffb74d; font-size: 12px; }
QLabel#stInfo  { color: #7f8ea3; font-size: 12px; }
QLabel#ledTxt  { color: #7f8ea3; font-size: 12px; }
QPushButton {
    background-color: #263041;
    color: #dfe6ee;
    border: none;
    border-radius: 4px;
    padding: 5px 14px;
    font-size: 13px;
}
QPushButton:pressed { background-color: #33415c; }
QRadioButton, QCheckBox { color: #dfe6ee; font-size: 14px; spacing: 6px; }
QSlider::groove:horizontal {
    height: 6px; background: #263041; border-radius: 3px;
}
QSlider::handle:horizontal {
    width: 22px; margin: -8px 0; border-radius: 11px;
    background: #4dd0e1;
}
QSpinBox {
    background-color: #0e1116;
    color: #dfe6ee;
    border: 1px solid #263041;
    border-radius: 4px;
    padding: 2px 6px;
    font-size: 15px;
}
QSpinBox::up-button, QSpinBox::down-button { width: 18px; }
)";

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("smart_agri");
    app.setOrganizationName("SmartAgri");
    app.setStyleSheet(QString::fromUtf8(kStyle));

    /* 跨线程队列信号的值类型需注册元类型 */
    qRegisterMetaType<Sample>("Sample");

    MainWindow w;
    /* 板上默认全屏显示; SMARTAGRI_WINDOWED=1 时开窗口(便于调试截图) */
    if (qgetenv("SMARTAGRI_WINDOWED") == "1")
        w.show();
    else
        w.showFullScreen();

    return app.exec();
}
