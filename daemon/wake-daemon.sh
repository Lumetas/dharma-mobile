#!/bin/bash
set -u

(
    while sleep 2; do
		pgrep -f '/dharma-mobile/dharma' >/dev/null || { kill 0; exit; }
    done
) &
WATCH_PID=$!

CONF="/etc/dharma/wake-daemon.conf"

find_power_btn() {
    awk '
        /^N: Name="pm8941_pwrkey"/ { found = 1; next }
        found && /^H: Handlers=/ {
            for (i = 1; i <= NF; i++)
                if ($i ~ /^event[0-9]+$/) {
                    print "/dev/input/" $i
                    exit
                }
        }
        /^$/ { found = 0 }
    ' /proc/bus/input/devices
}
BTN_DEV="$(find_power_btn)";

[ -f "$CONF" ] && . "$CONF"

: "${TOUCHSCREEN:?TOUCHSCREEN not set (must come from Dharma via set-environment)}"
: "${BTN_DEV:?BTN_DEV not set in $CONF}"
: "${SLEEP_DELAY:=60}"
: "${DBL:=0.4}"
: "${SHOT_DIR:=$HOME/screenshots}"

STATE="awake"
CLICK_PID=""
SLEEP_PID=""
LAST=""

log() { echo "[wake-daemon] $*" >&2; }

freeze() { systemctl --user freeze app.slice session.slice 2>/dev/null || true; }
thaw()   { systemctl --user thaw   app.slice session.slice 2>/dev/null || true; }

screen_off() { xinput disable "$TOUCHSCREEN" 2>/dev/null; xset dpms force off 2>/dev/null; }
screen_on()  { xinput enable  "$TOUCHSCREEN" 2>/dev/null; xset dpms force on  2>/dev/null; }

screen_is_on() {
    [ "$(xinput list-props "$TOUCHSCREEN" 2>/dev/null \
        | grep 'Device Enabled' | cut -d: -f2 | tr -d ' \t')" = "1" ]
}

shot() {
    mkdir -p "$SHOT_DIR"
    maim "$SHOT_DIR/$(date +%Y-%m-%d_%H-%M-%S).png" && notify-send "Screenshot saved"
}

cancel_sleep() { [ -n "$SLEEP_PID" ] && kill "$SLEEP_PID" 2>/dev/null; SLEEP_PID=""; }
cancel_click() { [ -n "$CLICK_PID" ] && kill "$CLICK_PID" 2>/dev/null; CLICK_PID=""; }

schedule_sleep() {
    cancel_sleep
    (
        sleep "$SLEEP_DELAY"
        if ! screen_is_on; then
            STATE="asleep"
            log "freezing"
            freeze
        fi
    ) &
    SLEEP_PID=$!
}

on_press() {
    if [ "$STATE" = "asleep" ]; then
        log "wake"
        STATE="awake"
        thaw
        screen_on
        return
    fi

    now=$(date +%s.%N)
    if [ -n "$LAST" ]; then
        delta=$(awk -v a="$now" -v b="$LAST" 'BEGIN{print a-b}')
        if awk -v d="$delta" -v w="$DBL" 'BEGIN{exit !(d<w)}'; then
            cancel_click
            LAST=""
            log "double -> screenshot"
            shot
            return
        fi
    fi

    LAST="$now"
    cancel_click
    (
        sleep "$DBL"
        if screen_is_on; then
            log "single -> screen off"
            screen_off
            schedule_sleep
        fi
    ) &
    CLICK_PID=$!
}

log "start, dev=$BTN_DEV, touch=$TOUCHSCREEN"
evtest --grab "$BTN_DEV" | while IFS= read -r l; do
    case "$l" in
        *"code 116 (KEY_POWER), value 1"*) on_press ;;
    esac
done
