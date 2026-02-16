﻿#include "MouseAssistantStd.h"
#include <windows.h>
#include <QVBoxLayout>
#include <QPushButton>
#include <QToolButton>
#include <QFrame>
#include <QGridLayout>
#include <QMessageBox>
#include <QIntValidator>
#include <QKeyEvent>
#include <QApplication>
#include <QSettings>
#include <QFileInfo>
#include <QDateTime>
#include <QRandomGenerator>
#include <QFileDialog>
#include "AboutMeDlg.h"
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>
#include <qlogging.h>

// 全局钩子处理函数

LRESULT CALLBACK MouseKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode >= 0) {
        KBDLLHOOKSTRUCT* kbStruct = (KBDLLHOOKSTRUCT*)lParam;
        if (MouseAssistantStd::instance) {
            int selectedKey = MouseAssistantStd::instance->triggerKeyComboBox->currentData().toInt();

            // 弹出框显示按键信息
            //QMessageBox::information(nullptr, "按键事件", QString("Key pressed: %1").arg(kbStruct->vkCode));

            // 检查是否是我们选择的按键
            if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
                if (kbStruct->vkCode == selectedKey || (selectedKey==164 && kbStruct->vkCode == 165)) {
                    //QMessageBox::information(nullptr, "触发事件", QString("Trigger key matched: %1").arg(selectedKey));
                    bool bRunning = (MouseAssistantStd::instance)->instructionLabel->text() == QStringLiteral("运行中");
                    bRunning ? (MouseAssistantStd::instance)->stopPressing() : (MouseAssistantStd::instance)->startPressing();
                    return 1; // 阻止进一步处理
                }
            }
        }
    }
    return CallNextHookEx(NULL, nCode, wParam, lParam);
}

HHOOK keyHook;
HHOOK mouseHook;

// 卸载钩子
void UninstallMouseHook() {
    if (mouseHook) {
        UnhookWindowsHookEx(mouseHook);
        mouseHook = NULL; // 清空钩子句柄
    }
}

LRESULT CALLBACK MouseHookProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION) {
        MSLLHOOKSTRUCT *mouseStruct = (MSLLHOOKSTRUCT *)lParam;
        if (wParam == WM_LBUTTONDOWN) {
            // 处理左键点击
            int x = mouseStruct->pt.x;
            int y = mouseStruct->pt.y;
            //label->setText(QString("点击位置: (%1, %2)").arg(x).arg(y));
            if((MouseAssistantStd::instance)->curPointEdit != nullptr){
                POINT pt = { x, y };
                ScreenToClient((MouseAssistantStd::instance)->targetHwnd, &pt);
                
                // 获取目标窗口的客户区大小
                RECT clientRect;
                GetClientRect((MouseAssistantStd::instance)->targetHwnd, &clientRect);
                int clientWidth = clientRect.right - clientRect.left;
                int clientHeight = clientRect.bottom - clientRect.top;
                
                // 计算相对坐标
                double relativeX = clientWidth > 0 ? (double)pt.x / clientWidth : 0;
                double relativeY = clientHeight > 0 ? (double)pt.y / clientHeight : 0;
                
                // 确定当前正在编辑的点索引
                int pointIndex = -1;
                for (int i = 0; i < 10; ++i) {
                    if ((MouseAssistantStd::instance)->curPointEdit == (MouseAssistantStd::instance)->pointEdits[i]) {
                        pointIndex = i;
                        break;
                    }
                }
                
                // 保存相对坐标和窗口大小信息
                if (pointIndex != -1) {
                    (MouseAssistantStd::instance)->pointInfos[pointIndex] = PointInfo(
                        relativeX, relativeY, clientWidth, clientHeight
                    );
                }
                
                (MouseAssistantStd::instance)->curPointEdit->setText(QString("%1,%2").arg(pt.x).arg(pt.y));
                (MouseAssistantStd::instance)->curPointEdit->setStyleSheet("color: black;");
            }
            UninstallMouseHook();
            return 1; // 返回非零值，表示事件已处理
        }
    }
    return CallNextHookEx(mouseHook, nCode, wParam, lParam); // 继续传递事件
}


void InstallMouseHook() {
    mouseHook = SetWindowsHookEx(WH_MOUSE_LL, MouseHookProc, NULL, 0);
}

// 安装钩子
void InstallKeyHook() {
    keyHook = SetWindowsHookEx(WH_KEYBOARD_LL, MouseKeyboardProc, GetModuleHandle(NULL), 0);
    //可以保存hook以便卸载
}

// 卸载钩子
void UninstallKeyHook() {
    UnhookWindowsHookEx(keyHook);
}

void sendMouseClick(HWND hwnd, int screenX, int screenY) {
    POINT pt = { screenX, screenY };
    //ScreenToClient(hwnd, &pt); // 转换为客户区坐标

    // 发送左键单击
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
}

void sendRightClick(HWND hwnd, int screenX, int screenY) {
    POINT pt = { screenX, screenY };
    //ScreenToClient(hwnd, &pt); // 转换为客户区坐标

    // 发送右键单击
    PostMessage(hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_RBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
}

void sendMiddleClick(HWND hwnd, int screenX, int screenY) {
    POINT pt = { screenX, screenY };
    //ScreenToClient(hwnd, &pt); // 转换为客户区坐标

    // 发送中键单击
    PostMessage(hwnd, WM_MBUTTONDOWN, MK_MBUTTON, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_MBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
}

void sendMouseDoubleClick(HWND hwnd, int screenX, int screenY) {
    POINT pt = { screenX, screenY };
    //ScreenToClient(hwnd, &pt); // 转换为客户区坐标

    // 发送左键双击
    PostMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_LBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, MAKELPARAM(pt.x, pt.y));
}

void sendRightDoubleClick(HWND hwnd, int screenX, int screenY) {
    POINT pt = { screenX, screenY };
    //ScreenToClient(hwnd, &pt); // 转换为客户区坐标

    // 发送右键双击
    PostMessage(hwnd, WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_RBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_RBUTTONDBLCLK, MK_RBUTTON, MAKELPARAM(pt.x, pt.y));
}

void sendMiddleDoubleClick(HWND hwnd, int screenX, int screenY) {
    POINT pt = { screenX, screenY };
    //ScreenToClient(hwnd, &pt); // 转换为客户区坐标

    // 发送中键双击
    PostMessage(hwnd, WM_MBUTTONDOWN, MK_MBUTTON, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_MBUTTONUP, 0, MAKELPARAM(pt.x, pt.y));
    PostMessage(hwnd, WM_MBUTTONDBLCLK, MK_MBUTTON, MAKELPARAM(pt.x, pt.y));
}

void sendMouseMsg(HWND hwnd, int screenX, int screenY, int MsgType){
    switch(MsgType){
    case 0:
        sendMouseClick(hwnd, screenX, screenY);
        break;
    case 1:
        sendMouseDoubleClick(hwnd, screenX, screenY);
        break;
    case 2:
        sendRightClick(hwnd, screenX, screenY);
        break;
    case 3:
        sendRightDoubleClick(hwnd, screenX, screenY);
        break;
    case 4:
        sendMiddleClick(hwnd, screenX, screenY);
        break;
    case 5:
        sendMiddleDoubleClick(hwnd, screenX, screenY);
        break;
    }
}

MouseAssistantStd *MouseAssistantStd::instance = nullptr;

MouseAssistantStd::MouseAssistantStd(QWidget *parent) : QWidget(parent) {
    setWindowTitle(QStringLiteral("鼠标助手标准版"));
    setWindowIcon(QIcon(":/png/mouse.ico"));
    instance = this;
    resize(300, 400);

    // 设置窗口的大小策略：宽度可扩展，高度自适应最小值
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Minimum);
    QVBoxLayout *globalLayout = new QVBoxLayout(this);
    globalLayout->setContentsMargins(0, 0, 0, 0);

    QVBoxLayout *layout = new QVBoxLayout();
    layout->setContentsMargins(9, 0, 9, 9);

    // 创建顶部按钮
    QToolButton *clearButton = new QToolButton(this);
    clearButton->setIcon(QIcon(":/png/reset.png"));
    clearButton->setText(QStringLiteral("重置"));
    clearButton->setToolTip(QStringLiteral("重置设置"));
    clearButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QToolButton *aboutButton = new QToolButton(this);
    aboutButton->setIcon(QIcon(":/png/about.png"));
    aboutButton->setText(QStringLiteral("关于"));
    aboutButton->setToolTip(QStringLiteral("关于"));
    aboutButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QToolButton *helpButton = new QToolButton(this);
    helpButton->setIcon(QIcon(":/png/help.png"));
    helpButton->setText(QStringLiteral("帮助"));
    helpButton->setToolTip(QStringLiteral("帮助"));
    helpButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    connect(helpButton, &QPushButton::clicked, this, []() {
        QDesktopServices::openUrl(QUrl("https://ocn1lplwg6gi.feishu.cn/wiki/Ual4wc4SxiKIodkuSSUcpLBTnsc?from=from_copylink"));
    });

    // 添加导入按钮
    QToolButton *importButton = new QToolButton(this);
    importButton->setIcon(QIcon(":/png/import.png"));
    importButton->setText(QStringLiteral("导入"));
    importButton->setToolTip(QStringLiteral("从文件导入设置"));
    importButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    // 添加导出按钮
    QToolButton *exportButton = new QToolButton(this);
    exportButton->setIcon(QIcon(":/png/export.png"));
    exportButton->setText(QStringLiteral("导出"));
    exportButton->setToolTip(QStringLiteral("导出设置到文件"));
    exportButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    QFrame *toolButtonFrame = new QFrame();
    toolButtonFrame->setFixedHeight(30);
    toolButtonFrame->setStyleSheet("QFrame { border-bottom: 1px solid #cccccc; border-radius: 0px; padding-bottom: 0px; background-color: #dadcde; }");
    QHBoxLayout *toolButtonLayout = new QHBoxLayout(toolButtonFrame);
    toolButtonLayout->setSpacing(0);
    toolButtonLayout->setContentsMargins(2, 1, 0, 0);
    globalLayout->addWidget(toolButtonFrame);

    // 在布局部分添加按钮
    toolButtonLayout->addWidget(clearButton);
    toolButtonLayout->addWidget(importButton);    // 添加导入按钮
    toolButtonLayout->addWidget(exportButton);    // 添加导出按钮
    toolButtonLayout->addWidget(aboutButton);
    toolButtonLayout->addWidget(helpButton);
    toolButtonLayout->addStretch();
    layout->addLayout(toolButtonLayout);

    connect(clearButton, &QPushButton::clicked, this, &MouseAssistantStd::clearSettings);
    connect(aboutButton, &QPushButton::clicked, this, &MouseAssistantStd::aboutMe);

    // 添加按钮事件处理
    connect(importButton, &QPushButton::clicked, this, [this]() {
        QString filename = QFileDialog::getOpenFileName(
            this,
            QStringLiteral("导入设置"),
            QDir::currentPath(),
            QStringLiteral("MouseAssistantStd设置文件 (*.masset)"));
            
        if (!filename.isEmpty()) {
            loadSettingsFromFile(filename);
            QMessageBox::information(this, QStringLiteral("成功"), 
                QStringLiteral("设置已成功导入！"));
        }
    });

    connect(exportButton, &QPushButton::clicked, this, [this]() {
        QString defaultFileName = QString("MouseAssistantStd_%1.masset")
                .arg(QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss"));
            QString filename = QFileDialog::getSaveFileName(
                this,
                QStringLiteral("导出设置"),
                QDir::currentPath() + "/" + defaultFileName,
                QStringLiteral("MouseAssistantStd设置文件 (*.masset)"));
            
        if (!filename.isEmpty()) {
            if(!filename.endsWith(".masset", Qt::CaseInsensitive)) {
                filename += ".masset";
            }
            saveSettingsToFile(filename);
            QMessageBox::information(this, QStringLiteral("成功"), 
                QStringLiteral("设置已成功导出！"));
        }
    });

    QLabel *label = new QLabel(QStringLiteral("选择窗口:"), this);
    layout->addWidget(label);

    selectedWindowLabel = new QLabel(QStringLiteral("未选择窗口"), this);
    selectedWindowLabel->setStyleSheet("color: green;");
    layout->addWidget(selectedWindowLabel);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    QPushButton *selectButton = new QPushButton(QStringLiteral("选择窗口"), this);
    buttonLayout->addWidget(selectButton);

    QPushButton *highlightButton = new QPushButton(QStringLiteral("凸显所选窗口"), this);
    buttonLayout->addWidget(highlightButton);

    connect(selectButton, &QPushButton::clicked, this, &MouseAssistantStd::selectWindow);
    connect(highlightButton, &QPushButton::clicked, this, &MouseAssistantStd::highlightWindow);

    layout->addLayout(buttonLayout);

    topmostCheckBox = new QCheckBox(QStringLiteral("保持选择窗口激活置顶（推荐勾选）"), this);
    layout->addWidget(topmostCheckBox);

    backgroundModeCheckBox = new QCheckBox(QStringLiteral("后台执行模式（不激活窗口）"), this);
    layout->addWidget(backgroundModeCheckBox);

    connect(topmostCheckBox, &QCheckBox::stateChanged, this, &MouseAssistantStd::onTopmostCheckBoxChanged);


    QLabel *keysLabel = new QLabel(QStringLiteral("自定义鼠标消息和点击位置、时间间隔 (毫秒) [范围]:"), this);
    layout->addWidget(keysLabel);

    QGridLayout *keysLayout = new QGridLayout();
    for (int i = 0; i < 10; ++i) {
        keyCheckBoxes[i] = new QCheckBox(this);
        keysLayout->addWidget(keyCheckBoxes[i], i, 0);

        keyCombos[i] = new QComboBox(this);
        populateKeyCombos(keyCombos[i]);
        keysLayout->addWidget(keyCombos[i], i, 1);

        pointEdits[i] = new QLineEdit(this);
        pointEdits[i]->setEnabled(false);
        pointEdits[i]->setPlaceholderText(QStringLiteral("未选择点击位置"));
        keysLayout->addWidget(pointEdits[i], i, 2);

        selectPointButton[i] = new QPushButton(this);
        selectPointButton[i]->setIcon(QIcon(":/png/selPt.png"));
        selectPointButton[i]->setToolTip(QStringLiteral("选择点击位置"));
        keysLayout->addWidget(selectPointButton[i], i, 3);
        connect(selectPointButton[i], &QPushButton::clicked,this, &MouseAssistantStd::onSelectPointButtonClicked);

        intervalLineEdits[i] = new QLineEdit(this);
        intervalLineEdits[i]->setValidator(new QIntValidator(this));
        intervalLineEdits[i]->setText("1000");
        intervalLineEdits[i]->setPlaceholderText(QStringLiteral("最小值"));
        keysLayout->addWidget(intervalLineEdits[i], i, 4);

        QLabel *dashLabel2 = new QLabel("-", this);
        keysLayout->addWidget(dashLabel2, i, 5);

        maxIntervalLineEdits[i] = new QLineEdit(this);
        maxIntervalLineEdits[i]->setValidator(new QIntValidator(this));
        maxIntervalLineEdits[i]->setText("1000");
        maxIntervalLineEdits[i]->setPlaceholderText(QStringLiteral("最大值"));
        keysLayout->addWidget(maxIntervalLineEdits[i], i, 6);

        QTimer *timer = new QTimer(this);
        connect(timer, &QTimer::timeout, [this, i]() {
            pressKeys(i);
            int minInterval = intervalLineEdits[i]->text().toInt();
            int maxInterval = maxIntervalLineEdits[i]->text().toInt();
            if (minInterval > maxInterval) std::swap(minInterval, maxInterval);
            int interval = getRandomInterval(minInterval, maxInterval);
            timers[i]->start(interval);
        });
        timers.push_back(timer);
    }
    layout->addLayout(keysLayout);

    QPushButton *startButton = new QPushButton(QStringLiteral("开始"), this);
    layout->addWidget(startButton);

    QPushButton *stopButton = new QPushButton(QStringLiteral("停止"), this);
    layout->addWidget(stopButton);

    // 添加“快捷键设置”标签和组合框
    QHBoxLayout *shortcutLayout = new QHBoxLayout();
    shortcutLayout->addStretch();
    QLabel *shortcutLabel = new QLabel(QStringLiteral("开始\\停止快捷键设置:"), this);
    shortcutLayout->addWidget(shortcutLabel);

    triggerKeyComboBox = new QComboBox(this);
    // 添加特殊按键（不易冲突的按键）
    triggerKeyComboBox->addItem("Home", VK_HOME);
    triggerKeyComboBox->addItem("*", 106);
    triggerKeyComboBox->addItem("Alt", 164);
    triggerKeyComboBox->addItem("Insert", VK_INSERT);
    triggerKeyComboBox->addItem("End", VK_END);
    triggerKeyComboBox->addItem("Page Up", VK_PRIOR);
    triggerKeyComboBox->addItem("Page Down", VK_NEXT);
    triggerKeyComboBox->addItem("Esc", VK_ESCAPE);
    // 添加功能键（可能会冲突，但仍可选择）
    triggerKeyComboBox->insertSeparator(triggerKeyComboBox->count());
    triggerKeyComboBox->addItem("F1", VK_F1);
    triggerKeyComboBox->addItem("F2", VK_F2);
    triggerKeyComboBox->addItem("F3", VK_F3);
    triggerKeyComboBox->addItem("F4", VK_F4);
    triggerKeyComboBox->addItem("F5", VK_F5);
    triggerKeyComboBox->addItem("F6", VK_F6);
    triggerKeyComboBox->addItem("F7", VK_F7);
    triggerKeyComboBox->addItem("F8", VK_F8);
    triggerKeyComboBox->addItem("F9", VK_F9);
    triggerKeyComboBox->addItem("F10", VK_F10);
    triggerKeyComboBox->addItem("F11", VK_F11);
    triggerKeyComboBox->addItem("F12", VK_F12);

    // 设置浮动提示
    triggerKeyComboBox->setToolTip(QStringLiteral("建议选择不会与自定义按键冲突的快捷键\n特殊按键(推荐): Home、*、Alt等\n功能键(谨慎使用): F1-F12"));
    shortcutLabel->setToolTip(QStringLiteral("鼠标悬停在下拉框上可查看按键选择建议"));
    shortcutLayout->addWidget(triggerKeyComboBox);

    layout->addLayout(shortcutLayout);

    QLabel *labelPrompt = new QLabel(QStringLiteral("注意：请在选择点击位置后，不要修改目标窗口大小，\n否则将导致点击位置不准确"), this);
    labelPrompt->setStyleSheet("color: red;");
    layout->addWidget(labelPrompt);

    instructionLabel = new QLabel(QStringLiteral("停止中"), this);
    instructionLabel->setStyleSheet("color: green;");
    layout->addWidget(instructionLabel);

    connect(selectButton, &QPushButton::clicked, this, &MouseAssistantStd::selectWindow);
    connect(startButton, &QPushButton::clicked, this, &MouseAssistantStd::startPressing);
    connect(stopButton, &QPushButton::clicked, this, &MouseAssistantStd::stopPressing);

    globalLayout->addLayout(layout);

    loadSettings();
    InstallKeyHook();
}

MouseAssistantStd::~MouseAssistantStd() {
    saveSettings();
    UninstallKeyHook();
}

void MouseAssistantStd::highlightWindow() {
    if (targetHwnd) {
        // 如果窗口最小化，先恢复窗口
        if (IsIconic(targetHwnd)) {
            ShowWindow(targetHwnd, SW_RESTORE);
        }

        // 将窗口置于前台并闪烁
        SetForegroundWindow(targetHwnd);
        FlashWindow(targetHwnd, TRUE);
        raise();
        activateWindow();

        // 获取窗口的矩形区域
        RECT rect;
        GetWindowRect(targetHwnd, &rect);

        // 获取窗口的设备上下文
        HDC hdc = GetWindowDC(NULL);  // 使用屏幕的设备上下文

        // 创建一个红色画笔
        HPEN hRedPen = CreatePen(PS_SOLID, 3, RGB(255, 0, 0));
        HPEN hWhitePen = CreatePen(PS_SOLID, 3, RGB(255, 255, 255));

        int borderWidth = 3;
        for(int i = 0; i < 10; i++){
            SelectObject(hdc, i%2 ? hRedPen : hWhitePen);
            // 绘制红色框，稍微扩大边界以避免遮盖
            Rectangle(hdc, rect.left - borderWidth, rect.top - borderWidth, rect.right + borderWidth, rect.bottom + borderWidth);
            Sleep(30);
        }

        // 释放资源
        //SelectObject(hdc, hOldPen);
        DeleteObject(hRedPen);
        DeleteObject(hWhitePen);
        ReleaseDC(NULL, hdc);

        // 等待3秒
        //Sleep(2000);

        // 重新绘制窗口以清除红色框
        InvalidateRect(targetHwnd, NULL, TRUE);
        UpdateWindow(targetHwnd);

    } else {
        QMessageBox::warning(this, QStringLiteral("警告"), QStringLiteral("请先选择一个窗口！"));
    }
}

void MouseAssistantStd::onTopmostCheckBoxChanged(int state)
{
    bool topmost = (state == Qt::Checked);
    if(!topmost) SetWindowPos(targetHwnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
}

void MouseAssistantStd::populateKeyCombos(QComboBox *comboBox) {
    comboBox->addItem(QStringLiteral("左键单击"), 0);
    comboBox->addItem(QStringLiteral("左键双击"), 1);
    comboBox->addItem(QStringLiteral("右键单击"), 2);
    comboBox->addItem(QStringLiteral("右键双击"), 3);
    comboBox->addItem(QStringLiteral("滚轮单击"), 4);
    comboBox->addItem(QStringLiteral("滚轮双击"), 5);
}

void MouseAssistantStd::selectWindow() {
    SetWinEventHook(EVENT_SYSTEM_FOREGROUND, EVENT_SYSTEM_FOREGROUND, NULL, WinEventProc, 0, 0, WINEVENT_OUTOFCONTEXT);
    selectedWindowLabel->setText(QStringLiteral("请点击目标窗口..."));
}

void MouseAssistantStd::startPressing() {
    qDebug() << "startPressing";

    if(!targetHwnd)
    {
        QMessageBox::warning(this,QStringLiteral("警告"),QStringLiteral("请选择窗口后，再点击开始！"));
        return;
    }
    bIsRunning = true;
    instructionLabel->setText(QStringLiteral("运行中"));
    stopAllTimers();
    //attachToTargetWindow();
    if (targetHwnd) {
        for (int i = 0; i < 10; ++i) {
            if (keyCheckBoxes[i]->isChecked() && keyCombos[i]->currentIndex() != -1) {
                pressKeys(i);
                int minInterval = intervalLineEdits[i]->text().toInt();
                int maxInterval = maxIntervalLineEdits[i]->text().toInt();
                if (minInterval > maxInterval) std::swap(minInterval, maxInterval);
                int interval = getRandomInterval(minInterval, maxInterval);
                timers[i]->start(interval);
            }
        }
    }
}

void MouseAssistantStd::stopPressing() {
    qDebug() << "stopPressing";
    bIsRunning = false;
    instructionLabel->setText(QStringLiteral("停止中"));
    stopAllTimers();
    detachFromTargetWindow();
}

void MouseAssistantStd::aboutMe()
{
    AboutMeDlg dlg(this);
    dlg.exec();
}

void MouseAssistantStd::stopAllTimers() {
    for (QTimer *timer : timers) {
        timer->stop();
    }
}


void MouseAssistantStd::pressKeys(int index) {
    if (targetHwnd) {
        // 检查是否启用后台执行模式
        bool isBackgroundMode = backgroundModeCheckBox->isChecked();
        
        // 只有在非后台模式且勾选了置顶选项时才激活窗口
        if(!isBackgroundMode && topmostCheckBox->isChecked()){
            // 如果窗口最小化则先恢复窗口
            if (IsIconic(targetHwnd)) {
                ShowWindow(targetHwnd, SW_RESTORE);
            }
            SetWindowPos(targetHwnd, topmostCheckBox->isChecked() ? HWND_TOPMOST : HWND_NOTOPMOST,
                         0, 0, 0, 0, SWP_SHOWWINDOW | SWP_NOMOVE | SWP_NOSIZE);
        }
        
        int key = keyCombos[index]->currentData().toInt();
        QString point = pointEdits[index]->text();
        auto pt = point.split(",");
        if(pt.size() != 2) return;
        
        // 获取当前窗口的客户区大小
        RECT clientRect;
        GetClientRect(targetHwnd, &clientRect);
        int currentWidth = clientRect.right - clientRect.left;
        int currentHeight = clientRect.bottom - clientRect.top;
        
        // 检查是否有保存的相对坐标信息
        if (pointInfos[index].windowWidth > 0 && pointInfos[index].windowHeight > 0) {
            // 使用相对坐标计算当前窗口的绝对坐标
            int newX = (int)(pointInfos[index].relativeX * currentWidth);
            int newY = (int)(pointInfos[index].relativeY * currentHeight);
            sendMouseMsg(targetHwnd, newX, newY, key);
        } else {
            // 如果没有相对坐标信息，使用原始绝对坐标
            sendMouseMsg(targetHwnd, pt[0].toInt(), pt[1].toInt(), key);
        }
    }
}

int MouseAssistantStd::getRandomInterval(int minInterval, int maxInterval) {
    return (minInterval == maxInterval) ? minInterval : QRandomGenerator::global()->bounded(minInterval, maxInterval + 1);
}

void MouseAssistantStd::loadSettings() {
    QSettings settings("FinnSoft", "MouseAssistantStd");
    triggerKeyComboBox->setCurrentIndex(settings.value("triggerKeyComboBox", 0).toInt());
    topmostCheckBox->setChecked(settings.value("topmostCheckBox", false).toBool());
    backgroundModeCheckBox->setChecked(settings.value("backgroundModeCheckBox", false).toBool());
    
    for (int i = 0; i < 10; ++i) {
        keyCheckBoxes[i]->setChecked(settings.value(QString("keyCheckBox%1").arg(i), false).toBool());
        keyCombos[i]->setCurrentIndex(settings.value(QString("keyCombo%1").arg(i), 0).toInt());
        intervalLineEdits[i]->setText(settings.value(QString("intervalLineEdit%1").arg(i), "1000").toString());
        maxIntervalLineEdits[i]->setText(settings.value(QString("maxIntervalLineEdit%1").arg(i), "1000").toString());
        pointEdits[i]->setText(settings.value(QString("pointEdit%1").arg(i), "").toString());
        
        // 加载相对坐标信息
        pointInfos[i].relativeX = settings.value(QString("pointInfo%1_relativeX").arg(i), 0).toDouble();
        pointInfos[i].relativeY = settings.value(QString("pointInfo%1_relativeY").arg(i), 0).toDouble();
        pointInfos[i].windowWidth = settings.value(QString("pointInfo%1_windowWidth").arg(i), 0).toInt();
        pointInfos[i].windowHeight = settings.value(QString("pointInfo%1_windowHeight").arg(i), 0).toInt();
    }
}

void MouseAssistantStd::saveSettings() {
    QSettings settings("FinnSoft", "MouseAssistantStd");

    settings.setValue("triggerKeyComboBox",triggerKeyComboBox->currentIndex());
    settings.setValue("topmostCheckBox", topmostCheckBox->isChecked());
    settings.setValue("backgroundModeCheckBox", backgroundModeCheckBox->isChecked());
    
    for (int i = 0; i < 10; ++i) {
        settings.setValue(QString("keyCheckBox%1").arg(i), keyCheckBoxes[i]->isChecked());
        settings.setValue(QString("keyCombo%1").arg(i), keyCombos[i]->currentIndex());
        settings.setValue(QString("intervalLineEdit%1").arg(i), intervalLineEdits[i]->text());
        settings.setValue(QString("maxIntervalLineEdit%1").arg(i), maxIntervalLineEdits[i]->text());
        settings.setValue(QString("pointEdit%1").arg(i), pointEdits[i]->text());
        
        // 保存相对坐标信息
        settings.setValue(QString("pointInfo%1_relativeX").arg(i), pointInfos[i].relativeX);
        settings.setValue(QString("pointInfo%1_relativeY").arg(i), pointInfos[i].relativeY);
        settings.setValue(QString("pointInfo%1_windowWidth").arg(i), pointInfos[i].windowWidth);
        settings.setValue(QString("pointInfo%1_windowHeight").arg(i), pointInfos[i].windowHeight);
    }
}

void MouseAssistantStd::clearSettings() {
    QSettings settings("FinnSoft", "MouseAssistantStd");
    settings.clear();  // 清除所有设置

    // 重置界面上的控件为默认值
    topmostCheckBox->setChecked(false);
    backgroundModeCheckBox->setChecked(false);

    for (int i = 0; i < 10; ++i) {
        keyCheckBoxes[i]->setChecked(false);
        keyCombos[i]->setCurrentIndex(0);
        intervalLineEdits[i]->setText("1000");
        maxIntervalLineEdits[i]->setText("1000");
        pointEdits[i]->setText("");
        
        // 重置相对坐标信息
        pointInfos[i] = PointInfo();
    }
}

// 新增：从配置文件加载设置
void MouseAssistantStd::loadSettingsFromFile(const QString &filename) {
    QSettings fileSettings(filename, QSettings::IniFormat);
    
    triggerKeyComboBox->setCurrentIndex(fileSettings.value("triggerKeyComboBox", 0).toInt());
    topmostCheckBox->setChecked(fileSettings.value("topmostCheckBox", false).toBool());
    backgroundModeCheckBox->setChecked(fileSettings.value("backgroundModeCheckBox", false).toBool());
    
    for (int i = 0; i < 10; ++i) {
        keyCheckBoxes[i]->setChecked(fileSettings.value(QString("keyCheckBox%1").arg(i), false).toBool());
        keyCombos[i]->setCurrentIndex(fileSettings.value(QString("keyCombo%1").arg(i), 0).toInt());
        intervalLineEdits[i]->setText(fileSettings.value(QString("intervalLineEdit%1").arg(i), "1000").toString());
        maxIntervalLineEdits[i]->setText(fileSettings.value(QString("maxIntervalLineEdit%1").arg(i), "1000").toString());
        pointEdits[i]->setText(fileSettings.value(QString("pointEdit%1").arg(i), "").toString());
        
        // 加载相对坐标信息
        pointInfos[i].relativeX = fileSettings.value(QString("pointInfo%1_relativeX").arg(i), 0).toDouble();
        pointInfos[i].relativeY = fileSettings.value(QString("pointInfo%1_relativeY").arg(i), 0).toDouble();
        pointInfos[i].windowWidth = fileSettings.value(QString("pointInfo%1_windowWidth").arg(i), 0).toInt();
        pointInfos[i].windowHeight = fileSettings.value(QString("pointInfo%1_windowHeight").arg(i), 0).toInt();
    }
}

// 新增：保存设置到配置文件
void MouseAssistantStd::saveSettingsToFile(const QString &filename) {
    QSettings fileSettings(filename, QSettings::IniFormat);
    fileSettings.clear(); // 清除可能存在的旧设置
    
    fileSettings.setValue("triggerKeyComboBox",triggerKeyComboBox->currentIndex());
    fileSettings.setValue("topmostCheckBox", topmostCheckBox->isChecked());
    fileSettings.setValue("backgroundModeCheckBox", backgroundModeCheckBox->isChecked());
    
    for (int i = 0; i < 10; ++i) {
        fileSettings.setValue(QString("keyCheckBox%1").arg(i), keyCheckBoxes[i]->isChecked());
        fileSettings.setValue(QString("keyCombo%1").arg(i), keyCombos[i]->currentIndex());
        fileSettings.setValue(QString("intervalLineEdit%1").arg(i), intervalLineEdits[i]->text());
        fileSettings.setValue(QString("maxIntervalLineEdit%1").arg(i), maxIntervalLineEdits[i]->text());
        fileSettings.setValue(QString("pointEdit%1").arg(i), pointEdits[i]->text());
        
        // 保存相对坐标信息
        fileSettings.setValue(QString("pointInfo%1_relativeX").arg(i), pointInfos[i].relativeX);
        fileSettings.setValue(QString("pointInfo%1_relativeY").arg(i), pointInfos[i].relativeY);
        fileSettings.setValue(QString("pointInfo%1_windowWidth").arg(i), pointInfos[i].windowWidth);
        fileSettings.setValue(QString("pointInfo%1_windowHeight").arg(i), pointInfos[i].windowHeight);
    }
}

void MouseAssistantStd::onSelectPointButtonClicked()
{
    if(!targetHwnd)
    {
        QMessageBox::warning(this,QStringLiteral("警告"),QStringLiteral("请先选择窗口！"));
        return;
    }
    QPushButton* button = qobject_cast<QPushButton*>(QObject::sender());
    if (button) {
        for(int i = 0; i < 10; i++){
            if(button == selectPointButton[i]){
                //QMessageBox::information(nullptr, "提示", QString::number(i) + " 被点击了！");
                pointEdits[i]->setStyleSheet("color: red;");
                pointEdits[i]->setText(QStringLiteral("请点击目标位置"));
                curPointEdit = pointEdits[i];
                InstallMouseHook();
                return;
            }
        }
    }
}

void MouseAssistantStd::attachToTargetWindow() {
    if (targetHwnd) {
        DWORD targetThreadId = GetWindowThreadProcessId(targetHwnd, NULL);
        DWORD currentThreadId = GetCurrentThreadId();
        AttachThreadInput(currentThreadId, targetThreadId, TRUE);  // 关联当前线程与目标窗口的线程
    }
}

void MouseAssistantStd::detachFromTargetWindow() {
    if (targetHwnd) {
        DWORD targetThreadId = GetWindowThreadProcessId(targetHwnd, NULL);
        DWORD currentThreadId = GetCurrentThreadId();
        AttachThreadInput(currentThreadId, targetThreadId, FALSE);  // 解除当前线程与目标窗口的线程关联
    }
}

void CALLBACK MouseAssistantStd::WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime) {
    if (event == EVENT_SYSTEM_FOREGROUND) {
        wchar_t windowTitle[256];
        GetWindowText(hwnd, windowTitle, 256);
        if (MouseAssistantStd::instance) {
            MouseAssistantStd::instance->targetHwnd = hwnd;
            MouseAssistantStd::instance->selectedWindowLabel->setText(QString::fromWCharArray(windowTitle));
            UnhookWinEvent(hWinEventHook);
            MouseAssistantStd::instance->raise();
            MouseAssistantStd::instance->activateWindow();
        }
    }
}

