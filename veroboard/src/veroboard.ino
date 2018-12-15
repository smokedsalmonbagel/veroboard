
char compile_date[] = __DATE__;
char compile_time[] = __TIME__;


String dataBuffer = "";
String logBuffer = "";

int timeok = 0;
int readFlag = 0;
int adcFlag = 0;
int sendDataFlag = 0;
int diagFlag = 0;

int publishFlag = 1;


int vbatt =0;
int battlevel = 0;
String rssi = "";

#include <dht.h>

dht DHT1;
dht DHT2;

#define DHT221_PIN A0
#define DHT222_PIN A1

struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} dht1data = { 0,0,0,0,0,0,0,0};


struct
{
    uint32_t total;
    uint32_t ok;
    uint32_t crc_error;
    uint32_t time_out;
    uint32_t connect;
    uint32_t ack_l;
    uint32_t ack_h;
    uint32_t unknown;
} dht2data = { 0,0,0,0,0,0,0,0};


int temp1 = -500;
int rh1 = -500;
int temp2 = -500;
int rh2 = -500;

int adclen = 4;
int adcpin[] = {A2, A3, A4, A5};
long adcsum[] = {0,0,0,0};
long adcsamples[] = {0,0,0,0};


long loopCount;

//system events
unsigned long  count_network_status_powering_on = 0;
unsigned long  count_network_status_on = 0;
unsigned long  count_network_status_powering_off = 0;
unsigned long  count_network_status_off = 0;
unsigned long  count_network_status_connecting = 0;
unsigned long  count_network_status_connected = 0;

unsigned long count_cloud_status_connecting = 0;
unsigned long count_cloud_status_connected = 0;
unsigned long count_cloud_status_disconnecting = 0;
unsigned long count_cloud_status_disconnected = 0;

unsigned long count_dht1_fail = 0;
unsigned long count_dht2_fail = 0;

void readDHT1()
{
	uint32_t start = micros();
    int chk = DHT1.read22(DHT221_PIN);
    uint32_t stop = micros();

    dht1data.total++;

    if (chk == DHTLIB_OK)
		{
        dht1data.ok++;
				temp1 = ((DHT1.temperature * 9/5) + 32) *10;
				rh1 = DHT1.humidity * 10;
      }
			else
			{
				count_dht1_fail += 1;
				temp1 = -500;
				rh1 = -500;
			}
}
void readDHT2()
{
	uint32_t start = micros();
    int chk = DHT2.read22(DHT222_PIN);
    uint32_t stop = micros();

    dht2data.total++;

    if (chk == DHTLIB_OK)
		{
        dht2data.ok++;
				temp2 = ((DHT2.temperature * 9/5) + 32) *10;
				rh2 = DHT2.humidity * 10;
      }
			else
			{
				count_dht2_fail += 1;
				temp2 = -500;
				rh2 = -500;
			}
}

void logNow(int d=0)
{
	String hd = String(millis());
	hd += String(",");
	hd += String(getCurrentDateTime());
	 hd += String(",");
	hd += logBuffer;
	logBuffer = hd;


	if(d ==3)//only log to Serial1
	{
		Serial1.println(logBuffer);
		logBuffer = "";
	}
	else if(d == 0)//log to serial and file
	{
		String line = logBuffer.substring(0);
		Serial1.println(logBuffer);
		logBuffer = "";

	}
}
void initadc()
{
	int n=0;
 while(n < adclen)
 {
	 pinMode(adcpin[n],INPUT);
	 n++;
 }

}
void sampleadc()
{
	int n=0;
 while(n < adclen)
 {
	 adcsum[n] += analogRead(adcpin[n]);
 	 adcsamples[n]++;
	 n++;
 }

}
void clearadc()
{
	int n=0;
 while(n < adclen)
 {
	 adcsum[n]=0;
 	 adcsamples[n]=0;
	 n++;
 }
}

String getCurrentDateTime()
{
		time_t mtime = Time.now();
		//Time.zone(-5.0);
	return Time.format(mtime, "%Y/%m/%d %H:%M:%S");
	 //return Time.timeStr();
}

String getCurrentDate()
{
		time_t mtime = Time.now();
		//Time.zone(-5.0);
	return Time.format(mtime, "%Y/%m/%d");
	 //return Time.timeStr();
}

String getCurrentTime()
{
		time_t mtime = Time.now();
		//Time.zone(-5.0);
	return Time.format(mtime, "%H:%M:%S");
	 //return Time.timeStr();
}
void set_dt_from_net()
 {


     logBuffer += String(",");
     logBuffer += String("setting_dt,");
     logBuffer += String(getCurrentDateTime());
     logNow(0);

     Particle.syncTime();
     long to = 5000; //timeout for getting network datetime
     long start = millis();

     //sync from network with timeout
     while(millis() - start < to && Particle.syncTimePending())
     {
       Particle.process();
     }
     if (Time.isValid() == true)
     {

         logBuffer += String(",");
         logBuffer += String("set_dt_ok");
         logNow(0);

       RGB.control(true);
       RGB.color(255, 255, 255);
       delay(100);
       RGB.color(0, 0, 0); //turn off
       delay(300);
       RGB.color(255, 255, 255); //turn on
       delay(100);
       RGB.color(0, 0, 0); //turn off
       delay(300);
       RGB.color(0, 255, 0); //turn on
       delay(100);
       RGB.color(0, 0, 0); //turn off
       RGB.control(false);
       timeok = 1;
     }
     else
     {

         logBuffer += String(",");
         logBuffer += String("set_dt_failed");
         logNow(0);

       RGB.control(true);
       RGB.color(255, 255, 255);
       delay(100);
       RGB.color(0, 0, 0); //turn off
       delay(300);
       RGB.color(255, 255, 255); //turn on
       delay(100);
       RGB.color(0, 0, 0); //turn off
       delay(300);
       RGB.color(255, 0, 0); //turn on
       delay(100);
       RGB.color(0, 0, 0); //turn off
       RGB.control(false);
       timeok = 0;
     }
 }
 void handle_all_the_events(system_event_t event, int param)
 {
   if (event == 32)
   {
     if(param == network_status_powering_on  )
        count_network_status_powering_on += 1;
     else if(param == network_status_on)
         count_network_status_on+=1;
     else if(param == network_status_powering_off)
         count_network_status_powering_off+=1;
     else if(param == network_status_off)
         count_network_status_off+=1;
     else if(param ==network_status_connecting  )
         count_network_status_connecting +=1;
     else if(param ==network_status_connected )
         count_network_status_connected+=1;


       logBuffer += String(",device_event,");
       logBuffer += String(int(event));
       logBuffer += String(",");
       logBuffer += String(int(param));
       logNow(0);

   }
   else if(event == 64)
   {
     if(param == cloud_status_connecting)
         count_cloud_status_connecting+=1;
     else if(param == cloud_status_connected)
         count_cloud_status_connected+=1;
     else if(param ==cloud_status_disconnecting )
         count_cloud_status_disconnecting+=1;
     else if(param == cloud_status_disconnected )
         count_cloud_status_disconnected +=1;



       logBuffer += String(",device_event,");
       logBuffer += String(int(event));
       logBuffer += String(",");
       logBuffer += String(int(param));
       logNow(0);

   }
 }


int sendSystem(String command)
{
	  unsigned long dts = Time.now();
	String sysBuffer = String(dts);
	sysBuffer += ",";
	sysBuffer += String(rssi);
	sysBuffer += ",";
	sysBuffer += String(vbatt);
	sysBuffer += ",";
	sysBuffer += String(battlevel);
	sysBuffer += ",";
	sysBuffer += String(count_dht1_fail);
	sysBuffer += ",";
	sysBuffer += String(count_dht2_fail);
sysBuffer += ",";
	sysBuffer += String(count_network_status_off);
	sysBuffer += ",";
		sysBuffer += String(count_network_status_on);


	if(publishFlag || command == "f")
	{
	 Particle.publish("system", sysBuffer);
 }
 logBuffer += sysBuffer;
 logNow(0);
  return 1;
}

int sendData(String command)
{

	makeDataBuffer();
	if(publishFlag || command == "f")
	{
	 Particle.publish("readings", dataBuffer);
 }

	 clearadc();
  return 1;
}
void makeDataBuffer()
{

	  unsigned long dts = Time.now();
	dataBuffer = String(dts);
	dataBuffer += String(",");
		int n=0;
	 while(n < adclen)
	 {
		 int avg = -1;
		 if(adcsamples[n] > 0)
		 {
		 	 avg = adcsum[n] / adcsamples[n];

		}

		dataBuffer += String(avg);

		 dataBuffer += String(",");

		 n++;
	 }

	 dataBuffer += String(adcsamples[0]);
	  dataBuffer += String(",");
	 dataBuffer += String(temp1);
	dataBuffer += String(",");
	dataBuffer += String(rh1);
	dataBuffer += String(",");
	dataBuffer += String(temp2);
	dataBuffer += String(",");
	dataBuffer += String(rh2);


}
void sendDataInt()
{
	sendDataFlag = 1;
}
void sampleSlow()
{
	readFlag=1;
}

void sampleFast()
{
	adcFlag = 1;
}
void sendDiag()
{
	diagFlag = 1;
}
void testadc()
{


	logBuffer = "";
		int n=0;
	 while(n < adclen)
	 {
		 logBuffer += String(analogRead(adcpin[n]));
		 logBuffer += String(",");

		 n++;
	 }
	 logNow(0);


}
int setPubFlag(String command)
{
	if(command == "1" || command == "True")
	{
		publishFlag = 1;

	}
	else
	{
		publishFlag =0;
	}
	return publishFlag;
}

Timer sampleSlowTimer(10000, sampleSlow);
Timer sampleFastTimer(1000, sampleFast);
Timer sendTimer(900000, sendDataInt);
Timer diagTimer(900000, sendDiag);

void setup()
{
	Serial1.begin(57600);

	System.on(all_events, handle_all_the_events);

	Particle.variable("vbatt", vbatt);
	Particle.variable("rssi", rssi);
	Particle.variable("battlevel", battlevel);
	Particle.function("sendSystem", sendSystem);
	Particle.function("sendData", sendData);
	Particle.function("publishFlag", setPubFlag);

	 initadc();

	logBuffer += String(",");
     logBuffer += String("___NEW_LOG___");
     logBuffer += String(",");
     logBuffer += String("compiled,");
     logBuffer += String(compile_date);
     logBuffer += String(",");
     logBuffer += String(compile_time);
     logNow(0);

 set_dt_from_net();

	loopCount = 0;
	//delay(2000);
	sampleSlowTimer.start();
	sampleFastTimer.start();
	sendTimer.start();
	diagTimer.start();
	sampleFast();
	sampleSlow();

}

void loop()
{
	CellularSignal sig = Cellular.RSSI();
	rssi = String(sig);
	if (millis() - Particle.timeSyncedLast() > (1000 * 60 * 60))
 {
	 set_dt_from_net();
 }

 if (readFlag == 1)
 {
	 readFlag = 0;
	 readDHT1();
 		readDHT2();
		makeDataBuffer();
		logBuffer = dataBuffer;
		logNow(0);

 }
 if(adcFlag == 1)
 {
	 adcFlag = 0;
	 sampleadc();
	 FuelGauge fuel;
	 vbatt = (fuel.getVCell() * 100);
	  battlevel = (fuel.getSoC() * 10);
 }
 if(sendDataFlag == 1)
 {
	sendDataFlag = 0;
	sendData("");
 }
 if(diagFlag == 1)
 {
 	diagFlag = 0;

 		sendSystem("");
 }
//Serial1.println("DEBUG2");
//testadc();

}
