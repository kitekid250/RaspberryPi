//////////////////////////////////////////////////////////////////////
///     Weather Station                                            ///
///    Written by Ward Prescott                                    ///
///    Sensor interfacing by Ryan Chaky                            ///
///    for BUSERT - Summer 2014                                    ///
///                                                                ///
//////////////////////////////////////////////////////////////////////

#include <Wire.h>
#include <SD.h>
#include <Ethernet.h>
#include <SPI.h>
#include <String.h>
#include <Time.h>
#include <Adafruit_BMP085.h>
#include "DHT.h"

#define DHTPIN 5
#define DHTTYPE DHT22

EthernetClient client;
Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);

long GMTOffset = -5 * 60 * 60; //The amount of seconds to be added to the local time to make it into GMT
unsigned int counter = 0;
bool maintainence = 1;
volatile int count;
volatile int r_count;
float array_wind_10[40];           //Array which will be updated to store and hold wind speed
float array_wind_direction_2[8];   //Array used to store and hold wind direction
float get_data[16];                //Array which referecnces all above-mentioned data
float r_fall;
float max_2;
float max_10;
float gust_dir_2;
float gust_dir_10;
int sensorPin = A1; 
int vane = A0;
char file_today_name[30]; 



void setup() {
  byte mac[] = {0x90, 0xA2, 0xDA, 0x00, 0xE1, 0x81};
  Serial.begin(9600);

  Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);
  while (!SD.begin(4)) {
    Serial.println("initialization failed!");
    delay(5000);
  }
  Serial.println("initialization done.");
  File logFile = SD.open("log.txt", FILE_WRITE);
  logFile.println("Weather Station Successfully Started");                                           //Add a timestamp to this
  

  if (Ethernet.begin(mac) == 0) {
    logFile.println("Failed to configure Ethernet using DHCP");                                      //Add a timestamp to this
  }
  delay(500);
  Serial.print("Local I.P: ");
  Serial.println(Ethernet.localIP());
  logFile.close();


  syncRTC();                                                                                          ////Fix this

  attachInterrupt(0,rain,FALLING);
  attachInterrupt(1,Wind,FALLING);
  pinMode(vane,INPUT);
  Serial.begin(9600);
  dht.begin();
  //Ensures the BMP Pressure and Temperature Sensor is working correctly
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP180 sensor, check wiring!");                          ///Add error reporting here

  }
  
  file_name();
  File csv = SD.open(file_today_name, FILE_WRITE);
  csv.print("Time,Wind Dir,Wind Speed,Humidity,Dewpoint,Temp,Rainin,Pressure,Solar Radiation,Panel Temp,2Min Gust,2 Min Gust Dir,Avg 2 Min,Avg 2 Min Dir,10 Min Gust,10 Min Gust Dir,Daily Rain,Success?");
  csv.close();

}

void loop() {
  
  
  
//  long beginTime = millis();
//  if (hour() == 0 && maintainence == 1){
//    dailyMaintainence();
//  }
//  if (hour() == 1){
//    maintainence = 1;
//  }
//
//  post(0);
//  long endTime = millis();
//  while(beginTime - endTime < 15000){
//    endTime = millis();
//  }
//  if (counter == 40){
//    counter = 0;
//  }
//  counter ++;


}

long requestTime() {
  String http_response = "";
  if (client.connect("api.timezonedb.com", 80)) { //TimeServer 1st arg
    Serial.println("connected");
    client.print("GET /?");
    client.print("zone=America/New_York");
    client.print("&");
    client.print("format=json");
    client.print("&");
    client.print("key=1LWJVJOSLNIK");
    client.print(" HTTP/1.1\r\n");
    client.print("Host: api.timezonedb.com\r\n");
    client.print("User-Agent: arduino-ethernet\r\n");
    client.print("Connection: close\r\n\r\n");
    delay(500);
    while (client.available()) {
      char c = client.read();
      http_response += c;
    }

    for (int x = 40; x < http_response.length() - 5; x++) {
      if (http_response.substring(x, x + 4) == "time") {
        String UTC = http_response.substring(x + 11, x + 21);
        client.stop();
        long time = UTC.toInt();
        return (time);
      }
      else{
        File logFile = SD.open("log.txt", FILE_WRITE);
        logFile.println("Recieved incorrect HTTP response from source at ");
                                                                                                                   ///logFile.print(currentTime)
        logFile.close();
      }
    
    }
  }
  else {
      File logFile = SD.open("log.txt", FILE_WRITE);
      logFile.println("Failed to connect to server while trying to obtain time at");
                                                                                                                   ///logFile.print(currentTime)
      logFile.close();
  client.stop();
}
}

void post(int mode) {//MODES ARE 0 FOR REGULAR 1 FOR POSTING TO BUFFER FILE AND 2 FOR POSTING THE DAILY WEATHER
  String http_response = "";
  if (client.connect("rtupdate.wunderground.com", 80)) {  //Weather server
    Serial.println("connected");
    client.print("GET /weatherstation/updateweatherstation.php?"); //Weather webpage
    client.print("ID=");
    client.print("KPALEWIS10");  //ID
    client.print("&PASSWORD=");
    client.print("pmjweather1");  //Password
    client.print("&dateutc=");
    client.print(year()); //Year
    client.print("-");
    client.print(month()); //Month
    client.print("-");
    client.print(day());  //Day
    client.print("+");
    client.print(3);    //Hour +5 for EST
    client.print("%3A");
    client.print(minute()); //Minute
    client.print("%3A");
    client.print(second()); //Second
    client.print("&winddir=");//  Current Wind Direction
    client.print(get_data[0]);
    client.print("&windspeedmph=");//  Wind Speed
    client.print(get_data[1]);
    client.print("&humidity=");//  Humidity
    client.print(get_data[8]);
    client.print("&dewptf=");//  Dewpoint
    client.print(get_data[9]);
    client.print("&tempf=");//  Temperature (in Degrees Farenheit)
    client.print(get_data[10]);
    client.print("&rainin=");//  Current Daily Rain Fall
    client.print(get_data[11]);
    client.print("&baromin=");//  Pressure
    client.print(get_data[13]);
    client.print("&solarradiation=");//  Solar Radiation
    client.print(get_data[14]);
    client.print("&indoortempf=");//  Additional Temperature (in Degrees Farenheit) for use of monitoring a PV cell temperature
    client.print(get_data[15]);
    if (counter % 8 == 0){
      client.print("&windgustmph=");//  2-minute Wind Gust
      client.print(get_data[2]);
      client.print("&windgustdir=");//  Direction at the 2-minute Wind Gust
      client.print(get_data[3]);
      client.print("&windspdmph_avg2m=");//  Average Wind Speed in a two minute interval 
      client.print(get_data[4]);
      client.print("&winddir_avg2m=");//  Average Wind Direction in a two minute interval
      client.print(get_data[5]);
    }
    if (counter % 40 == 0){
      //post the ten minute stuff
      client.print("&windgustmph_10m=");//  10-minute Wind Gust
      client.print(get_data[6]);
      client.print("&windgustdir_10m=");//  Direction at the 10-minute Wind Gust
      client.print(get_data[7]);
    }
    if (mode == 2){
      client.print("&dailyrainin=");//  Total Daily Rain Fall
      client.print(get_data[12]);
    }
    client.print("&realtime=1&rtfreq=5");
    client.print(" HTTP/1.1\r\n");
    client.print("Host: rtupdate.wunderground.com\r\n");
    client.print("User-Agent: arduino-ethernet\r\n");
    client.print("Connection: close\r\n\r\n");
    delay(500);
    while (client.available()) {
      char c = client.read();
      http_response += c;
    }
    for (int i = 50; i < http_response.length() - 1; i++) {
      if (http_response.substring(i, i + 4) == "succ") {
        client.stop();
        postCSV(mode, 1); 
        return;
      }
    }

    File logFile = SD.open("log.txt", FILE_WRITE);
    logFile.println("Post to wunderground did not return success at");
                                                                                                                                            ///logFile.print(currentTime);
    logFile.println(http_response);
    logFile.close();
    postCSV(mode, 0); 
  }
  else {
    Serial.println("connection failed");
    File logFile = SD.open("log.txt", FILE_WRITE);
    logFile.println("Failed to connect to server while trying to post time at");
                                                                                                                                           ///logFile.print(currentTime)
    logFile.close();
    postCSV(mode, 0); 
  }

  client.stop();
}


void postCSV(int mode, bool success){
  File csv = SD.open(file_today_name, FILE_WRITE);
  csv.println();                                                                                                                               ////add time stamp
  csv.print(",");
  csv.print(get_data[0]);//  Current Wind Direction
  csv.print(",");
  csv.print(get_data[1]);//  Wind Speed
  csv.print(",");
  csv.print(get_data[8]);//  Humidity
  csv.print(",");
  csv.print(get_data[9]);//  Dewpoint
  csv.print(",");
  csv.print(get_data[10]);//  Temperature (in Degrees Farenheit)
  csv.print(",");
  csv.print(get_data[11]);//  Current Daily Rain Fall
  csv.print(",");
  csv.print(get_data[13]);//  Pressure
  csv.print(",");
  csv.print(get_data[14]);//  Solar Radiation
  csv.print(",");
  csv.print(get_data[15]);//  Additional Temperature (in Degrees Farenheit) for use of monitoring a PV cell temperature
  csv.print(",");
  if (counter % 8 == 0){
    csv.print(get_data[2]);//  2-minute Wind Gust
    csv.print(",");
    csv.print(get_data[3]);//  Direction at the 2-minute Wind Gust
    csv.print(",");
    csv.print(get_data[4]);//  Average Wind Speed in a two minute interval 
    csv.print(",");
    csv.print(get_data[5]);//  Average Wind Direction in a two minute interval
    csv.print(",");
  }
  else{
    csv.print(",");
    csv.print(",");
    csv.print(",");
    csv.print(",");
  }
  if (counter % 40 == 0){
    csv.print(get_data[6]);//  10-minute Wind Gust
    csv.print(",");
    csv.print(get_data[7]);//  Direction at the 10-minute Wind Gust
    csv.print(",");
  }
  else{
    csv.print(",");
    csv.print(",");
  }
  if (mode == 2){
    csv.print(get_data[12]);//  Total Daily Rain Fall
  }
  else{
    csv.print(",");
  }
  if (success == 1){
    csv.print("Success");
  }
  else{
    csv.print("FAILURE");
  }
  csv.close();
}


void syncRTC(){                                                                                                                              ///Write this

}


void dailyMaintainence(){
  File logFile = SD.open("log.txt", FILE_WRITE);
  logFile.print("Executed daily maintainence at :");
  //logFile.print(currentTime)
  logFile.close();

  syncRTC();
                                                                                                                                           ////////////////////////Daily rain here
  post(2);
  
  file_name();
  File csv = SD.open(file_today_name, FILE_WRITE);
  csv.print("Time,Wind Dir,Wind Speed,Humidity,Dewpoint,Temp,Rainin,Pressure,Solar Radiation,Panel Temp,2Min Gust,2 Min Gust Dir,Avg 2 Min,Avg 2 Min Dir,10 Min Gust,10 Min Gust Dir,Daily Rain,Success?");
  csv.close();
                                                                                                                                    ///handle when space is low

}

String readIn(String fileNameString) {                                                                                                     //Test this
  int str_len = fileNameString.length() + 1;
  char fileName[str_len];
  fileNameString.toCharArray(fileName, str_len);
  String content = "";
  File aFile = SD.open(fileName, FILE_WRITE);
  while (aFile.available()) {
    char c = aFile.read();
    content += c;
  }
  aFile.close();
  return (content);
}

void file_name()//function to name the text files each day...
{
  
  char s_day[9],s_month[9],s_year[9];// Variables for storing the conversion from int to string
  const char zero[]="0";// Zero to add when ever the month is 9.. to be 09, so you always have the same string lenght

  
 itoa(day(),s_day,10);// convert day to string
 itoa(month(),s_month,10);  //convert month to string
 itoa(year(),s_year,10);  //convert year to string
 
            for(int i=0;i<30;i++)// wipe the file name to and empty string
           {                     // if not it will keep appending the string in the copy functions
             file_today_name[i]='\0';
                
            }
       

 if(month()<10);strcat(file_today_name,zero);// add trailing zeros for month
 strcat(file_today_name,s_month);
 if(day()<10);strcat(file_today_name,zero); // add trailing zeros for day
 strcat(file_today_name,s_day); // concatenate strings
 strcat(file_today_name,s_year);//concatenate strings
 strcat(file_today_name,".csv");// file name stored in file_today with .txt
}


void freeSpaceAlert(){
  
}





//////////////////////Everyting past here is Ryan's and works////////////////////////////////////////








int averageAnalogRead(int pinToRead){ //Computes the average reading of the wind 
  byte numberOfReadings = 8;
  unsigned int runningValue = 0; 

  for(int x = 0 ; x < numberOfReadings ; x++)
    runningValue += analogRead(pinToRead);
  runningValue /= numberOfReadings;
  return(runningValue);  
}

int get_wind_direction() {
  int adc = averageAnalogRead(vane); // get the current reading from the sensor

  // The following table is ADC readings for the wind direction sensor output, sorted from low to high.
  // Each threshold is the midpoint between adjacent headings. The output is degrees for that ADC reading.
 
  //if (adc < 75) return (113);
  //else if (adc < 88) return(68);
  if (adc < 110) return(90);
  //else if (adc < 156) return(158);
  else if (adc < 214) return(135);
  //else if (adc < 265) return(203);
  else if (adc < 346) return(180);
  //else if (adc < 433) return(23);
  else if (adc < 530) return(45);
  //else if (adc < 615) return(248);
  else if (adc < 666) return(225);
  //else if (adc < 744) return(338);
  else if (adc < 807) return(0);
  //else if (adc < 886) return(293);
  else if (adc < 900) return(315);
  else if (adc < 962) return(270);
  else return(-1);
}

float array_wind_direction2(){
  for (int x = 7; x > 0; x--)
    array_wind_direction_2[x] = array_wind_direction_2[x-1]; //Shifts all values in by one
  int temp = get_wind_direction();
  array_wind_direction_2[0] = temp; //Stores the current wind direction as the zero index
}

float average_direction(){
  //Returns the average value of the wind speed in a 2-min interval
  array_wind_direction2();
  float d_sum_2 = array_wind_direction_2[0];
  float d_Avg_2 = 0;
  for (int i = 1; i < 8; i++) //Computes the sum of the wind_direction reading in 2-minutes
    d_sum_2 = d_sum_2 + array_wind_direction_2[i]; 
  d_Avg_2 = d_sum_2 / 8; //Computes the average
  return(d_Avg_2);
}

void rain(){
  //Each time the reed switch of the rain sensor is switched, increments a counter
  r_count++;
}

void Wind(){
  //Every revolution of the anemometer increments a counter
  count++;
}

float rain_fall(){
  //Uses variable r_count to compute rainfall
  //Returns rainfall in in.
  r_fall = r_count*.011;
  return(r_fall);
}

float daily_rain(){
  //Resets the rainfall counter after a day has passed
  //Returns daily rainfall
  float daily_rain = r_fall;
  r_count = 0;
  return(daily_rain); 
}
  
float Wind_speed(){
  //Computes the current wind speed
  //Finds the 2-min and 10-min wind gust
  //Updates array of wind speed
  //Returns current wind speed
  float wind_dir = get_wind_direction();
  max_2 = 0;
  max_10 = 0;
  for (int x = 39; x > 0; x--){ // First test for 2-min and 10-min wind gust
    if (max_10 < array_wind_10[x-1])
      max_10 = array_wind_10[x-1];
    if (x <= 7){
      if (max_2 < array_wind_10[x-1])
        max_2 = array_wind_10[x-1];
    }
    array_wind_10[x] = array_wind_10[x-1]; //Shifts values in the array by an index of 1
  }
  array_wind_10[0] = count*1.492/15; //Computes wind speed in mph
  count = 0;
  if (max_2 < array_wind_10[0]){
    max_2 = array_wind_10[0];
    gust_dir_2 = wind_dir;}
  if (max_10 < array_wind_10[0]){
    max_10 = array_wind_10[0];
    gust_dir_10 = wind_dir;}
  return(array_wind_10[0]);
}

float Average_2(){
  //Returns the average wind speed in a 2-min interval
  float sum_2 = array_wind_10[0];
  float Avg_2;
  for (int i = 1; i < 8; i++)
    sum_2 = sum_2 + array_wind_10[i]; //Computes sum of wind speeds
  Avg_2 = sum_2 / 8; //Averages the wind speed
  return(Avg_2);
}

float Average_10(){
  //Returns the average wind speed in a 2-min interval
  float sum_10 = array_wind_10[0];
  float Avg_10;
  for (int i = 1; i < 40; i++)
    sum_10 = sum_10 + array_wind_10[i]; //Computes sum of wind speeds
  Avg_10 = sum_10 / 40; //Averages the wind speed
  return(Avg_10);
}

float DHT_temp(){
  //Returns the temperature (in degrees Farenheit) reading from the sensor DHT22
  //May or may not be used
  float t = dht.readTemperature();
  float tf = (t*9/5)+32;
  return(tf);
}

float DHT_humid(){
  //Returns the humidity reading from the sensor DHT22
  float h = dht.readHumidity();
  return(h);
}

float LM35(){
  //Returns the Temperature (in degrees Farenheit) reading from the sensor LM35
  int reading = analogRead(sensorPin);  
  float voltage = reading * (5.0/1024);
  float temperatureC = (voltage*100);
  float temperatureF = (temperatureC * 9.0 / 5.0) + 32.0;
  return(temperatureF); 
}

float Temperature(){
  //Returns the Temperature (in degrees Farenheit) reading from the sensor BMP180
  float Ctemp = bmp.readTemperature();
  float Ftemp = (Ctemp * 9.0/5)+32;
  return(Ftemp);
}

float Pressure(){
  //Returns the Pressure in in. Hg
  float P = bmp.readPressure();
  float P_inHg = P * .0002953;
  return(P_inHg);
}

float Altitude(){
  //Returns the altitude in meters
  //Not necessarily needed
  float A = bmp.readAltitude();
  float Real_A = bmp.readAltitude(101500);
  return(A);
  //  Serial.print("Real altitude = ");
  //  Serial.print(Real_A);
  //  Serial.println(" meters");
}

float dewPoint(float tempf, float humidity){
  //Returns the dewPoint (in degrees Farenheit)
  //Temperature used is from the BMP 180
  //Humidity used is from the DHT22
  float A0= 373.15/(273.15 + tempf);
  float SUM = -7.90298 * (A0-1);
  SUM += 5.02808 * log10(A0);
  SUM += -1.3816e-7 * (pow(10, (11.344*(1-1/A0)))-1) ;
  SUM += 8.1328e-3 * (pow(10,(-3.49149*(A0-1)))-1) ;
  SUM += log10(1013.246);
  float VP = pow(10, SUM-3) * humidity;
  float T = log(VP/0.61078);   
  return((241.88 * T) / (17.558-T));
}

float Radiation(){
  //Returns the Solar Irradiance in W/m2
  return(0);
}

void grab_data(){
  //Gathers all of the necessary data and puts it into an array
    get_data[0] = get_wind_direction();
    get_data[1] = Wind_speed();
    get_data[2] = max_2;
    get_data[3] = gust_dir_2;
    get_data[4] = Average_2();
    get_data[5] = average_direction();
    get_data[6] = max_10;
    get_data[7] = gust_dir_10;
    get_data[8] = DHT_humid();
    get_data[9] = dewPoint(Temperature(), DHT_humid());
    get_data[10] = Temperature();
    get_data[11] = rain_fall();
    get_data[12] = daily_rain();
    get_data[13] = Pressure();
    get_data[14] = Radiation();
    get_data[15] = LM35();
}
