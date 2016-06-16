#include "cli.h"
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

KeyMap::Layout resolveLayout(QString layout) {
    if (layout.compare("EU") == 0) return KeyMap::EU;
    else if (layout.compare("EU_DVORAK") == 0) return KeyMap::EU_DVORAK;
    else if (layout.compare("GB") == 0) return KeyMap::GB;
    else if (layout.compare("GB_DVORAK") == 0) return KeyMap::GB_DVORAK;
    else if (layout.compare("US") == 0) return KeyMap::US;
    else if (layout.compare("US_DVORAK") == 0) return KeyMap::US_DVORAK;
    else if (layout.compare("FR") == 0) return KeyMap::FR;
    else if (layout.compare("DE") == 0) return KeyMap::DE;
    else if (layout.compare("IT") == 0) return KeyMap::IT;
    else if (layout.compare("PL") == 0) return KeyMap::PL;
    else if (layout.compare("MX") == 0) return KeyMap::MX;
    else if (layout.compare("ES") == 0) return KeyMap::ES;
    else if (layout.compare("SE") == 0) return KeyMap::SE;

    return KeyMap::NO_LAYOUT;
}

int CommandLine::runGlobal() {
    if (cmdOffset >= commands.length()) return Command::CommandUnknown;
    switch (Command::resolveCommand(commands[cmdOffset++])) {
    case Command::CommandInfo:
        qDebug() << "  Print Global Info";
        break;
    case Command::CommandLayout:
        if (cmdOffset >= commands.length()) return Command::CommandUnknown;

        // further specify the layout command
        QString task = commands[cmdOffset++];
        if (task.compare("list") == 0) {
            qDebug() << "  Print Global Layouts";
        }
        else if (task.compare("set") == 0) {
            if (cmdOffset >= commands.length()) return Command::CommandUnknown;
            KeyMap::Layout kl = resolveLayout(commands[cmdOffset++]);
            qDebug() << "  Set Global Layout: " << (int) kl;
        }
        else {
            return CommandLineUnknown;
        }
        break;
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
        qDebug() << "Global: ";
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
