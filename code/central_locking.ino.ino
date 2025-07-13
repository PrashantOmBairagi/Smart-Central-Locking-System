#include <RCSwitch.h>
#include <avr/power.h>
#include <avr/sleep.h>

#include <avr/io.h>

RCSwitch rf = RCSwitch();

// Pin Definitions
#define ACT_IN1 7
#define ACT_IN2 8
#define HAZARD_PIN 9
#define BUZZER_PIN 6
#define DOOR_SENSOR 12
#define RF_DATA_PIN 2
#define RF_POWER_PIN 5
#define ACT_POSITION 4

// Remote Button Codes
#define BUTTON_A 9248865
#define BUTTON_B 9248866
#define BUTTON_C 9248868
#define BUTTON_D 9248872
#define BUTTON_P 6989713
#define BUTTON_Q 6989714
#define BUTTON_R 6989716
#define BUTTON_S 6989720

// Timing Constants (ms)
#define SIGNAL_DEBOUNCE 600
#define NOISE_THRESHOLD 10
#define AUTO_RELOCK_TIME 60000
#define FEEDBACK_DURATION 650
#define ACTUATOR_DELAY 300
#define RF_DUTY_ON 500
#define RF_DUTY_OFF 500
#define ACTIVE_MODE_DURATION 60000

// System States
bool doorsUnlocked = false;
bool newPressExpected = true;
bool activeMode = false;
unsigned long unlockTime = 0;
unsigned long lastRfSignalTime = 0;
unsigned long activeModeStart = 0;
unsigned long lastRfCycle = 0;
bool rfPowerState = false;

void setup() {
  // clock_prescale_set(clock_div_8);
  //Serial.begin(9600);
  //Serial.println("\nBAIRAGI's Central Locking System");
  //Serial.println("Ultra Low Power // Encrypted // AutoRelock");
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);

  pinMode(ACT_IN1, OUTPUT);
  pinMode(ACT_IN2, OUTPUT);
  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, LOW);

  pinMode(HAZARD_PIN, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(HAZARD_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);

  pinMode(DOOR_SENSOR, INPUT_PULLUP);
  pinMode(RF_DATA_PIN, INPUT);
  pinMode(RF_POWER_PIN, OUTPUT);
  digitalWrite(RF_POWER_PIN, LOW);

  power_adc_disable();
  power_spi_disable();
  power_twi_disable();

  byte usedPins[] = {ACT_IN1, ACT_IN2, HAZARD_PIN, BUZZER_PIN, 
                    DOOR_SENSOR, RF_DATA_PIN, RF_POWER_PIN,13};
  for (byte i = 0; i < 20; i++) {
    bool isUsed = false;
    for (byte j = 0; j < sizeof(usedPins)/sizeof(usedPins[0]); j++) {
      if (i == usedPins[j]) {
        isUsed = true;
        break;
      }
    }
    if (!isUsed) {
      pinMode(i, INPUT_PULLUP);
    }
  }

  //Serial.println("System Ready (Ultra Low Power Mode). Waiting for commands...");
  lastRfCycle = millis();
}

void loop() {
 
  manageRfPower();

  if (rfPowerState) {
    if (rf.available()) {
      processRF();
      lastRfSignalTime = millis();

      if (!activeMode) {
        activeMode = true;
        activeModeStart = millis();
        //Serial.println("Entering active mode (100% RF power)");
      }
    }
    else if (millis() - lastRfSignalTime > 50) {
      newPressExpected = true;
    }
  }

  if (activeMode && (millis() - activeModeStart > ACTIVE_MODE_DURATION)) {
    activeMode = false;
    //Serial.println("Returning to 50% RF duty cycle");
  }
 if (digitalRead(ACT_POSITION)==LOW){
   doorsUnlocked = true;
 }
  //checkAutoRelock();
  lowPowerDelay(10);
}

void manageRfPower() {
  unsigned long currentMillis = millis();

  if (activeMode) {
    if (!rfPowerState) {
      digitalWrite(RF_POWER_PIN, HIGH);
      rfPowerState = true;
      rf.enableReceive(digitalPinToInterrupt(RF_DATA_PIN));
    }
  } else {
    if (rfPowerState) {
      if (currentMillis - lastRfCycle >= RF_DUTY_ON) {
        digitalWrite(RF_POWER_PIN, LOW);
        rfPowerState = false;
        rf.disableReceive();
        lastRfCycle = currentMillis;
      }
    } else {
      if (currentMillis - lastRfCycle >= RF_DUTY_OFF) {
        digitalWrite(RF_POWER_PIN, HIGH);
        rfPowerState = true;
        rf.enableReceive(digitalPinToInterrupt(RF_DATA_PIN));
        lastRfCycle = currentMillis;
        newPressExpected = true;
      }
    }
  }
}

void lowPowerDelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    asm volatile("nop");
  }
}

void processRF() {
  digitalWrite(13, HIGH); delay(100); digitalWrite(13, LOW);
  long code = rf.getReceivedValue();
  unsigned int length = rf.getReceivedBitlength();
  unsigned int protocol = rf.getReceivedProtocol();

  if (length < NOISE_THRESHOLD || protocol == 0) {
    rf.resetAvailable();
    return;
  }

  if (newPressExpected) {
    // Serial.print("\nRF Code: ");
    // Serial.print(code);
    // Serial.print(" (Proto: ");
    // Serial.print(protocol);
    // Serial.print(", Bits: ");
    // Serial.print(length);
    // Serial.println(")");

    addToSequence(code);
    newPressExpected = false;
  }

  rf.resetAvailable();
}

void addToSequence(long code) {
  static String sequence = "";
  static unsigned long lastSequenceTime = 0;
  unsigned long now = millis();

  if (now - lastSequenceTime > SIGNAL_DEBOUNCE) {
    sequence = "";
    //Serial.println("Sequence timeout");
  }

  if (sequence.length() > 6) {
    sequence = "";
    //Serial.println("Sequence cleared (max length)");
  }

  if (code == BUTTON_A || code == BUTTON_P) sequence += "A";
  else if (code == BUTTON_B || code == BUTTON_Q) sequence += "B";
  else if (code == BUTTON_C || code == BUTTON_R) sequence += "C";
  else if (code == BUTTON_D || code == BUTTON_S) sequence += "D";

  lastSequenceTime = now;
  // Serial.print("Sequence: ");
  // Serial.println(sequence);

  if (sequence == "A") {
    unlockDoors();
    sequence = "";
  } else if (sequence == "CB") {
    lockDoors();
    sequence = "";
  } else if (sequence == "D") {
    activateFeedback();
    sequence = "";
  } else if (sequence == "CCCC") {
    //Serial.println("Special function activated");
    sequence = "";
     digitalWrite(RF_POWER_PIN, LOW);
     power_timer0_disable();
  }
}

void unlockDoors() {
  //Serial.println("Unlocking doors");

  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, LOW);
  lowPowerDelay(50);

  digitalWrite(ACT_IN1, HIGH);
  digitalWrite(ACT_IN2, LOW);
  lowPowerDelay(ACTUATOR_DELAY);

  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, LOW);

  activateFeedback();
  doorsUnlocked = true;
  unlockTime = millis();
}

void lockDoors() {
  //Serial.println("Locking doors");

  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, LOW);
  lowPowerDelay(50);

  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, HIGH);
  lowPowerDelay(ACTUATOR_DELAY - 100);

  digitalWrite(ACT_IN1, LOW);
  digitalWrite(ACT_IN2, LOW);

  activateFeedback();
  doorsUnlocked = false;
  unlockTime = millis();
}

// void checkAutoRelock() {
//   if (doorsUnlocked && (millis() - unlockTime > AUTO_RELOCK_TIME)) {
//    // Serial.println("Auto-relocking");
//     lockDoors();
//   }
// }

void activateFeedback() {
  //Serial.println("Activating feedback");
  //digitalWrite(HAZARD_PIN, HIGH);
  digitalWrite(BUZZER_PIN, HIGH);
  lowPowerDelay(FEEDBACK_DURATION);
  digitalWrite(HAZARD_PIN, LOW);
  lowPowerDelay(500);
  digitalWrite(BUZZER_PIN, HIGH);
  lowPowerDelay(FEEDBACK_DURATION);
  //digitalWrite(HAZARD_PIN, LOW);
  digitalWrite(BUZZER_PIN, LOW);
}