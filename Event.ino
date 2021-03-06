// ------------------- Инициализация  Динамика
void initBuzer() {
  int pin = readArgsInt();
  sendOptions("pinBuzer", pin);
  pinMode(pin, OUTPUT);
  sendStatus("stateBuzer", readArgsInt());
  analogWrite(pin,   getStatusInt("stateBuzer"));
  analogWriteFreq(0);
}

void buzerBeep() {
  analogWrite(getStatusInt("stateBuzer"), readArgsInt());
  analogWriteFreq(readArgsInt());
}


// -----------------  Аналоговый вход A0
void initA0() {
  HTTP.on("/analog.json", HTTP_GET, []() {
    HTTP.send(200, "application/json", graf(analogRead(A0), 10, 3000, "low:0"));
  });
  modulesReg("analog");
}
// ----------------- Кнопка GPIO 0
void initTach() {
  // Кнопка будет работать по прерыванию
  uint8_t TACH_PIN = 0;
  attachInterrupt(TACH_PIN, Tach_0, RISING); //прерывание сработает, когда состояние вывода изменится с низкого уровня на высокий
  configOptions = jsonWrite(configOptions, "TachCommand", readArgsString());
}
// Выполняется при нажатии на кнопку
void Tach_0() {
  static unsigned long millis_prev;
  // Устроняем дребезг контакта
  if (millis() - 500 > millis_prev) {
    command = jsonRead(configOptions, "TachCommand");
  }
  millis_prev = millis();
}

// -----------------  Движение
void initMotion() {
  int pin = readArgsInt();
  pinMode(pin, INPUT);
  attachInterrupt(pin, motionOn, RISING); //прерывание сработает, когда состояние вывода изменится с низкого уровня на высокий
  String cmd = readArgsString();
  configOptions = jsonWrite(configOptions, "Command", cmd);
  modulesReg("movement");
}

void motionOn() {
  motion.attach(120, motionOff);
  command = jsonRead(configOptions, "Command") + "on";

}
void motionOff() {
  motion.detach();
  command = jsonRead(configOptions, "Command") + "off";

}

// -----------------  DHT
void initDHT() {
  dht.setup(readArgsInt());
  //delay(dht.getMinimumSamplingPeriod());
  delay (1000);

  String temp = "";
  temp += dht.getTemperature();
  if (temp != "nan") {
    HTTP.on("/temperature.json", HTTP_GET, []() {
      float temp = dht.getTemperature();
      if (temp == 'NaN') {
        temp = 0;
      }
      HTTP.send(200, "application/json", graf(temp, 10, 3000, "low:0"));
    });
    modulesReg("temperature");
    HTTP.on("/humidity.json", HTTP_GET, []() {
      float temp = dht.getHumidity();;
      if (temp == 'NaN') {
        temp = 0;
      }
      HTTP.send(200, "application/json", graf(temp, 10, 3000, "low:0"));
    });
    modulesReg("humidity");
  }
}

// ---------- initDS18B20
void initDS18B20() {
  pinMode(DS18B20PIN, INPUT_PULLUP);
  DS18B20.begin();
  DS18B20.requestTemperatures();
  float ok = DS18B20.getTempCByIndex(0);
  DS18B20.setResolution(12);
  if (ok != -127) {
    HTTP.on("/temperature.json", HTTP_GET, []() {
      DS18B20.requestTemperatures();
      float temp = DS18B20.getTempCByIndex(0);
      if (temp == -127) {
        temp = 0;
      }
      HTTP.send(200, "application/json", graf(temp, 10, 3000));
    });
    modulesReg("temperature");
  }
}
// ----------------------Приемник на 433мГ
void rfReceived() {
  byte pin = readArgsInt();
  if (pin == 1 || pin == 3)  Serial.end();
  mySwitch.enableReceive(pin);
  // задача опрашивать RC код
  ts.add(3, 5, [&](void*) {
    handleRfReceiv();
  }, nullptr, true);
  sendStatus("rfReceived", 0);
  modulesReg("rfReceived");
}

void handleRfReceiv() {
  if (mySwitch.available()) {
    int value = mySwitch.getReceivedValue();
    if (value == 0) {
      configJson = jsonWrite(configJson, "rfReceived", 0);
    } else {
      int codeRC = mySwitch.getReceivedValue(); 
      flag = sendStatus("rfReceived", codeRC);
    }
    mySwitch.resetAvailable();
  }
}

// ----------------------Приемник ИK
void irReceived() {
  byte pin = readArgsInt();
  if (pin == 1 || pin == 3)  Serial.end();
  irReceiver = new IRrecv(pin);  // Create a new IRrecv object. Change to what ever pin you need etc.
  irReceiver->enableIRIn(); // Start the receiver   
   // задача опрашивать RC код
  ts.add(5, 100, [&](void*) {
    handleIrReceiv();
  }, nullptr, true);
  sendStatus("irReceived", "ffffffff");
  modulesReg("irReceived");
}

void handleIrReceiv() {
 if (irReceiver->decode(&results)) {
      //serialPrintUint64(results.value, HEX);
    //Serial.println("");
    dump(&results);
    flag = sendStatus("irReceived", String((uint32_t) results.value, HEX));
    irReceiver->resume();  // Continue looking for IR codes after your finished dealing with the data.
  }    
  
}

void dump(decode_results *results) {
  // Dumps out the decode_results structure.
  // Call this after IRrecv::decode()
  uint16_t count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    sendOptions("irDecodeType", "UNKNOWN"); 
  } else if (results->decode_type == NEC) {
    sendOptions("irDecodeType", "NEC"); 
  } else if (results->decode_type == SONY) {
    sendOptions("irDecodeType", "SONY"); 
  } else if (results->decode_type == RC5) {
    sendOptions("irDecodeType", "RC5"); 
  } else if (results->decode_type == RC5X) {
    sendOptions("irDecodeType", "RC5X"); 
  } else if (results->decode_type == RC6) {
    sendOptions("irDecodeType", "RC6"); 
  } else if (results->decode_type == RCMM) {
    sendOptions("irDecodeType", "RCMM"); 
  } else if (results->decode_type == PANASONIC) {
    sendOptions("irDecodeType", "PANASONIC"); 
  } else if (results->decode_type == LG) {
    sendOptions("irDecodeType", "LG"); 
  } else if (results->decode_type == JVC) {
    sendOptions("irDecodeType", "JVC"); 
  } else if (results->decode_type == AIWA_RC_T501) {
    sendOptions("irDecodeType", "AIWA_RC_T501"); 
  } else if (results->decode_type == WHYNTER) {
    sendOptions("irDecodeType", "WHYNTER"); 
  } else if (results->decode_type == NIKAI) {
    sendOptions("irDecodeType", "NIKAI"); 
  }  
}

// ----------------------Передатчик ИK
void irTransmitter() {
  String moduleName = "irTransmitter";
  byte pin = readArgsInt();
  if (pin == 1 || pin == 3)  Serial.end();
  irSender = new IRsend(pin);  // Create a new IRrecv object. Change to what ever pin you need etc.
  irSender->begin();  
  sCmd.addCommand("irsend",handleIrTransmit);
  commandsReg("irsend", moduleName);
  modulesReg(moduleName);
}
 
void handleIrTransmit() {
  uint32_t  tmp = strtol(("0x" + readArgsString()).c_str(), NULL, 0);
  if (tmp >= 0x000000 && tmp <= 0xFFFFFF) {
    irSender->sendNEC(tmp, 32); 
  }
 
  
  
}
