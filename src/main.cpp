#include <defs.h>
#include <SPI.h>
#include <MFRC522.h>
#include <TinyStepper.h>
#include <U8g2lib.h>
#include <MorseLetters.h>
#include <EEPROM.h>
#include <Encoder.h>

//#define debugMessages
//#define debugProgress

int gameState = 0;
String scannedCard;
String lastScannedCard;
bool donePlaying = true;
bool skipAllowed = true;
unsigned long floppyReleaseTime = 0;
bool cardDeployed = false;
byte dontSkip[4] = {0x03,0x04,0x05,0x06}; //Array of filenumber which are for floppy disk play
bool startRadioCheck = false;

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
U8G2_ST7920_128X64_1_SW_SPI u8g2(U8G2_R0, /* clock=*/ 13, /* data=*/ 11, /* CS=*/ 10, /* reset=*/ 8);
TinyStepper drawer1(4096, DRAWER_1_IN1, DRAWER_1_IN2, DRAWER_1_IN3, DRAWER_1_IN4);
TinyStepper drawer2(4096, DRAWER_2_IN1, DRAWER_2_IN2, DRAWER_2_IN3, DRAWER_2_IN4);
Morse morse(MORSE_LAMP, MORSE_STRING, MORSE_DOT_LENGTH);
Encoder encoder(ENC_PIN_A, ENC_PIN_B);

void releaseVoiceRecording(byte recordingNum);
void moveDrawer(byte drawerNum, bool open);
void checkForCard();
void checkInstrPanel();
void setupSpritePlayer();
void playSprite(int spriteNum);
int handleEEPROM(bool read, int address, int value = 0);
void handleRadio();
void startInterrupt();

void setup() {
  gameState = handleEEPROM(true, GAME_STATE_ADDR);
	Serial.begin(9600);		// Initialize serial communications with the PC
  setupSpritePlayer();
	SPI.begin();			// Init SPI bus
	mfrc522.PCD_Init();
  drawer1.Enable();
  drawer2.Enable();

  pinMode(INSTR_PANEL_PIN, INPUT);
  pinMode(FLOPPY_1, OUTPUT);
  pinMode(FLOPPY_2, OUTPUT);
  pinMode(FLOPPY_3, OUTPUT);
  pinMode(FLOPPY_4, OUTPUT);
  digitalWrite(FLOPPY_1, LOW);
  digitalWrite(FLOPPY_2, LOW);
  digitalWrite(FLOPPY_3, LOW);
  digitalWrite(FLOPPY_4, LOW);

  while(!bitRead(gameState, INST_PANEL_BIT)){
    checkInstrPanel();
  }
}

void loop() {
  checkForCard();

  if(cardDeployed && millis() - floppyReleaseTime > FLOPPY_TURN_OFF_DELAY){
    cardDeployed = false;
    digitalWrite(FLOPPY_1, LOW);
    digitalWrite(FLOPPY_2, LOW);
    digitalWrite(FLOPPY_3, LOW);
    digitalWrite(FLOPPY_4, LOW);
  }

  if(bitRead(gameState, RADIO_DONE_BIT) && !bitRead(gameState, MORSE_DONE_BIT)){
    morse.sendMorse2Lamp();
  }
  if(bitRead(gameState, MORSE_DONE_BIT)){
    morse.stop();
  }

  if(startRadioCheck){
    handleRadio();
  }
}

String dump_byte_array(byte *buffer, byte bufferSize) {
  String out = "";
  for (byte i = 0; i < bufferSize; i++) {
    out += String(buffer[i], HEX);
  }
  out.toUpperCase();
  return out;
}

String lookForCard(){
  mfrc522.PCD_Init(); //RFID lezer in scan modus zetten
  if(mfrc522.PICC_IsNewCardPresent()) { //Controleren of er een kaart voor de scanner gehouden wordt
    mfrc522.PICC_ReadCardSerial();  //Kaart lezen
    mfrc522.PICC_HaltA(); //Scanner stoppen met lezen, anders wordt dezelfde kaart meerdere keren gelezen
    mfrc522.PCD_StopCrypto1();  //Zelfde als hierboven
    return dump_byte_array(mfrc522.uid.uidByte, mfrc522.uid.size);  //ID van de kaart uit de data halen
  }
  lastScannedCard = "0";
  return "0";
}

void serialEvent1(){
  byte* message;
  while(spritePlayer.available()){
    spritePlayer.readBytes(message, 1);
    for(byte i = 0; i < sizeof(message); i++){
      #ifdef debugSerial
        Serial.print(message[i], HEX);
      #endif
      if(message[i] == DONE_PLAYING_INSTRUCTION){
        donePlaying = true;
        #ifdef debugProgress
          Serial.println("Done playing");
        #endif
      }else{
        skipAllowed = true;
        for(byte j = 0; j < sizeof(dontSkip); j++){
          if(message[i] == dontSkip[j]){
            #ifdef debugMessages
              Serial.println("Skip not allowed");
            #endif
            skipAllowed = false;
          }
        }
      }
    }
  }
}

void releaseVoiceRecording(byte recordingNum){
  switch(recordingNum){
    case 1:
      #ifdef debugProgress
        Serial.println("Release voicerecording 1");
      #endif
      digitalWrite(FLOPPY_1, HIGH);
      cardDeployed = true;
      floppyReleaseTime = millis();
      break;
    case 2:
      #ifdef debugProgress
        Serial.println("Release voicerecording 2");
      #endif
      digitalWrite(FLOPPY_2, HIGH);
      cardDeployed = true;
      floppyReleaseTime = millis();
      break;
    case 3:
      #ifdef debugProgress
        Serial.println("Release voicerecording 3");
      #endif
      digitalWrite(FLOPPY_3, HIGH);
      cardDeployed = true;
      floppyReleaseTime = millis();
      break;
    case 4:
      #ifdef debugProgress
        Serial.println("Release voicerecording 4");
      #endif
      digitalWrite(FLOPPY_4, HIGH);
      cardDeployed = true;
      floppyReleaseTime = millis();
      break;
    default:
      #ifdef debugMessages
        Serial.println("Invalid recording number");
      #endif
      break;
  }
}

void moveDrawer(byte drawerNum, bool open){
  if(open){
    switch(drawerNum){
      case 1:
        drawer1.AccelMove(DRAWER_MOVE_DEG);
        break;
      case 2:
        drawer2.AccelMove(DRAWER_MOVE_DEG);
        break;
      default:
        #ifdef debugMessages
          Serial.println("Invalid drawer number");
        #endif
        break;
    }
  }else{
    switch(drawerNum){
      case 1:
        drawer1.AccelMove(-DRAWER_MOVE_DEG);
        break;
      case 2:
        drawer2.AccelMove(-DRAWER_MOVE_DEG);
        break;
      default:
        #ifdef debugMessages
          Serial.println("Invalid drawer number");
        #endif
        break;
    }  
  }
}

void checkForCard(){
  scannedCard = lookForCard();
  if(scannedCard != "0" && scannedCard != lastScannedCard){
    if(scannedCard == TAG1){
      playSprite(1);
    }else if(scannedCard == TAG2){
      playSprite(2);
    }else if(scannedCard == TAG3){
      playSprite(3);
    }else if(scannedCard == TAG4){
      playSprite(4);
    }else{
      #ifdef debugMessages
        Serial.println("Invalid card scanned");
      #endif
    }
    lastScannedCard = scannedCard;
  }
}

void checkInstrPanel(){
  if(digitalRead(INSTR_PANEL_PIN) == HIGH){
    bitWrite(gameState, INST_PANEL_BIT, 1);
    handleEEPROM(false, GAME_STATE_ADDR, gameState);
    moveDrawer(1, true);
  }
}

void setupSpritePlayer(){
  spritePlayer.begin(9600); //Set Baudrate to standar Sprite Baudrate
  spritePlayer.write(0xDA); //Set video output to 1080p_60
  spritePlayer.write(0xEC); //Set HDMI audio
}

void playSprite(int spriteNumber){
  if(skipAllowed || donePlaying){
    spritePlayer.write(spriteNumber);
  }
}

int handleEEPROM(bool read, int address, int value = 0){
  if(read){
    return (EEPROM.read(address) << 8) + EEPROM.read(address + 1);
  }else{
    EEPROM.write(address, value >> 8);
    EEPROM.write(address, value & 0xFF);
    return -1;
  }
}

void handleRadio(){
  static int32_t oldPos = -999;
  static float frequency = 800.0;
  int32_t newPos = encoder.read();

  if(newPos != oldPos){
    frequency += 0.1;
    oldPos = newPos;
  }

  if(frequency == CORRECT_FREQUENCY && TIMSK4 != 0x02){   //Ingevoerde frequentie is goed en de interrupt is nog niet geactiveerd
    startInterrupt();
  }else{
    TIMSK4 = 0; //Interrupt timer stoppen
  }
}

void startInterrupt(){
  //Interrupts uitschakelen
  cli();
  //Arduino registers instellen om interrupts te kunnen doen
  TCCR4A = 0;
  TCCR4B = 0;
  TCNT4 = 0;
  TCCR4B |= (1 << WGM12);
  TCCR4B |= (1 << CS12) | (1 << CS10);
  TIMSK4 |= (1 << OCIE4A);

  //Interrupt frequentie instellen op 2 seconden (kan worden aangepast door DESIRED_INTERRUPT_FREQ_S aan te passen)
  OCR4A = CLOCK_FREQUENCY / (DESIRED_INTERRUPT_FREQ_S * INTERRUPT_PRESCALER) -1;
  //Interrupts starten
  sei();
}

ISR(TIMER4_COMPA_vect){
  playSprite(RADIO_MESSAGE);
  startRadioCheck = false;
}