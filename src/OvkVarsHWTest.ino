/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Varslingsanlæg faste tider
 * Version: 0.1
 * Type: Program
 * Programmeret af: Jan Birch
 * Opdateret: 08-04-2021
 * Noter: Programmet bliver brugt til at teste at hardware er monteret korrekt
 * Se koncept og specifikation for en detaljeret beskrivelse af programmet, formål og anvendelse.
 */

// Opsætning af konstanter og specifikationer

// Arduino pins
struct {
  const int ManuelKnap = 2;
  const int OUSignAB = 7;
  const int OUSignBA = 8;
  const int VejKlokker = 10;
  const int VejLys = 11;
} ardPin;

// Manuel knap
  int manButtonValue = 0;
  boolean wasHigh = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(ardPin.ManuelKnap, INPUT_PULLUP);
  pinMode(ardPin.OUSignAB, OUTPUT);
  pinMode(ardPin.OUSignBA, OUTPUT);
  pinMode(ardPin.VejKlokker, OUTPUT);
  pinMode(ardPin.VejLys, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  manButtonValue = digitalRead(ardPin.ManuelKnap);
  if (manButtonValue == LOW) {
    digitalWrite(ardPin.OUSignAB, HIGH);
    digitalWrite(ardPin.OUSignBA, HIGH);
    digitalWrite(ardPin.VejKlokker, HIGH);
    digitalWrite(ardPin.VejLys, HIGH);
    wasHigh = true;
  }
  delay(1000);
  if (wasHigh == true) {
    digitalWrite(ardPin.OUSignAB, LOW);
    digitalWrite(ardPin.OUSignBA, LOW);
    digitalWrite(ardPin.VejKlokker, LOW);
    digitalWrite(ardPin.VejLys, LOW);
    wasHigh = false;   
  }
}
