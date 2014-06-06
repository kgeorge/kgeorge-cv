from PIL import Image
import numpy as np
import os
import fnmatch
import re
import cv2, cv
import hogc
import sys
import random, math


import unittest


almostEqual = lambda a, b: round(a -b, 4) == 0

class TestMisc(unittest.TestCase):
    def setUp(self):
        pass
    def test_Point2i(self):
        p = hogc.Point2i()
        self.assertEqual(p.r, 0)
        self.assertEqual(p.c, 0)
        q = hogc.Point2i(20, 21)
        self.assertEqual(q.r, 20)
        self.assertEqual(q.c, 21)
        r =  hogc.Point2i()
        r.r = 31
        r.c = 41
        self.assertEqual(r.r, 31)
        self.assertEqual(r.c, 41)
        pass
    def test_copyToNpArray(self):
        floatVec = hogc.makeTestFloatVec()
        npFloatVec = np.zeros((len(floatVec)+2), dtype=np.float32)
        hogc.copyToNpArray(floatVec, 2, npFloatVec)
        for i in range(0, len(floatVec)):
            self.assertTrue(almostEqual(npFloatVec[i+2], floatVec[i]))
            pass


        pass




class TestHogScheme(unittest.TestCase):

    def setUp(self):
        self.height =64
        self.width = 128
        self.grayImg = np.zeros((self.height,self.width), np.uint8)
        self.hogScheme = hogc.HogScheme(self.width, self.height)

        self.grayImg[:,:] = 128
        pass


    def fillGrayGradationImg(self, colorFun):
        #colorFun = lambda r,c: (r * 16 * 2 + c * 2)
        numCellsHorizontal = self.width/self.hogScheme.numPixelsInCellPerSide
        numCellsVertical = self.height/self.hogScheme.numPixelsInCellPerSide
        for r in range(0, numCellsVertical):
            for c in range(0, numCellsHorizontal):
                grayCol = colorFun(r, c) #r * 16 * 2 + c * 2
                assert(grayCol <= 255)
                self.grayImg[r*8:r*8+8,c*8:c*8+8] = grayCol
                pass
        #cv2.imwrite("testHogImg.png", self.grayImg)

    def test_HogScheme(self):
        self.assertEqual(self.hogScheme.numPixelsInCellPerSide,8)
        self.assertEqual(self.hogScheme.numCellsInUnitPerSide,2)
        self.assertEqual(self.hogScheme.numPixelsInBlockPerSide,16)
        self.assertEqual(self.hogScheme.numBlocksHorizontal,15)
        self.assertEqual(self.hogScheme.numBlocksVertical,7)
        self.assertLessEqual(3, self.hogScheme.numPixelsInBlockPerSide)


    def test_HogSchemeEquality(self):
        scheme2 = hogc.HogScheme(self.width*2, self.height)
        self.assertFalse(scheme2 == self.hogScheme)
        scheme3 = hogc.HogScheme(self.width, self.height)
        self.assertTrue(scheme3 == self.hogScheme)

    def test_HogScheme_validate(self):
        self.assertTrue(self.hogScheme.validateImage(self.grayImg))


    def test_HogScheme_computeIndexLimits(self):
        random.seed(42)
        for i in range(0, 10):
            c = int(math.floor(random.random() * self.hogScheme.numBlocksHorizontal))
            r = int(math.floor(random.random() * self.hogScheme.numBlocksVertical))
            assert(c < self.hogScheme.numBlocksHorizontal)
            assert(r < self.hogScheme.numBlocksVertical)
            topLeft = hogc.Point2i()
            botRight = hogc.Point2i()
            self.hogScheme.computeIndexLimits(r,c, topLeft, botRight)
            self.assertEqual(topLeft.r,  r * self.hogScheme.numPixelsInCellPerSide)
            self.assertEqual(topLeft.c,  c * self.hogScheme.numPixelsInCellPerSide)
            self.assertEqual(botRight.r,  topLeft.r + self.hogScheme.numCellsInUnitPerSide * self.hogScheme.numPixelsInCellPerSide)
            self.assertEqual(botRight.c,  topLeft.c + self.hogScheme.numCellsInUnitPerSide * self.hogScheme.numPixelsInCellPerSide)
            pass



    def test_ComputeAngleAndMagnitudeSigned(self):
        almostEqual = lambda a, b: round(a -b, 4) == 0
        p1 = hogc.Point2f(0.0, 0.0)
        am = hogc.AngleAndMagnitude()
        hogc.computeAngleAndMagnitudeSigned(p1, am)
        self.assertTrue(almostEqual(0.0, am.a))
        self.assertTrue(almostEqual(0.0, am.m))
        p2 = hogc.Point2f(1.0, 1.0)
        hogc.computeAngleAndMagnitudeSigned(p2, am)
        self.assertTrue(almostEqual(math.atan(1.0), am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))

        p3 = hogc.Point2f(-1.0, 1.0)
        hogc.computeAngleAndMagnitudeSigned(p3, am)
        self.assertTrue(almostEqual(math.atan(-1.0) + math.pi*2, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))



        p4 = hogc.Point2f(1.0, -1.0)
        hogc.computeAngleAndMagnitudeSigned(p4, am)
        self.assertTrue(almostEqual(math.atan(-1.0) + math.pi, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))

        p5 = hogc.Point2f(-1.0, -1.0)
        hogc.computeAngleAndMagnitudeSigned(p5, am)
        self.assertTrue(almostEqual(math.atan(1.0) + math.pi, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))

        p6 = hogc.Point2f(0.0, -1.0)
        hogc.computeAngleAndMagnitudeSigned(p6, am)
        self.assertTrue(almostEqual(math.atan(0.0) + math.pi, am.a))
        self.assertTrue(almostEqual(1.0, am.m))



        p7 = hogc.Point2f(0.0, 1.0)
        hogc.computeAngleAndMagnitudeSigned(p7, am)
        self.assertTrue(almostEqual(math.atan(0.0), am.a))
        self.assertTrue(almostEqual(1.0, am.m))


        p8 = hogc.Point2f(1.0, 0.0)
        hogc.computeAngleAndMagnitudeSigned(p8, am)
        self.assertTrue(almostEqual(math.atan(float("inf")), am.a))
        self.assertTrue(almostEqual(1.0, am.m))


        p9 = hogc.Point2f(-1.0, 0.0)
        hogc.computeAngleAndMagnitudeSigned(p9, am)
        self.assertTrue(almostEqual(math.atan(-float("inf")) + math.pi*2, am.a))
        self.assertTrue(almostEqual(1.0, am.m))

        print "angleMag = ", am.a, " mag", am.m
        pass



    def test_ComputeAngleAndMagnitudeUnSigned(self):
        almostEqual = lambda a, b: round(a -b, 4) == 0
        p1 = hogc.Point2f(0.0, 0.0)
        am = hogc.AngleAndMagnitude()
        hogc.computeAngleAndMagnitudeUnSigned(p1, am)
        self.assertTrue(almostEqual(math.pi/2, am.a))
        self.assertTrue(almostEqual(0.0, am.m))
        p2 = hogc.Point2f(1.0, 1.0)
        hogc.computeAngleAndMagnitudeUnSigned(p2, am)
        self.assertTrue(almostEqual(math.atan(1.0) + math.pi/2, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))

        p3 = hogc.Point2f(-1.0, 1.0)
        hogc.computeAngleAndMagnitudeUnSigned(p3, am)
        self.assertTrue(almostEqual(math.atan(-1.0) + math.pi/2, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))



        p4 = hogc.Point2f(1.0, -1.0)
        hogc.computeAngleAndMagnitudeUnSigned(p4, am)
        self.assertTrue(almostEqual(math.atan(-1.0) + math.pi/2, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))

        p5 = hogc.Point2f(-1.0, -1.0)
        hogc.computeAngleAndMagnitudeUnSigned(p5, am)
        self.assertTrue(almostEqual(math.atan(1.0) + math.pi/2, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))

        p6 = hogc.Point2f(0.0, -1.0)
        hogc.computeAngleAndMagnitudeUnSigned(p6, am)
        self.assertTrue(almostEqual(math.atan(0.0) + math.pi/2, am.a))
        self.assertTrue(almostEqual(1.0, am.m))



        p7 = hogc.Point2f(0.0, 1.0)
        hogc.computeAngleAndMagnitudeUnSigned(p7, am)
        self.assertTrue(almostEqual(math.atan(0.0) + math.pi/2, am.a))
        self.assertTrue(almostEqual(1.0, am.m))


        p8 = hogc.Point2f(1.0, 0.0)
        hogc.computeAngleAndMagnitudeUnSigned(p8, am)
        self.assertTrue(almostEqual(math.atan(float("inf")) + math.pi/2, am.a))
        self.assertTrue(almostEqual(1.0, am.m))


        p9 = hogc.Point2f(-1.0, 0.0)
        hogc.computeAngleAndMagnitudeUnSigned(p9, am)
        self.assertTrue(almostEqual(math.atan(-float("inf")) + math.pi/2, am.a))
        self.assertTrue(almostEqual(1.0, am.m))

        print "angleMag = ", am.a, " mag", am.m
        pass


class TestHog(unittest.TestCase):

    def setUp(self):
        self.height =64
        self.width = 128
        self.grayImg = np.zeros((self.height,self.width), np.uint8)
        self.hogScheme = hogc.HogScheme(self.width, self.height)
        self.grayImg[:,:] = 128
        pass

    def test_hogInit(self):
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
        scheme = hog.scheme()
        self.assertEqual(hog.gaussianPolicy, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian)
        self.assertEqual(hog.normalizationPolicy, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
        self.assertTrue(scheme ==  self.hogScheme)
        pass


    def test_hogComputeCore(self):
        almostEqual = lambda a, b: round(a -b, 4) == 0
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
        p3 = hogc.Point2f(-1.0, 1.0)
        am = hogc.AngleAndMagnitude()
        hog.computeCore(p3, 0, 0, am)
        print "hogComputeCore", am.a
        self.assertTrue(almostEqual(math.atan(-1.0) + math.pi/2, am.a))
        self.assertTrue(almostEqual(math.sqrt(2.0), am.m))
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.YesGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        gwb = hogc.GaussianSpatialWeightForBlock(self.hogScheme.numPixelsInBlockPerSide, self.hogScheme.numPixelsInBlockPerSide/2)
        for i in range(0,10):
            r = int(math.floor(random.random()*self.hogScheme.numPixelsInBlockPerSide))
            c = int(math.floor(random.random()*self.hogScheme.numPixelsInBlockPerSide))
            r = (r-1) if (r >= self.hogScheme.numPixelsInBlockPerSide) else r
            c =  (c-1) if (c >= self.hogScheme.numPixelsInBlockPerSide) else c
            hog.computeCore(p3, r, c, am)
            self.assertTrue(almostEqual(math.sqrt(2.0)*gwb.weight(r,c), am.m))



    def test_computeCell(self):
        smAngle = hogc.StatsMaker("angle")
        smMag = hogc.StatsMaker("magnitude")
        blockTopLeft = hogc.Point2i(0,0)
        aHist = hogc.AngleHistogram(18, 0, 2*math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
        hog.computeCell( self.grayImg, 2, 3, blockTopLeft, smAngle, smMag, aHist, False)
        print "angle stats Maker", hogc.describeStatsMaker(smAngle)
        print "magnitude stats Maker", hogc.describeStatsMaker(smMag)
        pass


    def test_computeCellHorizontal(self):
        #black-en image
        self.grayImg[:,:] = 0

        #Take the cell whose topLeft is (8,8)
        # Arrange data as shown below for the cell.
        # this should give 6 *2 elements with a horizontal gradient
        # so  4th bin in the angle should get contribution from the 12 samples

        r_cellTopLeft=8
        c_cellTopLeft=8
        """
          #cell 8,8
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
        """
        self.grayImg[r_cellTopLeft:r_cellTopLeft+8,c_cellTopLeft:c_cellTopLeft+4] = 64
        self.grayImg[r_cellTopLeft:r_cellTopLeft+8,c_cellTopLeft+4:c_cellTopLeft+8] = 128
        smAngle = hogc.StatsMaker("angle")
        smMag = hogc.StatsMaker("magnitude")

        blockTopLeft = hogc.Point2i(r_cellTopLeft,c_cellTopLeft)
        aHist = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        hog.computeCell( self.grayImg, r_cellTopLeft, c_cellTopLeft, blockTopLeft, smAngle, smMag, aHist, False)
        hist = aHist.hist()
        self.assertEqual( hist[4], 12)
        for i in range(0,9):
            if i == 4:
                continue
            self.assertEqual(hist[i], 0)

        aHist = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        hog.computeCell( self.grayImg, r_cellTopLeft, c_cellTopLeft, blockTopLeft,  smAngle, smMag, aHist, False)
        hist = aHist.hist()
        self.assertEqual( hist[4], 12 * (128 - 64)*0.5)
        for i in range(0,9):
            if i ==4:
                continue
            self.assertEqual(hist[i], 0)

    def test_computeCellVertical(self):
        #black-en image
        self.grayImg[:,:] = 0
        almostEqual = lambda a, b: round(a -b, 4) == 0

        #Take the cell whose topLeft is (8,8)
        # Arrange data as shown below for the cell.
        # this should give 6 *2 elements with a horizontal gradient
        # so  8th bin in the angle should get contribution from the 12 samples

        r_cellTopLeft=8
        c_cellTopLeft=8
        """
          #cell 8,8
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64

          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128

        """
        blockTopLeft = hogc.Point2i(r_cellTopLeft,c_cellTopLeft)
        self.grayImg[r_cellTopLeft:r_cellTopLeft+4,c_cellTopLeft:c_cellTopLeft+8] = 64
        self.grayImg[r_cellTopLeft+4:r_cellTopLeft+8,c_cellTopLeft:c_cellTopLeft+8] = 128
        smAngle = hogc.StatsMaker("angle")
        smMag = hogc.StatsMaker("magnitude")
        aHist = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        hog.computeCell( self.grayImg, r_cellTopLeft, c_cellTopLeft, blockTopLeft,  smAngle, smMag, aHist, False)
        hist = aHist.hist()
        #print "LLLLLLLLLLL~~~~~vertical: " , hogc.describeHistogram1ff(aHist, verbose=True)
        self.assertEqual( hist[8], 12)
        for i in range(0,9):
            if i == 8:
                continue
            self.assertEqual(hist[i], 0)
        aHist = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        hog.computeCell( self.grayImg, r_cellTopLeft, c_cellTopLeft, blockTopLeft, smAngle, smMag, aHist, False)
        hist = aHist.hist()
        self.assertEqual( hist[8], 12 * (128 - 64)*0.5)
        for i in range(0,9):
            if i ==8:
                continue
            self.assertEqual(hist[i], 0)

    def test_computeBlockInit(self):
        smAngle = hogc.StatsMaker("angle")
        smMag = hogc.StatsMaker("magnitude")
        aHistTemplate = hogc.AngleHistogram(18, 0, 2*math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        histVec = hogc.FloatVec()
        hog.computeBlock( self.grayImg, aHistTemplate, 2, 3, smAngle, smMag,False, histVec)
        print "angle stats Maker", hogc.describeStatsMaker(smAngle)
        print "magnitude stats Maker", hogc.describeStatsMaker(smMag)
        pass



    def test_computeBlockHorizontal(self):
        #black-en image
        self.grayImg[:,:] = 0

        #Take the cell whose topLeft is (8,8)
        # Arrange data as shown below for the cell.
        # this should give 6 *2 elements with a horizontal gradient
        # so  4th bin in the angle should get contribution from the 12 samples
        r_blockIndex = 1
        c_blockIndex = 1

        r_cellTopLeft=8 * r_blockIndex
        c_cellTopLeft=8 * c_blockIndex

        """
          #cell 8,8
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
          64 64 64 64 128 128 128 128
        """
        self.grayImg[r_cellTopLeft:r_cellTopLeft+8,c_cellTopLeft:c_cellTopLeft+4] = 64
        self.grayImg[r_cellTopLeft:r_cellTopLeft+8,c_cellTopLeft+4:c_cellTopLeft+8] = 128
        smAngle = hogc.StatsMaker("angle")
        smMag = hogc.StatsMaker("magnitude")
        aHistTemplate = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        histVec = hogc.FloatVec()
        hog.computeBlock( self.grayImg, aHistTemplate, r_blockIndex, c_blockIndex, smAngle, smMag,False, histVec)
        self.assertEqual( histVec[4], 12)
        for i in range(0,len(histVec)):
            if i == 4:
                continue
            self.assertEqual(histVec[i], 0)
        histVec = hogc.FloatVec()
        #print "LLLLLLLLLLL~~~~~vertical: " , hogc.describeHistogram1ff(aHist, verbose=True)
        hog.computeBlock( self.grayImg, aHistTemplate, r_blockIndex-1, c_blockIndex, smAngle, smMag,False, histVec)
        desc = ""
        for i in histVec:
            desc += ", " + str(i)
        print " block horoz: ################## ", desc

        aHistTemplate = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        histVec = hogc.FloatVec()
        hog.computeBlock( self.grayImg, aHistTemplate, r_blockIndex, c_blockIndex, smAngle, smMag,False, histVec)
        self.assertEqual( histVec[4], 12 * (128 - 64)*0.5)
        for i in range(0,len(histVec)):
            if i ==4:
                continue
            self.assertEqual(histVec[i], 0)


    def test_computeBlockVertical(self):
        #black-en image
        self.grayImg[:,:] = 0

        almostEqual = lambda a, b: round(a -b, 3) == 0
        #Take the cell whose topLeft is (8,8)
        # Arrange data as shown below for the cell.
        # this should give 6 *2 elements with a horizontal gradient
        # so  4th bin in the angle should get contribution from the 12 samples
        r_blockIndex = 1
        c_blockIndex = 1
        r_cellTopLeft=8 * r_blockIndex
        c_cellTopLeft=8 * c_blockIndex
        """
          #cell 8,8
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64

          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128

        """
        self.grayImg[r_cellTopLeft:r_cellTopLeft+4,c_cellTopLeft:c_cellTopLeft+8] = 64
        self.grayImg[r_cellTopLeft+4:r_cellTopLeft+8,c_cellTopLeft:c_cellTopLeft+8] = 128


        r_cellTopLeft=8 * r_blockIndex + 8
        c_cellTopLeft=8 * c_blockIndex + 8

        self.grayImg[r_cellTopLeft:r_cellTopLeft+4,c_cellTopLeft:c_cellTopLeft+8] = 64
        self.grayImg[r_cellTopLeft+4:r_cellTopLeft+8,c_cellTopLeft:c_cellTopLeft+8] = 128

        smAngle = hogc.StatsMaker("angle")
        smMag = hogc.StatsMaker("magnitude")
        aHistTemplate = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
        histVec = hogc.FloatVec()
        hog.computeBlock( self.grayImg, aHistTemplate, r_blockIndex, c_blockIndex, smAngle, smMag,False, histVec)
        oneBySqrt2 =  1.0/math.sqrt(2.0)
        self.assertTrue( almostEqual(histVec[8],oneBySqrt2))
        self.assertTrue( almostEqual(histVec[35],oneBySqrt2))
        for i in range(0,len(histVec)):
            if i == 8 or i == 35:
                continue
            self.assertEqual(histVec[i], 0)
        aHistTemplate = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.L2Normalization)
        histVec = hogc.FloatVec()
        hog.computeBlock( self.grayImg, aHistTemplate, r_blockIndex, c_blockIndex, smAngle, smMag,False, histVec)
        self.assertTrue(almostEqual( histVec[8],  oneBySqrt2))
        self.assertTrue(almostEqual( histVec[35], oneBySqrt2))
        for i in range(0,len(histVec)):
            if i ==8 or i == 35:
                continue
            self.assertEqual(histVec[i], 0)

    def test_computeBlock(self):
        #black-en image
        self.grayImg[:,:] = 0

        #Take the cell whose topLeft is (8,8)
        # Arrange data as shown below for the cell.
        # this should give 6 *2 elements with a horizontal gradient
        # so  4th bin in the angle should get contribution from the 12 samples
        r_blockIndex = 1
        c_blockIndex = 1
        r_cellTopLeft=8 * r_blockIndex
        c_cellTopLeft=8 * c_blockIndex
        """
          #cell 8,8
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64
          64    64  64   64   64   64   64   64

          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128
          128  128  128  128  128  128  128  128

        """
        self.grayImg[r_cellTopLeft:r_cellTopLeft+4,c_cellTopLeft:c_cellTopLeft+8] = 64
        self.grayImg[r_cellTopLeft+4:r_cellTopLeft+8,c_cellTopLeft:c_cellTopLeft+8] = 128



        smAngle = hogc.StatsMaker("angle")
        smMag = hogc.StatsMaker("magnitude")
        aHistTemplate = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        histVec = hogc.FloatVec()
        hog.computeBlock( self.grayImg, aHistTemplate, r_blockIndex, c_blockIndex, smAngle, smMag,False, histVec)
        self.assertTrue(hog.validateHistogramResultForBlock(aHistTemplate, histVec))
        self.assertEqual( histVec[8], 12)
        for i in range(0,len(histVec)):
            if i == 8:
                continue
            self.assertEqual(histVec[i], 0)
        aHistTemplate = hogc.AngleHistogram(9, 0, math.pi, hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        hog = hogc.Hog(self.hogScheme, hogc.Hog.BlockGaussianWeightingPolicy.NoGaussian, hogc.Hog.BlockNormalizationPolicy.NoNormalization)
        histVec = hogc.FloatVec()
        hog.computeBlock( self.grayImg, aHistTemplate, r_blockIndex, c_blockIndex, smAngle, smMag,False, histVec)
        self.assertTrue(hog.validateHistogramResultForBlock(aHistTemplate, histVec))
        self.assertEqual( histVec[8], 12 * (128 - 64)*0.5)
        for i in range(0,len(histVec)):
            if i ==8:
                continue
            self.assertEqual(histVec[i], 0)

class TestHistogram1fi(unittest.TestCase):

    def setUp(self):
        self.myAlmostEqual = lambda a, b: round(a -b, 4) == 0
        random.seed(42)
        pass

    def test_Init(self):
        hist = hogc.Histogram1fi(9, -4.5, 4.5)
        self.assertEqual(hist.nBins, 9)
        self.assertTrue(self.myAlmostEqual(hist.minSample, -4.5))
        self.assertTrue(self.myAlmostEqual(hist.maxSample, 4.5))
        self.assertEqual(hist.nSamples, 0)
        pass


    def test_addSamples1(self):
        hist = hogc.Histogram1fi(9, -4.5, 4.5)
        #add border bin value, this shoud land in the second bin (ie bin indexed 1)
        hist.addSample(-3.5)
        nSamples = 1
        #add 4 more samples to the first bin
        while(nSamples < 5):
            sample = -3.5 + random.random()
            if (sample >= -2.5):
                continue
            hist.addSample(sample)
            nSamples +=1
        self.assertEqual(hist.nSamples, nSamples)
        histData = hist.hist()
        print "typeof histData", type(histData)
        self.assertEqual(histData[0], 0)
        self.assertEqual(histData[1], nSamples)
        for i in range(2, hist.nBins):
            self.assertEqual(histData[i], 0)
        pass


    def test_addSamples2(self):
        hist = hogc.Histogram1fi(9, -4.5, 4.5)
        #add border bin value, this shoud land in the second bin (ie bin indexed 1)
        hist.addSample(4.5)
        self.assertEqual(hist.nSamples, 1)
        histData = hist.hist()

        for i in range(2, hist.nBins-1):
            self.assertEqual(histData[i], 0)
        self.assertEqual(histData[hist.nBins-1], 1)
        histData[3] = 10
        desc =  hogc.describeHistogram1fi(hist, verbose=True)
        print "histogra, desc: ", desc
        pass

    def test_describe(self):
        r1 = re.compile(r'\d+[:]-?\d+[.]\d*[<][=]\d+[<]-?\d+[.]\d*')
        r2 = re.compile(r'-?\d+[.]?(\d+)?')
        hist = hogc.Histogram1fi(9, -4.5, 4.5)
        desc =  hogc.describeHistogram1fi(hist, verbose=True)
        self.assertTrue(type(desc), type(""))
        self.assertEqual(len(desc.split(",")[1:-1]), 9)
        for i in desc.split(",")[1:-1]:
            self.assertTrue(r1.match(i.strip()) != None)

        desc =  hogc.describeHistogram1fi(hist, verbose=False)
        self.assertEqual(len(desc.split(",")[:]), 9)
        for i in desc.split(","):
            self.assertTrue(r2.match(i.strip()) != None)
        pass

class TestHistogram1ff(unittest.TestCase):

    def setUp(self):
        self.myAlmostEqual = lambda a, b: round(a -b, 4) == 0
        random.seed(42)
        pass

    def test_Init(self):
        hist = hogc.Histogram1ff(9, -4.5, 4.5)
        self.assertEqual(hist.nBins, 9)
        self.assertTrue(self.myAlmostEqual(hist.minSample, -4.5))
        self.assertTrue(self.myAlmostEqual(hist.maxSample, 4.5))
        self.assertEqual(hist.nSamples, 0)
        pass


    def test_addSamples1(self):
        hist = hogc.Histogram1ff(9, -4.5, 4.5)
        #add border bin value, this shoud land in the second bin (ie bin indexed 1)
        hist.addSample(-3.5)
        nSamples = 1
        #add 4 more samples to the first bin
        while(nSamples < 5):
            sample = -3.5 + random.random()
            if (sample >= -2.5):
                continue
            hist.addSample(sample)
            nSamples +=1
        self.assertEqual(hist.nSamples, nSamples)
        histData = hist.hist()
        print "typeof histData", type(histData)
        self.assertEqual(histData[0], 0)
        self.assertEqual(histData[1], nSamples)
        for i in range(2, hist.nBins):
            self.assertEqual(histData[i], 0)
        pass


    def test_addSamples2(self):
        hist = hogc.Histogram1ff(9, -4.5, 4.5)
        #add border bin value, this shoud land in the second bin (ie bin indexed 1)
        hist.addSample(4.5)
        self.assertEqual(hist.nSamples, 1)
        histData = hist.hist()

        for i in range(2, hist.nBins-1):
            self.assertEqual(histData[i], 0)
        self.assertEqual(histData[hist.nBins-1], 1)
        histData[3] = 10
        desc =  hogc.describeHistogram1ff(hist, verbose=True)
        print "histogra, desc: ", desc


        pass



    def test_describe(self):
        r1 = re.compile(r'\d+[:]-?\d+[.]\d*[<][=]\d+[.]?\d*[<]-?\d+[.]\d*')
        r2 = re.compile(r'-?\d+[.]?(\d+)?')
        hist = hogc.Histogram1fi(9, -4.5, 4.5)
        desc =  hogc.describeHistogram1fi(hist, verbose=True)
        self.assertTrue(type(desc), type(""))
        self.assertEqual(len(desc.split(",")[1:-1]), 9)
        for i in desc.split(",")[1:-1]:
            self.assertTrue(r1.match(i.strip()) != None)

        desc =  hogc.describeHistogram1fi(hist, verbose=False)
        self.assertEqual(len(desc.split(",")[:]), 9)
        for i in desc.split(","):
            self.assertTrue(r2.match(i.strip()) != None)
        pass

class TestGaussianSpatialWeightForBlock(unittest.TestCase):

    def setUp(self):
        self.myAlmostEqual = lambda a, b: round(a -b, 4) == 0
        random.seed(42)
        pass


    def test_Init(self):
        gwb = hogc.GaussianSpatialWeightForBlock(16, 8)
        self.assertTrue(self.myAlmostEqual (gwb.sigma, 8.0))
        self.assertEqual(gwb.blockWidth, 16)
        self.assertEqual(gwb.blockHeight, 16)
        lut = gwb.lut()
        self.assertEqual(len(lut), (gwb.blockWidth * gwb.blockWidth + gwb.blockHeight * gwb.blockHeight)/4)
        pass



    def test_weight(self):
        def computeWeightForVerification(r, c, blockWidth, blockHeight):
            x = r + 0.5 - blockHeight/2
            y = c + 0.5 - blockWidth/2
            return math.pow(math.e, -(x*x + y *y) / (2.0 * sigmaSq))*oneBy2PiSigmaSq
        gwb = hogc.GaussianSpatialWeightForBlock(16, 8)
        sigmaSq = gwb.sigma * gwb.sigma
        oneBy2PiSigmaSq = 1.0/(2 * math.pi* sigmaSq)
        # test 10 random r, c pairs
        for i in range(0, 10):
            c = math.floor(random.random()*gwb.blockWidth)
            r = math.floor(random.random()*gwb.blockHeight)
            if(c == gwb.blockWidth):
                c -= 1
            if(r == gwb.blockHeight):
                r -= 1
            weight=computeWeightForVerification(r, c, gwb.blockWidth, gwb.blockHeight)
            self.assertTrue(self.myAlmostEqual(gwb.weight(int(r),int(c)), weight))
        #test some border values
        rcpairs = [(0,0), (0, gwb.blockWidth-1), (gwb.blockHeight-1, 0), (gwb.blockHeight-1, gwb.blockWidth-1)]
        topLeftCornerWeight = computeWeightForVerification(0, 0, gwb.blockWidth, gwb.blockHeight)
        for p in rcpairs:
            r = p[0]
            c = p[1]
            self.assertTrue(self.myAlmostEqual(gwb.weight(int(r),int(c)), topLeftCornerWeight))
        rcpairs = [(gwb.blockHeight/2-1,gwb.blockWidth/2-1), (gwb.blockHeight/2-1, gwb.blockWidth/2), (gwb.blockHeight/2, gwb.blockWidth/2-1), (gwb.blockHeight/2, gwb.blockWidth/2)]
        firstWeight = computeWeightForVerification(rcpairs[0][0], rcpairs[0][1], gwb.blockWidth, gwb.blockHeight)
        for p in rcpairs:
            r = p[0]
            c = p[1]
            self.assertTrue(self.myAlmostEqual(gwb.weight(int(r),int(c)), firstWeight))

        pass



class TestAngleHistogram(unittest.TestCase):

    def setUp(self):
        self.myAlmostEqual = lambda a, b: round(a -b, 4) == 0
        random.seed(42)
        pass

    def test_Init(self):
        hist = hogc.AngleHistogram(9, -4.5, 4.5,
                                   hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        self.assertEqual(hist.eContributionPolicy, hogc.AngleHistogram.ContributionPolicy.One)
        self.assertEqual(hist.eInterpolationPolicy, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)

        pass


    def test_addSamplesContributionPolicyOne(self):
        hist = hogc.AngleHistogram(9, -4.5, 4.5,
                                   hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        #add border bin value, this shoud land in the second bin (ie bin indexed 1)
        hist.addSample(-3.5, 100.0)
        nSamples = 1

        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 0.5))
        self.assertTrue(self.myAlmostEqual(histData[1], 0.5))
        hist.addSample(-2.7, 100.0)
        nSamples += 1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 0.5))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.2))
        self.assertTrue(self.myAlmostEqual(histData[2], 0.3))
        hist.addSample(-4.3, 100.0)
        nSamples +=1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 1.5))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.2))
        self.assertTrue(self.myAlmostEqual(histData[2], 0.3))
        hist.addSample(4.3, 100.0)
        nSamples +=1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 1.5))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.2))
        self.assertTrue(self.myAlmostEqual(histData[2], 0.3))
        self.assertTrue(self.myAlmostEqual(histData[8], 1.0))
        hist.addSample(-2.3, 100.0)
        nSamples += 1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 1.5))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.5))
        self.assertTrue(self.myAlmostEqual(histData[2], 1.0))
        self.assertTrue(self.myAlmostEqual(histData[8], 1.0))

        self.assertEqual(hist.nSamples, nSamples)
        for i in range(3, 8):
            self.assertEqual(histData[i], 0)
        pass


    def test_addSamplesContributionPolicyMagnitude(self):
        hist = hogc.AngleHistogram(9, -4.5, 4.5,
                                   hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        #add border bin value, this shoud land in the second bin (ie bin indexed 1)
        hist.addSample(-3.5, 100.0)
        nSamples = 1

        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 0.5 * 100))
        self.assertTrue(self.myAlmostEqual(histData[1], 0.5 * 100))
        hist.addSample(-2.7, 100.0)
        nSamples += 1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 0.5 * 100))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.2 * 100))
        self.assertTrue(self.myAlmostEqual(histData[2], 0.3 * 100))
        hist.addSample(-4.3, 100.0)
        nSamples +=1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 1.5 * 100))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.2 * 100))
        self.assertTrue(self.myAlmostEqual(histData[2], 0.3 * 100))
        hist.addSample(4.3, 100.0)
        nSamples +=1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 1.5 * 100))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.2 * 100))
        self.assertTrue(self.myAlmostEqual(histData[2], 0.3 * 100))
        self.assertTrue(self.myAlmostEqual(histData[8], 1.0 * 100))
        hist.addSample(-2.3, 100.0)
        nSamples += 1
        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 1.5 * 100))
        self.assertTrue(self.myAlmostEqual(histData[1], 1.5 * 100))
        self.assertTrue(self.myAlmostEqual(histData[2], 1.0* 100))
        self.assertTrue(self.myAlmostEqual(histData[8], 1.0* 100))

        self.assertEqual(hist.nSamples, nSamples)
        for i in range(3, 8):
            self.assertEqual(histData[i], 0)
        pass


    def test_addSamplesLimitingValues(self):
        minSample =  - 4.5
        maxSample = 4.5
        nBins = 9
        hist = hogc.AngleHistogram(nBins, minSample, maxSample,
                                   hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        #add a sample below minSample, but almost equal
        sample = minSample - 0.00000001
        self.assertTrue(self.myAlmostEqual(sample, minSample))
        hist.addSample(sample, 100.0)
        nSamples = 1

        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 1.0))
        #add a sample below minSample, but almost equal

        sample = maxSample + 0.00000001
        self.assertTrue(self.myAlmostEqual(sample, maxSample))
        hist.addSample(sample, 100.0)
        nSamples += 1

        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[nBins-1], 1.0))


        hist = hogc.AngleHistogram(nBins, minSample, maxSample,
                                   hogc.AngleHistogram.ContributionPolicy.Magnitude, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        #add a sample below minSample, but almost equal
        sample = minSample - 0.00000001
        kMagnitude = 100.0
        self.assertTrue(self.myAlmostEqual(sample, minSample))
        hist.addSample(sample, kMagnitude)
        nSamples = 1

        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[0], 100.0))
        #add a sample below minSample, but almost equal

        sample = maxSample + 0.00000001
        self.assertTrue(self.myAlmostEqual(sample, maxSample))
        hist.addSample(sample, kMagnitude)
        nSamples += 1

        histData = hist.hist()
        self.assertTrue(self.myAlmostEqual(histData[nBins-1], 100.0))

    def test_describe(self):
        hist = hogc.AngleHistogram(9, -4.5, 4.5, hogc.AngleHistogram.ContributionPolicy.One, hogc.AngleHistogram.InterpolationPolicy.YesInterpolation)
        desc =  hogc.describeHistogram1ff(hist, verbose=True)
        print "histogra, desc: ", desc
        self.assertTrue(type(desc), type(""))
        pass

"""
    def test_addSamples2(self):
        hist = hogc.Histogram1ff(9, -4.5, 4.5)
        #add border bin value, this shoud land in the second bin (ie bin indexed 1)
        hist.addSample(4.5)
        self.assertEqual(hist.nSamples, 1)
        histData = hist.hist()

        for i in range(2, hist.nBins-1):
            self.assertEqual(histData[i], 0)
        self.assertEqual(histData[hist.nBins-1], 1)
        histData[3] = 10
        desc =  hogc.describeHistogram1ff(hist, verbose=True)
        print "histogra, desc: ", desc


        pass
"""




class TestStatsMaker(unittest.TestCase):

    def setUp(self):
        pass

    def test_Init(self):
        sm = hogc.StatsMaker("mystats")
        self.assertEqual(sm.name, "mystats")
        self.assertEqual(sm.nSamples, 0)
        self.assertEqual(sm.variance, 0.0)
        self.assertEqual(sm.mean, 0.0)
        self.assertLess(1000000, sm.minVal)
        self.assertLess(sm.maxVal, -1000000)

    def test_MemberVariables(self):
        sm = hogc.StatsMaker("mystats")
        sm.addSample(-2)
        sm.addSample(2)
        sm.addSample(-4)
        sm.addSample(4)

        self.assertEqual(sm.nSamples, 4)
        self.assertEqual(sm.variance, 10.0)
        self.assertEqual(sm.mean, 0.0)
        self.assertEqual(sm.minVal, -4)
        self.assertEqual(sm.maxVal, 4)
        pass

    def test_Describe(self):
        sm = hogc.StatsMaker("mystats")
        description = hogc.describeStatsMaker( sm )
        print "description: " , description
        self.assertEqual(type(description), type(""))

class TestGammaCorrect(unittest.TestCase):

    def setUp(self):
        pass

    def test_GammaCorrect(self):
        height =2
        width = 5
        grayImg = np.zeros((height,width,3), np.uint8)
        grayImg[:,:] = (128,128,128)
        hogc.gammaCorrect(grayImg, 2, False )
        self.assertEqual(list(grayImg[1,2]) , [64,64,64])
        hogc.gammaCorrect(grayImg, 2, reverse=True )
        self.assertEqual(list(grayImg[1,4]) , [128,128,128])

if __name__ == '__main__':
    unittest.main()


