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
from optparse import OptionParser
from sklearn import mixture


r_mainImage = re.compile('(?P<imgFilename>\w+)[.]jpg')




def getMaskImagename(imageFilename):
    m = r_mainImage.match(imageFilename)
    if m:
        maskImagename = m.group('imgFilename') + '_mask.png'
        return maskImagename
    return None


def processImageTupleYCbCr_gmm(rootDir, mainImagename,  qLimit):
    maskImagename = getMaskImagename(mainImagename)
    assert(os.path.exists(os.path.join(rootDir, mainImagename)) and os.path.exists(os.path.join(rootDir, maskImagename)))
    mainImage = Image.open(os.path.join(rootDir, mainImagename))
    yCbCrMainImage = mainImage.convert('YCbCr')
    maskImage = Image.open(os.path.join(rootDir, maskImagename))
    binMaskImage = maskImage.convert("1")

    print 'processing image %s, (%r), mask: %s, (%r),  mode: %s, qLimit: %d' %  (mainImagename, mainImage.size, maskImagename, maskImage.size, "CbCr", qLimit)
    assert(mainImage.size == maskImage.size)
    yMainImage, cbMainImage, crMainImage = yCbCrMainImage.split()
    cBcR_skin = []
    cBcR_noSkin = []
    for cb, cr, mask in zip( enumerate(cbMainImage.getdata()), enumerate(crMainImage.getdata()), enumerate(binMaskImage.getdata())):
        assert(cb[1] < 256 and cr[1] < 256)
        cb_2 = cb[1] * qLimit/256
        cr_2 = cr[1] * qLimit/256
        if mask[1] > 0:
            cBcR_skin.append((cb_2, cr_2))
        else:
            cBcR_noSkin.append((cb_2,cr_2))
    return (np.array(cBcR_skin), np.array(cBcR_noSkin))





def processImageTupleYCbCr_predict_gmm(rootDir, mainImagename, g_skin, g_noSkin, qLimit):
    mode = "YCbCr"
    maskImagename = getMaskImagename(mainImagename)
    def getClassifiedImagename(imageFilename, mode):
        m = r_mainImage.match(imageFilename)
        if m:
            maskImagename = "%s_gmm_%s" % (m.group('imgFilename'), mode) + '.png'
            return maskImagename
        return None
    def getClassifiedOverlayImagename(imageFilename, mode):
        m = r_mainImage.match(imageFilename)
        if m:
            maskImagename = "%s_gmm_%s_overlay" % (m.group('imgFilename'), mode) + '.png'
            return maskImagename
        return None
    maskImage = Image.open(os.path.join(rootDir, maskImagename))
    binMaskImage = maskImage.convert("1")
    assert(os.path.exists(os.path.join(rootDir, mainImagename)))
    mainImage = Image.open(os.path.join(rootDir, mainImagename))
    assert(binMaskImage.size == mainImage.size)
    yCbCrMainImage = mainImage.convert('YCbCr')
    maskImage = Image.new("RGB", mainImage.size, "black")
    yMainImage, cbMainImage, crMainImage = yCbCrMainImage.split()
    for cb, cr, mask in zip( enumerate(cbMainImage.getdata()), enumerate(crMainImage.getdata()), enumerate(binMaskImage.getdata())):
        xy = cb[0]
        assert(cb[1] < 256 and cr[1] < 256)
        cb_2 = cb[1] * qLimit/256
        cr_2 = cr[1] * qLimit/256
        skinScore = g_skin.score(np.array([[cb_2, cr_2]]))
        nonSkinScore = g_noSkin.score(np.array([(cb_2, cr_2)]))
        x =  xy % mainImage.size[0]
        y = int(xy / mainImage.size[0])
        bActualSkin = False
        if mask[1] > 0:
            bActualSkin = True
        bAlgorithmSkin = False

        if skinScore > nonSkinScore:
            bAlgorithmSkin = True
            maskImage.putpixel((x,y), (255,255,255))
    classifiedImagename = getClassifiedImagename(mainImagename, "CbCr")

    maskImgeCv = np.array(maskImage)
    maskImgeCv = cv2.cvtColor(maskImgeCv, cv2.COLOR_RGB2BGR)
    #cv2.imwrite(os.path.join(rootDir, "foo.png"), maskImgeCv)
    #kernel = np.ones((5,5),np.uint8)
    #dilated = cv2.dilate(maskImgeCv,kernel,iterations = 1)
    #kernel = np.ones((5,5),np.uint8)
    #eroded = cv2.erode(dilated,kernel,iterations = 2)
    cv2.imwrite(os.path.join(rootDir, classifiedImagename), maskImgeCv)
    mainImgeCv = np.array(mainImage)
    mainImgeCv = cv2.cvtColor(mainImgeCv, cv2.COLOR_RGB2BGR)
    classifiedOverlayImagename = getClassifiedOverlayImagename(mainImagename, "CbCr")
    overlayImage = cv2.addWeighted(mainImgeCv, 0.7, maskImgeCv, 0.3, 0.0)
    cv2.imwrite(os.path.join(rootDir, classifiedOverlayImagename), overlayImage)
    print 'processing image %s,\n' % (mainImagename)
    return


def saveHistImage(hist, filepath):
    print "saving hist as", filepath
    hist *= (255.0 / hist.max());
    histAsU1 = hist.astype(np.dtype('u1'))
    histImage = Image.fromarray(histAsU1[:,:], "L")
    histImageRgb = histImage.convert('RGB')
    histImage.save( filepath)

def fit2(dataSets, qlimit =128, mode = "YCbCr", rootDataDir= 'samples/skindetect/authoring/image'):
    def getMaskImagename2(imageFilename):
        maskImagename = imageFilename + '_mask.png'
        return maskImagename

    cBcRHist_skin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    cBcRHist_noSkin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    nrgHist_skin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    nrgHist_noSkin_sum = np.zeros((qlimit, qlimit), dtype=np.float)
    maxSampleFiles = -1
    cBcR_skin_sum_list = []
    cBcR_noSkin_sum_list = []
    nSampleFilesSoFar =0

    n_components = 5
    g_skin = g_noSkin = None
    try:
        for r, dataSet in dataSets.iteritems():
            for name in dataSet:
                if (maxSampleFiles > 0 and nSampleFilesSoFar >= maxSampleFiles):
                    raise StopIteration('done')
                maskImagename =  getMaskImagename2(name)
                imgName = name + '.jpg'
                if  os.path.exists(os.path.join(r, imgName))  and  os.path.exists(os.path.join(r, maskImagename)):
                    print '~~~~~~', r, imgName
                    if mode == "YCbCr":
                        bSkin = True
                        (cBcR_skin, cBcR_noSkin) = processImageTupleYCbCr_gmm(r, imgName, qlimit)
                        cBcR_skin_sum_list.append(cBcR_skin)
                        cBcR_noSkin_sum_list.append(cBcR_noSkin)
                        nSampleFilesSoFar += 1
                        pass
                    elif mode == "NRGB":
                        raise Exception('not implemented')
                        pass
                pass

    except StopIteration as e:
        pass
    if mode == "NRGB":
        raise Exception('not implemented')
    elif mode == "YCbCr":
        cBcR_skin_sum = np.concatenate(cBcR_skin_sum_list)
        cBcR_noSkin_sum = np.concatenate(cBcR_noSkin_sum_list)
        print "cBcR_skin_sum.shape", cBcR_skin_sum.shape
        g_skin = mixture.GMM(n_components=n_components)
        g_skin.fit(cBcR_skin_sum)
        g_noSkin = mixture.GMM(n_components=n_components)
        g_noSkin.fit(cBcR_noSkin_sum)
        saveHistImage(cBcRHist_skin_sum, os.path.join(r, "hist_cbcr_skin.png"))
        saveHistImage(cBcRHist_noSkin_sum, os.path.join(r, "hist_cbcr_noSkin.png"))
    return (g_skin, g_noSkin)



def main():
    parser = OptionParser()
    parser.add_option("-m", "--mode", dest="mode",
                  help="mode of color")
    parser.add_option("-t", "--threshold", dest="thrshld",
                  help="threshold")
    (options, args) = parser.parse_args()
    #africanList = ['f31', 'f32', 'f34']
    africanList = [
        #'f31', 'f32', 'f34', 'f36', 'f37',
        'f38', 'f40', 'f41', 'f42', 'f44', 'f46', 'f49', 'f50',  'f55'
    ]
    africanList = [
        'f31', 'f32', 'f34', 'f35', 'f36', 'f37', 'f38', 'f39', 'f40',
        'f41', 'f42', 'f44', 'f45', 'f46', 'f47', 'f48', 'f49', 'f50', 'f51',
        'f52', 'f53', 'f54', 'f55'

    ]
    caucasianList = [

                    'f74', 'f78' , 'f79',
                    'f81', 'f83', 'f84',
                    'f85', 'f86', 'f89',
                    'f90', 'f91', 'f92',
                    'f93', 'f95', 'f96',
                     'f98', 'f99', 'f100',
                    'f102',  'f110', 'f111',
                    'f112', 'f113', 'f114',
                    'f115', 'f116', 'f117',
                    'f118', 'f119', 'f120',
                    'f121', 'f122', 'f123',
                    'f124', 'f125', 'f126',
                    'f127', 'f128', 'f129',
                    'f130', 'f131', 'f132',
                    'f133', 'f134', 'f135',
                    'f136', 'f137', 'f138',
                    'f139', 'f140', 'f141',
                    'f142', 'f143', 'f144',
                    'f145', 'f146', 'f147',
                    'f148', 'f149', 'f150',
                    'f151', 'f152', 'f153',
                    'f154', 'f155', 'f156',
                    'f157', 'f158', 'f159',
                    'f160', 'f161', 'f162',
                    'f153', 'f154', 'f155',
                    'f156', 'f157', 'f158',
                    'f159', 'f160', 'f161',
                    'f162', 'f163', 'f164',
                    'f165', 'f_hand'
    ]

    treedetect = [
        'treedetect31'
    ]
    #considerList = caucasianList

    def getHistogramImagename(imageFilename, histType):
        m = r_mainImage.match(imageFilename)
        assert(m)
        return m.group('imgFilename') + "_hist_" + histType + ".png"



    def predict_gmm(dataSets):
        mode = "YCbCr"
        for r, dataSet in dataSets.iteritems():
            skinHistFilename = "hist_cbcr_skin.png"
            nonSkinHistFilename = "hist_cbcr_noSkin.png"
            for name in dataSet:
                imgName = name + ".jpg"
                maskImagename =  name + "_mask.png"
                print imgName, maskImagename
                if  os.path.exists(os.path.join(r, imgName)) and os.path.exists(os.path.join(r, maskImagename)):
                    if mode == "YCbCr":
                        processImageTupleYCbCr_predict_gmm(r, imgName, g_skin, g_noSkin, qlimit)
                        pass
                    elif mode == "NRGB":
                        pass
                pass


    print os.path.dirname(__file__)
    mode = options.mode
    #mode = "YIQ"
    #mode = "NRGB"
    print os.getcwd()
    rootDataDir = 'samples/skindetect/authoring/image'
    walkit = os.walk(rootDataDir)
    qlimit = 128
    africanList_fit = [ f for idx, f in filter ( lambda (idx, f): idx %2 == 0, enumerate(africanList) )]
    africanList_predict = [ f for idx, f in filter ( lambda (idx, f): idx %2 != 0, enumerate(africanList) )]
    considerList_fit = [ f for idx, f in filter ( lambda (idx, f): idx %2 == 0, enumerate(caucasianList) )]
    considerList_predict = [ f for idx, f in filter ( lambda (idx, f): idx %2 != 0, enumerate(caucasianList) )]
    g_skin, g_noSkin = fit2({rootDataDir : considerList_fit, os.path.join(rootDataDir, 'black') : africanList_fit})
    print "fit phase done: gmm_skin: %r, gmm_nonSkin: %r" % (g_skin, g_noSkin)
    predict_gmm(
        {
            os.path.join(rootDataDir, 'misc'): ['f_hand'],
            rootDataDir : considerList_predict,
            os.path.join(rootDataDir, 'black') : africanList_predict
        }
    )
    pass

if __name__ == '__main__':
    main()
