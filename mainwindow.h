#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>
#include <windows.h>
#include <setupapi.h>
#include <batclass.h>
#include <devguid.h>
#include <QEventLoop>
#include <QTimer>
#include <QMessageBox>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    void on_startbutton_clicked();
    void on_med_chl_clicked();
    void on_easy_chk_clicked();
    void on_action_about_triggered();
    void non_blk_slp(long long msec);
    void ban_all_btn();
    void unban_all_btn();
    bool set_language(const QString &suffix);

protected:
    void changeEvent(QEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void on_action_Lang_chinese_triggered();
    void on_action_Lang_eng_triggered();

private:
    enum class ResultState {
        Empty,
        Checking,
        Querying,
        Failed,
        OkAbsolute,
        OkRelative,
        Percent,
        PowerPresent
    };

    bool install_language(const QString &suffix);
    void install_system_language();
    void render_result();
    void fit_title_font();

    Ui::MainWindow *ui;
    QTranslator *lang_translator = nullptr;

    ResultState result_state = ResultState::Empty;
    ULONG design_mwh = 0;
    ULONG full_mwh = 0;
    ULONG current_mwh = 0;
    ULONG cycle_count = 0;
    int percent = 0;
};
#endif
