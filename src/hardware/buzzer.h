#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include "config/pins.h"

class Buzzer {
public:
    void begin();
    void beep(int count, int duration, int gap);
    void bootSequence();
    void detectionAlert();
    void heartbeat();

private:
};

extern Buzzer buzzer;

#endif // BUZZER_H
