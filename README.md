## Title:Smart Weight Measuring Demo using the ESP32 and HX711. 
       This demo shows a smart application to measure the current weight using ESP and HX711.
       The current weight can be displayed on the LED and the Blynk cloud. Users can also interact witht the demo 
       using the web interface, such as taring (zeroing) the scale or displaying the current weight. 
       
## Prerequisites
  # Software  
    - Arduino IDE or PlatformIO with Visual Code.
    - Windows operating system
    - Compile for cpp
    
 # Hardware 
    - ESP32 MCU 
    - 4 digits -7 segment display 
    - Load cell (5k or 10k)
    - HX711 
    - USB cable 
    - Laptop or PC 
    
# Create your Demo structure 
   # Display circuit wiring 
      - CLK  pin from display to   pin 48  from ESP32 MCU
      - DIO   pin from display to   pin 47 from  ESP32 MCU 
      - VCC  TO 5V from ESP32 
      - GND  TO GND from ESP32 

   # HX711 circuit wiring
        - DOUT pin from HX711 is connected to the pin  21  from the ESP32 MCU 
        - SCK  pin from HX711 is connected to the pin  20  from the ESP32 MCU
        - VCC  TO 5V from ESP32 
        - GND  TO GND from ESP32 
        
  # Load cell to XH711 circute wiring 
     - Red   --> E+
     - Black --> E-
     - White --> A+
     - Green --> A-
     
    
## Get the code  
   - Create your folder in your own location and use cd to move to your project folder. 
   - Clone the repository:
         git clone https://github.com/kp003919/SmartScaleRTOS_Demo.git
   - Open the arduino IDE or PlatformIO with Visual code

##  Usage
 To use the Smart Scale demo, follow these steps:

    - Connect your demo to your PC or Laptop. 
    - Open the Arduino IDE or PlatformIO 
    - Add the required libraries
    - Run demo 
    - Put your well known weight on the load cell. 
    - You should be able to see your weight on the LED display. 
    - Open the serial monitor to get your local IP for web interface. 
    - Open your browser and past your IP. 
    - Open your Blynk account and you should see your weight 
    - Try again with different weights. 


    
    
  
