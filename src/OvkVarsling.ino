/*
 * Projekt: Overkørsel st. enkeltsporet strækning
 * Produkt: Varslingsanlæg
 * Version: 1.0
 * Type: Program
 * Programmeret af: Jan Birch
 * Opdateret: 18-05-2021
 * GNU General Public License version 3
 * This file is part of "Varslingsanlæg faste tider".
 * 
 * "Varslingsanlæg" is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * "Varslingsanlæg" is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with "Varslingsanlæg".  If not, see <https://www.gnu.org/licenses/>.
 *  
 * Noter: 
 * Se koncept og specifikation for en detaljeret beskrivelse af programmet, formål og anvendelse.
 */

// Tidsenhed til urværk
enum {MSEC, SECONDS};
// Kontakttyper for trykknap
enum {NOPEN, NCLOSED};
// Overkørslens betjeningsenheder magasin kan konfigureres
enum {BISTABLE, ONESHOT};
// Overkørslens betjeningsenheder leverer
enum {OFF, ON};
// Overkørslens ydre enheder kan blive sat til
enum {BLOCK, PASS};

// Betjenings- og sensorenheder
const byte MaxNoCtrls = 1;
enum {MANUELBETJ};
// Overkørslens ydre enheder
const byte MaxNoDevices = 4;
enum {BANESIGNAB, BANESIGNBA, VEJSIGNAL, VEJKLOKKE};
// Overkørslens tilstande
const byte MaxNoStates = 5;
enum {IKKESIKRET, FORRING, SIKRET, OPLOES, BILISTTID};

// Overkørslens moduler
#include <Ovkoersel.h>

// Arduino pins
struct {
  const byte ManuelKnap = 2;
  const byte OUSignAB = 7;
  const byte OUSignBA = 8;
  const byte VejKlokker = 10;
  const byte VejLys = 11;
} ARDPin;

// Hardware drivere til den overkørsel, som dette program leverer
t_PushButton manuelKnapDrv(ARDPin.ManuelKnap, NCLOSED); // Knappen er forbundet til 5V og normally closed
t_SimpleOnOff OUSignABDrv(ARDPin.OUSignAB, HIGH);
t_SimpleOnOff OUSignBADrv(ARDPin.OUSignBA, HIGH);
t_SimpleOnOff vejKlokkerDrv(ARDPin.VejKlokker);
t_SimpleOnOff vejLysDrv(ARDPin.VejLys);

// Overkørslens betjenings- og ydre enheder
t_CrossingCtrl manuelBetj;
t_RailSignal OUSignAB;
t_RailSignal OUSignBA;
t_RoadSignal vejKlokker(PASS);
t_RoadSignal vejLys(PASS);
// Flipflop til manuelknap
t_FlipFlop manFF(NCLOSED);

// Tilstandsmaskine
class t_IkkesikretState: public t_StateMachine {
public:
  t_IkkesikretState(void) : t_StateMachine() {}
  byte doCondition(byte currentStateNo) {
    byte nextState = currentStateNo;
    if (crossing.status(MANUELBETJ) == ON) nextState = FORRING;
    return nextState;
  }
  void onExit(void) {
    crossing.reset(MANUELBETJ);
  }
} ikkesikret;

class t_ForringState: public t_StateMachine {
public:
  t_ForringState(void) : t_StateMachine() {}
  void onEntry(void) {
    const unsigned long defer = 2;
    crossing.to(VEJSIGNAL, BLOCK);
    crossing.to(VEJKLOKKE, BLOCK);
    clockWork.setDuration(defer, SECONDS);
  }
  byte doCondition(byte currentStateNo) {
    byte nextState = currentStateNo;
    if (clockWork.triggered() == true) nextState = SIKRET;
    return nextState;
  }
} forring;

class t_SikretState: public t_StateMachine {
public:
  t_SikretState(void) : t_StateMachine() {}
  void onEntry(void) {
    const unsigned long defer = 15;
    crossing.to(BANESIGNAB, PASS);
    crossing.to(BANESIGNBA, PASS);
    clockWork.setDuration(defer, SECONDS);
  }
  byte doCondition(byte currentStateNo) {
    byte nextState = currentStateNo;
    if (clockWork.triggered() == true) nextState = OPLOES;
    return nextState;
  }
  void onExit(void) {
    crossing.to(BANESIGNAB, BLOCK);
    crossing.to(BANESIGNBA, BLOCK);
  }
} sikret; 

class t_OploesState: public t_StateMachine {
public:
  t_OploesState(void) : t_StateMachine() {}
  void onEntry(void) {
    const unsigned long defer = 15;
    clockWork.setDuration(defer, SECONDS);
  }
  byte doCondition(byte currentStateNo) {
    byte nextState = currentStateNo;
    if (clockWork.triggered() == true) nextState = BILISTTID;
    return nextState;
  }
  void onExit(void) {
    crossing.to(VEJSIGNAL, PASS);
    crossing.to(VEJKLOKKE, PASS);
  }
} oploes;

class t_BillisttidState: public t_StateMachine {
public:
  t_BillisttidState(void) : t_StateMachine() {}
  void onEntry(void) {
    const unsigned long defer = 10;
    clockWork.setDuration(defer, SECONDS);
  }
  byte doCondition(byte currentStateNo) {
    byte nextState = currentStateNo;
    if (clockWork.triggered() == true) nextState = IKKESIKRET;
    return nextState;
  }
} billisttid;

void setup() {
  // put your setup code here, to run once:
// Drivere kobles til betjenings- og ydre enheder
  manuelBetj.setDriver(&manuelKnapDrv);
  OUSignAB.setDriver(&OUSignABDrv);
  OUSignBA.setDriver(&OUSignBADrv);
  vejKlokker.setDriver(&vejKlokkerDrv);
  vejLys.setDriver(&vejLysDrv);
// Konfiguration af manuelbetjening
  manuelBetj.setFlipFlop(&manFF);
// Opsætning af overkørsel
  collection.initialize();
  crossing.setCtrl(MANUELBETJ, &manuelBetj);
  crossing.setDevice(VEJSIGNAL, &vejLys);
  crossing.setDevice(VEJKLOKKE, &vejKlokker);
  crossing.setDevice(BANESIGNAB, &OUSignAB);
  crossing.setDevice(BANESIGNBA, &OUSignBA);
// Opsætning af tilstandsmaskine
  crossing.setState(IKKESIKRET, &ikkesikret);
  crossing.setState(FORRING, &forring);
  crossing.setState(SIKRET, &sikret);
  crossing.setState(OPLOES, &oploes);
  crossing.setState(BILISTTID, &billisttid);
// Start tilstandsmaskine
  crossing.initState(IKKESIKRET);
}

void loop() {
  // put your main code here, to run repeatedly:
  Clock::pendulum();
  Blinker::doClockCycle();
  crossing.doClockCycle();
}
