

4/2/2014
As of now, 

For each frame,

1) the program reads input video 

2)detects a single spoiIt circular card region.
(using HoughCircle)

3)proceeds to process the region of image neighboring to the circular region.
(processCircle)
processCircle produces a processed version of the output region of interest part of the image.
which can be saved in the current working directory as 'roiLastOutputImage.jpg'



