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

class Metrics(object):
    def __init__(self, thrshld, mode):
        self._tp=0
        self._tn=0
        self._fp=0
        self._fn=0
        self._thrshld = thrshld
        self._mode  = mode
        pass

    def sample(self, bActual, bAlgorithm):
        if(bActual and bAlgorithm):
            self._tp += 1
        elif (bActual and not bAlgorithm):
            self._fn += 1
        elif (not bActual and bAlgorithm):
            self._fp += 1
        elif (not bActual and not bAlgorithm):
            self._tn += 1

    def __add__(self, other):
        m = Metrics(self._thrshld, self._mode)
        m._tp = self._tp + other.tp
        m._tn = self._tn + other.tn
        m._fp = self._fp + other.fp
        m._fn = self._fn + other.fn
        return m

    def __str__(self):
        total = self.tp + self.tn + self.fp + self. fn
        s = "{mode: %s, thrshld: %f, tp: %f, tn: %f, fp: %f, fn: %f, total: %d }" % (self._mode, self.thrshld, self._tp, self._tn, self._fp, self._fn, total)
        if(total > 0):
            s = "{mode: %s, thrshld: %f,  tp: %f, tn: %f, fp: %f, fn: %f, total: %d }" % (self._mode, self.thrshld, float(self._tp)/total, float(self._tn)/total, float(self._fp)/total, float(self._fn)/total, total)
        return s

    @property
    def thrshld(self):
        """I'm the 'thrshld' property."""
        return self._thrshld


    @property
    def mode(self):
        """I'm the 'mode' property."""
        return self._mode

    @property
    def tp(self):
        """I'm the 'tp' property."""
        return self._tp

    @tp.setter
    def x(self, value):
        self._tp = value

    @property
    def tn(self):
        """I'm the 'tn' property."""
        return self._tn

    @tn.setter
    def x(self, value):
        self._tn = value

    @property
    def fp(self):
        """I'm the 'fp' property."""
        return self._fp

    @fp.setter
    def x(self, value):
        self._fp = value

    @property
    def fn(self):
        """I'm the 'fn' property."""
        return self._fn

    @fn.setter
    def x(self, value):
        self._fn = value


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

def processImageTupleYCbCr(rootDir, mainImagename, skinHistPath, nonSkinHistPath, qLimit):
    mode = "CbCr"
    thrshld = 40
    metrics = Metrics( thrshld, mode )
    maskImagename = getMaskImagename(mainImagename)
    def getClassifiedImagename(imageFilename, mode, thrshld):
        m = r_mainImage.match(imageFilename)
        if m:
            maskImagename = "%s_cls_%s_%d" % (m.group('imgFilename'), mode, thrshld) + '.png'
            return maskImagename
        return None
    maskImage = Image.open(os.path.join(rootDir, maskImagename))
    binMaskImage = maskImage.convert("1")
    assert(os.path.exists(os.path.join(rootDir, mainImagename)))
    mainImage = Image.open(os.path.join(rootDir, mainImagename))
    assert(binMaskImage.size == mainImage.size)
    yCbCrMainImage = mainImage.convert('YCbCr')
    skinHist = Image.open(os.path.join(rootDir, skinHistPath))
    print skinHist.size
    maskImage = Image.new("RGB", mainImage.size, "white")
    yMainImage, cbMainImage, crMainImage = yCbCrMainImage.split()
    for cb, cr, mask in zip( enumerate(cbMainImage.getdata()), enumerate(crMainImage.getdata()), enumerate(binMaskImage.getdata())):
        xy = cb[0]
        assert(cb[1] < 256 and cr[1] < 256)
        cb_2 = cb[1] * qLimit/256
        cr_2 = cr[1] * qLimit/256
        v = skinHist.getpixel((cr_2, cb_2))
        x =  xy % mainImage.size[0]
        y = int(xy / mainImage.size[0])
        bActualSkin = False
        if mask[1] > 0:
            bActualSkin = True
        bAlgorithmSkin = False
        if v > thrshld:
            bAlgorithmSkin = True
            maskImage.putpixel((x,y), (255,0,0))
        metrics.sample(bActualSkin, bAlgorithmSkin)
    classifiedImagename = getClassifiedImagename(mainImagename, "CbCr", thrshld)
    maskImage.save(os.path.join(rootDir, classifiedImagename))
    print 'processing image %s,\n %s' % (mainImagename, metrics)
    return metrics



def processImageTupleNRGB(rootDir, mainImagename, skinHistPath, nonSkinHistPath, qLimit):
    thrshld = 40
    mode = "NRGB"
    metrics = Metrics(thrshld, mode)
    maskImagename = getMaskImagename(mainImagename)
    def getClassifiedImagename(imageFilename, mode, thrshld):
        m = r_mainImage.match(imageFilename)
        if m:
            maskImagename = "%s_cls_%s_%d" % (m.group('imgFilename'), mode, thrshld) + '.png'
            return maskImagename
        return None
    maskImage = Image.open(os.path.join(rootDir, maskImagename))
    binMaskImage = maskImage.convert("1")
    assert(os.path.exists(os.path.join(rootDir, mainImagename)) )
    mainImage = Image.open(os.path.join(rootDir, mainImagename))
    assert(binMaskImage.size == mainImage.size)
    rMainImage, gMainImage, bMainImage = mainImage.split()
    skinHist = Image.open(os.path.join(rootDir, skinHistPath))
    maskImage = Image.new("RGB", mainImage.size, "white")
    for r,g,b, mask in zip( enumerate(rMainImage.getdata()), enumerate(gMainImage.getdata()), enumerate(bMainImage.getdata()), enumerate(binMaskImage.getdata())):
        xy = r[0]
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
        v = skinHist.getpixel((g, r))
        x =  xy % mainImage.size[0]
        y = int(xy / mainImage.size[0])
        bActualSkin = False
        if mask[1] > 0:
            bActualSkin = True
        bAlgorithmSkin = False
        if v > thrshld:
            maskImage.putpixel((x,y), (255,0,0))
            bAlgorithmSkin = True
        metrics.sample(bActualSkin, bAlgorithmSkin)
    classifiedImagename = getClassifiedImagename(mainImagename, mode, thrshld)
    maskImage.save(os.path.join(rootDir, classifiedImagename))

    print 'processing image %s,\n %s' % (mainImagename, metrics)
    return metrics




def main():
    #africanList = ['f31', 'f32', 'f34']
    africanList = [
        'f31', 'f32', 'f34', 'f36', 'f37',
        #'f38', 'f40', 'f41', 'f42', 'f44', 'f46', 'f49', 'f50',  'f55'
        ]
    #africanList = ['f31', 'f32']
    caucasianList = [
                    'f15', 'f74', 'f78' , 'f79',
                    #'f81', 'f83', 'f84',
                    'f85', 'f86', 'f89',
                    #'f90', 'f91', 'f92',
                    'f93', 'f95', 'f96',
                    #'f98', 'f99', 'f100',
                    'f102', 'f110', 'f111',
                    #'f112', 'f113', 'f114',
                    'f115', 'f116', 'f117'
                ]
    considerList = caucasianList


    def getClassifiedImagename(imageFilename, mode):
        m = r_mainImage.match(imageFilename)
        if m:
            maskImagename = m.group('imgFilename') + '_cls_' + mode + '.png'
            return maskImagename
        return None

    def getHistogramImagename(imageFilename, histType):
        m = r_mainImage.match(imageFilename)
        assert(m)
        return m.group('imgFilename') + "_hist_" + histType + ".png"

    def saveHistImage(hist, filepath):
        print "saving hist as", filepath
        #extractEllipse(hist)
        #histCentroid = findCentroid(hist)[0]
        hist *= (255.0 / hist.max());
        histAsU1 = hist.astype(np.dtype('u1'))
        histImage = Image.fromarray(histAsU1[:,:], "L")
        histImage.save( filepath)

    print os.path.dirname(__file__)
    mode = "YCbCr"
    #mode = "YIQ"
    #mode = "NRGB"
    print os.getcwd()
    rootDataDir = 'samples/skindetect/authoring/image'
    walkit = os.walk(rootDataDir)
    qlimit = 128
    skinHistPath = "hist_cbcr_skin.png"
    nonSkinHistPath = "hist_cbcr_noSkin.png"
    if mode == "YCbCr":
        pass
    elif mode == "NRGB":
        skinHistPath = "hist_nrg_skin.png"
        nonSkinHistPath = "hist_nrg_noSkin.png"
        pass
    thrshld = 20
    metrics = Metrics(thrshld, mode)
    for r, dirs, files in walkit:
        for name in files:
            if not r_mainImage.match(name):
                continue
            m = r_mainImage.match(name)
            if not (m.group('imgFilename') in considerList):
                continue
            maskImagename =  getMaskImagename(name)
            if maskImagename and os.path.exists(os.path.join(r, maskImagename)):
                if mode == "YCbCr":
                    metrics = metrics + processImageTupleYCbCr(r, name, skinHistPath, nonSkinHistPath, qlimit)
                    pass
                elif mode == "NRGB":
                    metrics  = metrics + processImageTupleNRGB(r, name,  skinHistPath, nonSkinHistPath, qlimit)
                    pass
                pass
        break
    print 'total metrics: %s' % metrics

if __name__ == '__main__':
    main()
