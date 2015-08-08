#include "ckbsettings.h"
#include "kbfirmware.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <cstdlib>
#include <QSharedMemory>
#include <QShortcut>
#include <QMessageBox>
#include <QMenuBar>
#include <unistd.h>

extern QSharedMemory appShare;

float ckbGuiVersion = 0.f;
// Assume daemon has no version limitations if it's not connected
float ckbDaemonVersion = INFINITY;
QString daemonVStr;

static const QString configLabel = "Settings";
#ifndef __APPLE__
QString devpath = "/dev/input/ckb%1";
#else
QString devpath = "/var/run/ckb%1";
#endif

QTimer* eventTimer = 0;
MainWindow* MainWindow::mainWindow = 0;

#ifdef USE_LIBAPPINDICATOR
extern "C" {
    void quitIndicator(GtkMenu* menu, gpointer data) {
        Q_UNUSED(menu);
        MainWindow* window = static_cast<MainWindow*>(data);
        window->quitApp();
    }

    void restoreIndicator(GtkMenu* menu, gpointer data) {
        Q_UNUSED(menu);
        MainWindow* window = static_cast<MainWindow*>(data);
        window->showWindow();
    }
}
#endif // USE_LIBAPPINDICATOR

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    mainWindow = this;

    restoreAction = new QAction(tr("Restore"), this);
    closeAction = new QAction(tr("Quit ckb"), this);

#ifdef USE_LIBAPPINDICATOR
    QString desktop = std::getenv("XDG_CURRENT_DESKTOP");
    unityDesktop = (desktop.toLower() == "unity");

    if(unityDesktop){
        trayIcon = 0;

        indicatorMenu = gtk_menu_new();
        indicatorMenuRestoreItem = gtk_menu_item_new_with_label("Restore");
        indicatorMenuQuitItem = gtk_menu_item_new_with_label("Quit ckb");

        gtk_menu_shell_append(GTK_MENU_SHELL(indicatorMenu), indicatorMenuRestoreItem);
        gtk_menu_shell_append(GTK_MENU_SHELL(indicatorMenu), indicatorMenuQuitItem);

        g_signal_connect(indicatorMenuQuitItem, "activate",
            G_CALLBACK(quitIndicator), this);
        g_signal_connect(indicatorMenuRestoreItem, "activate",
            G_CALLBACK(restoreIndicator), this);

        gtk_widget_show(indicatorMenuRestoreItem);
        gtk_widget_show(indicatorMenuQuitItem);

        indicator = app_indicator_new("ckb", "indicator-messages", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);

        app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
        app_indicator_set_menu(indicator, GTK_MENU(indicatorMenu));
        app_indicator_set_icon(indicator, "ckb");
    } else
#endif // USE_LIBAPPINDICATOR
    {
        trayIconMenu = new QMenu(this);
        trayIconMenu->addAction(restoreAction);
        trayIconMenu->addAction(closeAction);
        trayIcon = new QSystemTrayIcon(QIcon(":/img/ckb-logo.png"), this);
        trayIcon->setContextMenu(trayIconMenu);
        connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(iconClicked(QSystemTrayIcon::ActivationReason)));
     }
     toggleTrayIcon(!CkbSettings::get("Program/SuppressTrayIcon").toBool());

#ifdef Q_OS_MACX
    // Make a custom "Close" menu action for OSX, as the default one brings up the "still running" popup unnecessarily
    QMenuBar* menuBar = new QMenuBar(this);
    setMenuBar(menuBar);
    this->menuBar()->addMenu("ckb")->addAction(closeAction);
#else
    // On linux, add a handler for Ctrl+Q
    new QShortcut(QKeySequence("Ctrl+Q"), this, SLOT(quitApp()));
#endif

    connect(ui->actionExit, SIGNAL(triggered()), this, SLOT(quitApp()));
    connect(closeAction, SIGNAL(triggered()), this, SLOT(quitApp()));
    connect(restoreAction, SIGNAL(triggered()), this, SLOT(showWindow()));

    connect(qApp, SIGNAL(aboutToQuit()), this, SLOT(cleanup()));

    eventTimer = new QTimer(this);
    eventTimer->setTimerType(Qt::PreciseTimer);
    connect(eventTimer, SIGNAL(timeout()), this, SLOT(timerTick()));
    eventTimer->start(1000 / 60);

    ui->tabWidget->addTab(settingsWidget = new SettingsWidget(this), configLabel);
    settingsWidget->setVersion("ckb " CKB_VERSION_STR);

    ckbGuiVersion = PARSE_CKB_VERSION(CKB_VERSION_STR);
    scanKeyboards();
}

void MainWindow::toggleTrayIcon(bool visible) {
#ifdef USE_LIBAPPINDICATOR
    if(unityDesktop)
        app_indicator_set_status(indicator, visible ? APP_INDICATOR_STATUS_ACTIVE : APP_INDICATOR_STATUS_PASSIVE);
    else
#endif // USE_LIBAPPINDICATOR
        trayIcon->setVisible(visible);
}

void MainWindow::scanKeyboards(){
    QString rootdev = devpath.arg(0);
    QFile connected(rootdev + "/connected");
    if(!connected.open(QIODevice::ReadOnly)){
        // No root controller - remove all keyboards
        while(ui->tabWidget->count() > 1)
            ui->tabWidget->removeTab(0);
        foreach(KbWidget* w, kbWidgets)
            w->deleteLater();
        kbWidgets.clear();
        settingsWidget->setStatus("Driver inactive");
        ckbDaemonVersion = INFINITY;
        daemonVStr.clear();
        return;
    }
    // Check daemon version
    QFile version(rootdev + "/version");
    if(version.open(QIODevice::ReadOnly)){
        daemonVStr = QString::fromUtf8(version.readLine()).trimmed();
        version.close();
    } else
        daemonVStr = "<unavailable>";
    ckbDaemonVersion = PARSE_CKB_VERSION(daemonVStr);

    // Scan connected devices
    foreach(KbWidget* w, kbWidgets)
        w->active(false);
    QString line;
    while((line = connected.readLine().trimmed()) != ""){
        QStringList components = line.trimmed().split(" ");
        if(components.length() < 2)
            continue;
        QString path = components[0], serial = components[1];
        // Connected already?
        KbWidget* widget = 0;
        foreach(KbWidget* w, kbWidgets){
            if(w->device && w->device->matches(path, serial)){
                widget = w;
                w->active(true);
                break;
            }
        }
        if(widget)
            continue;
        // Add the keyboard
        widget = new KbWidget(this, path, "Devices");
        if(!widget->isActive()){
            delete widget;
            continue;
        }
        kbWidgets.append(widget);
        int count = ui->tabWidget->count();
        ui->tabWidget->insertTab(count - 1, widget, widget->name());
        if(ui->tabWidget->currentIndex() == count)
            ui->tabWidget->setCurrentIndex(count - 1);
        connect(eventTimer, SIGNAL(timeout()), widget->device, SLOT(frameUpdate()));
    }
    connected.close();

    // Remove any devices not found in the connected list
    bool updateShown = false;
    foreach(KbWidget* w, kbWidgets){
        if(w->isActive()){
            if(!updateShown){
                // Display firmware upgrade notification if a new version is available (and user has automatic updates enabled)
                if(CkbSettings::get("Program/DisableAutoFWCheck").toBool())
                    continue;
                float version = KbFirmware::versionForBoard(w->device->features);
                if(version > w->device->firmware.toFloat()){
                    if(w->hasShownNewFW)
                        continue;
                    w->hasShownNewFW = true;
                    w->updateFwButton();
                    // Don't display more than one of these at once
                    updateShown = true;
                    // Don't run this method here because it will lock up the timer and prevent devices from working properly
                    // Use a queued invocation instead
                    metaObject()->invokeMethod(this, "showFwUpdateNotification", Qt::QueuedConnection, Q_ARG(QWidget*, w), Q_ARG(float, version));
                }
            }
            w->saveIfNeeded();
        } else {
            int i = kbWidgets.indexOf(w);
            ui->tabWidget->removeTab(i);
            kbWidgets.removeAt(i);
            w->deleteLater();
        }
    }

    int count = kbWidgets.count();
    // Warn if the daemon version doesn't match the GUI
    QString daemonWarning;
    if(daemonVStr != CKB_VERSION_STR && !daemonVStr.isEmpty())
        daemonWarning = "<br /><br /><b>Warning:</b> Driver version mismatch (" + daemonVStr + "). Please upgrade ckb" + QString(ckbDaemonVersion > ckbGuiVersion ? "" : "-daemon") + ". If the problem persists, try rebooting.";
    if(count == 0)
        settingsWidget->setStatus("No devices connected" + daemonWarning);
    else if(count == 1)
        settingsWidget->setStatus("1 device connected" + daemonWarning);
    else
        settingsWidget->setStatus(QString("%1 devices connected").arg(count) + daemonWarning);
}

void MainWindow::showFwUpdateNotification(QWidget* widget, float version){
    static bool isShowing = false;
    if(isShowing)
        return;
    isShowing = true;
    showWindow();
    KbWidget* w = (KbWidget*)widget;
    // Ask for update
    if(QMessageBox::information(this, "Firmware update", tr("A new firmware is available for your %1 (v%2)\nWould you like to install it now?").arg(w->device->usbModel, QString::number(version, 'f', 2)), QMessageBox::StandardButtons(QMessageBox::Yes | QMessageBox::No), QMessageBox::Yes) == QMessageBox::Yes){
        // If accepted, switch to firmware tab and bring up update window
        w->showLastTab();
        ui->tabWidget->setCurrentIndex(kbWidgets.indexOf(w));
        w->showFwUpdate();
    }
    isShowing = false;
}

void MainWindow::closeEvent(QCloseEvent *event){
    // If the window is hidden already or the event is non-spontaneous (can happen on OSX when using the Quit menu), accept it and close
    if(!event->spontaneous() || isHidden()){
        event->accept();
        return;
    }
    if(!CkbSettings::get("Popups/BGWarning").toBool()){
        QMessageBox::information(this, "ckb", "ckb will still run in the background.\nTo close it, choose Exit from the tray menu\nor click \"Quit ckb\" on the Settings screen.");
        CkbSettings::set("Popups/BGWarning", true);
    }
    hide();
    event->ignore();
}

void MainWindow::timerTick(){
    // Check shared memory for changes
    if(appShare.lock()){
        void* data = appShare.data();
        QStringList commands = QString((const char*)data).split("\n");
        // Restore PID, remove all other data
        snprintf((char*)appShare.data(), appShare.size(), "PID %ld", (long)getpid());
        appShare.unlock();
        // Parse commands
        foreach(const QString& line, commands){
            // Old ckb option line - bring application to foreground
            if(line == "Open")
                showWindow();
            if(line.startsWith("Option ")){
                // New ckb option line
                QString option = line.split(" ")[1];
                if(option == "Open")
                    // Bring to foreground
                    showWindow();
                else if(option == "Close")
                    // Quit application
                    qApp->quit();
            }
        }
    }
    // Check for firmware updates (when appropriate)
    if(!CkbSettings::get("Program/DisableAutoFWCheck").toBool())
        KbFirmware::checkUpdates();
    // Scan for connected/disconnected keyboards
    scanKeyboards();
    // Poll for setting updates
    settingsWidget->pollUpdates();
}

void MainWindow::iconClicked(QSystemTrayIcon::ActivationReason reason){
    // On Linux, hide/show the app when the tray icon is clicked
    // On OSX this just shows the menu
#ifndef Q_OS_MACX
    if(reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::Trigger){
        if(isVisible())
            hide();
        else
            showWindow();
    }
#endif
}

void MainWindow::showWindow(){
    showNormal();
    raise();
    activateWindow();
}

void MainWindow::quitApp(){
    qApp->quit();
}

void MainWindow::cleanup(){
    foreach(KbWidget* w, kbWidgets)
        delete w;
    kbWidgets.clear();
    CkbSettings::cleanUp();
}

MainWindow::~MainWindow(){
    cleanup();
    delete ui;
}
