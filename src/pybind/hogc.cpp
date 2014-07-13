

#include "hogc.hpp"
#include "NumPyArrayData.hpp"

namespace np = boost::numpy;



// Functions t
void gammaCorrect( np::ndarray& imgArr, float gamma, bool reverse ) {
    //assert(imgArr.);
    NumPyArrayData<unsigned char> imgData(imgArr);
    Py_intptr_t const *pShape =   imgArr.get_shape();
    assert( imgArr.get_nd() == 3);
    
    int h = *(pShape+0);
    int w = *(pShape+1);
    int d = *(pShape+2);
    float lut[256];
    float reciprocal255 = 1.0f/255.0f;
    float gammaExponent =  (reverse) ? (1.0f/gamma) : gamma;
    for(int i=0; i < 256; ++ i) {
        lut[i] = pow(i * reciprocal255, gammaExponent);
    }
    
    for(int i=0; i < h; ++i) {
        for(int j=0; j < w; ++j) {
            imgData(i,j,0) = round(lut[imgData(i,j,0)] * 255);
            imgData(i,j,1) = round(lut[imgData(i,j,1)] * 255);
            imgData(i,j,2) = round(lut[imgData(i,j,2)] * 255);
        }
    }
}

//since I haven't fuond a  way to write boost=python method where std::string
//can e passed as a reference
std::string describeStatsMaker( const Kg::StatsMaker & sm) {
    std::string desc;
    sm.describe_(desc);
    return desc;
}


std::string serializeFloatVec(const std::vector<float> &floatVec) {
    std::stringstream ss;
    for( int i=0; i < floatVec.size(); ++i) {
        ss << floatVec[i];
        if(i < floatVec.size()-1) {
            ss << ", ";
        }
    }
    ss.flush();
    return ss.str();
}

void copyToNpArray( const::std::vector<float> &floatVec, int firstIdx, np::ndarray& dest ) {
    Py_intptr_t const *pShape =   dest.get_shape();
    NumPyArrayData<float> floatDest(dest);
    assert(floatVec.size() + firstIdx <= *pShape);
    for(int i=0; i < floatVec.size(); ++i) {
        floatDest(i+firstIdx) = floatVec[i];
    }
    return;
}

std::vector<float> makeTestFloatVec() {
    std::vector<float> ret(10, 0.0);
    for(int i=0; i < 10; ++i) {
        ret[i] = i;
    }
    return ret;
}

void computeAngleAndMagnitudeSigned( const Kg::Point2f & gradiant, AngleAndMagnitude & am) {
    float angle = atan(gradiant.r_/gradiant.c_);
    
    if(gradiant.r_ == 0) {
        angle = 0.0f;
    }
    //angle should vary from 0 to 2Pi
    if (gradiant.c_ < 0 && gradiant.r_ > 0) {
        angle += M_PI;
    } else if( gradiant.c_ < 0 && gradiant.r_ < 0) {
        angle += M_PI;
    } else if( gradiant.c_ < 0 && gradiant.r_ == 0) {
        angle += M_PI;
    } else if( gradiant.c_ > 0 && gradiant.r_ < 0 ){
        angle += 2* M_PI;
    } else if (gradiant.c_ == 0 && gradiant.r_ < 0) {
        angle += 2*M_PI;
    }
    assert(angle >= 0);
    am.set_a(angle);
    am.set_m(sqrt(gradiant.c_*gradiant.c_ + gradiant.r_ * gradiant.r_));
}


void computeAngleAndMagnitudeUnSigned( const Kg::Point2f & gradiant, AngleAndMagnitude & am) {
    float angle = atan(gradiant.r_/gradiant.c_);
    
    if(gradiant.r_ == 0) {
        angle = 0.0f;
    }
    angle = angle + M_PI/2.0;
    EpsilonEq<float> ep;
    if(!(((angle > 0) || ep(angle, 0)) && ((angle < M_PI) || ep(angle, M_PI)))) {
        //std::cout << "angle: " << angle << std::endl;
        assert(false);
    }
    am.set_a(angle);
    am.set_m(sqrt(gradiant.c_*gradiant.c_ + gradiant.r_ * gradiant.r_));
}

void Hog::computeCell(
                      np::ndarray& imgArr,
                      int r_cell, int c_cell,
                      Kg::Point2i &topLeft,
                      Kg::StatsMaker &smAngle,
                      Kg::StatsMaker &smMag,
                      AngleHistogram &aHist,
                      bool debug
                      ) {
    
    NumPyArrayData<unsigned char> imgData(imgArr);
    for(int r = r_cell+1; r < r_cell + scheme.numPixelsInCellPerSide_ - 1; ++r) {
        for(int c = c_cell+1; c < c_cell + scheme.numPixelsInCellPerSide_ - 1; ++c ) {
            Kg::Point2f p = Kg::Point2f(
                                        (imgData(r+1,c) - imgData(r-1,c)) * 0.5,
                                        (imgData(r,c+1) - imgData(r,c-1)) * 0.5
                                        );
            //std::cout << "\t" << r << "," << c << std::endl;
            AngleAndMagnitude am;
            int rOffset = r - topLeft.r_;
            int cOffset = c - topLeft.c_;
            computeCore_(p, rOffset, cOffset, am);
            if( debug) {
                std::cout << "~~~~~~~~~~~~r,c:" << r << "," << c << ", angle:" << am.get_a() << ", mag: " << am.get_m() << std::endl;
            }
            aHist.addSample(am.get_a(), am.get_m());
            smAngle.addSample(am.get_a());
            smMag.addSample(am.get_m());
        }
    }
}

void Hog::computeBlock(
                       np::ndarray& imgArr,
                       const AngleHistogram &aHistTemplate,
                       int r, int c,
                       Kg::StatsMaker &smAngle,
                       Kg::StatsMaker &smMag,
                       bool debug,
                       std::vector<float> &blockHist
                       ){
    blockHist.reserve(scheme.numCellsInUnitPerSide_ * scheme.numCellsInUnitPerSide_ * aHistTemplate.nBins);
    assert(scheme.validateImage( imgArr ));
    Kg::Point2i topLeft, botRight;
    scheme.computeIndexLimits( r, c, topLeft, botRight );
    for(int r_cell= topLeft.r_; r_cell < botRight.r_; r_cell += scheme.numPixelsInCellPerSide_) {
        for(int c_cell = topLeft.c_; c_cell < botRight.c_;  c_cell += scheme.numPixelsInCellPerSide_) {
            AngleHistogram aHist(aHistTemplate);
            computeCell(imgArr, r_cell, c_cell, topLeft, smAngle, smMag, aHist, debug);
            AngleHistogram::THist &ahistVec = aHist.get_hist();
            std::copy(ahistVec.begin(), ahistVec.end(), std::back_inserter(blockHist));
        }
    }
    if(eBlockNormalizationPolicy == eL2Normalization) {
        float blockSum =0.0;
        for(int i=0; i < blockHist.size(); ++i) {
            blockSum += blockHist[i] * blockHist[i];
        }
        blockSum = sqrt(blockSum);
        EpsilonEq<float> ep;
        if(ep(blockSum, 0) ) {
            //std::cout << blockHist.size() << ", " << blockSum << std::endl;
            blockSum = 2 * 1.0e-4;
        } else  {
            for(int i=0; i < blockHist.size(); ++i) {
                blockHist[i] = blockHist[i]/blockSum;
            }
        }
    }
}

bool Hog::validateHistogramResultForBlock(const AngleHistogram &aHistTemplate, const std::vector<float> &histVec) {
    bool retVal = histVec.size() == aHistTemplate.nBins * scheme.numCellsInUnitPerSide_ * scheme.numCellsInUnitPerSide_;
    if(eBlockNormalizationPolicy == eL2Normalization ) {
        float magnitude = 0.0f;
        for(int i=0; i < histVec.size(); ++i) {
            magnitude += histVec[i] * histVec[i];
        }
        magnitude = sqrt(magnitude);
        EpsilonEq<float> ep;
        retVal = retVal && ep(magnitude, 1.0f);
    }
    return retVal;
}

