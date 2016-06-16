#ifndef CKB_CLI_H
#define CKB_CLI_H

/* Include Section */
#include <QObject>
#include <keymap.h>


class Command : public QObject
{
    Q_OBJECT
public:
    enum CommandType {
        // Top Level Commands
        CommandGlobal,
        CommandDevice,
        CommandProfile,

        // Sub-commands
        CommandHelp,
        CommandInfo,
        CommandLayout,
        CommandModifier,
        CommandFramerate,
        CommandAnimationDir,
        CommandBrightnessPerMode,
        CommandSpatialDithering,
        CommandFirmwareAutocheck,
        CommandTrayIcon,
        CommandMouseAcceleration,
        CommandScrollAcceleration,
        CommandScrollAccelerationSpeed,

        // Generic OK/Fail Flags
        CommandOK,
        CommandUnknown
    };

    static int resolveCommand(QString cmd);
};


class CommandLine : public QObject
{
    Q_OBJECT
private:
    int cmdOffset;

    int runGlobal();
    int run();
public:
    QString base;
    QString flag;
    QStringList commands;

    enum CommandLineResult {
        CommandLineOK,
        CommandLineUnknown
    };
    static int execute(QStringList args);

    friend class Command;
};

#endif // CKB_CLI_H
