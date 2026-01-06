#include "buzzer.h"
#include "led_controller.h"

Buzzer buzzer;

void Buzzer::begin() {
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
}

void Buzzer::beep(int count, int duration, int gap) {
    for (int i = 0; i < count; i++) {
        digitalWrite(BUZZER_PIN, HIGH);
        delay(duration);
        digitalWrite(BUZZER_PIN, LOW);
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
    
    // Two beeps
    beep(2, BOOT_BEEP_DURATION, 100);
    
    printf("Audio and LED system ready\n\n");
}

void Buzzer::detectionAlert() {
    printf("FLOCK SAFETY DEVICE DETECTED!\n");
    printf("Playing alert sequence: 3 fast beeps + LED flash\n");
    
    // Red LED flash
    LED.flash(LEDController::COLOR_RED, 3, DETECT_BEEP_DURATION);
    
    // Three fast beeps
    beep(3, DETECT_BEEP_DURATION, 50);
    
    printf("Detection complete - device identified!\n\n");
}

void Buzzer::heartbeat() {
    printf("Heartbeat: Device still in range\n");
    
    // Orange LED pulse
    LED.pulse(LEDController::COLOR_ORANGE, 400);
    
    // Two beeps
    beep(2, HEARTBEAT_DURATION, 100);
}
