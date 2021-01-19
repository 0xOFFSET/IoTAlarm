# IoTAlarm

A simple Clock Alarm built-on arduino-like microcontroller called NodeMCU that based on ESP8266 WiFi chip.
It may be regarded as a replacement for the traditional clock alarms. As name implies, it tends to make an action upon running the alarm.
Actions may be like toggling a relay, running a motor or even sending any kind of signal/message at the abstarct level.

## How can it be helpful?
Suppose you have rough deadlines on the cold days of the winter, and you simply need to wake up at night.
Hence, the chip is connected with a servo motor, and plug all these on your room lamp's switch. 
Set your timer, wait for the time. Then, interestingly, servo motor turns on the lights.

There may be many useful usecase for sure.

## How does it work?
The Code is splitted into several mode to facilitate the flow of execution:
* NORMAL 		(The default mode)
* SET_ALARM 	(Change alarm time)
* STOP_ALARM	(deactivate the alarm)
* RUN_ALARM		(execute the required action)
* IDLE			(Go to Deep Sleep Mode to save power)

1- The Chip connects to your local gateway via WiFi.
2- Fetch current time via "NTP" protocol.
3- Set alarm time, and save it permanently on "EEPROM".
4- Create timestamps for both current time and alarm time.
5- Compare timestamps, if timer isn't over yet, go to sleep mode to save power.
6- When time is over, go to **RUN_ALARM** Mode.
6- Execute **runAlarm()** procedure, where you define your desired action.

## What about performance/accuracy ?
To save power consumption, the chip goes to Deep Sleep Mode to save power, where most of the chip's functions are suspended including WiFi connection.
During sleep mode, the microcontroller continues to count up to certain mintues then back to the Normal mode to check current time again.

In this scenario, there're two advantages:
* No need to use External "RTC" unit to keep the time, as we fetch the current time periodically after each sleep period.
* We don't worry about the accuracy of time after serveral hours, as it's frequently updated.

## How to run?
1- Open Arduino Program.
2- Download and open this sketch.
3- Connect your nodeMCU and choose the appropiate serial port.
4- Check NodeMCU driver for your Operating System.
5- Install ESP8266 boards from board manager.
6- Install the missing libraries.
7- Compile!
8- Open serial monitor with a "115200" baud rate.

## Beta version
This's just a demo, to set/change alarm time, you have to modify these global variables according to desired hour, minute and second values respectively:
* ALARM_H 
* ALARM_M  
* ALARM_S

## What Next ?
* Run the microcontroller as a server, acces it via certain IP address in order to set/modify alarm settings.