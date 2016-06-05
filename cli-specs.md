# CLI

The command line interface is a fully scriptable interface to the user.

## Idea

Pass in a specific switch/flag - for example `--cmd` - to enter command mode and interpret subsequent strings as the command to execute.

## Available operations

Gather all available operations/commands here to have an overview and a guideline for parsing the given operation strings.

Additionally there should be a help page to display the available operations and the commands for them. The tooltip text can be used for this endeavour.

### Settings

* list all available keyboard layouts
* set keyboard layout
* list available modifier keys (notify of keyboard profile/binding)
  * OSX has different source keys! (i.e. Option, Command instead of Alt, Super)
* set modifier key to different key
* reset modifier keys
* display and set _frame rate_
* display and set _directory of animation scripts_
* display and set _brightness per-mode_
* display and set _spatial dithering to simulate extra color resolution_
* display and set _disable mouse acceleration_ (only on OSX)
* display and set _disable scroll acceleration_ (only on OSX)
* display and set _scroll acceleration speed_ (only on OSX)
* display and set _check for new firmware automatically_
* display and set _show tray icon_
* display and set _macro delay_

### Keyboard

* List Profiles
* Create/Rename/Duplicate/Move Profile
* Save Profile to Hardware
* List Modes
* Create/Rename/Duplicate/Move Modes

#### Lighting

* display and set _key color_
* display and set _key brightness_
* display and set _key animation_

#### Binding

* display, set and/or add _key bind_
  * bind to keyboard input (char, function, media, modifier, etc.)
  * bind to mouse input (button, wheel, special button, change DPI, etc.)
  * bind to animation (currently assigned animations)
  * bind to special (switch mode, brightness, windows lock)
  * bind to program (run script/program on press/release)
* copy bindings to mode
* bind macro action, text and comment to key

#### Performance

* display and set _indicator intensity_
* display and set value of _indicator intensity_
* display and set _indicate brightness_
* display and set value of _indicate brightness_ (33%, 67%, 100%)
* display and set _indicate windows lock_
* display and set value of _indicate windows lock_ (on, off)
* display and set _indicate mute_
* display and set value of _indicate mute_ (on, off, unknown)
* list available modes for lock keys
* display and set _num lock_
* display and set value of _num lock_ (on, off)
* display and set _caps lock_
* display and set value of _caps lock_ (on, off)
* display and set _scroll lock_ (display warning because of scroll lock)
* display and set value of _scroll lock_ (on, off)

#### Device

* display information about device
  * _USB device ID_
  * _Poll Rate_
  * _Firmware Version_
  * _Status_
* update firmware
  * automatically (via internet)
  * manually (via file)

### Mouse

* List Profiles
* Create/Rename/Duplicate/Move Profile
* Save Profile to Hardware
* List Modes
* Create/Rename/Duplicate/Move Modes

#### Lighting

* display and set _key color_
* display and set _key brightness_
* display and add _key animation_

#### Binding

* display, set and/or add _key bind_
  * bind to keyboard input (char, function, media, modifier, etc.)
  * bind to mouse input (button, wheel, special button, change DPI, etc.)
  * bind to animation (currently assigned animations)
  * bind to special (switch mode, brightness, windows lock)
  * bind to programm (run script/program on press/release)
* copy bindings to mode

#### Performance

* display and set _use DPI indicator_
* display and set _DPI Indicator percentage_
* display and set _independent X/Y states_
* list DPI Stages with X/Y values
* display and set stages 1-5 + sniper & other
* display and set _angle snap_
* list _lift height_ values
* display and set _lift height_
* copy performance to mode

#### Device

* display information about device
  * _USB device ID_
  * _Poll Rate_
  * _Firmware Version_
  * _Status_
* update firmware
  * automatically (via internet)
  * manually (via file)

## Grammar

Generally the command line interface needs a different approach, than the GUI. For example does the GUI display all information concisely, while the _cli_ can only display a set of information at a time triggered by a command. Thus the grouping has to be different than the GUI does it.

```
<cmd> ::= "global" <global-cmd>
        | "device" [<device-id>] <device-cmd>
        | "profile" [<profile-id>] <profile-cmd>
<device-id> ::= <str>
<profile-id> ::= <str>

<direction> ::= "up" | "down"
<color> ::= "#" <int>
          | "(" <int> "," <int> "," <int> ")"
```

### Global Commands

```
<global-cmd> ::= "info"
               | "layout" <global-layout-cmd>
               | "modifier" <global-modifier-cmd>
               | "framerate" <input-int>
               | "animation-dir" <global-animation-dir-cmd>
               | "brightness-per-mode" <input-bool>
               | "spatial-dithering" <input-bool>
               | "firmware-autocheck" <input-bool>
               | "tray-icon" <input-bool>
               | "mouse-acceleration" <input-bool>
               | "scroll-acceleration" <input-bool>
               | "scroll-acceleration-speed" <input-int>

<input-int> ::= "show" | "set" <int>
<input-float> ::= "show" | "set" <float>
<input-str> ::= "show" | "set" <str>
<input-bool> ::= "show" | "enable" | "disable"

<global-layout-cmd> ::= "list" | "set" <layout>
<layout> ::= "EU" | "EU_DVORAK"
           | "GB" | "GB_DVORAK"
           | "US" | "US_DVORAK"
           | "FR" | "DE" | "IT" | "PL" | "MX" | "ES" | "SE"

<global-modifier-cmd> ::= "list" | "set <modifier-key> <modifier-name>" | "reset"
<modifier-key> ::= "lshift" | "lctrl" | "lwin" | "lalt" | "caps" | "menu"
                 | "rshift" | "rctrl" | "rwin" | "ralt" | "fn" | "altgr"
<modifier-name> ::= "caps" | "shift" | "ctrl" | "option" | "cmd" | "alt" | "super"

<global-animation-dir-cmd> ::= <input-str> | "scan"
```

### Device Commands

```
<device-cmd> ::= "" | "info"
               | "show profile"
               | "set profile" <profile-id>
               | "firmware" <firmware-cmd>

<firmware-cmd> ::= "show"
                 | "update" [<input-str>]
```

### Profile Commands

```
<profile-cmd> ::= "" | "list"
                | "create" <profile-name>
                | "duplicate" <profile-name>
                | "rename" <profile-name> <profile-name>
                | "move" <profile-name> <direction>
                | <profile-name> "mode" <mode-cmd>
<profile-name> ::= <str>

<mode-cmd> ::= "list"
             | "create" <mode-name>
             | "duplicate" <mode-name>
             | "rename" <mode-name>
             | "move" <direction>
             | "animation" <animation-cmd>
             | "lighting" <lighting-cmd>
             | "bind" <bind-cmd>
             | "performance" <performance-cmd>

<animation-cmd> ::= "list"
                  | "show"
                  | "delete" <animation-id>
                  | "set" <animation-id> <animation-specs>

<lighting-cmd> ::= "brightness" <lighting-brightness-cmd>
                 | "color" <key-id> <lighting-color-cmd>
                 | "animation" <key-id> <lighting-animation-cmd>
<lighting-brightness-cmd> ::= "show" | "set" <indicator-mode>
<lighting-color-cmd> ::= "show" | "set" <color>
<lighting-animation-cmd> ::= "show" | "set" <animation-id>

<bind-cmd> ::= "list"
             | "show" <key>
             | "set" <key> <key-def>
             | "copy" <mode-name>
             | "reset"
             | "set-macro" <macro-definition>
             | "list-macros"

<key-def> ::= <modifier-list><key-list>
<modifier-list> ::= ""
                  | <modifier-key>"+"
                  | <modifier-key><modifier-list>
<key-list> ::= <key> | <key>"+"<key-list>
<key> ::= <str>
        | <key-id>
<key-id> ::= <int>

<macro-definition> ::= <key-def> <macro-action> <macro-text> <macro-comment>
<macro-action> ::= "" | <str>
<macro-text> ::= "" | <str>
<macro-comment> ::= "" | <str>

<performance-cmd> ::= "copy" <mode-name>
                    | "intensity" <input-int>
                    | "indicator" <performance-indicator-cmd>
                    | "lock" <performance-lock-cmd>
                    | "dpi" <dpi-cmd>

<performance-indicator-cmd> ::= <indicator-key-name> <indicator-cmd>
<indicator-key-name> ::= "brightness" | "windows-lock" | "mute"
<indicator-cmd> ::= "show" | "enable" | "disable" | set <indicator-mode> <color>
<indicator-mode> ::= "0" | "1" | "2"

<dpi-cmd> ::= "list"
            | "indicator" <dpi-indicator-cmd>
            | "independent-axes" <input-bool>
            | "stage" <stage> "set" "x" <int> ["y" <int>]
            | "angle-snap" <input-bool>
            | "lift-height" <input-int>
<dpi-indicator-cmd> ::= <input-bool> | "set" <int>
<dpi-stage> ::= "0" | "1" | "2" | "3" | "4" | "5" | "other"

<performance-lock-cmd> ::= "show" | <lock-key> <lock-cmd>
<lock-key> ::= "num" | "caps" | "scroll"
<lock-cmd> ::= "set" <lock-mode> | "set" <lock-setting> <color>
<lock-mode> ::= "normal" | "always-on" | "always-off" | "RGB" | "normal-rgb"
<lock-setting> ::= "0" | "1"
```
