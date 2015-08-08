#include "keyaction.h"
#include "kb.h"
#include "kbprofile.h"
#include <QUrl>

KeyAction::Type KeyAction::type() const {
    if(_value.isEmpty())
        return UNBOUND;
    if(_value.at(0) == '$')
        return SPECIAL;
    return NORMAL;
}

KeyAction::KeyAction(const QString &action, QObject *parent)
    : QObject(parent), _value(action), preProgram(0), relProgram(0), sniperValue(0)
{
}

KeyAction::KeyAction(QObject *parent)
    : QObject(parent), _value(""), preProgram(0), relProgram(0), sniperValue(0)
{
}

KeyAction::~KeyAction(){
    // Clean up processes
    if(preProgram){
        preProgram->kill();
        delete preProgram;
    }
    if(relProgram){
        relProgram->kill();
        delete relProgram;
    }
}

QString KeyAction::defaultAction(const QString& key){
    // G1-G18 are unbound by default
    if(key.length() >= 2 && key[0] == 'g' && key[1] >= '0' && key[1] <= '9')
        return "";
    // TODO: default action for MR
    if(key == "mr")
        return "";
    // M1-M3 switch modes
    if(key == "m1")
        return "$mode:0";
    if(key == "m2")
        return "$mode:1";
    if(key == "m3")
        return "$mode:2";
    // Brightness and Win Lock are their own functions
    if(key == "light")
        return "$light:2";
    if(key == "lock")
        return "$lock:0";
    // DPI buttons
    if(key == "dpiup")
        return "$dpi:-2";
    if(key == "dpidn")
        return "$dpi:-1";
    if(key == "sniper")
        return "$dpi:0";
    // Everything else is a standard keypress
    return key;
}

QString KeyAction::friendlyName(const KeyMap& map) const {
    if(_value.isEmpty())
        return "Unbound";
    QStringList parts = _value.split(":");
    QString prefix = parts[0];
    if(parts.length() < 2){
        KeyMap::Layout layout = map.layout();
        QString name = KeyMap::friendlyName(_value, layout);
        if(name.isEmpty())
            return "(Unknown)";
        return name;
    }
    int suffix = parts[1].toInt();
    if(prefix == "$mode"){
        switch(suffix){
        case MODE_PREV:
        case MODE_PREV_WRAP:
            return "Switch to previous mode";
        case MODE_NEXT:
        case MODE_NEXT_WRAP:
            return "Switch to next mode";
        default:
            return tr("Switch to mode %1").arg(suffix + 1);
        }
    } else if(prefix == "$dpi"){
        // Split off custom parameters (if any)
        int level = parts[1].split("+")[0].toInt();
        switch(level){
        case DPI_UP:
            return "DPI up";
        case DPI_DOWN:
            return "DPI down";
        case DPI_SNIPER:
            return "Sniper";
        case DPI_CUSTOM:{
            QPoint xy;
            dpiInfo(xy);
            return tr("DPI: %1, %2").arg(xy.x()).arg(xy.y());
        }
        default:
            return tr("DPI stage %1").arg(level);
        }
    } else if(prefix == "$light"){
        switch(suffix){
        case LIGHT_UP:
        case LIGHT_UP_WRAP:
            return "Brightness up";
        case LIGHT_DOWN:
        case LIGHT_DOWN_WRAP:
            return "Brightness down";
        }
    } else if(prefix == "$lock"){
        switch(suffix){
        case LOCK_TOGGLE:
            return "Toggle Windows lock";
        case LOCK_ON:
            return "Windows lock on";
        case LOCK_OFF:
            return "Windows lock off";
        }
    } else if(prefix == "$program"){
        return "Launch program";
    }
    return "(Unknown)";
}

QString KeyAction::modeAction(int mode){
    return QString("$mode:%1").arg(mode);
}

QString KeyAction::dpiAction(int level, int customX, int customY){
    QString action = tr("$dpi:%1").arg(level);
    if(level == DPI_CUSTOM)
        action += tr("+%1+%2").arg(customX).arg(customY);
    return action;
}

QString KeyAction::lightAction(int type){
    return QString("$light:%1").arg(type);
}

QString KeyAction::lockAction(int type){
    return QString("$lock:%1").arg(type);
}

QString KeyAction::programAction(const QString& onPress, const QString& onRelease, int stop){
    // URL-encode the commands and place them in the string (":" and "+" are both replaced, so they won't interfere)
    return "$program:" + QString::fromUtf8(QUrl::toPercentEncoding(onPress.trimmed())) + "+" + QString::fromUtf8(QUrl::toPercentEncoding(onRelease.trimmed())) + QString("+%1").arg(stop);
}

QString KeyAction::specialInfo(int& parameter) const {
    QStringList list = _value.split(":");
    if(list.length() < 2){
        parameter = INT_MIN;
        return "";
    }
    parameter = list[1].toInt();
    return list[0].replace("$", "");
}

int KeyAction::programInfo(QString& onPress, QString& onRelease) const {
    if(!isProgram())
        return 0;
    QString param = _value.mid(9);
    QStringList programs = param.split("+");
    if(programs.length() != 3)
        return 0;
    onPress = QUrl::fromPercentEncoding(programs[0].toUtf8());
    onRelease = QUrl::fromPercentEncoding(programs[1].toUtf8());
    return programs[2].toInt();
}

int KeyAction::dpiInfo(QPoint& custom) const {
    if(!isDPI())
        return 0;
    QString param = _value.mid(5);
    QStringList lxy = param.split("+");
    int level = lxy[0].toInt();
    if(level == DPI_CUSTOM){
        if(lxy.length() != 3)
            return 0;
        custom = QPoint(lxy[1].toInt(), lxy[2].toInt());
    }
    return level;
}

QString KeyAction::driverName() const {
    if(isSpecial())
        return "";
    return _value;
}

void KeyAction::keyEvent(KbBind* bind, bool down){
    // No need to respond to standard actions
    if(!isSpecial())
        return;
    QStringList parts = _value.split(":");
    if(parts.length() < 2)
        return;
    QString prefix = parts[0];
    int suffix = parts[1].toInt();
    if(prefix == "$mode"){
        if(!down)
            return;
        // Change mode
        Kb* device = bind->devParent();
        KbProfile* currentProfile = device->currentProfile();
        int mode = currentProfile->indexOf(currentProfile->currentMode());
        int modeCount = currentProfile->modeCount();
        switch(suffix){
        case MODE_PREV_WRAP:
            mode--;
            if(mode < 0)
                mode = modeCount - 1;
            break;
        case MODE_NEXT_WRAP:
            mode++;
            if(mode >= modeCount)
                mode = 0;
            break;
        case MODE_PREV:
            mode--;
            break;
        case MODE_NEXT:
            mode++;
            break;
        default:
            // Absolute
            mode = suffix;
            break;
        }
        if(mode < 0 || mode >= modeCount)
            return;
        device->setCurrentMode(currentProfile->modes()[mode]);
    } else if(prefix == "$dpi"){
        KbPerf* perf = bind->perf();
        int level = parts[1].split("+")[0].toInt();
        switch(level){
        case DPI_UP:
            if(!down)
                return;
            perf->dpiUp();
            break;
        case DPI_DOWN:
            if(!down)
                return;
            perf->dpiDown();
            break;
        case DPI_SNIPER:
            if(down)
                sniperValue = perf->pushSniper();
            else {
                perf->popDpi(sniperValue);
                sniperValue = 0;
            }
            break;
        case DPI_CUSTOM:{
            QPoint xy;
            dpiInfo(xy);
            if(xy.x() <= 0 || xy.y() <= 0)
                break;
            if(down)
                sniperValue = perf->pushDpi(xy);
            else {
                perf->popDpi(sniperValue);
                sniperValue = 0;
            }
            break;
        }
        default:
            if(level < 1 || level >= KbPerf::DPI_COUNT
                    || !down)
                return;
            perf->dpi(level);
            break;
        }
    } else if(prefix == "$light"){
        if(!down)
            return;
        // Change brightness
        KbLight* light = bind->light();
        int dim = light->dimming();
        switch(suffix){
        case LIGHT_UP:
            if(dim > 0)
                dim--;
            break;
        case LIGHT_DOWN:
            if(dim < KbLight::MAX_DIM)
                dim++;
            break;
        case LIGHT_UP_WRAP:
            dim--;
            if(dim < 0)
                dim = KbLight::MAX_DIM;
            break;
        case LIGHT_DOWN_WRAP:
            dim++;
            if(dim > KbLight::MAX_DIM)
                dim = 0;
            break;
        }
        light->dimming(dim);
    } else if(prefix == "$lock"){
        if(!down)
            return;
        // Change win lock
        switch(suffix){
        case LOCK_TOGGLE:
            bind->winLock(!bind->winLock());
            break;
        case LOCK_ON:
            bind->winLock(true);
            break;
        case LOCK_OFF:
            bind->winLock(false);
            break;
        }
    } else if(prefix == "$program"){
        // Launch program
        QString onPress, onRelease;
        int stop = programInfo(onPress, onRelease);
        // Stop running programs based on setting
        QProcess* process = 0;
        if(down){
            if(stop & PROGRAM_PR_KPSTOP){
                process = preProgram;
                if(process)
                    process->kill();
                process = 0;
            }
            if(stop & PROGRAM_RE_KPSTOP)
                process = relProgram;
        } else {
            if(stop & PROGRAM_PR_KRSTOP)
                process = preProgram;
        }
        if(process)
            process->kill();
        // Launch new process if requested
        QString& program = down ? onPress : onRelease;
        if(program.isEmpty())
            return;
        // Check if the program is running already. If so, don't start it again.
        process = down ? preProgram : relProgram;
        if(process){
            if(process->state() == QProcess::NotRunning)
                delete process;
            else
                return;
        }
        // Start the program. Wrap it around sh to parse arguments.
        process = new QProcess(this);
        process->start("sh", QStringList() << "-c" << program);
        if(down)
            preProgram = process;
        else
            relProgram = process;
    }
}
