# 7-channel multi-tester
This simple device features an Ohm-meter, a diode-meter and a voltmeter with the (+) and (-) leads switched to several pin positions of a built-in DB-15 and HIROSE HR10A-10R-12P(73) connectors. The switching is done automatically by implementing seven internal relays controlled by a ESP32 device.

If you need to update the firmware of this device, please download the folder "DAFNE_TEST_v1-3". You need to have installed the Arduino IDE 1.8 or superior, downloadable from: arduino.cc. After that install the ESP32 compatibility for the Arduino IDE, following the next steps:

Open Arduino Preferences:
Open the Arduino IDE, go to the "File" menu, and select "Preferences."

Add the ESP32 Board Manager URL:
In the "Additional Boards Manager URLs" field, enter the following URL:

https://dl.espressif.com/dl/package_esp32_index.json
If there are already URLs in the field, separate them with a comma.

Install the ESP32 Board:
Click "OK" to close the Preferences window. Next, go to the "Tools" menu, select "Board," then "Boards Manager."

Install ESP32 Platform:
In the Boards Manager, type "ESP32" into the search bar. You should see an entry for "esp32 by Espressif Systems." Click on it, and an "Install" button will appear. Click "Install" to download and install the ESP32 platform.

Select ESP32 Board:
Once the installation is complete, close the Boards Manager. Now, go to the "Tools" menu, select "Board," and choose the "ESP32 Dev Module"

Select COM Port:
Connect the device to your computer using a USB cable. Then, in the "Tools" menu, select the appropriate COM port for your ESP32 board under the "Port" option.

Upload the Arduino code:
Open the "File" menu, select "Open," then choose "DAFNE_TEST_v1-3.ino" inside the folder "DAFNE_TEST_v1-3" and upload it by pressing the arrow (Upload).
