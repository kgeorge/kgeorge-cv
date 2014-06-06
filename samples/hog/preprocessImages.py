from PIL import Image
import os
import fnmatch
import re
import cv2
import cv
import numpy as nu
import hogc
import site
import sys



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
    (p,e) = os.path.splitext(filename)
    p += ".txt"
    annotationFile = os.path.join(dirname, "annotate", p)
    builddirname = os.path.join(dirname, "build")

    (l,t,w,h)=(-1,-1,-1,-1)
    with file(annotationFile) as fp:
        for  l in fp:
            l= l.strip()
            if not l:
                continue
            m = r1.match(l)
            m2 = r2.match(l)
            p2 = l.split(", ")
            if not m and m2:
                (l,t,w,h) =  (int(m2.group('left')), int(m2.group('top')), int(m2.group('width')), int(m2.group('height')))
                break
            pass
    pass
    if(l >=0 and t >= 0 and w >= 0 and h >= 0):
        imagefilename = os.path.join(dirname, filename)


        im2 = None
        img = cv2.imread(imagefilename)
        im2 = img[t:t+h, l:l+w]
        #im = Image.open(imagefilename)
        print filename, img.size, l, t, l+w, t+h
        #im2 = im.crop((l,t,w+l,h+t))

        im3 = cv2.resize(im2, (128, 64),  interpolation=cv2.INTER_LANCZOS4)
        im2 = None

        (n, e) = os.path.splitext(filename)
        n += "_canonical"
        if not os.path.exists(builddirname):
            os.mkdir(builddirname)

        #im3 = im2.resize((128, 64), Image.ANTIALIAS)
        #im3 = im2.resize((128, 64), Image.LINEAR)
        #cvimg = cv.CreateImageHeader(im3.size, cv.IPL_DEPTH_8U, 3)      # cimg2 is a OpenCV image
        #cv.SetData(cvimg, im3.tostring())
        numpyImg = nu.asarray(im3)
        hogc.gammaCorrect(numpyImg, 2.2, True)
        #print type(cvimg)
        #normalizedImg = cv2.cvtColor(numpyImg, cv2.COLOR_RGB2BGR)
        #normalizedImg = normalizeImage(numpyImg)
        #im3.save(os.path.join(builddirname, n + e))
        #im3 = None
        cv2.imwrite((os.path.join(builddirname, n + e)), numpyImg)
        cvimg= None
        pass

def processImage2(builddir, dirname, filename):
    if(True):
        imagefilename = os.path.join(dirname, filename)
        im2 = None
        img = cv2.imread(imagefilename)
        print filename, img.size
        im2 = cv2.resize(img, (128, 64),  interpolation=cv2.INTER_LANCZOS4)
        img = None
        (n, e) = os.path.splitext(filename)
        n += "_canonical"
        if not os.path.exists(builddir):
            os.mkdir(builddir)
        numpyImg = nu.asarray(im2)
        hogc.gammaCorrect(numpyImg, 2.2, True)
        #normalizedImg = normalizeImage(numpyImg)
        cv2.imwrite((os.path.join(builddir, n + e)), numpyImg)
        cvimg= None
        pass



def main():
    mode = sys.argv[1]
    print mode
    rootdir = "/Users/kgeorge/Dropbox/cars/tentative"
    if mode == "b":
        rootdir = "/Users/kgeorge/Dropbox/cars/bad"
    elif mode == "c":
        rootdir = "/Users/kgeorge/Dropbox/cars/crossvalidate"


    builddir = os.path.join(rootdir, "build")
    print site.getsitepackages()
    for root, dirs, files in os.walk(rootdir):
        (h,t) = os.path.split(root)
        if t.endswith('_delme'):
            continue
        for f in files:
            (p, e) = os.path.splitext(f)
            if ((e.lower() == ".png") or (e.lower() == ".jpg") or (e.lower() == ".jpeg")) and not p.endswith("canonical"):
                try:

                    processImage2(builddir, root, f)
                except AttributeError, e:
                    print e
                pass

if __name__ == "__main__":
    main()



