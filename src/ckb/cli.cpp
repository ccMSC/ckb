#include "cli.h"
#include <iostream>
#include <string>


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
