/*
  ARGUMENT BOX

  A box to allow fair arguments by monitoring participants word count.
  Participants choose a number of words for the argument. They speak into funnels and monitor
  their word count on an individual lcd screen. They are penalised for shouting and notified
  when their words are finished. They can emergency exit the argument by mutual agreement at any time.

  Pin Usage

  Digital:
  2  Toggle switch to choose between start arguing and prepare to argue
  3  Shouting penalty red flashing led player 1
  4  Shouting penalty red flashing led player 2
  5  Silence blue led player 1
  6  Silence blue led player 2
  7  lcd pin d7
  8  lcd pin d6
  9  lcd pin d5
  10 lcd pin d4
  11 lcd enable pin player 1
  12 lcd rs pin
  13 lcd enable pin player 2

  Analog

  0 Potentiometor to choose the word count
  1 Microphone for player 1
  2 Microphone for player 2
  3 Emergency exit push button player 1 ** used as digital pin 17
  4 Emergency exit push button player 2 ** used as digital pin 18


  The circuit and layout found here:
  https://github.com/jessewolpert/ArgumentBox

  created 2017
  by Jesse Wolpert

*/

// include the lcd library code:
#include <LiquidCrystal.h>

// set up the lcd pins
const int rs = 12, en1 = 11, en2 = 13, d4 = 10, d5 = 9, d6 = 8, d7 = 7;
LiquidCrystal lcd1(rs, en1, d4, d5, d6, d7);
LiquidCrystal lcd2(rs, en2, d4, d5, d6, d7);

//set up the variables
int wordsAllowed = 0;
int player1WordsSpoken = 0;
int player2WordsSpoken = 0;
bool playing = false;
bool exit1Pressed = false;
bool exit2Pressed = false;
bool player1WordStarted = false;
bool player2WordStarted = false;
bool player1Finished = false;
bool player2Finished = false;
int player1WordsRemaining = 0;
int player2WordsRemaining = 0;
int count1 = 0;
int count2 = 0;
bool finished = false;
bool emergencyFinished = false;
long player1ShoutingTimer = 0;
long player2ShoutingTimer = 0;
bool player1Shouted = false;
bool player2Shouted = false;
int flashingTime = 3000; //time the penalty light flashes for
int penaltyFreeTime = 4000; //can't get another penalty for this time
int shoutingValue = 400; //threshold for shouting penalty
int player1Penalty = 10; //amount of words removed for penalty
int player2Penalty = 10;
int soundThreshold = 0; // the background sound
int soundSpeed = 10; // register word after this many instances
long setupTime = 0;
int sample = 0;
int sampleCount = 0;
bool firstArgument = true;
int factorToAdd = 5; // how much to add to soundThreshold to register a word
int calibrationTime = 5000; //how long to calibrate for

// set up pins variables
int startSwitch =  2;
int penalty1Led = 3;
int penalty2Led = 4;
int silence1Led = 5;
int silence2Led = 6;
int emergencyExit1 = 17;
int emergencyExit2 = 18;
int wordCounter = 0;
int mic1 = 1;
int mic2 = 2;

void setup() {

  Serial.begin(9600);
  //set up the pins
  pinMode(startSwitch, INPUT); //sstart button
  pinMode(emergencyExit1, INPUT); // exit button player1
  pinMode(emergencyExit2, INPUT); // exit button player2
  pinMode(penalty1Led, OUTPUT); // Player 1 shouting light
  pinMode(penalty2Led, OUTPUT); // Player 2 shouting light
  pinMode(silence1Led, OUTPUT); // Player 1 light - end of game
  pinMode(silence2Led, OUTPUT); // Player 2 light - end of game

  // set up the LCD's number of columns and rows:
  lcd1.begin(16, 2);
  lcd2.begin(16, 2);
  setupTime = millis();

  //call the reset function
  reset();

  // collect data for calibration time to calibrate for background sound
  calibrating();
  while (millis() - setupTime < calibrationTime)
  {
    sample = sample + analogRead(mic1);
    sample = sample + analogRead(mic2);
    sampleCount = sampleCount + 2 ;
    Serial.print("  anlogRead1    ");
    Serial.print(analogRead(mic1));
    Serial.print("  anlogRead2    ");
    Serial.print(analogRead(mic2));
    Serial.print("  sample    ");
    Serial.print(sample);
    Serial.print("  sampleCount    ");
    Serial.println(sampleCount);
  }

  soundThreshold = (sample / sampleCount) + factorToAdd;
  Serial.print("  soundThreshold    ");
  Serial.print(soundThreshold);

}

void loop() {
  //when start switch is off map the word number for the potentiometer dial
  if (digitalRead(startSwitch) == LOW)
  { wordsAllowed = map(analogRead(wordCounter), 10, 1023, 1000, 0);
    reset();
    lcdNotPlaying();
    finished = false;
  }
  // when the start switch is on start the playing
  if (digitalRead(startSwitch) == HIGH)
  {
    playing = true;
    //check if emergency exit has been called
    if (digitalRead(emergencyExit1) == 1)
    {
      exit1Pressed = true;
    }
    if (digitalRead(emergencyExit2) == 1)
    {
      exit2Pressed = true;
    }
    //choose what to display on lcds
    if (emergencyFinished)
    {
      lcdEmergencyFinishedPlaying();
    }
    else if (finished)
    {
      lcdFinishedPlaying();
    }
    else if (!finished)
    {
      lcdIsPlaying();
    }

  }

  if (playing) //start of an argument
  {
    if ((exit1Pressed) && (exit2Pressed))

    {
      emergencyFinished = true;
    }

    if ((millis() - player1ShoutingTimer) > flashingTime) //amount of time shouting light flashes for
    { digitalWrite(penalty1Led, LOW);
      player1Shouted = false;
    }

    if ((millis() - player2ShoutingTimer) > flashingTime) //amount of time shouting light flashes for
    { digitalWrite(penalty2Led, LOW);
      player2Shouted = false;
    }

    if (analogRead(mic1) > soundThreshold) //read the words for player 1
    {
      player1WordStarted = true;
      count1 ++;
    }
    //update words remaining
    if ((player1WordStarted == true) && (player1Finished == false) && (count1 >= soundSpeed))
    { player1WordsSpoken = player1WordsSpoken + 1;
      player1WordsRemaining = wordsAllowed - player1WordsSpoken ;
      count1 = 0;
    }

    if (analogRead(mic2) > soundThreshold) //read the words for player 1
    {
      player2WordStarted = true;
      count2 ++;
    }

    if ((player2WordStarted == true) && (player2Finished == false) && (count2 >= soundSpeed))
    { player2WordsSpoken = player2WordsSpoken + 1;
      player2WordsRemaining = wordsAllowed - player2WordsSpoken ;
      count2 = 0;

    }
    //can't get a second shouting penalty for this amount of seconds
    if ((analogRead(mic1) > shoutingValue) && (player1Shouted == false) &&
        ((millis() - player1ShoutingTimer) >= penaltyFreeTime))// player one shouting
    { player1Shouted = true;
      digitalWrite(penalty1Led, HIGH);
      player1ShoutingTimer = millis();
      player1WordsSpoken = player1WordsSpoken + player1Penalty;
    }

    if ((analogRead(mic2) > shoutingValue) && (player2Shouted == false) &&
        ((millis() - player2ShoutingTimer) >= penaltyFreeTime)) // player two shouting
    { player2Shouted = true;
      digitalWrite(penalty2Led, HIGH);
      player2ShoutingTimer = millis();
      player2WordsSpoken = player2WordsSpoken + player2Penalty;
    }

    // player 1 finished words
    if ( player1WordsRemaining <= 0)
    { digitalWrite(silence1Led, HIGH);
      player1Finished = true;
      player1WordsRemaining = 0;
    }
    // player 2 finished words
    if ( player2WordsRemaining <= 0)
    { digitalWrite(silence2Led, HIGH);
      player2Finished = true;
      player2WordsRemaining = 0;
    }

    //stop game
    if  ((player1Finished == true) && (player2Finished == true) && (playing == true))
    {

      finished = true;
    }
  }//end of if playing

}

void reset() {
  playing = false;
  player2WordsSpoken = 0;
  player1WordsSpoken = 0;
  digitalWrite(silence1Led, LOW);
  digitalWrite(silence2Led, LOW);
  digitalWrite(penalty1Led, LOW);
  digitalWrite(penalty2Led, LOW);
  player2Finished = false;
  player1Finished = false;
  player2WordsRemaining = wordsAllowed;
  player1WordsRemaining = wordsAllowed;
  count1 = 0;
  count2 = 0;
  exit1Pressed = false;
  exit2Pressed = false;
  emergencyFinished = false;
  player1Shouted = false;
  player2Shouted = false;
}

void lcdIsPlaying() {
  lcd1.setCursor(0, 0);
  lcd2.setCursor(0, 0);
  lcd1.print("Your Words      ");
  lcd2.print("Your Words      ");
  lcd1.setCursor(0, 1);
  lcd2.setCursor(0, 1);
  lcd1.print("Remaining ");
  lcd2.print("Remaining ");
  lcd1.setCursor(11, 1);
  lcd2.setCursor(11, 1);
  lcd1.print(player1WordsRemaining);
  lcd1.print("    ");
  lcd2.print(player2WordsRemaining);
  lcd2.print("    ");
}

void lcdNotPlaying()
{ lcd1.setCursor(0, 0);
  lcd2.setCursor(0, 0 );
  lcd1.print("Choose the Limit");
  lcd2.print("Choose the Limit");
  lcd1.setCursor(0, 1);
  lcd2.setCursor(0, 1);
  lcd1.print(wordsAllowed);
  lcd1.print("                 ");
  lcd2.print(wordsAllowed);
  lcd2.print("                 ");
}

void lcdFinishedPlaying()
{ lcd1.setCursor(0, 0);
  lcd2.setCursor(0, 0 );
  lcd1.print("Argument Over   ");
  lcd2.print("Argument Over   ");
  lcd1.setCursor(0, 1);
  lcd2.setCursor(0, 1);
  lcd1.print("Argue Again?    ");
  lcd2.print("Argue Again?    ");
}

void lcdEmergencyFinishedPlaying()
{ lcd1.setCursor(0, 0);
  lcd2.setCursor(0, 0 );
  lcd1.print("Emergency Exit  ");
  lcd2.print("Emergency Exit  ");
  lcd1.setCursor(0, 1);
  lcd2.setCursor(0, 1);
  lcd1.print("Argue Again?    ");
  lcd2.print("Argue Again?    ");
}

void calibrating()
{ lcd1.setCursor(0, 0);
  lcd2.setCursor(0, 0 );
  lcd1.print("Calibrating     ");
  lcd2.print("Calibrating     ");
  lcd1.setCursor(0, 1);
  lcd2.setCursor(0, 1);
  lcd1.print("Please Wait     ");
  lcd2.print("Please Wait     ");
}
