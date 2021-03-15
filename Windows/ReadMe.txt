/*************/
Project converts Opencv webcam VideoCapture to Gstreamer file
Takes input images read from the Opencv VideoCapture, puts into Gstreamer pipeline which stores into file.

Run this project by below Command

/* Build project with below command line */ // Change Visual studio based on your version
cmake -H. -Bbuild -G "Visual Studio 12 2013 Win64"  

/* Compile project with below command line */
cmake --build build  --config Release

// compilation adds cvImageToGstreamerFile into folder
/* Run Project */
Just run cvImageToGstreamerFile in the terminal, which creates OutputFile.ts video file in Release folder

