bool verificarServerCrianza() {
  // Serial.println("Verificando server crianza");
  //En falla da: "" (largo 0) y demora unos 15600 mS. HTTP Code: -1
  String rtta = httpGetString("zzy");
  if (rtta.length()) {
    return true;
  }
    
  else {
    delay(3000);
    String rttaSegundoIntento = httpGetString("https://alecams.ddns.net");
    if (rttaSegundoIntento.length()) {
      return true;  
    }
    else {
      return false;
    }    
  }
  return false;
}

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


bool enviarWhatsapp(String texto, byte persona) {
  String phone, apikey;
  texto = urlEncode(texto);

  switch (persona) {
    case 1:
      {  //Vir
        phone = "+5493571599443";
        apikey = "861564";
        break;
      }

    case 2:
      {  //Facu
        phone = "+5492954618554";
        apikey = "424754";
        break;
      }

    default:
      {  //Yo
        phone = "+5492954692293";
        apikey = "2723989";
      }
  }

  String payload = httpGetString("https://api.callmebot.com/whatsapp.php?phone=" + phone + "&text=" + texto + "&apikey=" + apikey);
  return payload.indexOf("Message queued") >= 0 ? true : false;
}

#ifdef HORA_API
void getTimeFromApi(void) {
#ifdef SERIAL_SI
  Serial.println("llamando tiempo");
#endif
  String rtta = httpGetString("https://timeapi.io/api/Time/current/zone?timeZone=America/Buenos_Aires");

  //Serial.println(rtta);




  //StaticJsonDocument<384> doc;
  DynamicJsonDocument doc(400);

  //DeserializationError error = deserializeJson(doc, input);
  /*
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }
  */
  deserializeJson(doc, rtta);

  //int year = doc["year"]; // 2022
  ano = doc["year"];         // 2022
  int month = doc["month"];  // 12
  mes = doc["month"];        // 12
  int day = doc["day"];      // 31
  //int hour = doc["hour"]; // 19
  hora = doc["hour"];  // 19
  //int minute = doc["minute"]; // 56
  minuto = doc["minute"];         // 56
  int _seconds = doc["seconds"];  // 52
  //segundos = doc["seconds"]; // 52
  //int milliSeconds = doc["milliSeconds"]; // 999
  //const char* dateTime = doc["dateTime"]; // "2022-09-15T19:56:52.9927386"
  const char* _date = doc["date"];  // "09/15/2022"
  String date = "";
  date += _date;
  //const char* time = doc["time"]; // "19:56"
  //const char* timeZone = doc["timeZone"]; // "America/Buenos_Aires"
  const char* dayOfWeek = doc["dayOfWeek"];  // "Thursday"
  dowAsString = "";
  dowAsString += dayOfWeek;
  //bool dstActive = doc["dstActive"]; // false

  /*
  Serial.println(year);
  Serial.println(month);
  Serial.println(day);
  Serial.println(hour);
  //Serial.println(minute);
  //Serial.println(seconds);
  */
  //Serial.println(date);
  String _mes = date.substring(0, date.indexOf("/"));
  String _dia = date.substring(date.indexOf("/") + 1, date.lastIndexOf("/"));
  String _ano = date.substring(date.lastIndexOf("/") + 1, date.length());

  mes = _mes.toInt();
  ano = _ano.toInt();
  dia = _dia.toInt();
#ifdef SERIAL_SI
  Serial.println(String(hora) + ":" + String(minuto) + ":" + String(_seconds));
  Serial.println(String(dia) + "/" + String(mes) + "/" + String(ano));
#endif
  /*
  Serial.println(_dia);
  Serial.println(_mes);
  Serial.println(_ano);
  */
}
#endif

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
#ifdef SERIAL_DUCO_SI
      Serial.printf("[HTTP] GET... failed, error: %s\n", http_duco.errorToString(httpCode).c_str());
#endif
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

String climaMinutely() {
  String payload = httpGetString("https://api.openweathermap.org/data/3.0/onecall?lat=-31.4387428&lon=-64.1858807&appid=c17841cc5b21f1582ddc1b0548eecee8&lang=es&units=metric&exclude=current,hourly,daily,alerts");
  //Serial.println(payload);

  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, payload);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return "error";
  }

  //float lat = doc["lat"]; // -31.4387
  //float lon = doc["lon"]; // -64.1859
  //const char* timezone = doc["timezone"]; // "America/Argentina/Cordoba"
  int timezone_offset = doc["timezone_offset"];  // -10800

  String mensajeArmado = "Proximas lluvias:\n";

  for (JsonObject minutely_item : doc["minutely"].as< JsonArray >()) {
    unsigned long minutely_item_dt = minutely_item["dt"];                // 1677396300, 1677396360, 1677396420, 1677396480, ...
    float minutely_item_precipitation = minutely_item["precipitation"];  // 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, ...

    time_t t = minutely_item_dt - 10800;

    if (minutely_item_precipitation > 0.101) {
      mensajeArmado += String(timeFormat(t)) + ": " + String(minutely_item_precipitation) + " mm\n";
    }
    //Serial.println(String(timeFormat(t))+" - " + String(minutely_item_precipitation));
  }

  //Serial.println(mensajeArmado);
  if (mensajeArmado.length() > 20) {
    Serial.println("retornando mensaje");
    return mensajeArmado;
  }

  return "nada";
}

//https://api.openweathermap.org/data/3.0/onecall?lat=-31.4269085&lon=-64.1899073&appid=c17841cc5b21f1582ddc1b0548eecee8&lang=es&units=metric&exclude=minutely,daily
/*
void pronostico(void) {
  payloadPronostico = "";
  payloadPronostico = httpGetString("https://api.openweathermap.org/data/2.5/forecast?id=3860259&appid=c17841cc5b21f1582ddc1b0548eecee8&lang=es&units=metric&cnt="+String(CANT_PRONOSTICOS)); //cnt 6 ok, 7 8?

  DynamicJsonDocument doc(6144);
  DeserializationError error = deserializeJson(doc, payloadPronostico);

  if (error) {
    Serial.print(F("deserializeJson() Pronostico failed: "));
    Serial.println(error.f_str());
    actualizar(1);
    #ifdef BLYNK_SI
    terminal.println(F("deserializeJson() Pronostico failed: "));
    terminal.println(error.f_str());
    terminal.flush();
    #endif
    return;
  }
  else {
    lastPronostico = millis();
  }

 // const char* cod = doc["cod"]; // "200"
 // int message = doc["message"]; // 0
  //int cnt = doc["cnt"]; // 3
  int j=0;
  for (JsonObject elem : doc["list"].as<JsonArray>())
    {
    j++;
    //long dt = elem["dt"]; // 1610377200

    JsonObject main = elem["main"];
    float main_temp = main["temp"]; // 298.14
    //float main_feels_like = main["feels_like"]; // 299.67
    //float main_temp_min = main["temp_min"]; // 298.14
    //float main_temp_max = main["temp_max"]; // 306.53
    //int main_pressure = main["pressure"]; // 1007
    //int main_sea_level = main["sea_level"]; // 1007
    //int main_grnd_level = main["grnd_level"]; // 962
    //int main_humidity = main["humidity"]; // 72
    //float main_temp_kf = main["temp_kf"]; // -8.39

    JsonObject weather_0 = elem["weather"][0];
    int weather_0_id = weather_0["id"]; // 500
    condicion_actual_codigo[j]=weather_0_id;
    //const char* weather_0_main = weather_0["main"]; // "Rain"
    const char* weather_0_description = weather_0["description"]; // "lluvia ligera"
    //const char* weather_0_icon = weather_0["icon"]; // "10d"
  
  //  int clouds_all = elem["clouds"]["all"]; // 38
  
    float wind_speed = elem["wind"]["speed"]; // 2.81     
    wind_speed = wind_speed * 3.6;            //Pasamos de m/s a km/h con 3.6
    //Serial.println("\nVelocidad Viento: "+String(wind_speed));
    //int wind_deg = elem["wind"]["deg"]; // 166
  
   // int visibility = elem["visibility"]; // 10000
   // float pop = elem["pop"]; // 0.88
  
    float rain_3h = elem["rain"]["3h"]; // 1.37
  
   // const char* sys_pod = elem["sys"]["pod"]; // "d"
  
    const char* dt_txt = elem["dt_txt"]; // "2021-01-11 15:00:00"
   // Serial.println(dt_txt+": "+String(weather_0_id)+"= "+weather_0_description);
    String _hora=dt_txt;
    _hora=_hora.substring(11,13);
    String _cond=weather_0_description;

    horas[j-1] = _hora;
    pronosticos[j-1] =_cond;
    vientos[j-1] = wind_speed;
    lluvias[j-1] = rain_3h;
    temperaturas[j-1] = main_temp;

    for(int i =0; i<=2;i++) {
      pronosticosTextos[i] = "";
      pronosticosTextos[i] = horas[i] + ": ";        
        if(vientos[i]>=MAX_WIND_SPEED) {
          pronosticosTextos[i] += "V";
          pronosticosTextos[i] += String(int(vientos[i]));
          pronosticosTextos[i] +=  " ";
          }
      pronosticosTextos[i] += pronosticos[i];
      //Serial.println(pronosticosTextos[i]);
      }
    }
  }
*/

/*
void pronosticoCampo(void) {
  String payload = httpGetString("https://api.openweathermap.org/data/2.5/forecast?appid=c17841cc5b21f1582ddc1b0548eecee8&lang=es&units=metric&cnt=8&lat=-36.599&lon=-65.928"); //cnt 6 ok, 7 8?

  DynamicJsonDocument doc(3072);
  deserializeJson(doc, payload);

 // const char* cod = doc["cod"]; // "200"
 // int message = doc["message"]; // 0
  //int cnt = doc["cnt"]; // 3
  int j=0;
  for (JsonObject elem : doc["list"].as<JsonArray>())
    {
    j++;
    //long dt = elem["dt"]; // 1610377200

    JsonObject main = elem["main"];
    float main_temp = main["temp"]; // 298.14
    //float main_feels_like = main["feels_like"]; // 299.67
    //float main_temp_min = main["temp_min"]; // 298.14
    //float main_temp_max = main["temp_max"]; // 306.53
    //int main_pressure = main["pressure"]; // 1007
    //int main_sea_level = main["sea_level"]; // 1007
    //int main_grnd_level = main["grnd_level"]; // 962
    //int main_humidity = main["humidity"]; // 72
    //float main_temp_kf = main["temp_kf"]; // -8.39

    JsonObject weather_0 = elem["weather"][0];
    //int weather_0_id = weather_0["id"]; // 500
    //condicion_actual_codigo[j]=weather_0_id;
    //const char* weather_0_main = weather_0["main"]; // "Rain"
    const char* weather_0_description = weather_0["description"]; // "lluvia ligera"
    //const char* weather_0_icon = weather_0["icon"]; // "10d"
  
  //  int clouds_all = elem["clouds"]["all"]; // 38
  
    float wind_speed = elem["wind"]["speed"]; // 2.81     
    wind_speed = wind_speed * 3.6;            //Pasamos de m/s a km/h con 3.6
    //Serial.println("\nVelocidad Viento: "+String(wind_speed));
    //int wind_deg = elem["wind"]["deg"]; // 166
  
   // int visibility = elem["visibility"]; // 10000
   // float pop = elem["pop"]; // 0.88
  
    float rain_3h = elem["rain"]["3h"]; // 1.37
  
   // const char* sys_pod = elem["sys"]["pod"]; // "d"
  
    const char* dt_txt = elem["dt_txt"]; // "2021-01-11 15:00:00"
   // Serial.println(dt_txt+": "+String(weather_0_id)+"= "+weather_0_description);
    String _hora=dt_txt;
    _hora=_hora.substring(11,13);
    String _cond=weather_0_description;

    horasCampo[j-1] = _hora;
    pronosticosCampo[j-1] =_cond;
    vientosCampo[j-1] = wind_speed;
    lluviasCampo[j-1] = rain_3h;
    temperaturasCampo[j-1] = main_temp;    
    }
  }
*/

/*
void clima(void)
  {
 // String url1 = "http://api.openweathermap.org/data/2.5/weather?lat=-31.4262&lon=-64.1879&appid=c17841cc5b21f1582ddc1b0548eecee8&lang=es";
  String url = "https://api.openweathermap.org/data/2.5/weather?id=3860259&appid=c17841cc5b21f1582ddc1b0548eecee8&lang=es";

  
    String payload = httpGetString(url);
    DynamicJsonDocument doc(1024);
    //deserializeJson(doc, payload);

    DeserializationError error = deserializeJson(doc, payload);

    if (error) {
      Serial.print(F("deserializeJson() Clima failed: "));
      Serial.println(error.f_str());
      return;
    }
    JsonObject weather_0 = doc["weather"][0];
    condicion_actual_codigo[0] = weather_0["id"]; // 701
    //const char* weather_0_main = weather_0["main"]; // "Mist"
    const char* weather_0_description = weather_0["description"]; // "niebla"
    condicion_actual_str=weather_0_description;
    //const char* weather_0_icon = weather_0["icon"]; // "50d"



    //const char* base = doc["base"]; // "stations"

   // JsonObject main = doc["main"];

   // float main_temp = main["temp"]; // 292.15
    //    float main_feels_like = main["feels_like"]; // 287.39
    //float main_temp_min = main["temp_min"]; // 292.15
    //float main_temp_max = main["temp_max"]; // 292.15
    //int main_pressure = main["pressure"]; // 1008
    //int main_humidity = main["humidity"]; // 94
    //int visibility = doc["visibility"]; // 2500
    //float wind_speed = doc["wind"]["speed"]; // 10.8
    //int wind_deg = doc["wind"]["deg"]; // 200
    //int clouds_all = doc["clouds"]["all"]; // 75
    //long dt = doc["dt"]; // 1610368746

   // JsonObject sys = doc["sys"];
    //int sys_type = sys["type"]; // 1
    //int sys_id = sys["id"]; // 8226
    //const char* sys_country = sys["country"]; // "AR"
   // long sys_sunrise = sys["sunrise"]; // 1610357029
   // long sys_sunset = sys["sunset"]; // 1610407519

   // int timezone = doc["timezone"]; // -10800
   // long id = doc["id"]; // 3862744
   // const char* name = doc["name"]; // "Departamento de Capital"
    //int cod = doc["cod"]; // 200

    //horas[0] = "RN";
    //pronosticos[0] =condicion_actual_str;
    //vientos[0] = wind_speed;
    //lluvias[0] = rain_3h;
    //temperaturas[0] = main_temp;

  }
*/
//Esta función recibe un código de clima de Open Weather y devuelve si es de lluvia o no, según la tabla en:
// https://openweathermap.org/weather-conditions
/*
bool analizar_lluvia (uint16_t _cod)
  {
  if(_cod>200 and _cod<700) return 1;
  else return 0;
  }
*/

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
