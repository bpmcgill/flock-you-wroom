#ifndef BUZZER_H
#define BUZZER_H

#include <Arduino.h>
#include "config/pins.h"

enum BuzzerType {
    BUZZER_ACTIVE,   // 2-pin active buzzer (simple on/off)
    BUZZER_PASSIVE   // 3-pin passive buzzer (requires PWM/tones)
};

class Buzzer {
public:
    void begin();
    void setType(BuzzerType type);
    BuzzerType getType() { return buzzerType; }
    
    void beep(int count, int duration, int gap);
    void bootSequence();
    void detectionAlert();
    void knownDeviceBeep();  // Short beep for known devices
    void heartbeat();

private:
    BuzzerType buzzerType = BUZZER_ACTIVE;
    uint8_t pwmChannel = 0;  // PWM channel for passive buzzer
    
    void playToneActive(int duration);
    void playTonePassive(uint16_t frequency, uint16_t duration);
    void playMelody(const uint16_t* notes, const uint16_t* durations, uint8_t length);
};

extern Buzzer buzzer;

// Musical notes for passive buzzer
#define NOTE_C4  262
#define NOTE_D4  294
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_G4  392
#define NOTE_A4  440
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_D5  587
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_G5  784
#define NOTE_A5  880

#endif // BUZZER_H

