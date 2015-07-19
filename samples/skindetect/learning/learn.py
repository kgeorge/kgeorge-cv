__author__ = 'kgeorge'
import re
import cStringIO
import io
from PIL import Image
import urlparse
from io import  BytesIO
from cgi import parse_header, parse_multipart
import base64
import os
import json
import cv2
import colorsys
#import yaml
#import simplejson as json
import numpy as np


r_mainImage = re.compile('(?P<imgFilename>f\d+)[.]jpg')

def findCentroid(hist):
    rows,cols = hist.shape
    nNonzeroPixels = 0
    #finding centroid
    cr = 0
    cc = 0
    for r in xrange(rows):
        for c in xrange(cols):
            if hist[r,c] > 0.0:
                #print r,c, hist[r,c]
                nNonzeroPixels += hist[r,c]
                cr += r * hist[r,c]
                cc += c * hist[r,c]
            pass
    if nNonzeroPixels > 0:
        return ((cr/nNonzeroPixels, cc/nNonzeroPixels), nNonzeroPixels)
    else:
        raise IOError

def extractEllipse(hist):
    centroid, nNonZeroPixels= findCentroid(hist)
    eMat = np.zeros((2,2), dtype=np.float)
    rows,cols = hist.shape
    for r in xrange(rows):
        for c in xrange(cols):
            if hist[r,c] > 0.0:
                rdiff = (r - centroid[0])
                cdiff = (c - centroid[1])
                eMat[0,0] += rdiff * rdiff * hist[r,c]
                eMat[0,1] += rdiff * cdiff * hist[r,c]
                eMat[1,0] += rdiff * cdiff * hist[r,c]
                eMat[1,1] += cdiff * cdiff * hist[r,c]
            pass
    eMat /= nNonZeroPixels
    e = np.linalg.eig(eMat)
    print e[0], e[1]


def getMaskImagename(imageFilename):
    m = r_mainImage.match(imageFilename)
    if m:
        maskImagename = m.group('imgFilename') + '_mask.png'
        return maskImagename
    return None


def processImageTupleYCbCr(rootDir, mainImagename,  bSkin, qLimit):
    print 'processing image %s, mode: %s, qLimit: %d' %  (mainImagename, "CbCr", qLimit)
    maskImagename = getMaskImagename(mainImagename)
    assert(os.path.exists(os.path.join(rootDir, mainImagename)) and os.path.exists(os.path.join(rootDir, maskImagename)))
    mainImage = Image.open(os.path.join(rootDir, mainImagename))
    yCbCrMainImage = mainImage.convert('YCbCr')
    maskImage = Image.open(os.path.join(rootDir, maskImagename))
    binMaskImage = maskImage.convert("1")
    assert(mainImage.size == maskImage.size)
    yMainImage, cbMainImage, crMainImage = yCbCrMainImage.split()
    cBcRHist_skin = np.zeros((qLimit, qLimit), dtype=np.float)
    cBcRHist_noSkin = np.zeros((qLimit, qLimit), dtype=np.float)
    for cb, cr, mask in zip( enumerate(cbMainImage.getdata()), enumerate(crMainImage.getdata()), enumerate(binMaskImage.getdata())):
        assert(cb[1] < 256 and cr[1] < 256)
        cb_2 = cb[1] * qLimit/256
        cr_2 = cr[1] * qLimit/256
        if mask[1] > 0:
            v = cBcRHist_skin.item((cb_2, cr_2))
            cBcRHist_skin.itemset((cb_2, cr_2), (v + 1.0))
        else:
            v = cBcRHist_noSkin.item((cb_2, cr_2))
            cBcRHist_noSkin.itemset((cb_2, cr_2), (v + 1.0))
    if bSkin:
        return cBcRHist_skin
    else:
        return cBcRHist_noSkin


def processImageTupleNRGB(rootDir, mainImagename, bSkin, qLimit):
    print 'processing image %s, mode: %s, qLImit: %d' % (mainImagename, "NRGB", qLimit)
    maskImagename = getMaskImagename(mainImagename)
    assert(os.path.exists(os.path.join(rootDir, mainImagename)) and os.path.exists(os.path.join(rootDir, maskImagename)))
    mainImage = Image.open(os.path.join(rootDir, mainImagename))
    #yCbCrMainImage = mainImage.convert('YCbCr')
    maskImage = Image.open(os.path.join(rootDir, maskImagename))
    binMaskImage = maskImage.convert("1")
    assert(mainImage.size == maskImage.size)
    rMainImage, gMainImage, bMainImage = mainImage.split()
    nrgHist_skin = np.zeros((qLimit, qLimit), dtype=np.float)
    nrgHist_noSkin = np.zeros((qLimit, qLimit), dtype=np.float)
    for r,g,b,mask in zip( enumerate(rMainImage.getdata()), enumerate(gMainImage.getdata()), enumerate(bMainImage.getdata()), enumerate(binMaskImage.getdata())):
        r = r[1]
        g = g[1]
        b = b[1]
        s = r + g + b

        if s > 0:
            r = float(r)/s
            g = float(g)/s
            b = float(b)/s
        else:
            r=0.0
            g=0.0
            b=0.0

        r = int(r * (qLimit-1))
        g = int(g * (qLimit-1))
        #if r > 255 or g > 255:
        #print r,g, nrgHist_skin.shape
        if mask[1] > 0:
            v = nrgHist_skin.item((r, g))
            nrgHist_skin.itemset((r, g), (v + 1.0))
        else:
            v = nrgHist_noSkin.item((r, g))
            nrgHist_noSkin.itemset((r, g), (v + 1.0))

    pass
    if bSkin:
        return nrgHist_skin
    else :
        return nrgHist_noSkin





def main():
    #africanList = ['f31', 'f32', 'f34']
    africanList = [
        #'f31', 'f32', 'f34', 'f36', 'f37',
        'f38', 'f40', 'f41', 'f42', 'f44', 'f46', 'f49', 'f50',  'f55'
    ]
    #africanList = ['f31', 'f32']
    caucasianList = [
                    #'f15', 'f74', 'f78' , 'f79',
                    'f81', 'f83', 'f84',
                    #'f85', 'f86', 'f89',
                    'f90', 'f91', 'f92',
                    #'f93', 'f95', 'f96',
                    'f98', 'f99', 'f100',
                    #'f102', 'f110', 'f111',
                    'f112', 'f113', 'f114',
                    #'f115', 'f116', 'f117'
    ]
    considerList = caucasianList

    def getHistogramImagename(imageFilename, histType):
        m = r_mainImage.match(imageFilename)
        assert(m)
        return m.group('imgFilename') + "_hist_" + histType + ".png"

    def saveHistImage(hist, filepath):
        print "saving hist as", filepath
        #extractEllipse(hist)
        histCentroid = findCentroid(hist)[0]
        print histCentroid
        hist *= (255.0 / hist.max());
        histAsU1 = hist.astype(np.dtype('u1'))
        histImage = Image.fromarray(histAsU1[:,:], "L")
        histImageRgb = histImage.convert('RGB')
        histImageRgb.putpixel((int(histCentroid[1]), int(histCentroid[0])), (255, 0,0))
        histImage.save( filepath)

    print os.path.dirname(__file__)
    mode = "YCbCr"
    #mode = "YIQ"
    #mode = "NRGB"
    print os.getcwd()
    rootDataDir = 'samples/skindetect/authoring/image'
    walkit = os.walk(rootDataDir)
    qlimit = 128
    cBcRHist_skin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    cBcRHist_noSkin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    nrgHist_skin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    nrgHist_noSkin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    for r, dirs, files in walkit:
        for name in files:
            if not r_mainImage.match(name):
                continue
            m = r_mainImage.match(name)
            if not (m.group('imgFilename') in considerList):
                continue
            maskImagename =  getMaskImagename(name)
            if maskImagename and os.path.exists(os.path.join(r, maskImagename)):
                print '~~~~~~', r, name
                if mode == "YCbCr":
                    bSkin = True
                    cBcRHist_skin = processImageTupleYCbCr(r, name,  bSkin, qlimit)
                    cBcRHist_skin_sum = cBcRHist_skin_sum + cBcRHist_skin
                    bSkin= False
                    cBcRHist_noSkin = processImageTupleYCbCr(r, name,  bSkin, qlimit)
                    cBcRHist_noSkin_sum = cBcRHist_noSkin_sum + cBcRHist_noSkin
                    pass
                elif mode == "NRGB":
                    bSkin = True
                    nrgHist_skin = processImageTupleNRGB(r, name,  bSkin, qlimit)
                    nrgHist_skin_sum  = nrgHist_skin_sum  + nrgHist_skin
                    bSkin = False
                    nrgHist_noSkin = processImageTupleNRGB(r, name,  bSkin, qlimit)
                    nrgHist_noSkin_sum =  nrgHist_noSkin_sum + nrgHist_noSkin
                    pass
            pass
        if mode == "NRGB":
            saveHistImage(nrgHist_skin_sum, os.path.join(r, "hist_nrg_skin.png"))
            saveHistImage(nrgHist_noSkin_sum, os.path.join(r, "hist_nrg_noSkin.png"))
        elif mode == "YCbCr":
            saveHistImage(cBcRHist_skin_sum, os.path.join(r, "hist_cbcr_skin.png"))
            saveHistImage(cBcRHist_noSkin_sum, os.path.join(r, "hist_cbcr_noSkin.png"))

        break

if __name__ == '__main__':
    main()
