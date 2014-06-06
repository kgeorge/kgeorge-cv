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
    aHistTemplate = hogc.AngleHistogram(9, 0.0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)

    hogScheme = hogc.HogScheme(w,h)
    descriptor = []
    hog = hogc.Hog(hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.YesGaussian, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
    for r in range(0, hogScheme.numBlocksVertical):
        for c in range(0, hogScheme.numBlocksHorizontal):
            histVec = hogc.FloatVec()
            hog.computeBlock(convertedImg,aHistTemplate,  r, c, smAngle, smMag, False, histVec)
            descriptor.append(histVec)
            pass
    etime = time.time()
    print "%s, took %d millisecs, descriptor %r" % (filename, ((etime - stime) * 1000), descriptor[0])
    return descriptor


def processImagePIL(dirname, filename):
    im = Image.open( os.path.join(dirname, filename) )
    print filename, im.size, len(im.getdata())
    mypixels = []
    for i in range(0, 5):
        mypixels.append(im.getpixel((i, 2*i)))
    print mypixels
    pass

class HogOutput(object):
    def __init__(self, dirname, label):
        self.outputfilename = os.path.join(dirname, "hog_output.txt")
        self.hogResults = {}
        self.label = label
    def addResult(self, filename, result):
        self.hogResults[filename] = result
        if len(self.hogResults) >= 100:
            self.serialize()
            self.hogResults = {}
    def serialize(self):
        with file(self.outputfilename, "w") as fp:
            for k,v in self.hogResults.iteritems():
                #print k, v
                serializeResult="#%s\n" % k
                for i in range(0, len(v)):
                    serializeResult += str(i) + "," + str(self.label) + ", " +  hogc.serializeFloatVec(v[i]) + "\n"
                fp.write(serializeResult)
            fp.close()
            pass


def processDir(dirname, label):
    smAngle = hogc.StatsMaker("angle")
    smMag = hogc.StatsMaker("magnitude")
    hogOutput = HogOutput(dirname, label)
    for root, dirs, files in os.walk(dirname):
        for f in files:
            (p, e) = os.path.splitext(f)
            if ((e.lower() == ".png") or (e.lower() == ".jpg") or (e.lower() == ".jpeg")):
                try:
                    hogResult = processImage(root, f, smAngle, smMag)
                    hogOutput.addResult(f, hogResult)
                    print "no exception"
                except RuntimeError, h:
                    pass
    hogOutput.serialize()
    print "angle stats Maker", hogc.describeStatsMaker(smAngle)
    print  "magnitude stats Maker", hogc.describeStatsMaker(smMag)

if __name__ == "__main__":
    print 'processing good targets'
    processDir("/Users/kgeorge/Dropbox/cars/tentative/build", label=1)
    print 'processing bad targets'
    processDir("/Users/kgeorge/Dropbox/cars/bad/build", label=0)
    print 'processing crossvalidation targets'
    processDir("/Users/kgeorge/Dropbox/cars/crossvalidate/build", label=-1)



