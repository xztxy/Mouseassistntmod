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
    QLabel *selectedWindowLabel;
    QCheckBox *keyCheckBoxes[10];
    QComboBox *keyCombos[10];
    QLineEdit *intervalLineEdits[10];
    QLineEdit *maxIntervalLineEdits[10];
    QLineEdit *pointEdits[10];
    QPushButton *selectPointButton[10];
    std::vector<QTimer*> timers;


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
