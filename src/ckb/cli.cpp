#include "cli.h"
#include "animscript.h"
#include "ckbsettings.h"
#include "keymap.h"
#include "kbmanager.h"
#include "kbbind.h"
#include "kblight.h"
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
    case Command::CommandModifier:
        {
            // load the global remaps from the ckbsettings
            KbBind::loadGlobalRemap();

            // set the modifier keys and names to display
            QStringList modKeys, modNames;
            modKeys << "caps" << "lshift" << "lctrl" << "lalt" << "lwin";
#ifdef Q_OS_MACX
            modNames << "Caps Lock" << "Shift" << "Control (⌃)" << "Option (⌥)" << "Command (⌘)";
#else
            modNames << "Caps Lock" << "Shift" << "Control" << "Alt" << "Super";
#endif

            if (cmdOffset >= commands.length()) return CommandLineUnknown;
            QString task = commands[cmdOffset++].toLower();
            if (task.compare("list") == 0) {
                // print each modifier and it's rebind
                qOut()
                    << "These will override the keyboard profile. See \"Binding\" tab for more settings."
                    << endl;

                foreach (QString mod, modKeys) {
                    qOut()
                        << qSetFieldWidth(11) << left << modNames[modKeys.indexOf(mod)]
                        << qSetFieldWidth(2)  << left << ": "
                        << qSetFieldWidth(0)  << left << modNames[modKeys.indexOf(KbBind::globalRemap(mod))]
                        << endl;
                }
            }
            else if (task.compare("set") == 0) {
                // abort if arguments don't contain key and remap
                if (cmdOffset+1 >= commands.length()) return CommandLineUnknown;

                // get key and remap from arguments
                QString key = commands[cmdOffset++].toLower();
                QString mod = commands[cmdOffset++].toLower();
                QString lmod, rmod;

                // strip leading 'r' or 'l'
                mod = mod[0] == QChar('l') || mod[0] == QChar('r') ? mod.mid(1) : mod;

                // abort if remap not in list
                if ((QStringList() << "caps" << "shift" << "ctrl" << "alt" << "option" << "win" << "cmd").indexOf(mod) == -1)
                    return CommandLineUnknown;

                // initialize newMods
                QHash<QString, QString> newMods;
                newMods["caps"] = KbBind::globalRemap("caps");
                newMods["lshift"] = KbBind::globalRemap("lshift");
                newMods["rshift"] = KbBind::globalRemap("rshift");
                newMods["lctrl"] = KbBind::globalRemap("lctrl");
                newMods["rctrl"] = KbBind::globalRemap("rctrl");
                newMods["lalt"] = KbBind::globalRemap("lalt");
                newMods["ralt"] = KbBind::globalRemap("ralt");
                newMods["lwin"] = KbBind::globalRemap("lwin");
                newMods["rwin"] = KbBind::globalRemap("rwin");

                // coerce remaps to understandable format
                if (mod == "caps") {
                    lmod = rmod = mod;
                }
                else if (mod == "option") {
                    lmod = "lalt";
                    rmod = "ralt";
                }
                else if (mod == "cmd") {
                    lmod = "lwin";
                    rmod = "rwin";
                }
                else {
                    lmod = 'l' + mod;
                    rmod = 'r' + mod;
                }

                // remap requested key to new mod
                if (key.compare("caps") == 0) {
                    newMods["caps"] = lmod;
                }
                else if (key.compare("shift") == 0) {
                    newMods["lshift"] = lmod;
                    newMods["rshift"] = rmod;
                }
                else if (key.compare("ctrl") == 0) {
                    newMods["lctrl"] = lmod;
                    newMods["rctrl"] = rmod;
                }
                else if (key.compare("alt") == 0 || key.compare("option") == 0) {
                    newMods["lalt"] = lmod;
                    newMods["ralt"] = rmod;
                }
                else if (key.compare("win") == 0 || key.compare("cmd") == 0) {
                    newMods["lwin"] = lmod;
                    newMods["rwin"] = rmod;
                }
                else {
                    return CommandLineUnknown;
                }

                // persistently store new remaps
                KbBind::setGlobalRemap(newMods);
                KbBind::saveGlobalRemap();

                // wait until settings are written completely
                settings.cleanUp();
            }
            else if (task.compare("reset") == 0) {
                // initialize newMods
                QHash<QString, QString> newMods;
                newMods["caps"] = "caps";
                newMods["lshift"] = "lshift";
                newMods["rshift"] = "rshift";
                newMods["lctrl"] = "lctrl";
                newMods["rctrl"] = "rctrl";
                newMods["lalt"] = "lalt";
                newMods["ralt"] = "ralt";
                newMods["lwin"] = "lwin";
                newMods["rwin"] = "rwin";

                // persistently store new remaps
                KbBind::setGlobalRemap(newMods);
                KbBind::saveGlobalRemap();

                // wait until settings are written completely
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }

            break;
        }
    case Command::CommandFramerate:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            QString task = commands[cmdOffset++].toLower();
            if (task.compare("show") == 0) {
                // display current framerate
                qOut()
                    << "Current framerate: "
                    << settings.value("Program/framerate").toString()
                    << endl;
            }
            else if (task.compare("set") == 0) {
                if (cmdOffset >= commands.length()) return CommandLineUnknown;

                // get given framerate
                bool ok;
                QString frameRate = commands[cmdOffset++].toLower();
                int frameRateValue = frameRate.toInt(&ok);

                // check, whether it's a number between 0 and 60
                if (!ok || frameRateValue <= 0 || frameRateValue >= 60) {
                    qOut()
                        << "Framerate must be a number between 0 and 60."
                        << endl;
                    return CommandLineUnknown;
                }

                // persistently store framerate and set it for current session
                Kb::frameRate(frameRateValue);
                settings.set("Program/framerate", frameRateValue);

                // print warning, if value above 30
                if (frameRateValue > 30)
                    qOut()
                        << "Warning: high frame rates may cause stability issues."
                        << endl;

                // wait until settings are written completely
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
    case Command::CommandAnimationDir:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            // whether it's "show" or "scan", both do the same
            QString task = commands[cmdOffset++].toLower();
            if (task.compare("show") == 0 || task.compare("scan") == 0) {
                // scan and load the animations from the animation-dir
                AnimScript::scan();

                // display information about the results
                qOut()
                    << "Location: "
                    << AnimScript::path()
                    << " ";
                switch (AnimScript::count()) {
                case 0:
                    qOut() << "No animations found.";
                    break;
                case 1:
                    qOut() << "1 animation found.";
                    break;
                default:
                    qOut() << QString("%1 animations found").arg(AnimScript::count());
                    break;
                }
                qOut() << endl;
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
    case Command::CommandBrightnessPerMode:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            QString task = commands[cmdOffset++].toLower();

            // coerce "set" to "disable"/"enable" if possible
            if (task.compare("set") == 0) {
                if (cmdOffset >= commands.length()) return CommandLineUnknown;

                task = commands[cmdOffset].toLower();
                if (task.compare("on") == 0 || task.compare("1") == 0)
                    task = "enable";
                else if (task.compare("off") == 0 || task.compare("0") == 0)
                    task = "disable";
                else
                    return CommandLineUnknown;
            }

            if (task.compare("show") == 0) {
                // get current dimming from settings
                int dimming = settings.value("Program/GlobalBrightness").toInt();
                if (dimming < -1 || dimming > KbLight::MAX_DIM) {
                    // normalize dimming, if value is malformed
                    dimming = 0;
                }

                // display information about brightness per mode
                qOut()
                    << "Brightness per mode: "
                    << (dimming == -1 ? "Enabled" : "Disabled") << "."
                    << endl
                    << "(By default, the same brightness level will be applied to all profiles and all devices. Enable this to store it with the lighting mode instead.)"
                    << endl;
            }
            else if (task.compare("enable") == 0) {
                // enable brightness per mode -> disable shared dimming
                settings.set("Program/GlobalBrightness", -1);
                KbLight::shareDimming(-1);
                settings.cleanUp();
            }
            else if (task.compare("disable") == 0) {
                // disable brightness per mode -> enable shared dimming
                settings.set("Program/GlobalBrightness", 0);
                KbLight::shareDimming(0);
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
    case Command::CommandSpatialDithering:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            QString task = commands[cmdOffset++].toLower();

            // coerce "set" to "disable"/"enable" if possible
            if (task.compare("set") == 0) {
                if (cmdOffset >= commands.length()) return CommandLineUnknown;

                task = commands[cmdOffset].toLower();
                if (task.compare("on") == 0 || task.compare("1") == 0)
                    task = "enable";
                else if (task.compare("off") == 0 || task.compare("0") == 0)
                    task = "disable";
                else
                    if (cmdOffset >= commands.length()) return CommandLineUnknown;
            }

            if (task.compare("show") == 0) {
                // display information about spatial dithering
                qOut()
                    << "Use spatial dithering to simulate extra color resolution: "
                    << (settings.value("Program/Dither").toBool() ? "Enabled" : "Disabled") << "."
                    << endl
                    << "(May improve appearance on some keyboards.)"
                    << endl;
            }
            else if (task.compare("enable") == 0) {
                // enable spatial dithering
                settings.set("Program/Dither", true);
                Kb::dither(true);
                settings.cleanUp();
            }
            else if (task.compare("disable") == 0) {
                // disable spatial dithering
                settings.set("Program/Dither", false);
                Kb::dither(false);
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
    case Command::CommandFirmwareAutocheck:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            QString task = commands[cmdOffset++].toLower();

            // coerce "set" to "disable"/"enable" if possible
            if (task.compare("set") == 0) {
                if (cmdOffset >= commands.length()) return CommandLineUnknown;

                task = commands[cmdOffset].toLower();
                if (task.compare("on") == 0 || task.compare("1") == 0)
                    task = "enable";
                else if (task.compare("off") == 0 || task.compare("0") == 0)
                    task = "disable";
                else
                    if (cmdOffset >= commands.length()) return CommandLineUnknown;
            }

            if (task.compare("show") == 0) {
                // display information about firmware auto check
                qOut()
                    << "Check for new firmware automatically: "
                    << (settings.value("Program/DisableAutoFWCheck").toBool() ? "Disabled" : "Enabled") << "."
                    << endl
                    << "(You will be notified when new firmware versions are available. You'll have the option to install them immediately or wait until later.)"
                    << endl;
            }
            else if (task.compare("enable") == 0) {
                // enable firmware autocheck
                settings.set("Program/DisableAutoFWCheck", false);
                settings.cleanUp();
            }
            else if (task.compare("disable") == 0) {
                // disable firmware autocheck
                settings.set("Program/DisableAutoFWCheck", true);
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
    case Command::CommandTrayIcon:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            QString task = commands[cmdOffset++].toLower();

            // coerce "set" to "disable"/"enable" if possible
            if (task.compare("set") == 0) {
                if (cmdOffset >= commands.length()) return CommandLineUnknown;

                task = commands[cmdOffset].toLower();
                if (task.compare("on") == 0 || task.compare("1") == 0)
                    task = "enable";
                else if (task.compare("off") == 0 || task.compare("0") == 0)
                    task = "disable";
                else
                    if (cmdOffset >= commands.length()) return CommandLineUnknown;
            }

            if (task.compare("show") == 0) {
                // display information about the tray icon
                qOut()
                    << "Show tray icon: "
                    << (settings.value("Program/SuppressTrayIcon").toBool() ? "Disabled" : "Enabled") << "."
                    << endl
                    << "(The tray icon will not be displayed, if disabled. The application will still run in the background; re-launch the app to see the GUI again.)"
                    << endl;
            }
            else if (task.compare("enable") == 0) {
                // suppress tray icon
                settings.set("Program/SuppressTrayIcon", false);
                settings.cleanUp();
            }
            else if (task.compare("disable") == 0) {
                // allow tray icon
                settings.set("Program/SuppressTrayIcon", true);
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
#ifdef Q_OS_MACX
    case Command::CommandMouseAcceleration:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            QString task = commands[cmdOffset++].toLower();

            // coerce "set" to "disable"/"enable" if possible
            if (task.compare("set") == 0) {
                if (cmdOffset >= commands.length()) return CommandLineUnknown;

                task = commands[cmdOffset].toLower();
                if (task.compare("on") == 0 || task.compare("1") == 0)
                    task = "enable";
                else if (task.compare("off") == 0 || task.compare("0") == 0)
                    task = "disable";
                else
                    if (cmdOffset >= commands.length()) return CommandLineUnknown;
            }

            if (task.compare("show") == 0) {
                // display information about mouse acceleration
                qOut()
                    << "Mouse acceleration: "
                    << (settings.value("Program/DisableMouseAccel").toBool() ? "Disabled" : "Enabled") << "."
                    << endl
                    << "(Try this if you're having problems with mouse movement.)"
                    << endl;
            }
            else if (task.compare("enable") == 0) {
                // suppress mouse acceleration
                settings.set("Program/DisableMouseAccel", false);
                Kb::mouseAccel(true);
                settings.cleanUp();
            }
            else if (task.compare("disable") == 0) {
                // allow mouse acceleration
                settings.set("Program/DisableMouseAccel", true);
                Kb::mouseAccel(false);
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
    case Command::CommandScrollAcceleration:
        {
            if (cmdOffset >= commands.length()) return CommandLineUnknown;

            QString task = commands[cmdOffset++].toLower();

            // coerce "set" to "disable"/"enable" if possible
            if (task.compare("set") == 0) {
                if (cmdOffset >= commands.length()) return CommandLineUnknown;

                task = commands[cmdOffset].toLower();
                if (task.compare("on") == 0 || task.compare("1") == 0)
                    task = "enable";
                else if (task.compare("off") == 0 || task.compare("0") == 0)
                    task = "disable";
                else
                    if (cmdOffset >= commands.length()) return CommandLineUnknown;
            }

            if (task.compare("show") == 0) {
                // display information about scroll acceleration
                qOut()
                    << "Scroll acceleration: "
                    << (settings.value("Program/DisableScrollAccel").toBool() ? "Disabled" : "Enabled") << "."
                    << endl
                    << "(Try this if you're having problems with the scroll wheel.)"
                    << endl;
            }
            else if (task.compare("enable") == 0) {
                // suppress scroll acceleration
                settings.set("Program/DisableScrollAccel", false);
                Kb::scrollSpeed(settings.value("Program/ScrollSpeed", 3).toInt());
                settings.cleanUp();
            }
            else if (task.compare("disable") == 0) {
                // allow scroll acceleration
                settings.set("Program/DisableScrollAccel", true);
                Kb::scrollSpeed(0);
                settings.cleanUp();
            }
            else {
                return CommandLineUnknown;
            }
            break;
        }
#endif
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
