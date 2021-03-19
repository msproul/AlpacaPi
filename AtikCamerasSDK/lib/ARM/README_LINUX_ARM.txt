Atik Cameras 2020

Placing atik.rules into /usr/lib/udev/rules allows you to use our dll with out admin privileges. 
You will need to reload udev rules using "sudo udevadm control --reload-rules && sudo udevadm trigger" if you do not wish to restart your machine after placing the file.

We provide two versions of our shared libraries for Linux and ARM with and without FlyCapture. We use the Flycapture libraries to drive the Atik GP camera.
So if you do not intend to use a GP camera then use the NoFlyCapture shared library.