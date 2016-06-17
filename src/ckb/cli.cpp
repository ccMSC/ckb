#include "cli.h"
#include "ckbsettings.h"
#include "keymap.h"
#include "kbmanager.h"
#include <string>
#include <QDebug>

/**
 * resolveCommand - Resolve the given String to a Command identifier.
 */
int Command::resolveCommand(QString cmd) {
    QString lowerCmd = cmd.toLower();
    if (lowerCmd.compare("global") == 0) return Command::CommandGlobal;
    else if (lowerCmd.compare("device") == 0) return Command::CommandDevice;
    else if (lowerCmd.compare("profile") == 0) return Command::CommandProfile;
    else if (lowerCmd.compare("help") == 0) return Command::CommandHelp;
    else if (lowerCmd.compare("info") == 0) return Command::CommandInfo;
    else if (lowerCmd.compare("layout") == 0) return Command::CommandLayout;
    else if (lowerCmd.compare("modifier") == 0) return Command::CommandModifier;
    else if (lowerCmd.compare("framerate") == 0) return Command::CommandFramerate;
    else if (lowerCmd.compare("animation-dir") == 0) return Command::CommandAnimationDir;
    else if (lowerCmd.compare("brightness-per-mode") == 0) return Command::CommandBrightnessPerMode;
    else if (lowerCmd.compare("spatial-dithering") == 0) return Command::CommandSpatialDithering;
    else if (lowerCmd.compare("firmware-autocheck") == 0) return Command::CommandFirmwareAutocheck;
    else if (lowerCmd.compare("tray-icon") == 0) return Command::CommandTrayIcon;
    else if (lowerCmd.compare("mouse-acceleration") == 0) return Command::CommandMouseAcceleration;
    else if (lowerCmd.compare("scroll-acceleration") == 0) return Command::CommandScrollAcceleration;
    else if (lowerCmd.compare("scroll-acceleration-speed") == 0) return Command::CommandScrollAccelerationSpeed;

    return Command::CommandUnknown;
}

int CommandLine::runGlobal() {
    if (cmdOffset >= commands.length()) return CommandLineUnknown;
    switch (Command::resolveCommand(commands[cmdOffset++])) {
    case Command::CommandInfo:
        {
            // TODO: the next section is almost verbosely taken from `MainWindow::updateVersion()`
            //       Outsource one of these routines and use it for both frontends.
            QString daemonVersion = KbManager::ckbDaemonVersion();
            QString deviceLabel;
            if(daemonVersion == DAEMON_UNAVAILABLE_STR){
                deviceLabel = "Driver inactive";
            }
            else {
                int count = KbManager::devices().count();
                // Warn if the daemon version doesn't match the GUI
                QString daemonWarning;
                if(daemonVersion != CKB_VERSION_STR)
                    daemonWarning = "\n\nWarning: Driver version mismatch (" + daemonVersion + "). Please upgrade ckb" + QString(KbManager::ckbDaemonVersionF() > KbManager::ckbGuiVersionF() ? "" : "-daemon") + ". If the problem persists, try rebooting.";
                if(count == 0)
                    deviceLabel = "No devices connected" + daemonWarning;
                else if(count == 1)
                    deviceLabel = "1 device connected" + daemonWarning;
                else
                    deviceLabel = QString("%1 devices connected").arg(count) + daemonWarning;
            }

            qOut()
                << "ckb " << CKB_VERSION_STR
                << endl
                << "Open Source Corsair Input Device Driver for Linux and OSX."
                << endl << endl
                << deviceLabel
                << endl << endl
                << "See https://github.com/ccMSC/ckb"
                << endl
                << QString::fromUtf8("©") << " 2014-2016. Licensed under GPLv2."
                << endl;
            break;
        }
    case Command::CommandLayout:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            // further specify the layout command
            QString task = commands[cmdOffset++].toLower();
            if (task.compare("list") == 0) {
                // get currently set layout and available layouts
                KeyMap::Layout layout = KeyMap::getLayout(settings.value("Program/KbdLayout").toString());
                QStringList layoutNames = KeyMap::layoutNames();

                // iterate through available layouts
                for (int layoutIndex = 0; layoutIndex < layoutNames.count(); layoutIndex++) {
                    // print out all available keyboard layouts
                    KeyMap::Layout currentLayout = KeyMap::Layout(layoutIndex);
                    qOut()
                        << qSetFieldWidth(0) << left << ((layout == currentLayout) ? "[x]" : "[ ]") << " "
                        << qSetFieldWidth(9) << left << KeyMap::getLayout(currentLayout).toUpper() << " | "
                        << qSetFieldWidth(0) << left << layoutNames[layoutIndex]
                        << endl;
                }
            }
            else if (task.compare("set") == 0) {
                // get next argument to set as the new layout
                if (cmdOffset >= commands.length()) return CommandLineUnknown;
                KeyMap::Layout kl = KeyMap::getLayout(commands[cmdOffset++]);

                // if layout is invalid, abort
                if (kl == KeyMap::NO_LAYOUT) {
                    qOut()
                        << "Could not set layout."
                        << endl;

                    return CommandLineUnknown;
                }
                else {
                    // persistently save new layout
                    settings.set("Program/KbdLayout", KeyMap::getLayout(kl));
                    Kb::layout(kl);

                    // wait until settings are written completely
                    settings.cleanUp();

                    qOut()
                        << "New layout is " << KeyMap::layoutNames()[(int)Kb::layout()] << "."
                        << endl;
                }
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
    default:
        return CommandLineUnknown;
    }

    return CommandLineOK;
}

/**
 * run - Run specified Commands.
 */
int CommandLine::run() {
    cmdOffset = 0;

    // parse commands and execute requested operation
    if (cmdOffset >= commands.length()) return CommandLineUnknown;
    switch (Command::resolveCommand(commands[cmdOffset++])) {
    case Command::CommandGlobal:
        return runGlobal();
    case Command::CommandDevice:
        qDebug() << "Device:";
        break;
    case Command::CommandProfile:
        qDebug() << "Profile:";
        break;
    default:
        return CommandLineUnknown;
    }

    // return with appropriate status
    return CommandLineOK;
}


/**
 * execute - Parse arguments and run specified commands.
 *
 * @param args  QStringList of arguments to parse.
 */
int CommandLine::execute(QStringList args) {
    CkbSettings settings("Program");
    KbManager::init(CKB_VERSION_STR);
    KbManager::kbManager()->scanKeyboards();
    QStringList::const_iterator constIterator;
    CommandLine cli;

    // setup command by splitting base, flag and actual command
    int index = 0;
    QString item;
    for (constIterator = args.constBegin(); constIterator != args.constEnd(); ++constIterator) {
        item = (*constIterator);
        if (index == 0) {
            // first arg is the base
            cli.base = item;
        }
        else if (index == 1) {
            // second arg is the flag
            cli.flag = item;
        }
        else {
            // the remaining args are the command to execute
            cli.commands << item;
        }

        index++;
    }

    // run command
    return cli.run();
}
