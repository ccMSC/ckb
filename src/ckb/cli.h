#ifndef CKB_CLI_H
#define CKB_CLI_H

/* Include Section */
#include <QObject>

class CommandLine : public QObject
{
    Q_OBJECT
public:
    static void execute(QStringList args);
};

#endif // CKB_CLI_H
