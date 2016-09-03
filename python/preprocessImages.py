from PIL import Image
import os
import fnmatch
import re
import cv2
import cv
import numpy as nu
import hogc



r1 =  re.compile(r'^[#]+\S*')
r2 =  re.compile(r'^\s*(?P<left>\d+)\s*,\s*(?P<top>\d+)\s*,\s*(?P<width>\d+)\s*,\s*(?P<height>\d+)')
#r2 =  re.compile(r'^\s*(?P<left>\d+)\s*')

def normalizeImage(img):
    convertedImg = cv2.cvtColor(img, cv2.COLOR_BGR2YCR_CB)
    channels = cv2.split(convertedImg)
    channels[0] = cv2.equalizeHist(channels[0])
    convertedImg = cv2.merge(channels)
    convertedImg = cv2.cvtColor(convertedImg, cv2.COLOR_YCR_CB2BGR)
    return convertedImg

def processImage(dirname, filename):
    if(True):
        imagefilename = os.path.join(dirname, filename)
        builddirname = os.path.join(dirname, "build")
        im2 = None
        img = cv2.imread(imagefilename)
        print filename, img.size
        (n, e) = os.path.splitext(filename)
        n += "_normalized"
        if not os.path.exists(builddirname):
            os.mkdir(builddirname)
        numpyImg = nu.asarray(img)
        hogc.gammaCorrect(numpyImg, 2.2, True)
        normalizedImg = normalizeImage(numpyImg)
        cv2.imwrite((os.path.join(builddirname, n + e)), normalizedImg)
        cvimg= None
        pass


def main():
    for root, dirs, files in os.walk("/Users/kgeorge/Dropbox/cars/input"):
        for f in files:
            (p, e) = os.path.splitext(f)
            processImage(root, f)
            pass

if __name__ == "__main__":
    main()



