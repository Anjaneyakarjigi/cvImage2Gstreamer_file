/*************/
Project converts Opencv webcam VideoCapture to Gstreamer file
Takes input images read from the Opencv VideoCapture, puts into Gstreamer pipeline which stores into file.

Run this project by below Command

/* Build project */
cmake -H. -Bbuild

/* Compile project */
cmake --build build

// compilation adds cvImageToGstreamerFile into folder
/* Run Project */
just run cvImageToGstreamerFile in the terminal, which creates OutputFile.ts video file 

