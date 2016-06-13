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

/**
 * run - Run specified Commands.
 */
int CommandLine::run() {
    // TODO: parse commands and execute requested operation
    printf("BaseCommand: %s\n", base.toLocal8Bit().data());
    printf("Flag       : %s\n", flag.toLocal8Bit().data());
    printf("Arguments  :\n");

    for (int i = 0; i < commands.length(); i++) {
        printf("    (%03d) %s\n", i, commands[i].toLocal8Bit().data());
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
