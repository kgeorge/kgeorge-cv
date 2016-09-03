
To use the image authoring tool,

1)
launch a python server from git root.

python -m SimpleHTTPServer


2)
bring up samples/skindetect/authoring/index.html in your browser.


ie: http://localhost:8000/samples/skindetect/authoring/


3)
The image to edit should be in samples/skindetect/authoring/image as fxxx.jpg

Eg: if you need to edit f16.jpg, it can be brought up in the editor as .

http://localhost:8000/samples/skindetect/authoring/index.html?baseDataDir=skindetect/authoring/image,sourceImgFilename=f16.jpg

4)
In the draw mode you can draw curves to outline the skin

for connecting two curves end o end choose "Connect", click on first curve (mother curve) and then on the curve to be joined and double click.

For closing an open curve, choose "Connect", click on the curve and double click.

Ctrl-z will erase the last drawing in draw mode

for making a closed curve (child) as a hole of another closed curve (mother), in the "Connect" mode, click on mother and then on "child" and press c,
If successful an alert dialog will announce success.

To choose between filled and non-filled display in the "Draw" mode, press 'f' or 'c'. respectively

To edita few points in a curve, in 'edit' mode, draw a selection tool curve around a few points in the curve to be edited, close the newly drawn
selection tool curve and drag the selection around. Doublclick when you are satisfied.

5)
In the save mode, double click and your mask will be stored as a png image as "skindetect-f16.png' in the dwonloads folder.

7)
run
python samples/skindetect/authoring/python/checkImages.py -f f16.jpg to copy the correctly converted mask image as f16_mask.png in the samples/skindetect/authoring/image folder.
