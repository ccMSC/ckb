#ifndef CKB_CLI_H
#define CKB_CLI_H

/* Include Section */
#include <QObject>

class CommandLine : public QObject
{
    Q_OBJECT
private:
    int run();
public:
    QString base;
    QString flag;
    QStringList commands;

    enum CommandLineResult {
        CommandLineOK
    };
    static int execute(QStringList args);
};

#endif // CKB_CLI_H
