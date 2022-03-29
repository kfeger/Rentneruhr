/*
   Position anfahren
*/

void moveStepper(int Position) {
  OldPosition = stepper.currentPosition();
  stepper.moveTo(Position);
  while (stepper.run()) {
    yield();
  }
  CurrentPosition = stepper.currentPosition();
}

void moveStepperWithOff(int Position) {
  #ifdef STEPPER_OFF
    stepper.enableOutputs();
    delay(100);
  #endif
  OldPosition = stepper.currentPosition();
  stepper.moveTo(Position);
  while (stepper.run()) {
    yield();
  }
  CurrentPosition = stepper.currentPosition();
  #ifdef STEPPER_OFF
    stepper.disableOutputs();
  #endif
}

/*
   Null-Kalibrierung
*/
void homeStepper(void) {
  HomeRun = true;
  Serial.println("Skala kalibrieren");
  stepper.setCurrentPosition(0);
  //Serial.println("Stepper geht auf 0");
  moveStepper(2048);
  //Serial.println("An Anschlag gefahren");
  moveStepper(0);
  moveStepper(OFFSET_STEPPER);
  stepper.setCurrentPosition(MIN_POS);
  //stepper.moveTo(MAX_POS);
  Serial.println("Stepper auf Soft-Null");
  HomeRun = false;
}

void HalfPos(void) {
  moveStepper(2048);
  moveStepper(HALF_POS);
}

void CalcNewPosition(void) {
  int NextPosition = 0;
  int HourPos = 0;
  DayIndex = tm.tm_wday;  // Achtung: 0 = Sonntag lt. time.h
  // auf Skale aber 6 = Sonntag!
  if (DayIndex != 0)      // Index auf Skala umrechnen
    DayIndex -= 1;        // Mo = 0
  else
    DayIndex = 6;         // So = 6

  if(tm.tm_hour == 0)
    NextPosition = Day0Position[DayIndex];
  else
    NextPosition = Day0Position[DayIndex] +  (tm.tm_hour * StepWidth[DayIndex]);
    
  if (CurrentPosition != NextPosition) {
    Serial.printf("Wochentag: %d(%s), Stunde: %d, Schritte pro Stunde: %d\nSchritte bis %s 0h: %d, Schritte ab %s 0h: %d\n", tm.tm_wday, \
                  Wochentag[tm.tm_wday], tm.tm_hour, STEPS_1H, Wochentag[tm.tm_wday],Day0Position[DayIndex], Wochentag[tm.tm_wday], tm.tm_hour * STEPS_1H);
    moveStepperWithOff(NextPosition);
  }
}

// Einmal rauf, einmal runter...
void DayStepDemo(void) {
  Serial.println("Rauf...");
  for (int i = 0; i < 8; i++) {
    moveStepper(Day0Position[i]);
    Serial.println(CurrentPosition);
    server.handleClient();
    delay(2000);
  }
  Serial.println("Runter...");
  moveStepper(0);
  /*for (int i = 6; 0>= 0; i--) {
    moveStepper(Day0Position[i]);
    Serial.println(CurrentPosition);
    server.handleClient();
    delay(2000);    
  }*/
}
