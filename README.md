## Title:
       Smart Weight Measuring Demo using ESP32 and HX711. 
       This demo shows the smart application to measure your own weight using ESP and HX711.
       The current weight can be displayed on LED and Blynk cloud. User can also interact witht the demo 
       using web interfacee, such as taring (rezeroing) the scale or displaying the current weight. 
       
## Prerequisites
  # Softwares  
    - Arduino IDE or PlatfiormIO with Visual Code.
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

   # HX711 circuit wiring
        - DOUT pin from HX711 is conencted to the pin  21  from the ESP32 MCU 
        - SCK  pin from HX711 is conencted to the pin  20  from the ESP32 MCU
  # Load cell to XH711 circute wiring 
     - Red --> E+
     - Black --> E-
     - White --> A+
     - Green --> A-
    
## Get the code  
   - Create your own folder in your own location and cd to move to your project. 
   - Clone the repository:
         git clone https://github.com/kp003919/SmartScaleRTOS_Demo.git
   - Open arduino IDE or PlatforIO with Visual code

##  Usage
 To use the Smart Scale demo, follow these steps:

    - Connect your demo to your PC or Laptop. 
    - Open Arduion or PlatformIO 
    - Run demo 
    - Put your well known weight on the load cell. 
    - You should be able to see your weight on the LED display. 
    - Open the serial monitor to get your local IP for web interface. 
    - Open your browser and past your IP. 
    - Open your Blynk account and you should see your weight 
    - Try again with different weights. 


    
    
  
