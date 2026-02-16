#ifndef MouseAssistantStd_H
#define MouseAssistantStd_H

#include <QWidget>
#include <QTimer>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QSettings>
#include <windows.h>
#include <vector>
#include <QGroupBox>
#include <QPushButton>

// 存储相对坐标和窗口大小信息的结构体
struct PointInfo {
    double relativeX;  // 相对X坐标（0-1）
    double relativeY;  // 相对Y坐标（0-1）
    int windowWidth;   // 保存时的窗口宽度
    int windowHeight;  // 保存时的窗口高度
    
    PointInfo() : relativeX(0), relativeY(0), windowWidth(0), windowHeight(0) {}
    PointInfo(double x, double y, int width, int height) : 
        relativeX(x), relativeY(y), windowWidth(width), windowHeight(height) {}
};

class MouseAssistantStd : public QWidget {
    Q_OBJECT

public:
    MouseAssistantStd(QWidget *parent = nullptr);
    ~MouseAssistantStd();

    static MouseAssistantStd *instance;
    QLabel *instructionLabel;
    QComboBox *triggerKeyComboBox;  // 新增的控件声明
    QLineEdit *curPointEdit = nullptr;
    HWND targetHwnd = nullptr;

    bool bIsRunning = false;
public slots:
    void selectWindow();
    void startPressing();
    void stopPressing();
    void aboutMe();
    void pressKeys(int index);
    void clearSettings();
    void onSelectPointButtonClicked();
    void loadSettingsFromFile(const QString &filename);
    void saveSettingsToFile(const QString &filename);
private:
    static void CALLBACK WinEventProc(HWINEVENTHOOK hWinEventHook, DWORD event, HWND hwnd, LONG idObject, LONG idChild, DWORD dwEventThread, DWORD dwmsEventTime);

    QCheckBox *topmostCheckBox;
    QCheckBox *backgroundModeCheckBox;  // 后台执行模式选项
    QLabel *selectedWindowLabel;
    QCheckBox *keyCheckBoxes[10];
    QComboBox *keyCombos[10];
    QLineEdit *intervalLineEdits[10];
    QLineEdit *maxIntervalLineEdits[10];
    QLineEdit *pointEdits[10];
    QPushButton *selectPointButton[10];
    std::vector<QTimer*> timers;
    PointInfo pointInfos[10];  // 存储每个点击任务的相对坐标和窗口大小信息

    void populateKeyCombos(QComboBox *comboBox);
    void stopAllTimers();
    void loadSettings();
    void saveSettings();
    void attachToTargetWindow();
    void detachFromTargetWindow();
    int getRandomInterval(int minInterval, int maxInterval);
    void highlightWindow();
    void onTopmostCheckBoxChanged(int state);
};

#endif // KEYPRESSER_H
