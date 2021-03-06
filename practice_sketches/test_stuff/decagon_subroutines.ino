float dielec_p = 0;
float temp = 0;
float elec_c = 0;

void deca_gs3_setup() {
  mySDI12.begin();
  delay(2000);
  //first command to take a measurement
  myCommand = String(SENSOR_ADDRESS) + "I!";
  Serial.println(myCommand);     // echo command to terminal

  mySDI12.sendCommand(myCommand);
  delay(30);                     // wait a while for a response

  while (mySDI12.available()) {  // build response string
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(5);
    }
  }
  Serial.println(sdiResponse); //write the response to the screen
  mySDI12.clearBuffer();

  delay(1000);                 // delay between taking reading and requesting data
  sdiResponse = "";           // clear the response string
}

void measure_decagon() {
//first command to take a measurement
  myCommand = String(SENSOR_ADDRESS) + "M!";
  //Serial.println(myCommand);     // echo command to terminal

  mySDI12.sendCommand(myCommand);
  delay(30);                     // wait a while for a response

  while (mySDI12.available()) {  // build response string
    digitalWrite(LED_PIN, HIGH);
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(5);
    }
    digitalWrite(LED_PIN, LOW);
  }
  //if (sdiResponse.length() > 1) Serial.println(sdiResponse); //write the response to the screen
  mySDI12.clearBuffer();


  delay(1000);                 // delay between taking reading and requesting data
  sdiResponse = "";           // clear the response string


// next command to request data from last measurement
  myCommand = String(SENSOR_ADDRESS) + "D0!";
  //Serial.println(myCommand);  // echo command to terminal

  mySDI12.sendCommand(myCommand);
  delay(30);                     // wait a while for a response

  while (mySDI12.available()) {  // build string from response
    char c = mySDI12.read();
    if ((c != '\n') && (c != '\r')) {
      sdiResponse += c;
      delay(5);
    }
  }

  sdiResponse.toCharArray(buf, sizeof(buf));

  p = buf;

  strtok_r(p, "+", &p);
  dielec_p = atof(strtok_r(NULL, "+", &p));
  temp = atof(strtok_r(NULL, "+", &p));
  elec_c = atof(strtok_r(NULL, "+", &p));
  
  
  //if (sdiResponse.length() > 1) Serial.println(sdiResponse); //write the response to the screen
  /*Serial.print("Dielectric Permittivity: ");
  Serial.println(dielec_p);
  Serial.print("Temperature (C): ");
  Serial.println(temp);
  Serial.print("Electric Conductivity: ");
  Serial.println(elec_c);*/
  mySDI12.clearBuffer();
}
