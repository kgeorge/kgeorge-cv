

## Feature detection challenge 

This implementation is kind of very experimental. I am trying out with various algorithms out there.


*
4/2/2014

As of now,

For each frame,

- the program reads input video


- detects a single [spotIt](http://www.blueorangegames.com/index.php/games/spotit) circular card region. (using HoughCircle)


- When pressed 'n' or 'N' proceeds to process the region of image within the  circular region. (SpotIt::processCircle) SpotIt::processCircle produces a processed version of the output region of interest part of the image.
which can be saved in the current working directory as 'roiLastOutputImage.jpg'. The current processing of the roi is rudimental. It
does some minimal clustering which need be improved.

--The roiOutputImage of last processing would be overlayed on the top-left corner of video output

--[geometric hashing](http://graphics.stanford.edu/courses/cs468-01-winter/papers/wr-ghao-97.pdf) where a query point set can be matched against a predefined set of well defined template point set, sounds promising here.
[geometric hashing](http://graphics.stanford.edu/courses/cs468-01-winter/papers/wr-ghao-97.pdf) should find out partial matches and is invavriant to translation, rotation and scaling. We also implement,
a variant of [locality sensitive hashing](http://www.mit.edu/~andoni/LSH) which is based in p-stable distributions, to implement the basic hashing functionality.




## After compiling, run this program as,
<code>
./build/spotIt
</code>
*
Show a spotIt card to the camera and the processed portion of the card appears on the video at the top left portion as a feedback.

*
Press 's' or 'S' to save the region of interest image as 'roiLastOutputImage.jpg'

*
Press 'n' or 'N' to process the frame next time
*
Press 'Escape' to quit

*
The program was tested and observed to run with opencv 2.4.8 in my mac powerbook, osx 10.9

## Debug Build

*
Change the Makefile to update the variable <code>OPENCV_DEBUG_INSTALL_DIR</code> to point to your curent opencv debug installation.

*
make DEBUG=1


