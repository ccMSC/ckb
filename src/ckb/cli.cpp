#include "cli.h"


int CommandLine::execute(QStringList args) {
    QStringList::const_iterator constIterator;

    // TODO: delegate accumulated commands to own handler, where command is executed
    //       * create instance of cli class/namespace
    //       * pass all remaining arguments to cli handler
    //       * catch return value and handle teardown code
    int index = 0;
    for (constIterator = args.constBegin(); constIterator != args.constEnd(); ++constIterator) {
        fprintf(stdout, "%03d: %s\n", index++, qPrintable((*constIterator).toLocal8Bit().constData()));
    }

    return CommandLineOK;
}
