This is a manual to flash and use the tag-setup in visual studio code.

It is recommended to use VSC instead of Arduino, because the important libraries are already defined in this repo.

Click on the [VSC-link](https://code.visualstudio.com/download) to go to the site to install Visual Studio Code.

After VSC is downloaded an important extension has to be installed to flash hardware through VSC. 

![Go to the tab extensions and fill in platformIO](PlatformExtension.png).
Go to the tab extensions and fill in platformIO

Flashing
-
There are 2 kinds of devices used in this project; the Tag and the Anchor.

To flash the Tag or Anchor go to [Settings.cpp](src/Settings.cpp)(line 18).

TAG

To flash the tag properly certain options have to be set the right way. At this moment de UWB-settings are set correctly, but the WiFi-settings aren't working. This is because the SSID and Password are not set for the right Router. To change these settings go to [WiFi.cpp](src/Wifi.cpp)(line 123).

Make sure that the value of the constant ANTENNA_DELAY is 16385. This value can be found at [Settings.cpp](src/Settings.cpp)(line 152). This constant is needed to get a distance value from de DW1000 and changing this constant will make the product less accurate.   

ANCHOR

To flash the anchors it is important that the right anchor number is undefined in (src/Settings.cpp) (line 94, 101, 108, 115). Check on the back of the 3d-print of the anchor you want to flash which number it contains and uncomment that line.

Every anchor also has a ANTENNA_DELAY constant. The best test results were with the values those constant have at this moment. 

RUNNiNG
-
When all devices are connected to a power supply check the little screen on the tag. This screen will count how many anchors it has found with the following format: Active: [number of anchors found]. 

If the number is lower than the amount of anchors connected something went wrong with flashing. It could also be the case that something is blocking the signal between the anchor and tag or the distance between them is to high. On the small screen of the tag you can see which anchor numbers are connected so the anchor with issues can be filtered out easily by the user.

When Active: >= 3 the tag is ready to send coordinates. This can be done by using the small menu which is controlled by three buttons. The way the menu works will be discussed underneath.

MENU

The menu consists out of 4 options that can be changed. To scroll through those options use the 'up'(left button) or 'dwn'(right button) buttons. To enter the option press the 'enter'(middle button). Let's say you want to Start sending data:

- Scroll untill you see Start_send.

- Press the enter button and check of the screen says Start_send = on.

Now the data will be send over UART and displayed on the small screen. If you do not see any coordinates printed on the small screen one of the previous steps went wrong.

uuhdehviuhdfjb

