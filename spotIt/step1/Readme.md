

# Feature detection challenge from (Silicon-Valley-Computer-Vision-Meetup)[http://www.meetup.com/Silicon-Valley-Computer-Vision/events/]


*
4/2/2014

As of now,

For each frame,

- the program reads input video


- detects a single (spotIt)[http://www.blueorangegames.com/index.php/games/spotit] circular card region. (using HoughCircle)

- proceeds to process the region of image neighboring to the circular region. (SpotIt::processCircle) SpotIt::processCircle produces a processed version of the output region of interest part of the image.
which can be saved in the current working directory as 'roiLastOutputImage.jpg'



