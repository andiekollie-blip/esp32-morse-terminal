# Wireless Morse Code Terminal (ESP32 + CYD)
Wireless Morse code terminal using ESP32 and a cheap yellow display.

wireless Morse code communication system built using two ESP32 microcontrollers.
One ESP32 captures Morse code input via a button and transmits it wirelessly using ESP-NOW.
A second ESP32 with a Cheap Yellow Display [CYD] receives, decodes, and displays both raw Morse and decoded text in real time.

 [Features]                                              

- Button-based Morse code input (dots & dashes)           
- Real-time buzzer + LED feedback                        
- Wireless transmission via ESP-NOW
                      
 [Why ESP-NOW]
 - fast, peer-to-peer communication between ESP32 devices                                                       
Cuts out:                                                          
- Wi-Fi infrastructure
- Bluetooth pairing
 - Internet access
   
[CYD terminal display]                                     
- Raw Morse stream
- Decoded text shown
- Clear screen using BOOT button
- Low latency & no Wi-Fi router required




 [Hardware-Used]
- Component	Quantity
- ESP32 Dev Board:	            2
- Momentary Push Button:	      1
- Active Buzzer:	              1
- LED + resistor:	              1
- Cheap Yellow Display (CYD): 	1

[How It Works]

Sender (ESP32)

* Button presses are timed:
  - Short press → dot (.)
  - Long press → dash (-)

* Gaps between presses determine:
    - / → End of letter 
    - | → End of word

* Morse symbols are sent wirelessly via ESP-NOW
* LED and buzzer provide immediate feedback while pressing

* Receiver (CYD)
  - Receives Morse symbols over ESP-NOW
  - Displays:
    - RAW Morse input (dots & dashes)
    - Decoded text below

- Uses a built-in Morse lookup table (A–Z, 0–9)
- Pressing the BOOT button clears the terminal

[Libraries Used]
* ESP32 Arduino Core
* esp_now
* WiFi
* TFT_eSPI (for CYD display)

| Libraries are installed via the Arduino Library Manager.

| No third-party code is included in this repository.
