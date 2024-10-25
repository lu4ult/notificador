/* Notificador
 *  Este proyecto reemplaza el viejo DHT de pared. Le cambiamos el 16F887 por un ESP12, con este cambio podemos enviar notificaciones desde otros dispositivos a la pantalla LCD,
 *  además de mostrar el clima (que lo sacamos de IFTTT e interactuar con telegram.
 *
 *
 *  Versión:
 *    0.1: 01/01/1990: Función base
 *    0.46: 13/02/2020: Se cambia both token y client insecure porque telegram cambió el método SSL. Se necesita placa al menos 2.5.0
 *    0.47: 20/02/2020: Se agrega una variable virtual para enviar textos al LCD, para usar con mercadolibre e IFTTT
 *    0.49: 15/03/2020: Debug level 9
 *    0.50: 04/11/2020: Se agregan partes para que ande el bot (certificado de telegram).
 *    0.51: 11/01/2021: Se empieza a sacar todas las funciones del clima de IFTTT y el telegram Bot. Se agrega OpenWeather.
 *    0.52: 20/02/2021: Se analiza lluvia cada seis minutos y se actualiza el display si alguno de los textos cambio.
 *    0.53: 20/02/2021: Se hace ping a owncloud para ver cuando esta caido.
 *    0.54: 09/03/2021: Se agrega Telegram Bot solo para enviar y se saca WiFi Manager para ahorrar espacio.
 *    0.55: 10/03/2021: Se vuelve a 0.52 porque el telegram bot se maneja desde otro modulo, y también el ping a owncloud.
 *    0.57: 15/06/2021: Display se enciende al actualizar el clima sólo si hay lluvia, porque sino me distrae.
 *    0.58: 19/06/2021: Enciende display si tiene tiempo, para evitar parpadeo.
 *    0.59: 06/07/2021: Se agrega recepción EspNow para hacer de Monitor de Rais.
 *    0.60: 08/07/2021: EspNow se puede habilitar que inicie o no, para que los rais funcionen sólo con el monitor.
 *    0.61: 16/02/2022: Se escribe en V28 un JSON con la fecha y hora y DOW, de forma que se deja preparado si se tiene que agregar a los Timbres para que actulicen la hora.
 *    0.62: 15/04/2022: Recibe en V30 un JSON con la latitud y longitus (además de chipId, dispositivo y mac), y pone ese puntero en el mapa en V31
 *    0.63: 20/04/2022: Va contando cuántos dispositivos se han logueado en el mapa desde las 00:00 para ver si hay más de 25 activos o no.
 *    0.64: 01/05/2022: Contamos individualmente los dispositivos
 *    0.66: 22/08/2022: Se saca mapa Blynk, se agrega WOL y se cambia cómo llama a openWeather con httpGetString debido al cambio de versión de placa, también se saca WTD por ese cambio.
 *    0.68: 01/09/2022: Se agregan arrays de viento y temperatura para que envíe por Whatsapp con CallMeBot
 *    0.69: 06/09/2022: Se agrega lluvia en whatsapp y mediodia solar (que antes estaba en la cortina)
 *    0.70: 13/09/2022: Se cambia API callmeBot y se agrega a Facu y Vir
 *    0.71: 15/09/2022: Agregamos pronóstico del Campo de Facu
 *    0.72: 16/09/2022: Se agregan whatsapp por cumpleaños. Se saca reloj de Blynk y utilizamos la API para observar cómo funciona
 *    0.73: 17/09/2022: Seguimos buscando el error que causa wtd, aparentemente es al escribir en V104 y V107 cada minuto
 *    0.74: 21/09/2022: cambios menores, cambios en horas que se envia clima y pide clima y pronostico en momentos distintos
 *    0.75: 26/09/2022: Deserialize error json
 *    0.76: 03/10/2022: Parece que versión 0.75 está funcionando correctamente. Se agrega mediodía solar que evidentemente nunca se había agregado durante las pruebas.
 *    0.77: 03/10/2022: Lectura de V27 externo para cuando se cambie la API legacy
 *    0.78: 10/01/2023: Sacamos el auth viejo que ya quedó deprecado
 *    0.79: 26/02/2023: Se empieza a reemplazar la API de Open Weather, se quitan muchísimas funciones
 *    0.81: 20/09/2023: Agrega MQTT
 */

#define FIRM_VERSION 0.81

#define MAX_WIND_SPEED 25  // Subir a 15 o 20  //Km/h
#define MAX_TEMPERATURA 35
#define MIN_TEMPERATURA 5

#define WA_TODOS

// #define WATCHDOG_DUCO
// #define SERIAL_SI
// #define DEBUG
// #define BLYNK_PRINT Serial // Enables Serial Monitor

#define MQTT
#define BLYNK_SI
#define RTC_BLYNK
// #define HORA_API
// #define HORA_UDP
//#define DHT_SI

#define BLYNK_TEMPLATE_ID "TMPLP0hi1DQH"  // Requiere para Blynk IOT (nuevo)
#define BLYNK_TEMPLATE_NAME "Notificador"
// #define BLYNK_AUTH_TOKEN "40faYKApg37ONbh8YIPQWLe5NcdDRtBu"

#include <ESP8266WiFi.h>  //esp standalone
// #define BLYNK_PRINT Serial    //para que ESP publique por serial
#include <ArduinoOTA.h>
#include <BlynkSimpleEsp8266.h>
#include <EEPROM.h>
#include <WiFiClientSecure.h>
// #include <UniversalTelegramBot.h>
#include <WiFiUdp.h>
#include <WakeOnLan.h>
WiFiUDP UDP;
WakeOnLan WOL(UDP);


#include <ESP8266WiFiMulti.h>  //MultiWiFi: elige la mejor SSID
ESP8266WiFiMulti wifiMulti;

// Wifi manager (auto ssid)
/*#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
*/

// Ticker tickerOSWatch;
// #define OSWATCH_RESET_TIME 300
unsigned long last_loop = 0, previousMillis = 0, previousMillis_bl = 0;  //, previousMillisSegundo = 0;

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>

#ifdef WATCHDOG_DUCO
#include <Ticker.h>
Ticker lwdTimer;
#define LWD_TIMEOUT 2 * 60 * 1000  // Dos minutos
uint32_t lwdCurrentMillis = 0;
uint32_t lwdTimeOutMillis = LWD_TIMEOUT;
#endif

// WidgetMap myMap(V31);

#include <TimeLib.h>

// Real-Time clock
#ifdef RTC_BLYNK

#include <WidgetRTC.h>
BlynkTimer timer;
WidgetRTC rtc;
#endif
// bool acomodar_fecha=1;

#ifdef HORA_UDP
#include <NTPClient.h>
// #include <WiFiUdp.h> //Se usará para RTC UDP
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800);
// WiFiUDP ntpUDP;
// NTPClient timeClient(UDP, "pool.ntp.org", -10800);
#endif

// int seconds;

byte segundos;
byte minuto = 0, minutoAnt = 0;
byte hora = 0, horaAnt = 0;
byte dia = 0, diaAnt;
byte mes = 0;

unsigned long ano = 1970, anoAnt = 1970;
String currentTime, currentDate;
String dowAsString = "";

// #include <NTPClient.h>
// #include <WiFiUdp.h>
// WiFiUDP ntpUDP;
// NTPClient timeClient(ntpUDP, "pool.ntp.org", -10800);
// NTPClient timeClient(UDP, "pool.ntp.org", -10800);

// char ssid[] = "lu4ult";
// char pass[] = "lucaprodan";
// char auth[] = "sPbdkYZhYbfd0o4s_K-n8sFdPVmb7UKu"; //OTA Base (proyecto Blynk)

#ifdef BLYNK_SI
WidgetTerminal terminal(V0);
// WidgetBridge Barcode(V30);
// WidgetBridge DetectorAmigos(V31);
#endif

#ifdef DHT_SI
#include "DHT.h"
#define DHTPIN 12
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
#endif

// Puesta de Sol
// #include <Dusk2Dawn.h>
// Dusk2Dawn cordoba(-31.4282, -64.1858, -3);
// unsigned int cordobaSunrise = 0, cordobaSunset = 0, momentoActual, mediodia;
// byte mediodiaHora,mediodiaMinuto;


#ifdef MQTT
#include <PubSubClient.h>
const char *mqtt_server = "broker.emqx.io";
WiFiClient espClient;
PubSubClient clienteMqtt(espClient);
char msg[50];
#endif

MD5Builder md5;


#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);

byte pin_backlight = 16;
byte pin_boton = 14;
byte PIN_ZUMBADOR = 2;

//#include <TimerFreeTone.h>
#define SONIDO_ENCENDIDO -1
#define SONIDO_APAGADO -2
#define SONIDO_BT -3  //bluetooth
#define SONIDO_EN -4  //sonido habilitado (enable)
#define SONIDO_BATERIABAJA -5

/*
extern "C" {
#include <espnow.h>
}
uint32_t lastEspNow=0;
bool monitorRaisHabilitado=0, iniciarEspNow=0;
*/
#define PERIODO 1  // En segundos para "periodicamente"
// #define PERIODO_MS 10000 //En milisegundos
#define BACKLIGHT_ENCENDIDO 1
#define BACKLIGHT_APAGADO 0

// int i=0;
// unsigned int periodo=1;
bool isFirstConnect = 1;
bool boton = 0, boton_ant = 1;

float temperatura_interior = 1, humedad_interior = 0;
String linea1 = "12345678901234567890", linea2 = "12345678901234567890", linea3 = "12345678901234567890", linea4 = "12345678901234567890";

unsigned int tiempoBackLight = 0;
bool backlight_habilitado = 1, telegram_habilitado = 1, telegram_a_barcode = 0;

// String condicion_hoy="12345678901234567890", condicion_manana="12345678901234567890", temperatura_minima="11111", temperatura_maxima="22222", condicion_actual="12345678901234567890";
// String viento="";
byte lluvia = 0, lluvia_ant = 1;

// #define BOTtoken "1077546088:AAHoqMbfor32zYHAUH7TqvI7Gct2PH5mYFY"  // your Bot Token (Get from Botfather)
// X509List cert(TELEGRAM_CERTIFICATE_ROOT);
// WiFiClientSecure client;
// UniversalTelegramBot bot(BOTtoken, client);
// int Bot_mtbs = 500; //mean time between scan messages     //Podemos ponerlo rapido porque cuando no hay mensajes no tarda nada, al rededor de 2mS.
// Cuando si hay mensajes, ahí si tarda, al rededor de 10 segundos.
// long Bot_lasttime;   //last time messages' scan has been done

// int numNewMessages=0;

// const String devid = "v7A7291849901916"; //device ID from Pushingbox //PROBAR #define DEV_ID "V2ETC" (no anda)
// WiFiClient client_sheets;  //Instantiate WiFi object
// unsigned int cantidad_mensajes_a_publicar=0;

// #include "./Listado_admins.h"

// unsigned long inicio_mensaje_telegram=0;
String personas_presentes = "no sincronizado todavia";
// String texto_a_enviar="", chat_a_enviar="";
// String bs_as="";
// bool mostrar_informacion=0;

// uint16_t condicion_actual_codigo[4]={1,2},condicion_actual_codigo_ant[4]={0,4,6,8};
String condicion_actual_str = "-";  //,pronostico_3hs="-",pronostico_6hs="-",pronostico_9hs="-";

/*
#define CANT_PRONOSTICOS 8
String pronosticosTextos[3];
String horas[CANT_PRONOSTICOS+1];
String pronosticos[CANT_PRONOSTICOS+1];
float vientos[CANT_PRONOSTICOS+1];
float temperaturas[CANT_PRONOSTICOS+1];
float lluvias[CANT_PRONOSTICOS+1];
*/
/*
//String pronosticosTextosCampo[3];
String horasCampo[CANT_PRONOSTICOS+1];
String pronosticosCampo[CANT_PRONOSTICOS+1];
float vientosCampo[CANT_PRONOSTICOS+1];
float temperaturasCampo[CANT_PRONOSTICOS+1];
byte lluviasCampo[CANT_PRONOSTICOS+1];
*/

uint32_t lastPronostico = 0;
String payloadPronostico;
bool enviarClimaPorWhatsapp = 0;
bool enviarClimaCampoPorWhatsapp = 0;
bool enviarClimaPorMinutoPorWhatsapp = false;
bool whatsappSoloParaMi = 0;
// bool iniciadoEnDebug=0;
uint32_t momentoBotonPresionado = 0;

bool pitar = false;
String mensajeClimaMinutely = "";

bool serverCrianzaOk = true, serverCrianzaOkAnt = true;


#ifdef WATCHDOG_DUCO
void ICACHE_RAM_ATTR lwdtcb(void) {
  if ((millis() - lwdCurrentMillis > LWD_TIMEOUT) || (lwdTimeOutMillis - lwdCurrentMillis != LWD_TIMEOUT)) {
    // Serial.println("Resetting ESP...");
    // EEPROM.write(ADDRESS_WATCHDOG_RECOVERY,1);
    // EEPROM.commit();
    delay(100);
    ESP.reset();
  }
}
#endif



void setup() {
  delay(200);

  Serial.begin(115200);
  Serial.println("\nIniciando...");

  if (WiFi.macAddress() == "AC:0B:FB:D3:C0:A4") {
    pin_boton = 12;
    PIN_ZUMBADOR = 14;
    pinMode(14, OUTPUT);
    pinMode(12, INPUT);
  }

  // WiFi.begin("lu4ult", "lucaprodan");
  // while (WiFi.status() != WL_CONNECTED) {
  //   delay(500);
  //   Serial.print(".");
  // }

  wifiMulti.addAP("lu4ult", "lucaprodan");
  wifiMulti.addAP("CET CRIANZA", "eU41e1vI9lcl");
  wifiMulti.addAP("Carancho 5147", "Carancho5147");


  Serial.println("Conectando a WiFi");
  for (byte i = 0; (wifiMulti.run() != WL_CONNECTED) and i <= 20; i++) {
    delay(500);
    Serial.println(".");
  }

  pinMode(pin_backlight, OUTPUT);
  digitalWrite(pin_backlight, BACKLIGHT_APAGADO);

  String ota_host_name = String("Notificador ") + WiFi.macAddress();
  ArduinoOTA.setHostname((const char *)ota_host_name.c_str());
  ArduinoOTA.setPassword((const char *)"301");
  ArduinoOTA.begin();

  // Blynk.config("sPbdkYZhYbfd0o4s_K-n8sFdPVmb7UKu");   //Auth ok del notificador
  // Blynk.config("7e5223c6f3c04c7e9c7187262512ef00");

#ifdef BLYNK_SI
  if (WiFi.macAddress() == "AC:0B:FB:D3:C0:A4") {  //"Timbre sandbox"
    // Blynk.config("eGwnXPvBIrC8ynx29fixVGyzQ80vRhwM");       //Ota Base
    // Blynk.config("40faYKApg37ONbh8YIPQWLe5NcdDRtBu");         //Blynk IOT (nuevo)
    // Blynk.config("sPbdkYZhYbfd0o4s_K-n8sFdPVmb7UKu");       //Notificador

    // digitalWrite(14,1);
    // delay(100);
    // digitalWrite(14,0);
  }
  /*
  if(WiFi.macAddress() == "DC:4F:22:5E:FE:27")                //Notificador
    //Blynk.config("sPbdkYZhYbfd0o4s_K-n8sFdPVmb7UKu");       //Notificador
    Blynk.config("40faYKApg37ONbh8YIPQWLe5NcdDRtBu");         //Blynk IOT (nuevo)
  */
  Blynk.config("40faYKApg37ONbh8YIPQWLe5NcdDRtBu");  // Blynk IOT (nuevo)
  Blynk.connect();
// Blynk.config("sPbdkYZhYbfd0o4s_K-n8sFdPVmb7UKu");
#endif

#ifdef DHT_SI
  dht.begin();
#endif

  // lcd.begin();
  lcd.init();
  lcd.clear();
  digitalWrite(pin_backlight, BACKLIGHT_APAGADO);
  // pinMode(pin_boton, INPUT_PULLUP);
  EEPROM.begin(512);
  // iniciarEspNow=EEPROM.read(68);
  delay(20);
  /*
    if(iniciarEspNow)
      {
      if(esp_now_init()!=0) {
        //Serial.println("Protocolo ESP-NOW no inicializado...");
        ESP.restart();
        delay(100);
      }
      esp_now_set_self_role(2);
      }
  */
  WOL.setRepeat(3, 100);
  WOL.calculateBroadcastAddress(WiFi.localIP(), WiFi.subnetMask());

#ifdef MQTT
  clienteMqtt.setServer(mqtt_server, 1883);
  clienteMqtt.setCallback(callback);
#endif

  // timeClient.begin();


#ifdef HORA_API
  getTimeFromApi();
#endif
  // pronostico();
  // clima();
  // actualizarSol();
  // pronosticoCampo();

#ifdef RTC_BLYNK
  setSyncInterval(20);  // Intervalo de actualización del reloj muy rapido hasta que se sincronize
  // setSyncInterval(5*60);
  rtc.begin();
#endif

  // Serial.println(WiFi.macAddress());

#ifdef WATCHDOG_DUCO
  lwdCurrentMillis = millis();
  lwdTimeOutMillis = lwdCurrentMillis + LWD_TIMEOUT;
  lwdTimer.attach_ms(LWD_TIMEOUT, lwdtcb);
#endif
  previousMillis = millis();
  Serial.println(WiFi.macAddress());

  mensajeClimaMinutely.reserve(980);

  /*
  for(int i=-6;i<=0;i++) {
    Serial.println(i);
    pitidos(i,100);
    delay(1000);
  }

 

  espera(2000);*/


  Serial.println("Iniciado");
  pitidos(SONIDO_EN, 100);

  //TODO: AGREGAR PITIDOS COMO EN TIMBRE WHATSAPP EL DEL ERROR, COMO TAMBIÉN EN BARCODE READER
}

#ifdef BLYNK_SI
BLYNK_CONNECTED() {
  if (isFirstConnect) {

    // Serial.println("Iniciando sincro");
    // Blynk.syncAll();
    Blynk.syncVirtual(V27);
    Blynk.syncVirtual(V53);
    isFirstConnect = false;
    // Blynk.virtualWrite(V103, FIRM_VERSION); //debug level 4
    // Blynk.virtualWrite(V105, String(3014)); //debug level 7
    // Blynk.virtualWrite(V106, String("SSID: "+WiFi.SSID()+" - Pass: "+WiFi.psk()+" MAC: " + WiFi.macAddress()));
    // Serial.println("fin sincro");
    // pitidos(SONIDO_ENCENDIDO,0);
  }
}
#endif

void loop() {
  if (WiFi.waitForConnectResult() == WL_CONNECTED) {
#ifdef BLYNK_SI
    Blynk.run();
#endif
    ArduinoOTA.handle();
#ifdef RTC_BLYNK
    timer.run();
#endif
    wifiMulti.run();
  }


#ifdef MQTT
  if (!clienteMqtt.connected()) {
    reconnect();
  }
  clienteMqtt.loop();
#endif


  if (pitar) {
    pitar = false;
    // pitidos(SONIDO_BT, 0);
    // delay(1000);

    for (byte i = 0; i < 3; i++) {
      digitalWrite(PIN_ZUMBADOR, 1);
      delay(100);
      digitalWrite(PIN_ZUMBADOR, 0);
      delay(100);
    }
  }

  if (enviarClimaPorMinutoPorWhatsapp) {
    // Serial.println("enviar wa");
    enviarClimaPorMinutoPorWhatsapp = false;
    mensajeClimaMinutely = climaMinutely();

    // Serial.println(rtta);
    if (mensajeClimaMinutely.indexOf("error") == -1 and mensajeClimaMinutely.indexOf("nada") == -1) {
      // lastEnvioLluviasPorMinutoExitoso = millis();
      Serial.println("Enviando whatsapp:");

      if (dia == 6 and mes == 3 and ano >= 2022 and millis() > 5 * 60 * 1000) {
        enviarWhatsapp(mensajeClimaMinutely, 1);
        espera(500);
        enviarWhatsapp(mensajeClimaMinutely, 2);
        espera(500);
      }

      if (enviarWhatsapp(mensajeClimaMinutely, 0)) {
        Serial.println("whatsapp enviado exitosamente");
      } else {
        Serial.println("Whatsapp falló");
      }
    }
  }

  if (Serial.available()) {
    delay(50);
    String entrada = "";
    while (Serial.available()) {
      char c = Serial.read();
      entrada += c;
    }
    Serial.println("Recibi:\n" + entrada);

    /*
    if(entrada.indexOf("clima") >= 0) {
      pronostico();
      }
    */
    if (entrada.indexOf("wa") >= 0) {
      horaAnt = random(60, 70);
      previousMillis = 0;
      enviarClimaPorWhatsapp = 1;
      Serial.println("whatsapp");
    }
    if (entrada.indexOf("campo") >= 0) {
      horaAnt = random(60, 70);
      previousMillis = 0;
      enviarClimaCampoPorWhatsapp = 1;
    }

    if (entrada.indexOf("min") >= 0) {
      enviarClimaPorMinutoPorWhatsapp = true;
      pitar = true;
    }
  }

  if (digitalRead(pin_boton)) {
    tiempoBackLight = 10;

    uint32_t inicio = millis();
    while (digitalRead(pin_boton)) {
      if (millis() - inicio > 250) {
        pitidos(SONIDO_APAGADO, 0);
        Blynk.virtualWrite(V27, "-");
        publicar_lcd("", "", "", "");
      }
    }
  }

  if (tiempoBackLight and millis() - previousMillis_bl >= 1000) {
    previousMillis_bl = millis();

    if (tiempoBackLight) {
      //Serial.println("Tiempo backlight: "+String(tiempoBackLight));
      lcd.backlight();
      digitalWrite(pin_backlight, BACKLIGHT_ENCENDIDO);
      tiempoBackLight--;
      if (tiempoBackLight == 0) {
        digitalWrite(pin_backlight, BACKLIGHT_APAGADO);
        lcd.noBacklight();
      }
    }
  }


  //  unsigned long currentMillis = millis();
  last_loop = millis();
  if (millis() - previousMillis >= PERIODO * 1000) {
    previousMillis = millis();
    periodicamente();
  }
}

void periodicamente() {
#ifdef DEBUG
  // Serial.println("periodicamente");
  Serial.print(".");
#endif
#ifdef WATCHDOG_DUCO
  lwdCurrentMillis = millis();
  lwdTimeOutMillis = lwdCurrentMillis + LWD_TIMEOUT;
#endif
#ifdef HORA_API
  getTimeFromApi();
#endif


#ifdef RTC_BLYNK
  minuto = minute();
  hora = hour();
  segundos = second();
  // Serial.println(hora);
  // Serial.println(minuto);
  dia = day();
  mes = month();
  ano = year();
#endif


  if (ano != anoAnt and ano > 2020) {
    anoAnt = ano;

#ifdef BLYNK_SI
    setSyncInterval(15 * 60);
//terminal.println("Iniciado!");
#endif
    //actualizar(1);
    //publicar_lcd("", "","", "");
    //tiempoBackLight = 5;
    //Blynk.syncVirtual(V27);

    // Serial.println("rtc lento");
    // publicar_lcd(condicion_actual_str,pronosticosTextos[0],pronosticosTextos[1],pronosticosTextos[2],120*lluvia);
  }

  // if (dia != diaAnt) {
  //   diaAnt = dia;
  //   actualizarSol();
  // }

  // momentoActual = hora * 60 + minuto;

  if (minuto != minutoAnt) {
    minutoAnt = minuto;
    // #ifdef DEBUG
    // Serial.println("minuto ant");
    // #endif

    if (minuto % 5 == 0 && WiFi.waitForConnectResult() == WL_CONNECTED) {
      serverCrianzaOk = verificarServerCrianza();
      if (serverCrianzaOk != serverCrianzaOkAnt) {
        serverCrianzaOkAnt = serverCrianzaOk;

        Serial.println("Cambio server Crianza");

        if (serverCrianzaOk) {
          Serial.println("Server OK");
          // enviarWhatsapp("Server Crianza Ok", 0);
          enviarDisplay("Server Crianza Ok");
        } else {
          Serial.println("Server Caido");
          // enviarWhatsapp("Server Crianza Caído!", 0);
          enviarDisplay("Server Crianza Caído!");
        }
      }
    }

    // if (momentoActual == mediodia and hora > 8 and hora < 18) {
    //   publicar_lcd("", " Excelente momento", " para lavar platos", "");
    //   tiempoBackLight = 15 * 60;
    // }
  }

  if (hora != horaAnt and ano > 2020 and millis() > 5 * 60 * 1000) {
    // if(hora != horaAnt) {
    // Serial.println("hora distinta");
    horaAnt = hora;
#ifdef DEBUG
    Serial.println("hora ant");
#endif

    // if (timestampProximoWhatsapp == 0) {
    //   Serial.println("Sin mensajes programados");
    // }

    // else {
    //   unsigned long currentTimestamp = now() + 10800;
    //   unsigned long tiempoHastaProximoWhatsapp = 6 * 60 * 1000;

    //   if (currentTimestamp >= timestampProximoWhatsapp && (hora >= 8 || hora == 0)) {
    //     //timestampProximoWhatsapp = 0;
    //     //Serial.println("aca enviar whatsapp");
    //     enviarWhatsapp("Flush Whatsapps programados", 0);
    //   } else {
    //     tiempoHastaProximoWhatsapp = timestampProximoWhatsapp - currentTimestamp;
    //     // Serial.print("Proximo whatsapp en: ");
    //     // Serial.println(formatedTimeInMinutesAsString(tiempoHastaProximoWhatsapp));
    //   }

    //   if (tiempoHastaProximoWhatsapp < 5 * 60) {
    //     tiempoBackLight = 120;
    //     publicar_lcd("", "WA Programado:", formatedTimeInMinutesAsString(tiempoHastaProximoWhatsapp), "");
    //   }
    // }
  }
}

#ifdef BLYNK_SI
//V26: Fecha de la notificacion
BLYNK_WRITE(V27) {  // Mensajes Externos (IFTTT)

  //tiempoBackLight = 120;
  // String _recibido = param.asString();

  enviarDisplay(param.asString());
}

BLYNK_WRITE(V50) {
  if (param.asInt()) {
    Blynk.virtualWrite(V50, 0);
    WOL.sendMagicPacket("30:9C:23:67:92:95");
  }
}

//51: TNs
//52: mensajes programados

// BLYNK_WRITE(V53) {
//   timestampProximoWhatsapp = param.asInt();
//   if (timestampProximoWhatsapp)
//     timestampProximoWhatsapp -= 5;  //Restamos 5 segundos al timestamp real para que en el cambio de hora se dispare si o si
// }
#endif
