#include "Arduino.h"
#ifndef Morse_h
#define Morse_h
class Morse
{
    public: 
        Morse(int pin, String message, int dotLength);
        int pin;
        String message;
        int dotLength;
        String getMorseInstruct(char letter);
        void sendMorse2Lamp();
        void stop();
    private:
        int _dashLength;
        int _dotLength;
        int _previousMillis;
        int _interval;
        int _j;
        int _i;
        int _pin;
        String _text;
};

#endif
