#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qaction.h>
#include <qapplication.h>
#include <qevent.h>
#include <qeventloop.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qlabel.h>
#include <qlocale.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qstringlist.h>
#include <qtranslator.h>
#include <setupapi.h>
#include <winnt.h>
#include <winsock.h>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow) {
    ui->setupUi(this);
    ui->ChkRes->setAlignment(Qt::AlignCenter);

    ui->mainLayout->setStretch(0, 2);
    ui->mainLayout->setStretch(1, 3);
    ui->mainLayout->setStretch(4, 0);
    ui->mainLayout->setStretch(5, 2);
    connect(ui->StartButton,&QPushButton::clicked,this,&MainWindow::on_startbutton_clicked);
    connect(ui->actionAbout,&QAction::triggered,this,&MainWindow::on_action_about_triggered);
    connect(ui->EasyChk,&QPushButton::clicked,this,&MainWindow::on_easy_chk_clicked);
    connect(ui->MedChk,&QPushButton::clicked,this,&MainWindow::on_med_chl_clicked);

    install_system_language();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::fit_title_font() {
    QLabel *title = ui->label;

    if(!title || title->text().isEmpty()) {
        return;
    }

    const int availW = title->width() - 8;
    const int availH = title->height() - 8;

    if(availW <= 0 || availH <= 0) {
        return;
    }

    QFont font = title->font();
    int lo = 8;
    int hi = 200;
    int best = lo;

    while(lo <= hi) {
        const int mid = (lo + hi) / 2;
        font.setPointSize(mid);
        const QFontMetrics fm(font);

        if(fm.horizontalAdvance(title->text()) <= availW && fm.height() <= availH) {
            best = mid;
            lo = mid + 1;
        } else {
            hi = mid - 1;
        }
    }

    font.setPointSize(best);

    if(font.pointSize() != title->font().pointSize()) {
        title->setFont(font);
    }
}

void MainWindow::resizeEvent(QResizeEvent *event) {
    QMainWindow::resizeEvent(event);
    fit_title_font();
}

bool MainWindow::install_language(const QString &suffix) {
    QTranslator *translator = new QTranslator(this);
    if(!translator->load(QStringLiteral(":/i18n/Battery-Chker_") + suffix)) {
        delete translator;
        return false;
    }

    if(lang_translator) {
        QApplication::removeTranslator(lang_translator);
        delete lang_translator;
    }
    lang_translator = translator;
    QApplication::installTranslator(lang_translator);
    return true;
}

void MainWindow::install_system_language() {
    QStringList candidates;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for(const QString &locale : uiLanguages) {
        const QLocale l(locale);
        candidates << l.name() << l.name().section(QLatin1Char('_'), 0, 0);
    }
    candidates.removeDuplicates();
    candidates.removeAll(QString());

    for(const QString &suffix : candidates) {
        if(install_language(suffix)) {
            return;
        }
    }
}

void MainWindow::on_action_Lang_chinese_triggered() {
    if(!install_language(QStringLiteral("zh_CN"))) {
        QMessageBox::warning(this, tr("语言切换失败"),
                             tr("无法加载翻译文件：%1")
                             .arg(QStringLiteral(":/i18n/Battery-Chker_zh_CN.qm")));
    }
}

void MainWindow::on_action_Lang_eng_triggered() {
    if(!install_language(QStringLiteral("en"))) {
        QMessageBox::warning(this, tr("语言切换失败"),
                             tr("无法加载翻译文件：%1")
                             .arg(QStringLiteral(":/i18n/Battery-Chker_en.qm")));
    }
}

void MainWindow::changeEvent(QEvent *event) {
    if(event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this);
        render_result();
        fit_title_font();
    }
    QMainWindow::changeEvent(event);
}

void MainWindow::render_result() {
    const int health = (design_mwh > 0 && full_mwh > 0)
                       ? qRound(double(full_mwh) * 100.0 / double(design_mwh))
                       : -1;
    const QString health_text = health >= 0 ? QStringLiteral("%1%").arg(health)
                                            : tr("未知");
    const QString cycle_text = cycle_count > 0 ? QString::number(cycle_count)
                                               : tr("未知");

    switch(result_state) {
    case ResultState::Empty:
        break;
    case ResultState::Checking:
        ui->ChkRes->setText(tr("检测中："));
        break;
    case ResultState::Querying:
        ui->ChkRes->setText(tr("查询中："));
        break;
    case ResultState::Failed:
        ui->ChkRes->setText(tr("查询失败"));
        break;
    case ResultState::OkAbsolute: {
        QStringList lines;
        lines << tr("查询成功：");
        lines << tr("设计容量：%1 mWh").arg(design_mwh);
        lines << tr("满充容量：%1 mWh").arg(full_mwh);
        lines << tr("当前容量：%1 mWh").arg(current_mwh);
        lines << tr("健康度：%1").arg(health_text);
        lines << tr("循环次数：%1").arg(cycle_text);
        ui->ChkRes->setText(lines.join(QLatin1Char('\n')));
        break;
    }
    case ResultState::OkRelative: {
        QStringList lines;
        lines << tr("查询成功（相对单位，无法换算为 mAh/mWh）：");
        lines << tr("设计容量：%1（相对值）").arg(design_mwh);
        lines << tr("满充容量：%1（相对值）").arg(full_mwh);
        lines << tr("当前容量：%1（相对值）").arg(current_mwh);
        lines << tr("健康度：%1").arg(health_text);
        lines << tr("循环次数：%1").arg(cycle_text);
        ui->ChkRes->setText(lines.join(QLatin1Char('\n')));
        break;
    }
    case ResultState::Percent:
        ui->ChkRes->setText(tr("当前电量：%1%").arg(percent));
        break;
    case ResultState::PowerPresent:
        ui->ChkRes->setText(tr("有电"));
        break;
    }
}

void MainWindow::ban_all_btn() {
    ui->StartButton->setDisabled(1);
    ui->EasyChk->setDisabled(1);
    ui->MedChk->setDisabled(1);
    return;
}

void MainWindow::unban_all_btn() {
    ui->StartButton->setDisabled(0);
    ui->EasyChk->setDisabled(0);
    ui->MedChk->setDisabled(0);
    return;
}

bool MainWindow::set_language(const QString &suffix) {
    return install_language(suffix);
}

void MainWindow::non_blk_slp(long long msec) {
    QEventLoop loop;
    QTimer::singleShot(msec,&loop,&QEventLoop::quit);
    loop.exec();
    return;
}

void MainWindow::on_startbutton_clicked() {
    ban_all_btn();
    result_state = ResultState::Checking;
    render_result();
    ui->CheckingProgress->reset();
    for(int i = 1; i <= 100; i++) {
        non_blk_slp(100);
        ui->CheckingProgress->setValue(i);
    }

    bool suc = false;
    ULONG designMwh = 0;
    ULONG fullMwh = 0;
    ULONG currentMwh = 0;
    ULONG cycleCount = 0;
    bool capacityRelative = false;

    HDEVINFO hinfo = SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, 0, 0,
                                         DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);

    if(hinfo != INVALID_HANDLE_VALUE) {
        for(DWORD idev = 0; ; idev++) {
            SP_DEVICE_INTERFACE_DATA did = {0};
            did.cbSize = sizeof(did);

            if(!SetupDiEnumDeviceInterfaces(hinfo, 0, &GUID_DEVCLASS_BATTERY,
                                            idev, &did)) {
                break;
            }

            DWORD cbRequired = 0;
            SetupDiGetDeviceInterfaceDetail(hinfo, &did, 0, 0, &cbRequired, 0);

            if(GetLastError() != ERROR_INSUFFICIENT_BUFFER || cbRequired == 0) {
                continue;
            }

            PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd =
                (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR, cbRequired);
            if(!pdidd) {
                continue;
            }
            pdidd->cbSize = sizeof(*pdidd);

            if(!SetupDiGetDeviceInterfaceDetail(hinfo, &did, pdidd,
                                                cbRequired, &cbRequired, 0)) {
                LocalFree(pdidd);
                continue;
            }

            HANDLE hBattery = CreateFile(
                                  pdidd->DevicePath,
                                  GENERIC_READ | GENERIC_WRITE,
                                  FILE_SHARE_READ | FILE_SHARE_WRITE,
                                  NULL,
                                  OPEN_EXISTING,
                                  FILE_ATTRIBUTE_NORMAL,
                                  NULL
                              );

            LocalFree(pdidd);

            if(hBattery == INVALID_HANDLE_VALUE) {
                continue;
            }

            BATTERY_QUERY_INFORMATION bqi = {0};
            DWORD dwWait = 0;
            DWORD dwOut = 0;

            if(DeviceIoControl(hBattery,
                               IOCTL_BATTERY_QUERY_TAG,
                               &dwWait,
                               sizeof(dwWait),
                               &bqi.BatteryTag,
                               sizeof(bqi.BatteryTag),
                               &dwOut,
                               NULL)
                    && bqi.BatteryTag != 0) {

                BATTERY_INFORMATION bi = {0};
                bqi.InformationLevel = BatteryInformation;

                if(DeviceIoControl(hBattery,
                                   IOCTL_BATTERY_QUERY_INFORMATION,
                                   &bqi,
                                   sizeof(bqi),
                                   &bi,
                                   sizeof(bi),
                                   &dwOut,
                                   NULL)) {

                    capacityRelative =
                        (bi.Capabilities & BATTERY_CAPACITY_RELATIVE) != 0;

                    designMwh = bi.DesignedCapacity;
                    fullMwh   = bi.FullChargedCapacity;

                    cycleCount = bi.CycleCount;
                    BATTERY_WAIT_STATUS bws = {0};
                    bws.BatteryTag = bqi.BatteryTag;

                    BATTERY_STATUS bs = {0};
                    if(DeviceIoControl(hBattery,
                                       IOCTL_BATTERY_QUERY_STATUS,
                                       &bws,
                                       sizeof(bws),
                                       &bs,
                                       sizeof(bs),
                                       &dwOut,
                                       NULL)) {
                        currentMwh = bs.Capacity;
                        suc = true;
                    }
                }
            }

            CloseHandle(hBattery);

            if(suc) {
                break;
            }
        }

        SetupDiDestroyDeviceInfoList(hinfo);
    }

    if(!suc) {
        result_state = ResultState::Failed;
        render_result();
        unban_all_btn();
        return;
    }

    design_mwh = designMwh;
    full_mwh = fullMwh;
    current_mwh = currentMwh;
    cycle_count = cycleCount;
    result_state = capacityRelative ? ResultState::OkRelative
                                    : ResultState::OkAbsolute;
    render_result();
    unban_all_btn();
}

void MainWindow::on_med_chl_clicked() {
    ban_all_btn();
    ui->CheckingProgress->reset();
    result_state = ResultState::Querying;
    render_result();
    for(int i = 1; i <= 100; i++) {
        non_blk_slp(60);
        ui->CheckingProgress->setValue(i);
    }
    SYSTEM_POWER_STATUS sps = {0};
    if(GetSystemPowerStatus(&sps) && sps.BatteryLifePercent != 255) {
        percent = sps.BatteryLifePercent;
        result_state = ResultState::Percent;
    } else {
        result_state = ResultState::Failed;
    }
    render_result();
    unban_all_btn();
    return;
}

void MainWindow::on_easy_chk_clicked() {
    ban_all_btn();
    ui->CheckingProgress->reset();
    result_state = ResultState::Querying;
    render_result();
    for(int i = 1; i <= 100; i++) {
        non_blk_slp(30);
        ui->CheckingProgress->setValue(i);
    }
    result_state = ResultState::PowerPresent;
    render_result();
    unban_all_btn();
    return;
}

void MainWindow::on_action_about_triggered() {
    QMessageBox about;
    about.setWindowTitle(tr("关于电池检测器"));
    about.setText(tr("由 https://github.com/programming666 编写"));
    about.setDetailedText(tr("基于 Windows SetupAPI 与电池 IOCTL 实现。"));
    about.setIcon(QMessageBox::Information);
    about.setStandardButtons(QMessageBox::Ok);
    about.exec();
    return;
}
