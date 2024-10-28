#ifdef MQTT
void reconnect() {
  // Loop until we're reconnected
  while (!clienteMqtt.connected()) {
    // Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = WiFi.macAddress();
    clientId += String(random(0xffff), HEX);
    // String clientId = "mqttx_22b22d5a";

    if (clienteMqtt.connect(clientId.c_str(), "lu4ult", "12345678")) {
      // if (clienteMqtt.connect(clientId.c_str())) {
      // Serial.println("connected");
      // Once connected, publish an announcement...
      // clienteMqtt.publish(WiFi.macAddress().c_str(), "hola desde ESP");
      // ... and resubscribe

      md5.begin();
      md5.add("susc" + WiFi.macAddress() + "lu4ult");
      md5.calculate();
      String recibir = md5.toString();
      // String recibir = "susc" + WiFi.macAddress() + "lu4ult";
      clienteMqtt.subscribe(recibir.c_str());
    } else {
      // Serial.print("failed, rc=");
      // Serial.print(clienteMqtt.state());
      // Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void callback(char* topic, byte* payload, unsigned int length) {
  String mensaje = "";


  // Serial.println("Length: " + String(length));
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");
  for (int i = 0; i < length; i++) {
    // Serial.print((char)payload[i]);
    mensaje += (char)payload[i];
  }
  // Serial.println();

  enviarDisplay(mensaje);

  for (byte i = 0; i < 4; i++) {
    digitalWrite(2, !digitalRead(2));
    delay(100);
  }


  //Con 1 debe leer los nuevos datos desde firestore, con 2 simplemente indicamos que está activo para que publique el historial
  //El primer 5 es de validación
  // if ((char)payload[0] == '5' && (char)payload[1] == '1') {
  //   debeLeerFirestore = true;
  //   clienteActivo = millis();
  // }

  // if ((char)payload[0] == '5' && (char)payload[1] == '2') {
  //   clienteActivo = millis();
  // }

  // if ((char)payload[0] == '5' && (char)payload[1] == '9' && (char)payload[2] == 'r') {
  //   forzarReinicio = 1;
  // }

  if ((char)payload[0] == '5' && (char)payload[1] == '9' && (char)payload[2] == 's') {
    ESP.reset();
  }
  // if ((char)payload[0] == '5' && (char)payload[1] == '9' && (char)payload[2] == 'd') {
  //   // ESP.eraseConfig(); //Borra todo el sketch
  //   ESP.reset();
  // }
}
#endif


void enviarDisplay(String txt) {
  if (ano >= 2022) {
    lcd.backlight();
  }

  String _recibido = corregir_string(txt);
  if (WiFi.macAddress() == "AC:0B:FB:D3:C0:A4" and ano >= 2022 and _recibido.length() > 2 and _recibido.indexOf("[NS]") == -1) {
    pitar = true;
  }
  // if (ano >= 2022) {
  //   Blynk.virtualWrite(V26, formatedTimeInMinutesAsString(hour() * 3600 + minute() * 60 + second()));
  // }

  if (_recibido.length() >= 2) {  //Si no hay mensajes que se apague la pantalla
    tiempoBackLight = 180;
  }

  if (_recibido.indexOf("[BLC]") >= 0) {
    tiempoBackLight = 10;
  }
  _recibido.replace("|", " ");
  _recibido.replace("[NS]", "");   //Evita que pite
  _recibido.replace("[BLC]", "");  //Tiempo backlight cortito
  publicar_lcd(_recibido.substring(0, 10), _recibido.substring(10, 30), _recibido.substring(30, 50), _recibido.substring(50, 70));
}

void pitidos(int _cantidad, long _duracion) {
  int r2d2Tones[] = { 3520, 3136, 2637, 2093, 2349, 3951, 2794, 4186 };
  switch (_cantidad) {
    case SONIDO_ENCENDIDO:
      {
        //tone(PIN_ZUMBADOR, 2000,100);
        tone(PIN_ZUMBADOR, 2000, 100);
        espera(110);
        tone(PIN_ZUMBADOR, 2500, 100);
        espera(110);
        //espera(110);
        //tone(PIN_ZUMBADOR, 2500,100);
        break;
      }

    case SONIDO_APAGADO:
      {
        tone(PIN_ZUMBADOR, 2500, 100);
        //tone(PIN_ZUMBADOR, 2500,100);
        espera(110);
        tone(PIN_ZUMBADOR, 2000, 100);
        espera(110);
        //tone(PIN_ZUMBADOR, 2000,100);
        break;
      }

    case SONIDO_BT:
      {
        for (int _i = 0; _i <= 6; _i++) {
          tone(PIN_ZUMBADOR, r2d2Tones[_i], 80);
          espera(110);
        }
        break;
      }

    case SONIDO_EN:
      {
        //tone(PIN_ZUMBADOR, 2000,100);
        tone(PIN_ZUMBADOR, 2000, 100);
        espera(110);
        tone(PIN_ZUMBADOR, 2500, 100);
        espera(110);
        tone(PIN_ZUMBADOR, 1500, 100);
        //tone(PIN_ZUMBADOR, 2500,100);
        espera(110);
        break;
      }
    case SONIDO_BATERIABAJA:
      {
        //tone(PIN_ZUMBADOR, 2000,100);
        tone(PIN_ZUMBADOR, r2d2Tones[7], 100);
        espera(110);
        tone(PIN_ZUMBADOR, r2d2Tones[5], 100);
        espera(110);
        tone(PIN_ZUMBADOR, r2d2Tones[0], 100);
        espera(110);
        tone(PIN_ZUMBADOR, r2d2Tones[1], 100);
        espera(110);
        //tone(PIN_ZUMBADOR, 2500,100);
        break;
      }


    default:
      {
        if (_cantidad >= 1) {
          for (int _i = 1; _i <= _cantidad; _i++) {
            digitalWrite(PIN_ZUMBADOR, 1);
            delay(_duracion / 2);
            digitalWrite(PIN_ZUMBADOR, 0);
            delay(_duracion / 2);
          }
        } else {
          //...
        }
      }
  }
  digitalWrite(PIN_ZUMBADOR, 0);
}

String urlEncode(String url) {
  url.replace(" ", "%20");
  url.replace("\n", "%0A");
  url.replace("!", "%21");
  url.replace("*", "%2A");
  url.replace("ó", "%C3%B3");
  return url;
}



String httpGetString(String _URL) {
  String _payload = "";
  WiFiClientSecure client_duco;
  client_duco.setInsecure();
  HTTPClient http_duco;
  if (http_duco.begin(client_duco, _URL)) {
    delay(20);
    int httpCode = http_duco.GET();
    // Serial.println("HTTP Code: " + String(httpCode));
    if (httpCode == HTTP_CODE_OK) {


      _payload = http_duco.getString();
    } else {
      Serial.printf("[HTTP] GET... failed, error: %s\n", http_duco.errorToString(httpCode).c_str());
    }
    http_duco.end();
  }
  return _payload;
}


// void actualizarSol(void) {
//   cordobaSunrise = cordoba.sunrise(ano, mes, dia, false);
//   cordobaSunset = cordoba.sunset(ano, mes, dia, false);


//   //cordobaSunrise = 17*60 + 20;
//   //cordobaSunset = cordobaSunrise + 10;

//   mediodia = cordobaSunset - cordobaSunrise;  //Calculamos la cantidad de minutos entre la puesta y la salida del sol
//   mediodia = mediodia / 2;                    //Luego dividimos por dos para obtener justo la mitad del tiempo
//   mediodia = mediodia + cordobaSunrise;       //Lo sumamos a la salida para desplazarnos hasta la posición correcta.
//   mediodia += 60;                             //Le agregamos una hora

// //mediodiaHora = mediodia/60;
// //mediodiaMinuto = mediodia - (mediodiaHora*60);

// //Serial.println("Nuevo mediodia: "+String(mediodia));
// #ifdef BLYNK_SI
//   terminal.println("Nuevo mediodia: " + String(mediodia));
// #endif
//   actualizar(1);
// }


String formatedTimeInMinutesAsString(uint32_t _totalSeconds) {
  byte _hora = 0, _minuto = 0, _segundo = 0;
  String _horaS, _minutoS, _segundoS;

  if (_totalSeconds < 10)
    return "0:00:0" + String(_totalSeconds);

  if (_totalSeconds < 60)  //Menos de un minuto
    return "0:00:" + String(_totalSeconds);

  if (_totalSeconds < 3600)  //Mas de un minuto pero menos de una hora
  {
    _minuto = (int)_totalSeconds / 60;
    _totalSeconds -= _minuto * 60;  //Esta variable la dejo en segundos para volver a llamarla (recursividad)

    if (_minuto < 10)
      _minutoS = "0";
    _minutoS += String(_minuto);

    if (_totalSeconds < 10)
      _segundoS = "0";
    _segundoS += String(_totalSeconds);

    return "0:" + _minutoS + ":" + _segundoS;
  }

  if (_totalSeconds >= 3600)  //Si es más de una hora
  {
    _hora = (int)_totalSeconds / 3600;
    _totalSeconds -= _hora * 3600;
    _minuto = (int)_totalSeconds / 60;
    _totalSeconds -= _minuto * 60;
    _segundo = _totalSeconds;

    if (_minuto < 10)
      _minutoS = "0";
    _minutoS += String(_minuto);

    if (_totalSeconds < 10)
      _segundoS = "0";
    _segundoS += String(_totalSeconds);

    return String(_hora) + ":" + _minutoS + ":" + _segundoS;
  }

  return "";
}



//https://forum.arduino.cc/t/how-to-combine-rtczero-h-and-timelib-h-to-store-a-timestamp/689562/3
char* timeFormat(time_t t) {
  static char cstr[16] = "\0";
  sprintf(cstr, "%02d", hour(t));
  sprintf(cstr + strlen(cstr), "%s", ":");
  sprintf(cstr + strlen(cstr), "%02d", minute(t));
  //sprintf(cstr + strlen(cstr), "%s",":");
  //sprintf(cstr + strlen(cstr), "%02d",second(t));
  return cstr;
}

void publicar_lcd(String _1, String _2, String _3, String _4) {
  // if (_tiempo) digitalWrite(pin_backlight, BACKLIGHT_ENCENDIDO);      //Mostramos brevemente el mensaje anterior

  _1 = _1.substring(0, 20);  //Evitamos que si el texto es muy largo se impriman caracteres en otras líneas que no deberían
  _2 = _2.substring(0, 20);
  _3 = _3.substring(0, 20);
  _4 = _4.substring(0, 20);

  bool mostrarFecha = 0;
  if (_1.length() < 14) mostrarFecha = 1;

  /*terminal.println("cacho:");
  terminal.println(_1.substring(15,20));
  terminal.flush();*/
  if (_1.substring(15, 20) == "     " and year() > 2022) mostrarFecha = 1;


  //delay(1000,0);
  delay(1000);
  lcd.clear();
  //delay(250,0);
  delay(250);
  lcd.setCursor(0, 0);
  lcd.print(_1);

  //if(mostrarFecha and !acomodar_fecha)                  //Si hay espacio para publicar la hora, la publicamos
  if (mostrarFecha) {  //Si hay espacio para publicar la hora, la publicamos
    lcd.setCursor(15, 0);
    if (hora < 10) lcd.print("0");
    lcd.print(hora);
    lcd.print(":");
    if (minuto < 10) lcd.print("0");
    lcd.print(minuto);
  }
  lcd.setCursor(0, 1);
  lcd.print(_2);
  lcd.setCursor(0, 2);
  lcd.print(_3);
  lcd.setCursor(0, 3);
  lcd.print(_4);
  //tiempo_backlight = _tiempo;
}



void actualizar(bool _publicar_hora) {
#ifdef RTC_BLYNK
  //currentTime = String(hora) + ":" + (minuto) + ":" + (second());
  currentTime = String(hora) + ":";
  if (minuto < 10) currentTime += "0";
  currentTime += String(minuto) + ":";
  if (second() < 10) currentTime += "0";
  currentTime += String(second());
  currentDate = "";
  if (day() < 10) currentDate += "0";
  currentDate += String(day()) + " / ";
  if (month() < 10) currentDate += "0";
  currentDate += String(month()) + " / " + String(year());
  if (_publicar_hora) {
    //terminal.print("Hora: ");
    terminal.print("\nHora: ");
    terminal.print(currentTime);
    terminal.print(" - ");
    terminal.println(currentDate);
    terminal.flush();
  }
#endif
}


//Esta funcion recibe un string y lo vuelve con las vocales sin acentos y con las ¨ñ por n, para poder publicarlo en el display
String corregir_string(String _acomodar) {
  _acomodar.replace("á", "a");
  _acomodar.replace("é", "e");
  _acomodar.replace("í", "i");
  _acomodar.replace("ó", "o");
  _acomodar.replace("ú", "u");
  _acomodar.replace("Á", "A");
  _acomodar.replace("É", "E");
  _acomodar.replace("Í", "I");
  _acomodar.replace("Ó", "O");
  _acomodar.replace("Ú", "U");
  _acomodar.replace("ñ", "n");
  _acomodar.replace("Ñ", "N");

  _acomodar.replace("u00e1", "a");  //á        //todo esto es para telegram
  _acomodar.replace("u00e9", "e");  //é
  _acomodar.replace("u00ed", "i");
  _acomodar.replace("u00f3", "o");
  _acomodar.replace("u00fa", "u");
  _acomodar.replace("u00c1", "A");  //Á
  _acomodar.replace("u00c9", "E");  //É
  _acomodar.replace("u00cd", "I");
  _acomodar.replace("u00d3", "O");
  _acomodar.replace("u00da", "U");
  _acomodar.replace("u00fc", "u");  //ü
  _acomodar.replace("u00dc", "U");  //Ü
  _acomodar.replace("u00f1", "n");  //ñ
  _acomodar.replace("u00d1", "N");  //Ñ
  _acomodar.replace("u00bf", "?");  //¿
  _acomodar.replace("u00a1", "!");  //¡

  //Para MQTT
  _acomodar.replace("¡", "");
  _acomodar.replace("¿", "");

  return _acomodar;
}

void espera(unsigned long _tiempo) {
  //noTone(PIN_ZUMBADOR);
  unsigned long _inicio = millis();
  while ((millis() - _inicio) < _tiempo) {
  }
}
