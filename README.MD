The readme file will follow later. For now it is used as a scrapbook.

Code from https://ryanboland.com/blog/reading-rc-receiver-values
Original code is from Ryan Boland and adapted for my own needs.

https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/rc%20receiver

Board	                            Digital Pins Usable for Interrupts
Uno, Nano, Mini, other 328-based	2, 3
Uno WiFi Rev.2	                    all digital pins
Mega, Mega2560, MegaADK	            2, 3, 18, 19, 20, 21
Micro, Leonardo, other 32u4-based	0, 1, 2, 3, 7
Zero	                            all digital pins, except 4
MKR Family boards	                0, 1, 4, 5, 6, 7, 8, 9, A1, A2
Due	                                all digital pins    
101	                                2, 5, 7, 8, 10, 11, 12, 13


Restrictions Nano:
Max power per pin 40mA (20mA recommended)
Absolute Max power entire package 200mA
Absolute Max power when RAW powered 150mA

Bron: http://www.dominicdube.com/wp-content/uploads/ProMiniPinout.png