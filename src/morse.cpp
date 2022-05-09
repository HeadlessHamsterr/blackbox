#include "Arduino.h"
#include "MorseLetters.h"


//Define dash and dot
#define MORSE_DOT '*'
#define MORSE_DASH '_'

//Define morse letters
#define A "*_"
#define B "_***"
#define C "_*_*"
#define D "_**"
#define E "*"
#define F "**_*"
#define G "__*"
#define H "****"
#define I "**"
#define J "*___"
#define K "_*_"
#define L "*_**"
#define M "__"
#define N "_*"
#define O "___"
#define P "*__*"
#define Q "__*_"
#define R "*_*"
#define S "***"
#define T "_"
#define U "**_"
#define V "***_"
#define W "*__"
#define X "_**_"
#define Y "_*__"
#define Z "__**"

Morse::Morse(int pin, String message, int dotLength)
{
    _pin = pin;
    _text = message;
    _dotLength = dotLength;
    _dashLength = 3 * dotLength;
    pinMode(pin, OUTPUT);
}


void Morse::sendMorse2Lamp (){
  int currentMillis = millis();
  String morse = getMorseInstruct(_text[_i]);
    if((currentMillis - _previousMillis) >= _interval){ //Blink without delay to avoid code stops
      _previousMillis = currentMillis;

      digitalWrite(_pin, !digitalRead(_pin)); //Inverse lamp state

      //If the lamp is off then
      if(digitalRead(_pin)==false){
        _j++;  //Go to next dash/dot
        _interval = _dotLength;  //And keep the light off for a dot (standard time between dashes and dot)
      }
      //If the lamp is on
      else{
        //Keep it on for the correct amount of time by setting correct interval
        if(morse[_j] == MORSE_DOT){
          _interval = _dotLength;
        }
        else if (morse[_j] == MORSE_DASH){
          _interval = _dashLength;
        }
      }

      //If there are no more dashes or dots left
      if(_j >= morse.length()){
        _j=0;  //Reset indexer
        _i++;  //Go to next letter
        _interval = _dashLength; //And stop for a dash (standard time between letters)
      }

      //If there are no more letters
      if(_i >= _text.length()){
        _i=0; //Reset indexer
        _interval = 7 * _dotLength; //And wait for 7 dots
      }
    }
}

void Morse::stop(){
    digitalWrite(pin, LOW);
}


String Morse::getMorseInstruct(char letter){
        String MorseInstruct;
            switch (letter)
        {
        case 'A':
            MorseInstruct = A;
            break;
        case 'B':
            MorseInstruct = B;
            break;
        case 'C':
            MorseInstruct = C;
            break;
        case 'D':
            MorseInstruct = D;
            break;
        case 'E':
            MorseInstruct = E;
            break;
        case 'F':
            MorseInstruct = F;
            break;
        case 'G':
            MorseInstruct = G;
            break;
        case 'H':
            MorseInstruct = H;
            break;
        case 'I':
            MorseInstruct = I;
            break;
        case 'J':
            MorseInstruct = J;
            break;
        case 'K':
            MorseInstruct = K;
            break;
        case 'L':
            MorseInstruct = L;
            break;
        case 'M':
            MorseInstruct = M;
            break;
        case 'N':
            MorseInstruct = N;
            break;
        case 'O':
            MorseInstruct = O;
            break;
        case 'P':
            MorseInstruct = P;
            break;
        case 'Q':
            MorseInstruct = Q;
            break;
        case 'R':
            MorseInstruct = R;
            break;
        case 'S':
            MorseInstruct = S;
            break;
        case 'T':
            MorseInstruct = T;
            break;
        case 'U':
            MorseInstruct = U;
            break;
        case 'V':
            MorseInstruct = V;
            break;
        case 'W':
            MorseInstruct = W;
            break;  
        case 'X':
            MorseInstruct = X;
            break;
        case 'Y':
            MorseInstruct = Y;
            break;
        case 'Z':
            MorseInstruct = Z;
            break;

        default:
            break;
        }
        return MorseInstruct;
    }
