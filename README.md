# AlpacaPi
Astronomy control software using Alpaca protocol on the Raspberry Pi

(C) 2019-2021 by Mark Sproul msproul@skychariot.com

AlpacaPi is an open source project written in C/C++

This project was intended primarily for use on the Raspberry Pi but will work
on most any linux platform.  I do my development and testing on desktop Ubuntu

Use of this source code for private or individual use is granted
Use of this source code, in whole or in part for commercial purpose requires
written agreement in advance.

You may use or modify this source code in any way you find useful, provided
that you agree the above terms and that the author(s) have no warranty, obligations or liability.
You must determine the suitability of this source code for your use.

Redistributions of this source code must retain this copyright notice.


More documentation can be found at  https://msproul.github.io/AlpacaPi/

===================================================

## Getting started:

If you haven''t already downloaded the git repository,

	connect to the directory you want the installation to be in
	(you can move it later if desired)

	>git clone https://github.com/msproul/AlpacaPi.git
    cd AlpacaPi


There is a script setup.sh
run that script
	>./setup.sh

	It will check for various requirements and prompt you to download and install certain libraries.


	install_rules.sh
		this installs the USB rules files in the appropriate directories.
		It needs to be run as root
	>sudo ./install_rules.sh


===================================================

## 3rd Party Libraries:

In most cases AlpacaPi relies on libraries supplied by the vendors to talk to the devices.
These libraries are required for the following devices:

	ATIK cameras
	QHY Cameras
	FLIR Cameras
	SONY Cameras
	ToupTek Cameras
	ZWO ASI cameras
	ZWO EFW Filter Wheel

External libraries are NOT required for

	MoonLite Focusers
	LX200 Telescope mount (not finished)
	SkyWatcher Telescope mount (not finished)
	Calibration control of Flat panel


There is a script for installing many of these libraries

	./install_libraries.sh
It will bring up this screen
The "Present" indicator means that the folder is there and can be installed.
It does NOT mean it is installed.  The script will ask you for each library
that is present if you want to install it or not.
As with everything, this script still has more work needed.
For example, touptec script is only finished for 32bit Arm as of 3/19/2021


		**********************************************
		*        AlpacaPi library installation       *
		*                                            *
		* It is OK to run this script multiple times *
		*                                            *
		* Not all of these libraries are required    *
		* For example, if you don't use QHY cameras  *
		* then the QHY library is not needed         *
		**********************************************

			ASI_lib                 	Present
			AtikCamerasSDK          	Present
			EFW_linux_mac_SDK       	Present
			FLIR-SDK                	Present
			QHY                     	Present
			toupcamsdk              	Present
		*********************************************
		Running on intel x64 : true
		Running on Arm 32 bit: false
		Running on Arm 64 bit: false
		Installing libraries into /usr/lib
		***************************************************************
		AtikCamerasSDK SDK folder found
		Would you like to install AtikCamerasSDK support [y/n]?



===================================================

## Development:

AlpacaPi is written in C and C++ to run on Linux operating systems.
It is built using a conventional Makefile.
The make file has many defines in to enable various features.

Dependencies:
	openCV
	cfitsio

On the Raspberry Pi, some of the drivers require the wiringPi library.
wiringPi is pre-installed on Raspberrian.


There is a lot of documentation that needs to be written and I am working on it
as fast as I can.  If there is a particular part that you need help with or
need better documentation, please let me know and I will try my best to get
more done on that particular part.

There are many different driver modules included in this project, almost every one supported by
Alpaca/ASCOM plus a couple extras I created for my own use.

AlpacaPi has been tested on the following platforms

	Ubuntu 16.04 LTS x86_64

	Raspberry Pi 3 (32 bit)

	Raspberry Pi 4 (32 bit)

	Raspberry Pi 4 (64 bit)

	NVIDIA Jetson Nano (64 bit)

===================================================

## Status:

For those familiar with ASCOM and ASCOM development, I use the CONFORM tool to
verify the workings of my drivers.  Here my current results
(as of Jun 19, 2021)

	Apr  1,	2020	<MLS> CONFORM-filterwheel -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Apr  2,	2020	<MLS> CONFORM-focuser -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Apr  2,	2020	<MLS> CONFORM-rotator -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Apr  2,	2020	<MLS> CONFORM-switch -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Dec  7,	2020	<MLS> CONFORM-dome -> 4 errors, 0 warnings and 0 issues

	Dec  7,	2020	<MLS> CONFORM-telescope -> 0 errors, 0 warnings and 6 issues

	Jan 12,	2021	<MLS> CONFORM-dome/ror -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Jan 20,	2021	<MLS> CONFORM-camera/zwo -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Mar  1,	2021	<MLS> CONFORM-observingconditions -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Apr  6,	2021	<MLS> CONFORM-covercalibrator -> PASSED!!!!!!!!!!!!!!!!!!!!!

	Apr 30,	2021	<MLS> CONFORM-filterwheel/atik -> PASSED!!!!!!!!!!!!!!!!!!!!!

===================================================

## Documentation:

Documentation is available at https://msproul.github.io/AlpacaPi/

When you download AlpacaPi, you get the full documentation in the "docs" folder

===================================================


## Contact:

msproul@skychariot.com

I program embedded systems for a living, I have been programming for over 40 years.
AlpacaPi is open source, but if you find it useful and care to make a donation to my efforts.
A paypal donation to the above address would be appreciated.

I have developed this for the purpose of running my own observatory which I put a lot of money
and time into.
I expect to be running this observatory and keeping this software up to date for a long time.


