__author__ = 'kgeorge'
from PIL import Image
import numpy as np
import os
import fnmatch
import re
import cv2, cv
import math
import time
import random
import hogc

import time
import sklearn.svm as skl


almostEqual = lambda a, b: round(a -b, 1) == 0



def normalizeImage(img):
    convertedImg = cv2.cvtColor(img, cv2.COLOR_BGR2YCR_CB)
    channels = cv2.split(convertedImg)
    channels[0] = cv2.equalizeHist(channels[0])
    convertedImg = cv2.merge(channels)
    convertedImg = cv2.cvtColor(convertedImg, cv2.COLOR_YCR_CB2BGR)
    return convertedImg

def putDataInX( X, y,  first_row, data, expectedLabel):
    j=first_row
    for i in range(0, data.shape[0]):
        colnum = data[i,0]
        #assert(expectedLabel == data[i,1])
        X[j,colnum*36:(colnum+1)*36] = data[i,2:]
        if colnum==104:
            y[j] = float(expectedLabel)
            j += 1
        pass
    assert((j - first_row) * 105 == data.shape[0])

def validateData(clf, data_tobe_validated):
    W = np.zeros(((data_tobe_validated.shape[0])/105, (data_tobe_validated.shape[1]-2)*105))
    j=0
    for i in range(0, data_tobe_validated.shape[0]):
        colnum = data_tobe_validated[i,0]
        #assert(-1 == data_tobe_validated[i,1])
        W[j,colnum*36:(colnum+1)*36] = data_tobe_validated[i,2:]
        if colnum==104:
            j += 1
        pass
    assert(j * 105 == data_tobe_validated.shape[0])
    for i in range(0, W.shape[0]):
        thisRow = W[i,:]
        assert(thisRow.shape[0] == 36 * 105)
        print clf.predict(thisRow)
        pass


def learn():
    fname_good = "/Users/kgeorge/Dropbox/cars/tentative/build/hog_output.txt"
    good_data = np.genfromtxt(fname_good, delimiter=",")

    fname_bad = "/Users/kgeorge/Dropbox/cars/bad/build/hog_output.txt"
    bad_data = np.genfromtxt(fname_bad, delimiter=",")


    fname_crossvalidate = "/Users/kgeorge/Dropbox/cars/crossvalidate/build/hog_output.txt"
    crossvalidate_data = np.genfromtxt(fname_crossvalidate, delimiter=",")

    assert(good_data.shape[1] == bad_data.shape[1])
    assert(((good_data.shape[0] + bad_data.shape[0]) % 105) == 0)
    X = np.zeros(((good_data.shape[0] + bad_data.shape[0])/105, (good_data.shape[1]-2)*105))
    y = np.zeros(((good_data.shape[0] + bad_data.shape[0])/105))
    putDataInX(X, y, 0, good_data, 1)
    putDataInX(X, y, good_data.shape[0]/105, bad_data, -1)
    clf = skl.SVC(kernel="linear", verbose=True, probability=True)
    fitResult = clf.fit(X, y)
    validateData(clf, crossvalidate_data)
    return clf
    pass

def hog(img, hogScheme, aHistTemplate, smAngle, smMag, descriptor):
    #print img.width, img.height
    imgCopy = cv2.resize(img, (128, 64))
    numpyImg = np.asarray(imgCopy)
    hogc.gammaCorrect(numpyImg, 2.2, True)
    #normalizedImg = normalizeImage(numpyImg)
    convertedImg = cv2.cvtColor( numpyImg, cv2.COLOR_BGR2GRAY)
    h = convertedImg.shape[0]
    w = convertedImg.shape[1]
    assert(descriptor.shape[0] == hogScheme.numBlocksVertical *  hogScheme.numBlocksHorizontal * hogScheme.numCellsInUnitPerSide * hogScheme.numCellsInUnitPerSide * 9)
    assert(descriptor.dtype == np.float32)
    hog = hogc.Hog(hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.YesGaussian, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
    for r in range(0, hogScheme.numBlocksVertical):
        for c in range(0, hogScheme.numBlocksHorizontal):
            histVec = hogc.FloatVec()
            hog.computeBlock( convertedImg,aHistTemplate,  r, c, smAngle, smMag, False, histVec)
            hogc.copyToNpArray(histVec, (r*15 +c)*36, descriptor)
    return


def main():
    clf = learn()
    print "finished learning"
    cap = cv2.VideoCapture("data/sample.mov")
    #read first frame
    ret, firstFrame = cap.read()
    if not ret:
        return -1
    firstFramePyrDown = cv2.pyrDown(firstFrame)
    outRows = firstFramePyrDown.shape[0]
    outCols = firstFramePyrDown.shape[1]
    vout = cv2.VideoWriter()
    fps = 25
    fourcc = cv2.cv.CV_FOURCC('m', 'p', '4', 'v') # note the lower case
    capSize = (  int(cap.get(cv2.cv.CV_CAP_PROP_FRAME_WIDTH)), int(cap.get(cv2.cv.CV_CAP_PROP_FRAME_HEIGHT)) )
    success = vout.open('data/output.mov',fourcc,fps,(outCols, outRows),True)
    hogScheme = hogc.HogScheme(128,64)
    aHistTemplate = hogc.AngleHistogram(9, 0.0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
    smAngle = hogc.StatsMaker("angle")
    smMag = hogc.StatsMaker("magnitude")
    npFloatVec = np.zeros(105*9*4, dtype=np.float32)
    X = np.zeros((105*4*9))
    sampleWidth = 150
    sampleHeight = 75
    while(True):
        ret, frame = cap.read()

        if(ret):
            img = None
            img = cv2.pyrDown(frame)
            rows = img.shape[0]
            cols = img.shape[1]
            print rows, cols
            to_draw_rects= []
            selected_to_draw_rects=[]
            print "====================="

            for r in range(180, rows-sampleHeight, 2):
                for c in range(0, cols-sampleWidth, 2):

                    curImg = img[r:r+sampleHeight, c:c+sampleWidth]
                    hog(curImg, hogScheme, aHistTemplate, smAngle, smMag, npFloatVec)
                    prob = clf.predict_proba(npFloatVec)[0][1]
                    if (prob >= 0.6):
                        to_draw_rects.append((c,r, prob))
            print "reached here"
            if(len(to_draw_rects) > 0):
                to_draw_rects = sorted(to_draw_rects, key=lambda rec: rec[2], reverse=True)
                selected_to_draw_rects = [to_draw_rects[0]]
                for i in range(1, len(to_draw_rects)):
                    tup = to_draw_rects[i]
                    non_maximal = False
                    print "~~~~~~~~~~~~~~~~~~~~~~~"
                    for s in selected_to_draw_rects:
                        cx = s[0] - tup[0]
                        rx = s[1] - tup[1]
                        dsq = cx * cx + rx * rx
                        if(dsq <= 400):
                            non_maximal = True
                            break
                        else :
                            print dsq
                    if  not non_maximal:
                        selected_to_draw_rects.append(tup)


            for cr in selected_to_draw_rects:
                r = cr[1]
                c = cr[0]
                col = (0,255, 255)
                lineWidth =2

                if 0.6 <= cr[2] and cr[2] < 0.95:
                    lineWidth =1
                    col = (0, 255, 0)
                else:
                    assert(cr[2] >= 0.85)

                if cr[2] >= 0.85:
                    cv2.putText(img, "%.2f" % cr[2], (c+10, r+10), cv2.FONT_HERSHEY_COMPLEX_SMALL, 1.0, (255,255,255))
                    cv2.rectangle(img, (c,r), (c+sampleWidth,r+sampleHeight), col,lineWidth, 8 )
            vout.write(img)
            cv2.imshow('frame',img)
        if cv2.waitKey(1) & 0xFF == 27:
            break
        pass

    pass
    cap.release()
    cap = None
    vout.release()
    vout = None
    cv2.destroyAllWindows()

if __name__ == "__main__":
    main()



