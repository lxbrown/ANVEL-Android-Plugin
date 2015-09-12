# ANVEL-Android-Plugin
A plugin for ANVEL to send image data from a camera sensor to an Android application and another optional plugin to control the vehicle that the camera is attached to.

## Dependencies
### ANVEL Headers and Libs
To build you will need to add all ANVEL headers and libs included with the ANVEL Academic Edition install. To add the headers and libs, simply copy the folders 'Core', 'Deps', 'lib', 'ode', 'Simulation', 'VaneOgreRender' from the ANVEL install into the 'Dependencies' folder in this project.

### ZeroMQ
To run these plugins you will need to have ZeroMQ installed which can be downloaded here: http://zeromq.org/area:download.
A couple notes on the ZeroMQ setup:

* Download the x86 (32 bit) build of the Windows installer
* When installing, make sure to include ZeroMQ source code on the 'Choose Components' install screen
* You will need to create an Environment Variable called 'ZEROMQ_HOME' (without quotes) that points to the ZeroMQ install directory (e.g. 'C:\Program Files (x86)\ZeroMQ 4.0.4\')

## Running Plugins
When running in ANVEL, libzmq-v110-mt-4_0_4.dll will also need to be placed in the ANVEL Plugin folder along with the .dll created from building the plugin.

Plugins and Android application created by Alex Brown - lxbrown@umich.edu

Under the supervision and guidance of Justin Storms - jgstorms@umich.edu
