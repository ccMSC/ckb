#ifndef KBLIGHT_H
#define KBLIGHT_H

#include <QFile>
#include <QObject>
#include <QSettings>
#include "animscript.h"
#include "kbanim.h"
#include "keymap.h"

class KbMode;

// Keyboard lighting setup

class KbLight : public QObject
{
    Q_OBJECT
public:
    // New lighting setup
    KbLight(KbMode* parent, const KeyMap& keyMap);
    // Copy a lighting setup
    KbLight(KbMode *parent, const KeyMap& keyMap, const KbLight& other);
    ~KbLight();

    // Key map
    inline const KeyMap&    map() { return _map; }
    void                    map(const KeyMap& map);
    // Key -> color map
    typedef QHash<QString, QRgb> ColorMap;
    inline const ColorMap&  colorMap() { return _colorMap; }
    // Color a key
    inline void             color(const QString& key, const QColor& newColor) { _needsSave = true; _colorMap[key] = newColor.rgb(); }
    // Color all keys in the current map
    void                    color(const QColor& newColor);

    // Overall dimming. 0 = max brightness, 3 = off
    static const int    MAX_DIM = 3;
    inline int          dimming()               { return _dimming; }
    void                dimming(int newDimming);
    // Shared brightness between all modes (-1 for sharing disabled)
    static int          shareDimming();
    static void         shareDimming(int newShareDimming);

    // Inactive indicator level. -1 for no dimming, 2 for off
    static const int    MAX_INACTIVE = 2;
    inline int          inactive()              { return _inactive; }
    inline void         inactive(int in)        { _needsSave = true; _inactive = in; emit updated(); }

    // Whether or not to indicate the mute key
    inline bool showMute()                  { return _showMute; }
    inline void showMute(bool newShowMute)  { _needsSave = true; _showMute = newShowMute; emit updated(); }

    // Lighting animations
    typedef QList<KbAnim*> AnimList;
    KbAnim*             addAnim(const AnimScript* base, const QStringList& keys, const QString& name, const QMap<QString, QVariant>& preset);
    KbAnim*             duplicateAnim(KbAnim* oldAnim);
    const AnimList&     animList()                              { return _animList; }
    void                animList(const AnimList& newAnimList)   { _needsSave = true; _animList = newAnimList; }
    // Preview animation - temporary animation displayed at the top of the animation list
    void previewAnim(const AnimScript* base, const QStringList& keys, const QMap<QString, QVariant>& preset);
    void stopPreview();
    // Stops and restarts all animations
    void restartAnimation();
    // Sends a keypress event to active animations
    void animKeypress(const QString& key, bool down);

    // Start the mode
    void open();
    // Whether or not all animations have started
    bool isStarted();
    // Make the lighting idle, stopping any animations.
    void close();

    // Write a new frame to the keyboard. Write "mode %d" first. Optionally provide a list of keys to show in a dimmed state and a list of keys to
    // use as indicators. (note: indicator list and dim list should not overlap)
    void frameUpdate(QFile& cmd, const QStringList &dimKeys = QStringList(), const ColorMap& indicators = ColorMap());
    // Write the mode's base colors without any animation
    void base(QFile& cmd, bool ignoreDim = false);

    // Load and save from stored settings
    void load(CkbSettings& settings);
    void save(CkbSettings& settings);
    bool needsSave() const;

signals:
    void didLoad();
    void updated();
    void frameDisplayed(ColorMap animatedColors);

private:
    AnimList    _animList;
    KbAnim*     _previewAnim;
    KeyMap      _map;
    ColorMap    _colorMap;
    quint64     lastFrameSignal;
    int         _dimming;
    int         _inactive;
    bool        _showMute;
    bool        _start;
    bool        _needsSave;

    void printRGB(QFile& cmd, const QHash<QString, QRgb>& animMap);
};

#endif // KBLIGHT_H
