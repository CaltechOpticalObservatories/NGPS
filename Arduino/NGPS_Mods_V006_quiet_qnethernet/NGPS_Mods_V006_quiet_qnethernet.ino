#include <QNEthernet.h>
using namespace qindesign::network;
#include <elapsedMillis.h>


//include the ability to soft restart the teensy
#define CPU_RESTART_ADDR (uint32_t*)0XE000ED0C
#define CPU_RESTART_VAL 0x5FA0004
#define CPU_RESTART (*CPU_RESTART_ADDR = CPU_RESTART_VAL);

//Default values
#define lambda 25000           // time for a open/close command to last (time OE pin stays on)(us)
#define defaultDC .5           // Duty cycle (.5= 50%)
#define defaultPeriod 1000000  // time of pulse length to mods(ms)
/*=============================================================================================
  /    HANDLE MEMORY
  /=============================================================================================*/
const byte numChars = 32;  // number of characters in the buffer to read

char receivedChars[numChars];
char tempChars[numChars];
char messageFromPC[numChars];

const int nBoard = 8;     // number of modulators attached to arduino
int onArray[nBoard];      // for each modulator, 0 for off, 1 for on.  all 0 on start
int pinNumbers[nBoard];   // Array of pin numbers for "in" signal to pin drivers
int oeNumbers[nBoard];    // Array of pin numbers for "oe" input to pin drivers
int toggleArray[nBoard];  // Array tracking toggle status of each modulator

int commaIter = 0;  // Count the number of commas in the input
int commaNum;       // Used in parseData() to make sure the appropriate number of inputs is present

elapsedMicros modTimer[nBoard];  // Array of timers that count events for each mod
float modPeriods[nBoard];        // Array of periods for each mod: all start at 1s by default.. [ms]
elapsedMicros pulseTimer[nBoard];

elapsedMicros toggleTimer[nBoard];  //Array of timers that count to lambda for toggling state single time

float modDC[nBoard];   // Array of duty cycles for each mod
float modNus[nBoard];  // Array of time for each mod between open and close.......... [ms]

//                                    /*
//==========================================================================================
// For Teensy 4.1:
int allPinNumbers[16] = { 3, 4, 6, 7, 8, 9, 10, 11, 12, 24, 25, 28, 29, 37, 36, 33 };
int allOENumbers[16] = { 22, 21, 19, 18, 17, 16, 15, 14, 13, 41, 40, 39, 38, 35, 34, 32 };
//==========================================================================================
//                                     */
/*
//==========================================================================================
// For Teensy 3.2:
int allPinNumbers[10] = {3, 4, 5, 6, 9, 10, 23, 22, 21, 20};
int allOENumbers[10] = {19, 18, 17, 16, 15, 14, 13, 7, 8, 11};
//==========================================================================================
*/



/*=============================================================================================
  /    Ethernet Address Configuration
  /=============================================================================================*/
// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

IPAddress ip(10, 0, 0, 36);          //Instrument
IPAddress gateway(10, 0, 0, 1);      //instrument
IPAddress subnet(255, 255, 255, 0);  //instrument

// Create a server listening on the given port.
EthernetServer server{ 23 };


/*=============================================================================================
  /     Function declarations
  /=============================================================================================*/
String readRequest(EthernetClient* client);


/*=============================================================================================
  /    ONBOARD MEMORY
  /=============================================================================================*/
int errorcode = 0;


/*=============================================================================================
  /   setup
  /=============================================================================================*/
void setup() {
  //Serial.println("Entering setup");
  Ethernet.begin(mac, ip);  //, gateway, subnet);
  //Ethernet.begin(mac, ip);
  Serial.begin(9600);  // opens serial port, sets data rate to 9600 bps
  //server.println("Loading Server");
  server.begin();
  map_pins();
  array_init();
  //printServerStatus();
  all_off();
}

/*=============================================================================================
  /     printServerStatus()
  /=============================================================================================*/
void printServerStatus() {
  Serial.print("Server address: ");
  Serial.println(Ethernet.localIP());
}

/*=============================================================================================
  /   loop()
  /=============================================================================================*/
void loop() {
  // Listen for incoming client requests.
  EthernetClient client = server.available();
  if (!client) {
    //run the modulators before connection, keeps the mods running when disconnected
    cycleTheMods();
    toggleMods();
    return;
  }
  //server.println("Client connected");
  while (client.connected()) {
    String request = readRequest(&client);  //<--cycle the mods is in here
    request.toCharArray(receivedChars, request.length());
    strcpy(tempChars, receivedChars);
    parseData();
  }
}

/*=============================================================================================
  /      readRequest(EthernetClient* client)
  /=============================================================================================*/
// Read the request line,
String readRequest(EthernetClient* client) {
  String request = "";
  // Loop while the client is connected.
  while (client->connected()) {
    //This is the loop that cycles the mods while waiting for commands
    cycleTheMods();
    toggleMods();
    // Read available bytes.
    while (client->available()) {
      // Read a byte.
      char c = client->read();
      // Count number of commas:
      if (',' == c) {
        commaIter += 1;
      }
      // Exit loop if end of line.
      if ('\n' == c) {
        //Count number of inputs:
        commaNum = commaIter;
        commaIter = 0;
        // create an exception for empty commands (otherwise disconects)
        if (request.length() < 2) {
          //server.println("empty commands break the server");
          //poll_status();
        } else {
          return request.toLowerCase();
        }
      }
      // Add byte to request line.
      request += c;
    }
  }
  return request;
}

/*=============================================================================================
  /      Cycle the mods that are turned on
  /=============================================================================================*/
void cycleTheMods() {
  for (int i = 0; i < nBoard; i++) {
    if (onArray[i] > 0) {

      if ((digitalRead(oeNumbers[i]) == HIGH) && (pulseTimer[i] >= lambda)) {
        digitalWrite(oeNumbers[i], LOW);
      }

      if ((modTimer[i] >= modNus[i]) && (digitalRead(pinNumbers[i]) == HIGH)) {
        digitalWrite(pinNumbers[i], LOW);  //close mod
        //start pulse
        digitalWrite(oeNumbers[i], HIGH);
        pulseTimer[i] = 0;
      }

      if (modTimer[i] >= modPeriods[i]) {
        digitalWrite(pinNumbers[i], HIGH);  //open mod
        //start pulse
        modTimer[i] = 0;
        digitalWrite(oeNumbers[i], HIGH);
        pulseTimer[i] = 0;
      }
    }
  }
}

void toggleMods() {
  for (int i = 0; i < nBoard; i++) {
    if ((toggleArray[i] == 1) && (toggleTimer[i] >= lambda)) {
      digitalWrite(oeNumbers[i], LOW);
      digitalWrite(pinNumbers[i], LOW);
      toggleArray[i] = 0;
    }
  }
}


//==========================================================================================
//                                       Functions
//==========================================================================================


//Open a single modulator
//============---------------------------
void open_one(int mod) {
  onArray[mod] = 0;
  toggleArray[mod] = 1;
  digitalWrite(oeNumbers[mod], HIGH);
  digitalWrite(pinNumbers[mod], HIGH);
  toggleTimer[mod] = 0;
}
//Close a single modulator
//============---------------------------
void close_one(int mod) {
  onArray[mod] = 0;
  toggleArray[mod] = 1;
  digitalWrite(oeNumbers[mod], HIGH);
  digitalWrite(pinNumbers[mod], LOW);
  toggleTimer[mod] = 0;
}

void open_all() {
  for (int i = 0; i < nBoard; i++) {
    open_one(i);
  }
}

void close_all() {
  for (int i = 0; i < nBoard; i++) {
    close_one(i);
  }
}

//Run a single modulator
//============---------------------------
//If there is only one input then it should use the existing duty cycle:
void run_one(int mod) {
  onArray[mod] = 1;
  digitalWrite(oeNumbers[mod], HIGH);
  digitalWrite(pinNumbers[mod], HIGH);
  pulseTimer[mod] = 0;
  modTimer[mod] = 0;
}

//If there are two inputs the second is the duty cycle:
//============---------------------------
void run_one(int mod, float DC) {
  onArray[mod] = 1;
  modDC[mod] = DC;
  modNus[mod] = DC * modPeriods[mod];
  digitalWrite(oeNumbers[mod], HIGH);
  digitalWrite(pinNumbers[mod], HIGH);
  pulseTimer[mod] = 0;
  modTimer[mod] = 0;
}


//Turn on all modulators
//============---------------------------
//No input
void run_all() {
  for (int i = 0; i < nBoard; i++) {
    run_one(i);
  }
}
//Single input
void run_all(float DC) {
  for (int i = 0; i < nBoard; i++) {
    run_one(i, DC);
  }
}


//Turn off a single modulator
//============---------------------------
void modStop(int modNum) {
  onArray[modNum] = 0;
  digitalWrite(pinNumbers[modNum], LOW);
  digitalWrite(oeNumbers[modNum], LOW);
  toggleArray[modNum] = 1;
  close_one(modNum);
}

//Turn off all modulators
//============---------------------------
void all_off() {
  for (int i = 0; i < nBoard; i++) {
    modStop(i);
  }
}


//initialize variables
//============---------------------------
void array_init() {
  for (int i = 0; i < nBoard; i++) {
    modPeriods[i] = defaultPeriod;
    modDC[i] = defaultDC;
    modNus[i] = modDC[i] * modPeriods[i];
  }
}

//map pins to board number
//============---------------------------
void map_pins() {
  for (int i = 0; i < nBoard; i++) {
    pinNumbers[i] = allPinNumbers[i];
    oeNumbers[i] = allOENumbers[i];
    pinMode(pinNumbers[i], OUTPUT);
    pinMode(oeNumbers[i], OUTPUT);
  }
}

//Print status
//============---------------------------
void poll_status() {
  for (int x = 0; x < 50; x++) server.println();
  server.println(" ====================================");
  server.println("  i : s :  dc  : period(ms)");
  server.println(" ====================================");
  for (int i = 0; i < nBoard; i++) {
    server.print(" ");
    if (i < 9) {
      server.print(" ");
    }
    server.print(i + 1);
    server.print(" : ");
    server.print(onArray[i]);
    server.print(" : ");
    server.print(modDC[i]);
    server.print(" : ");
    server.print(" ");
    server.println(modPeriods[i] / 1000);
  }
}

//Print status
//============---------------------------
void dev_poll_status() {
  for (int x = 0; x < 50; x++) server.println();
  server.println(" ====================================");
  server.println("  i : s :  p : oe :  dc  : period(ms)");
  server.println(" ====================================");
  for (int i = 0; i < nBoard; i++) {
    server.print(" ");
    if (i < 9) {
      server.print(" ");
    }
    server.print(i + 1);
    server.print(" : ");
    server.print(onArray[i]);
    server.print(" : ");
    if (pinNumbers[i] < 10) {
      server.print(" ");
    }
    server.print(pinNumbers[i]);
    server.print(" : ");
    if (oeNumbers[i] < 10) {
      server.print(" ");
    }
    server.print(oeNumbers[i]);
    server.print(" : ");
    server.print(modDC[i]);
    server.print(" : ");
    server.print(" ");
    server.println(modPeriods[i] / 1000);
  }
}

//Parse the inputs (all telnet inputs:)
//============---------------------------
void parseData() {  // split the data into its parts

  char* strtokIndx;                     // this is used by strtok() as an index
  strtokIndx = strtok(tempChars, ",");  // read the string
  strcpy(messageFromPC, strtokIndx);    // copy it to messageFromPC

  //run
  //============--------------
  if (strcmp(messageFromPC, "on") == 0) {
    if (commaNum < 1) {
      //server.print("The on command requires a minimum of 2 inputs: string: on, int: modNum \xd0\x84 [0,modNum],*optional* float: DC \xd0\x84 [0,1]");
    }

    else if (commaNum == 1) {
      strtokIndx = strtok(NULL, ",");       // read the second part (number)
      int mod = abs(atoi(strtokIndx)) - 1;  // convert this part to an integer

      if (mod == -1) {
        //server.println("Run all");
        run_all();
        //poll_status();
      } else {
        run_one(mod);
        //poll_status();
      }
    }

    else if (commaNum == 2) {

      strtokIndx = strtok(NULL, ",");       // read the second part (number)
      int mod = abs(atoi(strtokIndx)) - 1;  // convert this part to an integer

      strtokIndx = strtok(NULL, ",");
      float DC = abs(atof(strtokIndx));  // convert this part to a float

      if (mod == -1) {
        //server.println("Run all");
        run_all(DC);
        //poll_status();
      } else {
        run_one(mod, DC);
        //poll_status();
      }
    }

  }

  //change the period
  //============--------------

  else if (strcmp(messageFromPC, "period") == 0) {
    if (commaNum != 2) {
      //server.println("The period command requires 3 inputs: period, modNum \xd0\x84 [0,modNum], value \xd0\x84 [0,infty)");
    } else {
      strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
      int mod = abs(atoi(strtokIndx)) - 1;  // convert this part to an integer

      strtokIndx = strtok(NULL, ",");             // this continues where the previous call left off
      float modT = abs(atof(strtokIndx) * 1000);  // convert this part to a float

      if (mod == -1) {
        //server.println("Change all periods");
        for (int i = 0; i < nBoard; i++) {
          modPeriods[i] = modT;
          modNus[i] = modDC[i] * modPeriods[i];
        }
        //poll_status();
      } else {
        modPeriods[mod] = modT;
        modNus[mod] = modDC[mod] * modPeriods[mod];
        //poll_status();
      }
    }
  }
  //poll mod status
  //============--------------

  else if (strcmp(messageFromPC, "poll") == 0) {
    //poll_status();
  } else if (strcmp(messageFromPC, "dpoll") == 0) {
    dev_poll_status();
  }

  //open and close
  //============--------------

  else if (strcmp(messageFromPC, "open") == 0) {
    strtokIndx = strtok(NULL, ",");       // read the second part (number)
    int mod = abs(atoi(strtokIndx)) - 1;  // convert this part to an integer
    if (commaNum != 1) {
      //server.println("The period command requires 3 inputs: period, modNum \xd0\x84 [0,modNum], value \xd0\x84 [0,infty)");
    } else {
      if (mod != 0) {
        open_one(mod);
      } else {
        open_all();
      }
    }
  } else if (strcmp(messageFromPC, "close") == 0) {
    strtokIndx = strtok(NULL, ",");       // read the second part (number)
    int mod = abs(atoi(strtokIndx)) - 1;  // convert this part to an integer

    if (commaNum != 1) {
      //server.println("The close command requires 2 inputs: period, modNum \xd0\x84 [0,modNum], value \xd0\x84 [0,infty)");
    } else {
      close_one(mod);
    }
  }


  //Turn off mods
  //============--------------

  else if (strcmp(messageFromPC, "off") == 0) {
    if (commaNum != 1) {
      //server.println("The off command requires 2 inputs: off, modNum \xd0\x84 [0,modNum]");
    } else {
      strtokIndx = strtok(NULL, ",");       // read the second part (number)
      int mod = abs(atoi(strtokIndx)) - 1;  // convert this part to an integer

      if (mod == -1) {
        all_off();
        //poll_status();
      } else {
        modStop(mod);
        //poll_status();
      }
    }
  }
  //Change the duty cycle
  //============

  else if (strcmp(messageFromPC, "dc") == 0) {

    if (commaNum != 2) {
      //server.println("The DC command requires 3 inputs: dc, int: modNum \xd0\x84 [0,modNum], float: dc \xd0\x84 [0,1] ");
    } else {
      strtokIndx = strtok(NULL, ",");       // this continues where the previous call left off
      int mod = abs(atoi(strtokIndx)) - 1;  // convert this part to an integer

      strtokIndx = strtok(NULL, ",");    // this continues where the previous call left off
      float DC = abs(atof(strtokIndx));  // convert this part to a float

      if (mod == -1) {
        //server.println("Change all duty cycles");
        for (int i = 0; i < nBoard; i++) {
          modDC[i] = DC;
          modNus[i] = DC * modPeriods[i];
        }
        //poll_status();
      } else {
        modDC[mod] = DC;
        modNus[mod] = DC * modPeriods[mod];
        //poll_status();
      }
    }
  }

  //restart the Teensy
  //============

  else if (strcmp(messageFromPC, "restart") == 0) {
    //Serial.println("Restarting the Teensy");
    //server.println("Restarting the Teensy");
    CPU_RESTART;
  }

  //Help on commands
  //============

  else if (strcmp(messageFromPC, "help") == 0) {

    server.println("Enter data in this style: on, Mod-Number (1-10)>, DC (0-1");
    server.println("Mod-Number=0 applies to all modulators");
    server.println();
    server.println("Commands include:");
    server.println("==================================================");
    server.println("==================================================");
    server.println("to turn on a modulator, enter:");
    server.println(" ");
    server.println("on, Mod-Number, val");
    server.println(" ");
    server.println("val: float, value of the new duty cycle (between 0-1)");
    server.println();
    server.println("==================================================");
    server.println("to change the period, enter:");
    server.println(" ");
    server.println("period, Mod-Number, val");
    server.println(" ");
    server.println("val: float, value of the new period");
    server.println();
    server.println("==================================================");
    server.println("to poll the status of all modulators enter:");
    server.println(" ");
    server.println("poll");
    server.println();
    server.println("==================================================");
    server.println("to turn off a modulator:");
    server.println(" ");
    server.println("off, Mod-Number");
    server.println();
    server.println("==================================================");
    server.println("to change the duty cycle:");
    server.println(" ");
    server.println("dc, ind, val:");
    server.println(" ");
    server.println("ind: int from 1-numMod, decides which modulator to affect");
    server.println("val: float, value of the new duty cycle (0-1)");
    server.println("==================================================");
    server.println("to restart the teensy:");
    server.println(" ");
    server.println("restart");
  }

  /*
  ----------------------------------------------------------------------------------------------------------------------------------
~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~* Dave's Functions~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~*~
  ----------------------------------------------------------------------------------------------------------------------------------
*/
  else if (strcmp(messageFromPC, "power") == 0) {
    if (commaNum < 1) {  //avoid an error
      //server.println(" ");
    }

    else if (commaNum == 1) {  //return status
      strtokIndx = strtok(NULL, ",");
      int mod = max(min(abs(atoi(strtokIndx)) - 1, 8), 0);

      server.println(onArray[mod]);
    }

    else if (commaNum == 2) {

      strtokIndx = strtok(NULL, ",");
      int mod = max(min(abs(atoi(strtokIndx)) - 1, 8), 0);

      strtokIndx = strtok(NULL, ",");
      float sta = abs(atof(strtokIndx));

      if (sta == 1) {
        run_one(mod);
        server.println(onArray[mod]);
      } else if (sta == 0) {
        modStop(mod);
        server.println(onArray[mod]);
      } else {
      }
    }

  }


  else if (strcmp(messageFromPC, "mod") == 0) {
    if (commaNum < 1) {  //avoid an error

    }

    else if (commaNum == 1) {  //return status
      strtokIndx = strtok(NULL, ",");
      int mod = max(min(abs(atoi(strtokIndx)) - 1, 8), 0);

      server.print(modDC[mod]);
      server.print(", ");
      server.println(modPeriods[mod] / 1000);
    }

    else if (commaNum == 2) {

      strtokIndx = strtok(NULL, ",");
      int mod = max(min(abs(atoi(strtokIndx)) - 1, 8), 0);

      strtokIndx = strtok(NULL, ",");
      float DC = abs(atof(strtokIndx));


      modDC[mod] = DC;
      modNus[mod] = modDC[mod] * modPeriods[mod];

      server.print(modDC[mod]);
      server.print(", ");
      server.println(modPeriods[mod] / 1000);
    }

    else if (commaNum == 3) {

      strtokIndx = strtok(NULL, ",");
      int mod = max(min(abs(atoi(strtokIndx)) - 1, 8), 0);

      strtokIndx = strtok(NULL, ",");
      float DC = abs(atof(strtokIndx));

      strtokIndx = strtok(NULL, ",");
      float modT = abs(atof(strtokIndx) * 1000);
      ;

      modDC[mod] = DC;
      modPeriods[mod] = modT;
      modNus[mod] = modDC[mod] * modPeriods[mod];

      server.print(modDC[mod]);
      server.print(", ");
      server.println(modPeriods[mod] / 1000);
    }
  }
  /*
  ----------------------------------------------------------------------------------------------------------------------------------
                                                    Handle unrecognized commands:
  ----------------------------------------------------------------------------------------------------------------------------------
*/
  else {
    //server.println("command not recognized, type 'help' for a list of commands");
  }
}
