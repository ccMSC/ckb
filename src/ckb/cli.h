#ifndef CKB_CLI_H
#define CKB_CLI_H

/* Include Section */
#include <QObject>

class CommandLine : public QObject
{
    Q_OBJECT
public:
    enum CommandLineResult {
        CommandLineOK
    };
    static int execute(QStringList args);
};

#endif // CKB_CLI_H
