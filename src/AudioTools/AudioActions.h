#pragma once
#include "Arduino.h"
#include "AudioTools/AudioLogger.h"

#ifndef ACTIONS_MAX
#define ACTIONS_MAX 20
#endif

#ifndef DEBOUNCE_DELAY
#define DEBOUNCE_DELAY 500
#endif

namespace audio_tools {

/**
 * @brief A simple class to assign Functions to Pins e.g. to implement a simple
 * navigation control or volume control with buttons
 */
class AudioActions {
 public:
  enum ActiveLogic : uint8_t {
    ActiveLow,
    ActiveHigh,
    ActiveChange,
    ActiveTouch
  };

  /// Adds a action
  void add(int pin, void (*actionOn)(bool pinStatus, int pin, void* ref),
           ActiveLogic activeLogic = ActiveLow, void* ref = nullptr) {
    add(pin, actionOn, nullptr, activeLogic, ref);
  }

  /// Adds a action
  void add(int pin, void (*actionOn)(bool pinStatus, int pin, void* ref),
           void (*actionOff)(bool pinStatus, int pin, void* ref),
           ActiveLogic activeLogicPar = ActiveLow, void* ref = nullptr) {
    LOGI("ActionLogic::add pin: %d / logic: %d", pin, activeLogicPar);
    if (maxIdx + 1 >= ACTIONS_MAX) {
      LOGE("Too many actions: please increase ACTIONS_MAX")
      return;
    }

    int pos = findPin(pin);
    if (pos != -1) {
      actions[pos].actionOn = actionOn;
      actions[pos].actionOff = actionOff;
      actions[pos].activeLogic = activeLogicPar;
      actions[pos].ref = ref;
    } else {
      // setup pin mode
      if (activeLogicPar == ActiveLow) {
        pinMode(pin, INPUT_PULLUP);
        LOGI("pin %d -> INPUT_PULLUP", pin);
      } else {
        pinMode(pin, INPUT);
        LOGI("pin %d -> INPUT", pin);
      }

      actions[maxIdx].pin = pin;
      actions[maxIdx].actionOn = actionOn;
      actions[maxIdx].actionOff = actionOff;
      actions[maxIdx].activeLogic = activeLogicPar;
      actions[maxIdx].ref = ref;
      maxIdx++;
    }
  }

  /// enable/disable pin actions
  void setEnabled(int pin, bool enabled) {
    int pos = findPin(pin);
    if (pos != -1) {
      actions[pos].enabled = enabled;
    }
  }

  /**
   * @brief Execute all actions if the corresponding pin is low
   * To minimize the runtime: With each call we process a different pin
   */
  void processActions() {
    static int pos = 0;

    // execute action
    Action* a = &(actions[pos]);
    if (a->enabled) {
      bool value = readValue(a);
      if (a->actionOn != nullptr && a->actionOff != nullptr) {
        // we have on and off action defined
        if (value != a->lastState) {
          LOGI("processActions: case with on and off");
          // execute action -> reports active instead of pin state
          if ((value && a->activeLogic == ActiveHigh) ||
              (!value && a->activeLogic == ActiveLow)) {
            a->actionOn(true, a->pin, a->ref);
          } else {
            a->actionOff(false, a->pin, a->ref);
          }
          a->lastState = value;
        }
      } else if (a->activeLogic == ActiveChange) {
        bool active = (a->activeLogic == ActiveLow) ? !value : value;
        // reports pin state
        if (value != a->lastState) {
          LOGI("processActions: ActiveChange");
          // execute action
          a->actionOn(active, a->pin, a->ref);
          a->lastState = value;
        }
      } else {
        bool active = (a->activeLogic == ActiveLow) ? !value : value;
        if (active &&
            (active != a->lastState || millis() > a->debounceTimeout)) {
          LOGI("processActions: %d Active %d - %d", a->pin, value,
               digitalRead(a->pin));
          // execute action
          a->actionOn(active, a->pin, a->ref);
          a->lastState = active;
          a->debounceTimeout = millis() + DEBOUNCE_DELAY;
        }
      }
    }
    pos++;
    if (pos >= maxIdx) {
      pos = 0;
    }
  }

  /// Defines the debounce delay
  void setDeboundDelay(int value) { debounceDelayValue = value; }

 protected:
  int maxIdx = 0;
  int debounceDelayValue = DEBOUNCE_DELAY;
  int touchLimit = 20;

  struct Action {
    int16_t pin;
    void (*actionOn)(bool pinStatus, int pin, void* ref) = nullptr;
    void (*actionOff)(bool pinStatus, int pin, void* ref) = nullptr;
    void* ref = nullptr;
    unsigned long debounceTimeout;
    ActiveLogic activeLogic;
    bool lastState;
    bool enabled = true;
  } actions[ACTIONS_MAX];

  /// determines the value for the action
  bool readValue(Action* a) {
    bool result;
    if (a->activeLogic == ActiveTouch) {
      int value = touchRead(a->pin);
      result = value <= touchLimit;
      //LOGD("touch pin: %d value %d -> %s", a->pin, value, result ? "true":"false");
    } else {
      result = digitalRead(a->pin);
    }
    return result;
  }

  int findPin(int pin) {
    for (int j = 0; j < maxIdx; j++) {
      if (actions[j].pin == pin) {
        return j;
      }
    }
    return -1;
  }
};

}  // namespace audio_tools