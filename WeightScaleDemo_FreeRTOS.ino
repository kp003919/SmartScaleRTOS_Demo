/**
 * A smart weight measuring application using ESP32 and HX711.
 */

// libraries 
#include<WiFiMulti.h>  // wifi 
#include<WiFi.h>       
#include <TM1637.h>    // Display 
#include "HX711.h"     // Load cell 
#include <freeRTOS.h>   // freeRTOS 
#include<semphr.h>       // semaphore handler 
#include <WiFiClient.h>   // client requests

// define variables 
// Blyn Cloud template id and name 
#define BLYNK_TEMPLATE_ID "TMPL54GiWGKuC"
#define BLYNK_TEMPLATE_NAME "Muhsin"
#define BLYNK_AUTH_TOKEN "0U5MXMqH_NVYl6XEhvD7EyDtZoQxAXmD"
#define BLYNK_PRINT Serial

// Load cell variables 
#define maxScaleValue 5000         // load cell maximum weight is 5k = 5000grams.
#include <BlynkSimpleEsp32.h>  // Blynk in ESP32 

//Display circuit wiring 
#define CLK_PIN    48     // GPIO PIN 48 from the ESP32 MCU 
#define DIO_PIN    47     // GPIO PIN 47 from the ESP32 MCU 

// HX711 circuit wiring
#define DOUT_PIN  21           // GPIO PIN 21 from the ESP32 MCU 
#define SCK_PIN   20           // GPIO PIN 20 from the ESP32 MCU

// Access Provider (AP) details
#define AP_NAME  "VM1080293"       // AP name 
#define AP_PASS "Omidmuhsin2015"  // AP password 

// task handels for freeRTOS.
TaskHandle_t TaskHandle_1;  // get weight task 
TaskHandle_t TaskHandle_2;  // display weight task 
TaskHandle_t TaskHandle_3;  // web server task
TaskHandle_t TaskHandle_4;  // Blyn Task
SemaphoreHandle_t semaphore; 
const int shared_resource = 3; 

// Load cell reader 
HX711 scaleReader; 
// Blynk timer 
BlynkTimer timer;
// calibration factor to be used. 
float calibration_factor = -376.99; // Calibration factor to get the well known weight 
                                    // This number works for me.  
                                    // right value of the scale. Calibration factor = (reading \ known weight). 
                                  

// web server listening on port 80.
WiFiServer server(80); //web server listening on port 80.
// Variable to store the HTTP request
String header;
 long static  currentWeight = 0; // current weight 
// Current time 
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 10000; // 10 seconds
// display provider 
TM1637 displayScale(CLK_PIN,DIO_PIN);
WiFiMulti wifiMulti;  // wifi access


//******************************************************************************************** Help functions *********************************************************************

/**
 * Calibration process: 
 */

float getCalibrateFactor()
{
  // Step1: get raw reading with no scale factor set.
   // weight is 0 
    Serial.println("Do not put any weight");
    scaleReader.set_scale();    //no calibration 
    scaleReader.tare();    // removing weight on the scale
    float raw_reading1 = scaleReader.read_average(10); // zero weight reading 
    delay(5000);
    Serial.println("Put on your weight ");
    // my known weight is 50.0g 
    float myWeight = 50.0; //y2, y1 =0.
   // step 2: get another raw reading when your weight is on the scale 
    float raw_reading2 = scaleReader.read_average(10); //x2
    delay(5000);
    // factor = (y2 - y1) / (x2 - x1)
    float calibration_factor = (myWeight - 0.0)/(raw_reading2-raw_reading1);
    return calibration_factor;
  
}

/**
 * @Desc: This function puts weignt to the scale and return it. Max value is expected to be between 
 *        0 and 5000 g (5k). 
 * @return:  - If reading > max, error and return 0 
 *           - if reading <= 0, return 0 
 *           - return reading otherwise. 
 */

long  getWeight()
{  
      
    if (scaleReader.is_ready()) 
      {         
          // set calibration factor          
          scaleReader.set_scale(calibration_factor);
          // get reading from the load cell          
          long gram_reading = scaleReader.get_units();
          Serial.println(""); 
          Serial.print("Reading: ");
          Serial.println(gram_reading); 
          // reading must be less than max scale value.
          if ( (gram_reading > maxScaleValue) || (gram_reading <0) )
              return 0;                       // overweight 
          else  
              return gram_reading;           // return the reading 
      } 
      else 
      {
          Serial.println("Scale is not ready yet...");
          delay(1000); 
          return 0;
      } 
}


/**
 * Desc: Reseting the display with ----.
 *    LED will display ---- to display the accurate value of the measured weight. 
 */

void resetDisplay()
{
    displayScale.display("----"); // reset display
}

/**
 * Desc: Display the given weight to the 4 digits 7-segment display. 
 * @para  weightVal a value to be displayed. 
 */
void displayWeight(long weightVal)
{ 
        // reset the display first so that we can have a clear display.
        // when digit is not displayed, it is assumed to be "-". 
        resetDisplay();
        displayScale.display(weightVal);
}

//*************************************************************************** Web Server ****************************************************************************
/**
 * @Desc:  Response to client requests, if there is any. 
 * Reference [Rui Santos]. 
 */
void runClientRequest()
{   
   // get current time 
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";  
    String action; 

    // check if there is any request from any clients. 
    WiFiClient client = server.available();   // Listen for incoming clients   
    if (client) {   // if these is any requests
    Serial.print("New Client: "); 
    Serial.println(client); 
    currentTime = millis();
    previousTime = currentTime;
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();

            // actions to be done remotely. 
            // display LED or taring the scale 
            if (header.indexOf("GET /display") >= 0) 
            {
               Serial.println("Displaying");
               displayWeight(currentWeight);              
            } 
            else 
            {
              Serial.println("Taring");
              scaleReader.tare();
              Serial.println("Taring done");
            } 

             unsigned long elapsedTime = (currentTime - previousTime);
             
             Serial.println(currentWeight);
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the display/taring buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");                  
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #4CAF50;}</style></head>");
            //client.println(".button3 {background-color: #4CAF50;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>Weight Measuring Web Server</h1>");
            // display the LED or taring the scale             
             client.println("<p><a href=\"/display\"><button class=\"button\">Display Weight</button></a></p>");             
             client.println("<p><a href=\"/taring\"><button class=\"button button2\">Taring Scale</button></a></p>");
             client.println("</body></html>");            
            // The HTTP response ends with another blank line
             client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine                                                                                                                                
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
    delay(1000);
  } 
}
//*********************************************************************************************** Blynk Cloud ******************************

/**
 * @Desc:This function sends data to the pins (V0 and V1) of the Blynk. 
 * Updating the current weight to the Blynk cloud over a period of time. 
 * Reference [blynk.io]. 
 */
void runBlynk()
{
  Blynk.virtualWrite(V0,currentWeight); 
  Blynk.virtualWrite(V1,currentWeight);
  delay(1000); 
}


//********************************************************************* freeRTOS Tasks ********************************************************
/**
 * Task1:  getting weight from the load cell. 
 * FreeRTOS is used to run this task and get the current weight from the load cell.  
 */
void Task1( void *pvParameters )
{  
  while (1)
  {  
      // taking the semaphore 
      xSemaphoreTake(semaphore,portMAX_DELAY);    
      currentWeight = getWeight();   // current weight 
      //releasing the semaphore. 
      xSemaphoreGive(semaphore);  
      vTaskDelay(1000/ portTICK_PERIOD_MS); // run per (n) second. 
  }
}

/**
 * Task2: dislpaly the current weight to the LED. 
 */
void Task2( void *pvParameters )
{  
   while(1)
  { 
      // taking the semaphore 
       xSemaphoreTake(semaphore,portMAX_DELAY);
       Serial.println("Task2");
       Serial.println(currentWeight); 
       displayWeight(currentWeight);   
       //releasing the semaphore.  
       xSemaphoreGive(semaphore); 
       vTaskDelay((1000/ portTICK_PERIOD_MS)); // run per (n) second.
  }  
}

/**
 * Task3: runs web server 
 * Web server is ready for any client requests, if there is any. 
 * Web UI is used to display the current weight or tare the scale
 * This web interface (client) can be accessed using this IP address of the server. 
 * After program is excuted, you can see the IP address from the Serial Monitor.  
 * Copy this IP address and paste it into your browser. 
 */
void Task3( void *pvParameters )
{   
  while (1)
  {
      // taking the semaphore 
      xSemaphoreTake(semaphore,portMAX_DELAY);
      Serial.println("Task3");
      runClientRequest();
      xSemaphoreGive(semaphore);  //releasing the semaphore. 
      vTaskDelay((1000/ portTICK_PERIOD_MS)*5); // run per (n) second.
  }
}

/**
 * Sends the current weight to the Blynk using pins V0 and V1.
 * Blynk account must be created and a new template must be designed before using this function. 
 * The current weght is displayed on the Blynk cloud over time(5 second).  
 * Open your Blynk account and you should be able to see the weight. 
 */
void Task4( void *pvParameters )
{   
  while (1)
  {
    // this task take the semaphore 
     xSemaphoreTake(semaphore,portMAX_DELAY);
     Serial.println("Task4");
     Blynk.run();
     timer.run();
     xSemaphoreGive(semaphore);  //releasing the semaphore. 
     vTaskDelay((1000/ portTICK_PERIOD_MS)); // run per (n) second.
  }
}



//****************************************************************************** Sep up function  *****************************************************************************

// setup function
void setup() 
{
    // conenct to the available wifi using your AP name and password. 
    WiFi.begin(AP_NAME,AP_PASS); // access wifi 
  
    // set the speed to transfer data in the serial communication. 
    Serial.begin(115200);   // speed = 115200
  
    // 1- Initializing the display (4 digits 7 segment display) 
    Serial.println("Initializing the LED display");
    displayScale.init();
    displayScale.setBrightness(5); // set the brightness (0:dimmest, 7:brightest) 

    //2- Load cell setting and initilization 
    Serial.println("Initializing the scale");
    scaleReader.begin(DOUT_PIN,SCK_PIN);   
  
    scaleReader.set_scale();    //no calibration 
    scaleReader.tare();         // removing any weight on the scale.

    // create a new semaphpre and check if it has been created.
    semaphore = xSemaphoreCreateMutex();
    if (semaphore == NULL)
    {
         Serial.println("Semaphore could not be created!");
    }

   // wifi connection 
    if(WiFi.status() != WL_CONNECTED) 
    {        
        Serial.println("..... Connecting.... \n");
        delay(1000);   
    }  
    
     // This is your local IP address. 
     Serial.print("IP address: ");
     Serial.println(WiFi.localIP());  // you need this IP to access to the web interface
    
    //3 start the web server.
     server.begin();
   
    //4- Blynk interaction
     Blynk.begin(BLYNK_AUTH_TOKEN, AP_NAME, AP_PASS);
     // Blynck starts here. 
     //timer.setInterval(1000L, runBlynk); 
      
   
     //5- Creating different tasks(getting weight, displaying the current weight, web server and blynk interaction.
      xTaskCreate(Task1, "get Weight", 10000, NULL, 1, &TaskHandle_1); // getting weight 
      xTaskCreate(Task2, "Display 1", 10000,NULL, 1, &TaskHandle_2);  
      xTaskCreate(Task3, "Web server", 10000,NULL, 2, &TaskHandle_3);
      xTaskCreate(Task4, "Blynk Cloud", 10000,NULL,3, &TaskHandle_4);

      Serial.println("....Starting .... \n");  
}

//**************************************************************************** main function (loop) *****************************************************

// loop forever (empty).
void loop() 
{   
}
// end of demo 
