// Cristian Marquez 
//Based on the work by Nixon
//Light effects - Home Automation - AudioRythm ligths

//Includes
#include <SPI.h>
#include <Ethernet.h>
#include <WebServer.h>
#include <EEPROM.h>

//Pins
static const int RED_PIN = 3;
static const int GREEN_PIN = 5;
static const int BLUE_PIN = 6;
static const int BUTTON_PIN = 9;
static const int UV_LED_PIN= 7;
//Dont use pins 10 11 12 13 because are used by ethernet shield

//Sensors
int soundDetectorDigitalPin=8;
int potentiometerAnalogPin=A1;

//Colour Component Enum
static const int R = 0;
static const int G = 1;
static const int B = 2;

//Transition Mode Enum
static const int NO_EXEC = -1;
static const int INSTANT = 0;
static const int FADE_DIRECT = 1;
static const int FADE_BLACK = 2;

//Default Values
static const int DEFAULT_TRANSITION = FADE_DIRECT;
static const int DEFAULT_TIME = 200;

//Standard Light Modes
static int OFF[] = {
  0, 0, 0};
static int FULL_WHITE[] = {
  255, 255, 255};

//Webserver
static uint8_t MAC[] = {
  0xCA, 0xFE, 0xBA, 0xBE, 0xFE, 0xED};
static uint8_t IP[] = {
  192, 168, 1, 3};
WebServer webserver("", 80);

//Variables
int currentColour[3];
int lastWebColour[3];
int buttonLast;
int lightMode;
int uvLight;
int lastUsedTransition = DEFAULT_TRANSITION;
int lastUsedTime = DEFAULT_TIME;
int lastUsedUVLight = 0;
int remainingTime=0;
boolean uvactive = false; //FIXME This is ugly!
int counter = 15;
static int effect;

//HTML for web front end UI
P(frontEndHTML) = "<!DOCTYPE html PUBLIC '-//W3C//DTD XHTML 1.0 Strict//EN' 'http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd'>"
"<html xmlns='http://www.w3.org/1999/xhtml'>"
"<head>"
"<title>Lights</title>"
"<meta http-equiv='Content-type' content='text/html;charset=UTF-8' />"
"<style type='text/css'>"
"div.const-width {"
"font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif;"
"width: 300px;"
"margin-left: auto;"
"margin-right: auto;"
"margin-bottom: 5px;"
"}"
"#header, #footer {"
"border: 1px solid blue;"
"}"
"p.tight {"
"padding-top: 0px;"
"padding-bottom: 0px;"
"margin-top: 0px;"
"margin-bottom: 0px;"
"}"
"h1 {"
"font-family: Arial, 'Helvetica Neue', Helvetica, sans-serif;"
"padding-top: 0px;"
"padding-bottom: 0px;"
"margin-top: 0px;"
"margin-bottom: 0px;"
"}"
".slider, #time-spinner {"
"margin-top: 10px;"
"margin-bottom: 10px;"
"}"
"#time-spinner {"
"width: 120px;"
"}"
"#transition-select {"
"width: 150px;"
"}"
"</style>"
"<link href='http://code.jquery.com/ui/1.10.2/themes/smoothness/jquery-ui.css' rel='stylesheet' />"
"<script type='text/javascript' src='http://code.jquery.com/jquery-1.9.1.js'></script>"
"<script type='text/javascript' src='http://code.jquery.com/ui/1.10.2/jquery-ui.js'></script>"
"<script type='text/javascript'>"
"$(function() {"
"$.default = {"
"endpoint : 'http://router.cristianmarquez.com.ar:8081'"
"};"
"var sliderOptions = {min: 0, max: 255, step: 1, stop: updateHandler};"
"var UVsliderOptions = {min: 0, max: 1, step: 1, stop: updateHandler};"
"var SoundSliderOptions = {min: 0, max: 1, step: 1, stop: changeMode};"
"var tSpinnerOptions = {min: 0, max: 2000, stop: 1};"
"$('#red-slider').slider(sliderOptions);"
"$('#green-slider').slider(sliderOptions);"
"$('#blue-slider').slider(sliderOptions);"
"$('#uv-slider').slider(UVsliderOptions);"
"$('#sound-slider').slider(SoundSliderOptions);"
"$('#white-slider').slider({min: 0, max: 255, step: 1, stop: whiteHandler});"
"$('#time-spinner').spinner(tSpinnerOptions);"
"$('#white-button')"
".button()"
".click(function() {"
"masterHandler(255, 255, 50);"
"});"
"$('#full-on-button')"
".button()"
".click(function() {"
"masterHandler(255, 255, 255,1);"
"});"
"$('#off-button')"
".button()"
".click(function() {"
"masterHandler(0, 0, 0);"
"});"
"$('#refresh-button')"
".button()"
".click(function() {"
"getLightState();"
"});"
"getLightState();"
"});"
"function whiteHandler() {"
"var whiteLightLevel = $('#white-slider').slider('values', 0);"
"masterHandler(whiteLightLevel, whiteLightLevel, whiteLightLevel);"
"}"
"function masterHandler(r, g, b,uv) {"
"$('#red-slider').slider('value', r);"
"$('#green-slider').slider('value', g);"
"$('#blue-slider').slider('value', b);"
"$('#uv-slider').slider('value', uv);"
"updateHandler();"
"}"
"function setLights(red, green, blue, transition, ttime, uuv) {"
"$.post($.default.endpoint+'/service', {r: red, g: green, b: blue, trans: transition, time: ttime, uv: uuv})"
".done(function(data) {"
"handleXMLresponse(data);"
"});"
"}"
"function changeMode() {"
"$.ajax({"
"url: $.default.endpoint+'/service',"
"type: 'PUT'"
"});"
"}"
"function getLightState() {"
"$.get($.default.endpoint+'/service')"
".done(function(data) {"
"handleXMLresponse(data);"
"});"
"}"
"function handleXMLresponse(data) {"
"var parser = new DOMParser();"
"var doc = parser.parseFromString(data, 'text/xml');"
"xmlDoc = $.parseXML(data);"
"$xml = $(xmlDoc);"
"var red = $xml.find('r').text();"
"var green = $xml.find('g').text();"
"var blue = $xml.find('b').text();"
"var transition = $xml.find('lastTransition').text();"
"var time = $xml.find('lastTime').text();"
"var uv = $xml.find('uv').text();"
"$('#red-slider').slider('value', red);"
"$('#green-slider').slider('value', green);"
"$('#blue-slider').slider('value', blue);"
"$('#uv-slider').slider('value', uv);"
"$('#time-spinner').spinner('value', time);"
"var average = (parseInt(red) + parseInt(green) + parseInt(blue)) / 3;"
"$('#white-slider').slider('value', average);"
"document.getElementById('transition-select').value = transition;"
"}"
"function updateHandler() {"
"var red = $('#red-slider').slider('values', 0);"
"var green = $('#green-slider').slider('values', 0);"
"var blue = $('#blue-slider').slider('values', 0);"
"var time = $( '#time-spinner' ).spinner('value');"
"var transition = document.getElementById('transition-select').value;"
"var uv = $('#uv-slider').slider('values', 0);"
"setLights(red, green, blue, transition, time, uv);"
"}"
"</script>"
"</head>"
"<body>"
"<div class='const-width' style='text-align:center' id='header'>"
"<h1>Cristian Marquez Home Automation</h1>"
"<h2>Office Ligths</h2>"
"</div>"
"<div class='const-width' id='colour-sliders'>"
"<div class='slider' id='red-slider' style='background:red'></div>"
"<div class='slider' id='green-slider' style='background:green'></div>"
"<div class='slider' id='blue-slider' style='background:blue'></div>"
"<div class='slider' id='white-slider'></div>"
"<div class='slider' id='uv-slider' style='background:#7f1ae5'></div>"
"<div class='slider' id='sound-slider' style='background:#e51a7f'></div>"
"</div>"
"<div class='const-width' id='time-selector'>"
"<label>Transition Time:</label>"
"<input id='time-spinner' name='ttime' />"
"</div>"
"<div class='const-width' id='transition-selector'>"
"<label>Transition Type:</label>"
"<select id='transition-select'>"
"<option value='0'>No Transition</option>"
"<option value='1'>Fade</option>"
"<option value='2'>Fade Through Black</option>"
"</select>"
"</div>"
"<div class='const-width' id='function-buttons'>"
"<button id='white-button'>White</button>"
"<button id='full-on-button'>Full On</button>"
"<button id='off-button'>Off</button>"
"<button id='refresh-button'>Refresh</button>"
"</div>"
"<div id='camera' style='text-align:center; width: 550px; margin-left: auto; margin-right: auto; margin-bottom: 5px;'>"
"<label>IP CAMERA</label>"
"<img src='http://router.cristianmarquez.com.ar:8082/videofeed' alt='Video feed'></div>"
"<div class='const-width' id='footer' style='text-align:center'>"
"<p class='tight'><a href='http://www.cristianmarquez.com.ar' >Cristian Marquez</a> 2013</p>"
"<p class='tight'>Based on the work done by <a href='http://www.dan-nixon.com/' > Dan Nixon</a></p>"
"</div>"
"</body>"
"</html>";

//Serves web front end to control light from a web browser
void webUI(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  server.httpSuccess();
  if (type == WebServer::GET) {
    server.printP(frontEndHTML);
  }
}

//Provides back end to control lights from front end and other apps
void webBackend(WebServer &server, WebServer::ConnectionType type, char *, bool) {
  char name[16];
  char value[16];
  server.httpSuccess();
  int transition = NO_EXEC;
  if(type == WebServer::POST) {
    int colour[3];
    int time = DEFAULT_TIME;
    int uvLight=0;
    while (server.readPOSTparam(name, 16, value, 16)) {
      if(strcmp(name, "r") == 0) colour[R] = atoi(value);
      if(strcmp(name, "g") == 0) colour[G] = atoi(value);
      if(strcmp(name, "b") == 0) colour[B] = atoi(value);
      if(strcmp(name, "trans") == 0) transition = atoi(value);
      if(strcmp(name, "time") == 0) time = atoi(value);
      if(strcmp(name, "uv") == 0) uvLight = atoi(value);
    }
    if(transition != NO_EXEC) {
      lastWebColour[R] = colour[R];
      lastWebColour[G] = colour[G];
      lastWebColour[B] = colour[B];
      lightChange(colour, transition, time);
      turnUVLight(uvLight);
    }
  }
  if((type == WebServer::POST) || (type == WebServer::GET)) {
    server.println("<?xml version='1.0'?>");
    server.println("<xml>");
    server.println("<currentColour>");
    server.print("<r>");
    server.print(currentColour[R]);
    server.println("</r>");
    server.print("<g>");
    server.print(currentColour[G]);   
    server.println("</g>");
    server.print("<b>");
    server.print(currentColour[B]);
    server.println("</b>");
    server.println("</currentColour>");
    server.print("<uv>");
    server.print(lastUsedUVLight);
    server.println("</uv>");
    server.print("<mode>");
    server.print(lightMode);
    server.println("</mode>");
    server.print("<lastTime>");
    server.print(lastUsedTime);
    server.println("</lastTime>");
    server.print("<lastTransition>");
    server.print(lastUsedTransition);
    server.println("</lastTransition>");
    server.print("<exec>");
    if(transition == NO_EXEC) server.print("FALSE");
    else server.print("TRUE");
    server.println("</exec>");
    server.println("</xml>");
  }
  if(type == WebServer::PUT)
  {
    changeMode();
  }  
}

void changeMode()
{
  switch(lightMode)
  {
  case 0:
    lightMode = 1;
    break;
  case 1: 
    lightMode=0;
    break;
  }
}

//Used to handle a change in lighting
void lightChange(int colour[], int transition, int time) {
  if(transition == NO_EXEC) return;
  int oldColour[3];
  oldColour[R] = currentColour[R];
  oldColour[G] = currentColour[G];
  oldColour[B] = currentColour[B];
  switch(transition) {
  case INSTANT:
    setRGB(colour);
    break;
  case FADE_DIRECT:
    fade(oldColour, colour, time);
    break;
  case FADE_BLACK:
    int timeHalf = time / 2;
    fade(oldColour, OFF, timeHalf);
    fade(OFF, colour, timeHalf);
    break;
  }
  lastUsedTime = time;
  lastUsedTransition = transition;
}

//Controls a smooth lighting fade
void fade(int startColour[], int endColour[], int fadeTime) {
  for(int t = 0; t < fadeTime; t++) {
    int colour[3];
    colour[R] = map(t, 0, fadeTime, startColour[R], endColour[R]);
    colour[G] = map(t, 0, fadeTime, startColour[G], endColour[G]);
    colour[B] = map(t, 0, fadeTime, startColour[B], endColour[B]);
    setRGB(colour);
    delay(1);
  }
  setRGB(endColour);
}

//Sets an RGB colour
void setRGB(int colour[3]) {
  if(colour[R] < 0) colour[R] = 0;
  if(colour[R] > 255) colour[R] = 255;
  if(colour[G] < 0) colour[G] = 0;
  if(colour[G] > 255) colour[G] = 255;
  if(colour[B] < 0) colour[B] = 0;
  if(colour[B] > 255) colour[B] = 255;
  analogWrite(RED_PIN, colour[R]);
  analogWrite(GREEN_PIN, colour[G]);
  analogWrite(BLUE_PIN, colour[B]);
  currentColour[R] = colour[R];
  currentColour[G] = colour[G];
  currentColour[B] = colour[B];
}

void setup() {
  Serial.begin(115200);
  randomSeed(analogRead(0));
  pinMode(13, OUTPUT);
  digitalWrite(13, LOW);
  pinMode(BUTTON_PIN, INPUT);
  digitalWrite(BUTTON_PIN, HIGH);
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(UV_LED_PIN, OUTPUT);
  Ethernet.begin(MAC, IP);
  //Ethernet.begin(MAC);
  webserver.setDefaultCommand(&webUI);
  webserver.addCommand("index", &webUI);
  webserver.addCommand("service", &webBackend);
  webserver.begin();
  buttonLast = digitalRead(BUTTON_PIN);
}

//void testUVLight() //loop
//{
//  delay(3000);
//  turnUVLight(true);
//  delay(5000);
//  turnUVLight(false);
//}

void loop() {
  char buff[64];
  int len = 64;
  webserver.processConnection(buff, &len);
  //Serial.println(lightMode);
  if(lightMode)
  {
    soundDetector();
  }
}

void soundDetector()
{
  int pot = map(analogRead(potentiometerAnalogPin),0,1023,1,4);
  if(counter == 0) 
  {
    Serial.print("The counter is 0 - Changing efect " );
    effect=random(1,6);
    Serial.print("Effect -> ");
    Serial.println(effect);
    counter=15;
  }

  //Serial.println(counter);
  if (digitalRead(soundDetectorDigitalPin)==1){
    int colour[3];
    switch (pot)
    { 
    case 1:
      //red
      //Serial.println("red");
      colour[R]=random(100, 255);
      colour[G]=0;
      colour[B]=0;
      break;
    case 2:
      //BOCA
      //blue
      // Serial.println("Boca");
      if((random(1,10)) % 2 == 0)
      {
        colour[R]=0;
        colour[G]=0;
        colour[B]=255;
      }
      else
      {
        //yellow
        colour[R]=255;
        colour[G]=255;
        colour[B]=0;
      }
      break;

    case 3:
      counter--;
      Serial.print("Time remainging -> ");
      Serial.println(counter);
      Serial.print("current effect-> ");
      Serial.println(effect);
      switch(effect)
      {
      case 0:
        Serial.println("val is 0");
        colour[R]=0;
        colour[G]=random(100, 255);
        colour[B]=0;
        break;
      case 1:
        Serial.println("val is 1");
        colour[R]=0;
        colour[G]=0;
        colour[B]=random(100, 255);
        break;
      case 2:
        Serial.println("val is 2");
        colour[R]=random(100, 255);
        colour[G]=random(100, 255);
        colour[B]=0;
        break;
      case 3:
        Serial.println("val is 3");
        colour[R]=random(100, 255);
        colour[G]=0;
        colour[B]=random(100, 255);
        break;
      case 4:
        Serial.println("val is 4");
        colour[R]=0;
        colour[G]=random(100, 255);
        colour[B]=random(100, 255);
        break;
      case 5:
        Serial.println("val is 5");
        colour[R]=random(100, 255);
        colour[G]=random(100, 255);
        colour[B]=random(100, 255);
        break;
      }
      break;
    case 4:
      //uv but use with caution.. we dont want crazy people on the party
      uvactive = true;
      //Serial.println("uv");
      colour[R]=0;
      colour[G]=0;
      colour[B]=0;
      turnUVLight(1);
      break;
    }

    lightChange(colour,FADE_DIRECT,100);

  }
  else{
    setRGB(OFF);
    if(uvactive){
      turnUVLight(0);
      uvactive=false;
    }
  }
}
void turnUVLight(int value)
{
  if(value == 1)
  {
    digitalWrite(UV_LED_PIN, HIGH);   // turn the LED on (HIGH is the voltage level)
  }
  else
  {
    digitalWrite(UV_LED_PIN, LOW);   // turn the LED off (LOW is the voltage level)
  } 
  lastUsedUVLight = value;
}










