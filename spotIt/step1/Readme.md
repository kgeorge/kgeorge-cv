

## Feature detection challenge from (Silicon-Valley-Computer-Vision-Meetup)[http://www.meetup.com/Silicon-Valley-Computer-Vision]


*
4/2/2014

As of now,

For each frame,

- the program reads input video


- detects a single (spotIt)[http://www.blueorangegames.com/index.php/games/spotit] circular card region. (using HoughCircle)

- proceeds to process the region of image neighboring to the circular region. (SpotIt::processCircle) SpotIt::processCircle produces a processed version of the output region of interest part of the image.
which can be saved in the current working directory as 'roiLastOutputImage.jpg'. The current processing of the roi is rudimental. It
does some minimal clustering which need be improved.


## After compiling, run this program as,

./build/spotIt

*
Show a spotIt card to the camera and the processed portion of the card appears on the video at the top left portion as a feedback.

*
Press 's' or 'S' to save the region of interest image as 'roiLastOutputImage.jpg'

*
Press 'Escape' to quit

*
The program was tested and observed to run with opencv 2.4.8 in my mac powerbook, osx 10.9

## Debug Build

*
Change the Makefile to update the variable OPENCV_DEBUG_INSTALL_DIR to point to your curent opencv debug installation.

*
make DEBUG=1


