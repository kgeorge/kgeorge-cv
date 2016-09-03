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
    w, h = cbMainImage.size
    for cb, cr, mask in zip( enumerate(cbMainImage.getdata()), enumerate(crMainImage.getdata()), enumerate(binMaskImage.getdata())):
        assert(cb[1] < 256 and cr[1] < 256)
        cb_2 = cb[1] * qLimit/256
        cr_2 = cr[1] * qLimit/256


        if mask[1] > 0:
            cBcR_skin.append((cb_2, cr_2))
        else:
            cBcR_noSkin.append((cb_2,cr_2))
    return (np.array(cBcR_skin), np.array(cBcR_noSkin))

def normalizeFloatImage2(floatImage):
    mn_0 = np.nanmin(np.nanmin(floatImage[:, :,0]))
    mx_0 = np.nanmax(np.nanmax(floatImage[:, :, 0]))

    mn_1 = np.nanmin(np.nanmin(floatImage[:, :,1]))
    mx_1 = np.nanmax(np.nanmax(floatImage[:, :, 1]))
    rows = floatImage.shape[0]
    cols = floatImage.shape[1]
    fctr = 255.0/(mx_0 - mn_0)
    mn_0 = 1000
    mx_0 =-1000
    for r in range(rows):
        for c in range(cols):
            if floatImage[r,c, 0] > -99:
                mn_0 = mn_0 if mn_0 <=  floatImage[r,c, 0 ] else floatImage[r,c, 0 ]
                mx_0 = mx_0 if mx_0  >=  floatImage[r,c, 0 ] else floatImage[r,c, 0 ]

            if floatImage[r,c, 1] > -99:
                mn_1 = mn_1 if mn_1 <=  floatImage[r,c, 1 ] else floatImage[r,c, 1 ]
                mx_1 = mx_1 if mx_1  >=  floatImage[r,c, 1 ] else floatImage[r,c, 1 ]

    print "mn_0: ", mn_0, "  mx_0: ", mx_0
    print "mn_1: ", mn_1, "  mx_1: ", mx_1


def normalizeFloatImage3(floatImage):

    mn_0 = np.nanmin(np.nanmin(floatImage[:, :,1]))
    mx_0 = np.nanmax(np.nanmax(floatImage[:, :, 1]))

    rows = floatImage.shape[0]
    cols = floatImage.shape[1]
    mn_0 = 1000
    mx_0 =-1000
    for r in range(rows):
        for c in range(cols):
            if floatImage[r,c, 2] <= 0:
                mn_0 = mn_0 if mn_0 <=  floatImage[r,c, 1 ] else floatImage[r,c, 1 ]
                mx_0 = mx_0 if mx_0  >=  floatImage[r,c, 1 ] else floatImage[r,c, 1 ]

    fctr_0 = 255.0/(mx_0 - mn_0)

    for r in range(rows):
        for c in range(cols):
            if floatImage[r,c, 2]  <= 0:
                floatImage[r,c, 1] = (floatImage[r,c, 1] - mn_0) * fctr_0
            else:
                floatImage[r,c, 1]=0

    print "mn_0: ", mn_0, "  mx_0: ", mx_0


def processImageTupleYCbCr_predict_gmm(rootDir, mainImagename, g_skin, g_noSkin, qLimit):
    def getImageIndex(r, c, w):
        return r * w + c
    def inspectNeighborhood(r_y, c_x, halfWindow=3):
        for d_r in range(-1,2):
            for d_c in range(-1,2):
                xy_neighbor = getImageIndex(r_y + d_r, c_x + d_c, mainImage.size[0])
                assert(xy_neighbor < len(cbMainImage.getdata()))
                cb_neighbor = cbMainImage.getdata()[xy_neighbor]
                cr_neighbor = crMainImage.getdata()[xy_neighbor]
                cb_neighbor2 = cb_neighbor * qLimit/256
                cr_neighbor2 = cr_neighbor * qLimit/256

                skinScore = g_skin.score(np.array([[cb_neighbor2, cr_neighbor2]]))
                nonSkinScore = g_noSkin.score(np.array([(cb_neighbor2, cr_neighbor2)]))
        pass
    mode = "YCbCr"
    maskImagename = getMaskImagename(mainImagename)
    def getClassifiedImagename(imageFilename, mode):
        m = r_mainImage.match(imageFilename)
        if m:
            maskImagename = "%s_gmm_%s" % (m.group('imgFilename'), mode) + '.png'
            return maskImagename
        return None
    def getThresholdedImagename(imageFilename, mode):
        m = r_mainImage.match(imageFilename)
        if m:
            maskImagename = "%s_gmm_thrshld_diff_%s" % (m.group('imgFilename'), mode) + '.png'
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
    thresholdedOutImg_0 = np.zeros((yMainImage.size[1],yMainImage.size[0],3), dtype = np.float)
    thresholdedOutImg_1 = np.zeros((yMainImage.size[1],yMainImage.size[0],3), dtype = np.float)
    thresholdedOutImg = np.zeros((yMainImage.size[1], yMainImage.size[0]), dtype = np.float)
    for cb, cr, mask in zip( enumerate(cbMainImage.getdata()), enumerate(crMainImage.getdata()), enumerate(binMaskImage.getdata())):
        xy = cb[0]
        assert(cb[1] < 256 and cr[1] < 256)
        cb_2 = cb[1] * qLimit/256
        cr_2 = cr[1] * qLimit/256
        skinScore = g_skin.score(np.array([[cb_2, cr_2]]))
        nonSkinScore = g_noSkin.score(np.array([(cb_2, cr_2)]))


        c_x =  xy % mainImage.size[0]
        r_y = int(xy / mainImage.size[0])

        #inspectNeighborhood(r_y, c_x, cbMainImage.getdata(), crMainImage.getdata(), binMaskImage.getdata())
        thresholdedOutImg[r_y, c_x] = skinScore / nonSkinScore
        try:

            thresholdedOutImg_0[r_y, c_x, 2] =  128
            thresholdedOutImg_1[r_y, c_x, 2] =  0
            if mask[1] > 0:
                thresholdedOutImg_0[r_y, c_x, 2] =  0
                thresholdedOutImg_1[r_y, c_x, 2] =  128
                if skinScore <= nonSkinScore:
                    thresholdedOutImg_0[r_y, c_x, 1] = -(skinScore - nonSkinScore)
            else:
                if skinScore > nonSkinScore:
                    thresholdedOutImg_1[r_y, c_x, 1] = skinScore - nonSkinScore
        except IndexError:
            print xy, c_x, r_y
        bActualSkin = False
        if mask[1] > 0:
            bActualSkin = True
        bAlgorithmSkin = False

        if skinScore > nonSkinScore:
            #get neighbor hood
            assert(cb[0] == cr[0])
            assert(cb[0] == mask[0])
            bAlgorithmSkin = True
            maskImage.putpixel((c_x,r_y), (255,255,255))
    classifiedImagename = getClassifiedImagename(mainImagename, "CbCr")
    normalizeFloatImage3(thresholdedOutImg_0)
    normalizeFloatImage3(thresholdedOutImg_1)
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
    cv2.imwrite(os.path.join(rootDir, getThresholdedImagename(mainImagename, "CbCr_0")), thresholdedOutImg_0)
    cv2.imwrite(os.path.join(rootDir, getThresholdedImagename(mainImagename, "CbCr_1")), thresholdedOutImg_1)
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



def fit2(cBcR_skin_all_data_for_fit, cBcR_noSkin_all_data_for_fit, n_components=5, mode = "YCbCr"):

    g_skin = g_noSkin = None

    if mode == "NRGB":
        raise Exception('not implemented')
    elif mode == "YCbCr":
        print "cBcR_skin_all_data_for_fit.shape", cBcR_skin_all_data_for_fit.shape
        g_skin = mixture.GMM(n_components=n_components)
        g_skin.fit(cBcR_skin_all_data_for_fit)
        g_noSkin = mixture.GMM(n_components=n_components)
        g_noSkin.fit(cBcR_noSkin_all_data_for_fit)

    return (g_skin, g_noSkin)


def evaluate_gmm_with_n_components(cBcR_skin_all_data_for_fit, cBcR_noSkin_all_data_for_fit,  mode = "YCbCr"):
    g_skin = g_noSkin = None

    if mode == "NRGB":
        raise Exception('not implemented')
    elif mode == "YCbCr":
        print "cBcR_skin_all_data_for_fit.shape", cBcR_skin_all_data_for_fit.shape
        for k in range(1,11):
            g_skin = mixture.GMM(n_components=k, covariance_type='full')
            g_skin.fit(cBcR_skin_all_data_for_fit)
            print g_skin.means_
            dummyData = np.zeros(shape=(1, cBcR_skin_all_data_for_fit.shape[1]))
            dummyData[0, :] = cBcR_skin_all_data_for_fit[0, :]
            aic_skin = g_skin.aic(cBcR_skin_all_data_for_fit)
            g_noSkin = mixture.GMM(n_components=k, covariance_type='full')
            g_noSkin.fit(cBcR_noSkin_all_data_for_fit)
            aic_noSkin = g_noSkin.aic(cBcR_noSkin_all_data_for_fit)
            print k, aic_skin, aic_noSkin


def collectData(dataSets, qlimit =128, mode = "YCbCr"):
    def getMaskImagename2(imageFilename):
        maskImagename = imageFilename + '_mask.png'
        return maskImagename

    maxSampleFiles = -1
    cBcR_skin_sum_list = []
    cBcR_noSkin_sum_list = []
    nSampleFilesSoFar =0

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
    return (np.concatenate(cBcR_skin_sum_list), np.concatenate(cBcR_noSkin_sum_list))

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
    (cBcR_skin_data_for_fit, cBcR_noSkin_data_for_fit) = collectData({rootDataDir : considerList_fit, os.path.join(rootDataDir, 'black') : africanList_fit})
    #evaluate_gmm_with_n_components(cBcR_skin_data_for_fit, cBcR_noSkin_data_for_fit, mode="YCbCr")
    """
This produced the following result.
1 13794423.0642 42111054.0496
2 13142249.6363 37740761.1692
3 12666717.7048 36718344.8858
4 12422719.7046 35732041.1953
5 12273082.8733 35144556.9394
6 12213520.9491 34752581.6618
7 12154175.3038 34737119.6411
8 11616663.5811 32558300.2443
9 12120952.4259 30129390.2751
10 11397247.673 32367938.5858
    """
    (g_skin, g_noSkin) = fit2(cBcR_skin_data_for_fit, cBcR_noSkin_data_for_fit, n_components=5)
    print "fit phase done: gmm_skin: %r, gmm_nonSkin: %r" % (g_skin, g_noSkin)
    print "gmm_skin: weights, gmm_nonSkin: %r" %  g_skin.weights_
    print "gmm_skin: weights, gmm_nonSkin: %r" % g_noSkin.weights_
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
