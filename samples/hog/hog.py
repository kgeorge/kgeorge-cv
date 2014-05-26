from PIL import Image
import numpy as nu
import os
import fnmatch
import re
import cv2, cv
import hogc
import math
import time



r1 =  re.compile(r'^[#]+\S*')
r2 =  re.compile(r'^\s*(?P<left>\d+)\s*,\s*(?P<top>\d+)\s*,\s*(?P<width>\d+)\s*,\s*(?P<height>\d+)')
#r2 =  re.compile(r'^\s*(?P<left>\d+)\s*')

def processImage(dirname, filename, smAngle, smMag):
    stime = time.time()
    (n,e) = os.path.splitext(filename)
    img = cv2.imread(os.path.join(dirname, filename),3)
    convertedImg = cv2.cvtColor(img, cv2.COLOR_BGR2GRAY)
    h = convertedImg.shape[0]
    w = convertedImg.shape[1]
    hogScheme = hogc.HogScheme(w,h)
    descriptor = []
    hog = hogc.Hog(hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.YesGaussian)
    for r in range(0, hogScheme.numBlocksVertical):
        for c in range(0, hogScheme.numBlocksHorizontal):
            aHist = hogc.AngleHistogram(9, 0.0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
            hog.computeBlock(convertedImg, r, c, smAngle, smMag, aHist, False)
            descriptor.append(aHist.hist())
            pass
    etime = time.time()
    print "%s, took %d millisecs, descriptor %r" % (filename, ((etime - stime) * 1000), descriptor[0])
    pass


def processImagePIL(dirname, filename):
    im = Image.open( os.path.join(dirname, filename) )
    print filename, im.size, len(im.getdata())
    mypixels = []
    for i in range(0, 5):
        mypixels.append(im.getpixel((i, 2*i)))
    print mypixels
    pass

def main():

    smAngle = hogc.StatsMaker("angle")
    smMag = hogc.StatsMaker("magnitude")
    for root, dirs, files in os.walk("/Users/kgeorge/Dropbox/cars/good/build"):
        for f in files:
            (p, e) = os.path.splitext(f)
            if ((e.lower() == ".png") or (e.lower() == ".jpg") or (e.lower() == ".jpeg")):
                #for i in range(0, 100):
                processImage(root, f, smAngle, smMag)
                #pass

    print "angle stats Maker", hogc.describeStatsMaker(smAngle)
    print  "magnitude stats Maker", hogc.describeStatsMaker(smMag)

if __name__ == "__main__":
    main()



