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
  //delay(1000);
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

void CalcNewPosition(void) {
  int NextPosition = 0;
  int HourPos = 0;
  int DayIndex = 0;
  DayIndex = tm.tm_wday;  // Achtung: 0 = Sonntag lt. time.h
  // auf Skale aber 6 = Sonntag!
  if (DayIndex != 0)      // Index auf Skala umrechnen
    DayIndex -= 1;        // Mo = 0
  else
    DayIndex = 6;         // So = 6

  NextPosition = Day0Position[DayIndex] +  ((tm.tm_hour / 3) * STEPS_3H);
  if (CurrentPosition != NextPosition) {
    Serial.printf("Wochentag: %d, Stunde: %d, Stunde mod 3: %d\nSchritte bis heute 0: %d, Schritte ab heute 0: %d\n", tm.tm_wday, \
                  tm.tm_hour, tm.tm_hour / 3, Day0Position[DayIndex], ((tm.tm_hour / 3) * STEPS_3H));
    moveStepper(NextPosition);
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
