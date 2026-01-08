#include "buzzer.h"
#include "led_controller.h"

Buzzer buzzer;

void Buzzer::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    
    if (buzzerType == BUZZER_PASSIVE) {
        // Setup PWM for passive buzzer
        ledcSetup(pwmChannel, 2000, 8);  // 2kHz, 8-bit resolution
        ledcAttachPin(BUZZER_PIN, pwmChannel);
        ledcWrite(pwmChannel, 0);  // Start silent
    }
}

void Buzzer::setType(BuzzerType type) {
    buzzerType = type;
}

void Buzzer::playToneActive(int duration) {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(duration);
    digitalWrite(BUZZER_PIN, LOW);
}

void Buzzer::playTonePassive(uint16_t frequency, uint16_t duration) {
    ledcWriteTone(pwmChannel, frequency);
    ledcWrite(pwmChannel, 128);  // 50% duty cycle
    delay(duration);
    ledcWrite(pwmChannel, 0);    // Stop tone
}

void Buzzer::playMelody(const uint16_t* notes, const uint16_t* durations, uint8_t length) {
    for (uint8_t i = 0; i < length; i++) {
        if (notes[i] == 0) {
            delay(durations[i]);  // Rest
        } else {
            if (buzzerType == BUZZER_PASSIVE) {
                playTonePassive(notes[i], durations[i]);
            } else {
                playToneActive(durations[i]);
            }
        }
        delay(50);  // Small gap between notes
    }
}

void Buzzer::beep(int count, int duration, int gap) {
    for (int i = 0; i < count; i++) {
        if (buzzerType == BUZZER_PASSIVE) {
            playTonePassive(2000, duration);  // 2kHz tone
        } else {
            playToneActive(duration);
        }
        if (i < count - 1) delay(gap);
    }
}

void Buzzer::bootSequence() {
    printf("Initializing audio system...\n");
    printf("Playing boot sequence\n");
    
    // Blue LED fade-in animation
    LED.fadeIn(LEDController::COLOR_BLUE, 500);
    delay(200);
    LED.setAllLEDs(LEDController::COLOR_OFF);
    
    if (buzzerType == BUZZER_PASSIVE) {
        // Ascending musical scale
        uint16_t notes[] = {NOTE_C4, NOTE_E4, NOTE_G4, NOTE_C5};
        uint16_t durations[] = {100, 100, 100, 200};
        playMelody(notes, durations, 4);
    } else {
        // Two beeps for active buzzer
        beep(2, BOOT_BEEP_DURATION, 100);
    }
    
    printf("Audio and LED system ready\n");
    printf("Buzzer type: %s\n\n", buzzerType == BUZZER_PASSIVE ? "PASSIVE (PWM)" : "ACTIVE");
}

void Buzzer::detectionAlert() {
    printf("FLOCK SAFETY DEVICE DETECTED!\n");
    printf("Playing alert sequence: 3 fast beeps + LED flash\n");
    
    // Red LED flash
    LED.flash(LEDController::COLOR_RED, 3, DETECT_BEEP_DURATION);
    
    if (buzzerType == BUZZER_PASSIVE) {
        // Urgent high-pitched beeps
        for (int i = 0; i < 3; i++) {
            playTonePassive(2500, DETECT_BEEP_DURATION);
            if (i < 2) delay(50);
        }
    } else {
        // Three fast beeps
        beep(3, DETECT_BEEP_DURATION, 50);
    }
    
    printf("Detection complete - device identified!\n\n");
}

void Buzzer::heartbeat() {
    printf("Heartbeat: Device still in range\n");
    
    // Orange LED pulse
    LED.pulse(LEDController::COLOR_ORANGE, 400);
    
    if (buzzerType == BUZZER_PASSIVE) {
        // Two-tone heartbeat
        playTonePassive(1800, HEARTBEAT_DURATION);
        delay(100);
        playTonePassive(1800, HEARTBEAT_DURATION);
    } else {
        // Two beeps
        beep(2, HEARTBEAT_DURATION, 100);
    }
}

void Buzzer::knownDeviceBeep() {
    // Single short beep for known device re-detection
    printf("Known device re-detected\n");
    
    if (buzzerType == BUZZER_PASSIVE) {
        // Lower pitch for known device
        playTonePassive(1500, 200);
    } else {
        beep(1, 50, 0);  // Quick single beep
    }
}
