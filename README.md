# bthesis-tennisball
<b> ARM based SemiAutonomous Tennis Balls collector </b>
<br />
This robot was part of my B.Tech thesis. <br />
The project purpose is to use an OpenCV-based  machine-vision to collect the balls in a tennis court. <br />
<br />
Some technologies used are listed below. <br />
<br />
For robot: <br />
+ OpenCV imaging library <br />
+ Linux-based embedded system in an ARM microcontroller <br />
+ Contrast Limited Adaptive Histogram Equalization - CLAHE  <br /><br />

For the controller: <br />
+ Web based development <br />
+ Websockets via websockify <br />
<br /><br />
<b> Build Application </b>
<br />
Before build the tennisball_application, please check the dependencies in the Makefile.
<br />
You will need to setup some specific dependencies such as:
<br />
<br />
- OpenCV library <br />
- v4l2 <br />
- pthread </br>
<br />
With the dependencies installed, run the command "make install" inside the root of "tennisball_application" folder. 
<br />
If you installed the dependencies correctly, you'll find the application binary inside "bin" folder.
<br />
<b> System Setup </b>
<br />
To run the application correctly, you must setup websockify, the Python server and configure the rc.local file in your Linux embedded system.
<br />
<b> Http Controller </b>
<br />
The tennisball_application received commands from a html controller through the use of the websockets technology. 
<br />
This is the reason you need to setup websockify in you Linux embedded system first. 
