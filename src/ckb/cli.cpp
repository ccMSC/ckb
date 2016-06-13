#include "cli.h"
#include <iostream>
#include <string>

/**
 * resolveCommand - Resolve the given String to a Command identifier.
 */
int Command::resolveCommand(QString cmd) {
    if (cmd.compare("global") == 0) {
        return CommandGlobal;
    }
    else if (cmd.compare("device") == 0) {
        return CommandDevice;
    }
    else if (cmd.compare("profile") == 0) {
        return CommandProfile;
    }
    else if (cmd.compare("help") == 0) {
        return CommandHelp;
    }
    else if (cmd.compare("info") == 0) {
        return CommandInfo;
    }
    else if (cmd.compare("layout") == 0) {
        return CommandLayout;
    }
    else if (cmd.compare("modifier") == 0) {
        return CommandModifier;
    }
    else if (cmd.compare("framerate") == 0) {
        return CommandFramerate;
    }
    else if (cmd.compare("animation-dir") == 0) {
        return CommandAnimationDir;
    }
    else if (cmd.compare("brightness-per-mode") == 0) {
        return CommandBrightnessPerMode;
    }
    else if (cmd.compare("spatial-dithering") == 0) {
        return CommandSpatialDithering;
    }
    else if (cmd.compare("firmware-autocheck") == 0) {
        return CommandFirmwareAutocheck;
    }
    else if (cmd.compare("tray-icon") == 0) {
        return CommandTrayIcon;
    }
    else if (cmd.compare("mouse-acceleration") == 0) {
        return CommandMouseAcceleration;
    }
    else if (cmd.compare("scroll-acceleration") == 0) {
        return CommandScrollAcceleration;
    }
    else if (cmd.compare("scroll-acceleration-speed") == 0) {
        return CommandScrollAccelerationSpeed;
    }

    return CommandUnknown;
}

int CommandLine::runGlobal() {
    if (cmdOffset >= commands.length()) return Command::CommandUnknown;
    switch (Command::resolveCommand(commands[cmdOffset++].toLocal8Bit().data())) {
    case Command::CommandInfo:
        std::cout << "  Print Global Info" << std::endl;
        break;
    case Command::CommandLayout:
        if (cmdOffset >= commands.length()) return Command::CommandUnknown;

        // further specify the layout command
        QString task = commands[cmdOffset++].toLocal8Bit().data();
        if (task.compare("list") == 0) {
            std::cout << "  Print Global Layouts" << std::endl;
        }
        else if (task.compare("set") == 0) {
            if (cmdOffset >= commands.length()) return Command::CommandUnknown;
            QString layout = commands[cmdOffset++].toLocal8Bit().data();
            std::cout << "  Set Global Layout to " << layout.toLocal8Bit().data() << std::endl;
        }
        else {
            return Command::CommandUnknown;
        }
        break;
    }

    return Command::CommandOK;
}

/**
 * run - Run specified Commands.
 */
int CommandLine::run() {
    cmdOffset = 0;

    // parse commands and execute requested operation
    if (cmdOffset >= commands.length()) return CommandLineUnknown;
    switch (Command::resolveCommand(commands[cmdOffset++].toLocal8Bit().data())) {
    case Command::CommandGlobal:
        std::cout << "Global: " << std::endl;
        return runGlobal();
        break;
    case Command::CommandDevice:
        std::cout << "Device:" << std::endl;
        break;
    case Command::CommandProfile:
        std::cout << "Profile:" << std::endl;
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
        item = (*constIterator).toLocal8Bit().data();
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
