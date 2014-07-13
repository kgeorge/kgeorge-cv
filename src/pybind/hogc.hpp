#if !defined(HOGC_HPP_)
#define HOGC_HPP_

#include "kgeorge_cv/kgUtils.hpp"
#include <iostream>
namespace np = boost::numpy;

// Functions t
void gammaCorrect( np::ndarray& imgArr, float gamma, bool reverse );




struct GaussianSpatialWeightForBlock {
    GaussianSpatialWeightForBlock(int numPixelsInBlockPerSide, float sigma ):
    sigma(sigma),
    blockWidth(numPixelsInBlockPerSide),
    blockWidthBy2_(numPixelsInBlockPerSide/2),
    blockHeight(numPixelsInBlockPerSide),
    blockHeightBy2_(numPixelsInBlockPerSide/2){
        //block width and block height are assumed to be even numbers
        assert(numPixelsInBlockPerSide%2 == 0);
        int storageReq = (blockWidthBy2_*blockWidthBy2_) + (blockHeightBy2_ * blockHeightBy2_);
        lut_.assign(storageReq, -1.0f);
        initLut_();
    }
    
    void initLut_() {
        float sigmaSq = sigma * sigma;
        //for blockHeight == 8, r varies 0,1,2,3
        for(int r=0; r < blockHeight/2; ++r) {
            for(int c =0; c< blockWidth/2; ++c) {
                int rSq = r*r + c*c;
                //corresponding to r, y varies from , 0.5,1.5,2.5, 3.5
                float y = static_cast<float>(r+blockHeightBy2_) + 0.5 - static_cast<float>(blockHeightBy2_);
                float x = static_cast<float>(c+blockWidth/2) + 0.5 - static_cast<float>(blockWidthBy2_);
                assert(rSq < lut_.size());
                lut_[rSq] = pow(M_E, -(x*x + y*y)/(2.0f * sigmaSq)) /(2.0f * M_PI * sigmaSq);
            }
        }
    }
    
    
    float weight(int r, int c) {
        assert(r < blockHeight);
        assert(c < blockWidth);
        //for blockHeight == 8,
        //r could be anything from 0-7 (inclusive)
        r -= blockHeight/2;
        c -= blockWidth/2;
        r += (r < 0) ? 1:0;
        c += (c < 0) ? 1:0;
        //corresponding to input, r =0,1,2,34,5,6,7,
        //r at  this pointe varies as -3,-2,-1,0,0,1,2,3
        
        assert(r < blockHeight/2);
        assert(c < blockWidth/2);
        
        int rSq = r*r + c*c;
        assert(rSq < lut_.size());
        assert(lut_[rSq] != -1.0f);
        return lut_[rSq];
    }
    const std::vector<float> &get_lut()const { return lut_;}
    float sigma;
    std::vector<float> lut_;
    int blockWidth;
    int blockHeight;
    int blockWidthBy2_;
    int blockHeightBy2_;
};

class HogException: public std::runtime_error {
public:
    HogException(const std::string &except):runtime_error(except){}
    
    
};

struct AngleAndMagnitude : private Kg::Point2f {
    AngleAndMagnitude(){}
    AngleAndMagnitude(float a, float m){
        set_a(a);
        set_m(m);
    }
    float get_a()const { return r_;}
    float get_m()const {return c_;}
    void set_a(float  a){ r_ = a;}
    void set_m(float m){ c_ = m;}
};


typedef Kg::Histogram1<float, float> Histogram1ff;

struct AngleHistogram: public Histogram1ff {
    //when you add a sample add one to the chosen bin or add the magntude
    typedef enum {eOne, eMagnitude  } EContributionPolicy;
    //when you add a sample, interpolate the dample among the two nerarest bins and
    //distribute the contribution
    typedef enum {eNoInterpolation, eYesInterpolation} EInterpolationPolicy;
    AngleHistogram(
                   int nBins,
                   float minSample,
                   float maxSample,
                   EContributionPolicy eContributionPolicy = AngleHistogram::eOne,
                   EInterpolationPolicy eInterpolationPolicy = AngleHistogram::eYesInterpolation):
    Histogram1ff(nBins, minSample, maxSample),
    eContributionPolicy(eContributionPolicy),
    eInterpolationPolicy(eInterpolationPolicy)
    {}
    AngleHistogram( const AngleHistogram &rhs):Histogram1ff(rhs.nBins, rhs.minSample, rhs.maxSample),
    eContributionPolicy(rhs.eContributionPolicy),
    eInterpolationPolicy(rhs.eInterpolationPolicy)
    {}
    
    float getContribution( float magnitude ) {
        
        return (eContributionPolicy == eMagnitude) ? magnitude: (magnitude > 0) ? 1 : 0 ;
    }
    
    void addSample(float sample, float magnitude) {
        assert(eInterpolationPolicy == AngleHistogram::eYesInterpolation);
        
        assert(sample < maxSample || ep(sample, maxSample));
        assert(sample > minSample || ep(sample, minSample));
        sample = Kg::clamp(sample, minSample, maxSample);
        int whichBin = floor(static_cast<R>(sample - minSample)/binWidth_);
        if(whichBin >= nBins) {
            assert(ep(sample,maxSample));
            whichBin -=1;
        }
        float binRep = binRepresentative(whichBin);
        
        float contributionToBin = getContribution(magnitude);
        
        if(  sample < binRep && whichBin > 0) {
            assert(sample > (binRep - binWidth_));
            float contributionFactorToLowerBin =  (binRep - sample)/binWidth_;
            assert(contributionFactorToLowerBin  >= 0.0 && contributionFactorToLowerBin <= 1.0);
            hist[whichBin-1] += contributionToBin * contributionFactorToLowerBin;
            contributionToBin = (1 - contributionFactorToLowerBin) * contributionToBin;
        }
        if(sample > binRep && whichBin < (nBins-1)) {
            assert( sample < (binRep + binWidth_));
            float contributionFactorToUpperBin =  (sample - binRep)/binWidth_;
            assert(contributionFactorToUpperBin  >= 0.0 && contributionFactorToUpperBin <= 1.0);
            hist[whichBin+1] += contributionToBin * contributionFactorToUpperBin;
            contributionToBin = (1 - contributionFactorToUpperBin) * contributionToBin;
        }
        hist[whichBin] += contributionToBin;
        nSamples++;
    }
    EpsilonEq<float> ep;
    EContributionPolicy eContributionPolicy;
    EInterpolationPolicy eInterpolationPolicy;
    
    
};




//since I haven't fuond a  way to write boost=python method where std::string
//can e passed as a reference
std::string describeStatsMaker( const Kg::StatsMaker & sm);



//since I haven't fuond a  way to write boost=python method where std::string
//can e passed as a reference
template <typename H>
std::string describeHistogram( const H & hist, bool bVerbose);

std::string serializeFloatVec(const std::vector<float> &floatVec);

void copyToNpArray( const::std::vector<float> &floatVec, int firstIdx, np::ndarray& dest );

std::vector<float> makeTestFloatVec();

struct HogScheme {
    int numPixelsInCellPerSide_;//8
    int numCellsInUnitPerSide_;//2
    int numPixelsInBlockPerSide_;//16
    int numBlocksHorizontal_;
    int numBlocksVertical_;
    int w_;
    int h_;
    HogScheme(int w, int h):numPixelsInCellPerSide_(8),numCellsInUnitPerSide_(2),numPixelsInBlockPerSide_(16),w_(w),h_(h){
        numBlocksHorizontal_ = w/numPixelsInCellPerSide_ - 1;
        numBlocksVertical_ = h/numPixelsInCellPerSide_ - 1;
        assert(numPixelsInBlockPerSide_ >= 3);
    }
    
    int get_numPixelsInCellPerSide()const { return numPixelsInCellPerSide_;}
    int get_numCellsInUnitPerSide()const{ return numCellsInUnitPerSide_;}
    int get_numPixelsInBlockPerSide()const{ return numPixelsInBlockPerSide_;}
    int get_numBlocksHorizontal()const { return numBlocksHorizontal_;}
    int get_numBlocksVertical()const { return numBlocksVertical_;}
    
    void set_numPixelsInCellPerSide( int numPixelsInCellPerSide ){ numPixelsInCellPerSide_ = numPixelsInCellPerSide;}
    void set_numCellsInUnitPerSide( int numCellsInUnitPerSide ){ numCellsInUnitPerSide_ = numCellsInUnitPerSide; }
    void set_numPixelsInBlockPerSide( int numPixelsInBlockPerSide ){ numPixelsInBlockPerSide_ = numPixelsInBlockPerSide; }
    void set_numBlocksHorizontal( int numBlocksHorizontal ){ numBlocksHorizontal_ = numBlocksHorizontal;}
    void set_numBlocksVertical( int numBlocksVertical ){ numBlocksVertical_ = numBlocksVertical;}
    
    
    bool validateImage( np::ndarray& imgArr )const {
        bool bVal = true;
        bVal = bVal && (imgArr.get_shape()[0] == h_);
        bVal = bVal && (imgArr.get_shape()[1] == w_);
        //if it has dimensions 3, the  3rd domension should be of value  1
        if( imgArr.get_nd() > 2) {
            bVal = bVal && (imgArr.get_nd() == 3);
            bVal = bVal && (imgArr.get_shape()[2] == 1);
        }
        return bVal;
    }
    
    void computeIndexLimits( int row, int col, Kg::Point2i &topLeft, Kg::Point2i &botRight )const {
        topLeft.c_ = col * numPixelsInCellPerSide_;
        topLeft.r_ = row * numPixelsInCellPerSide_;
        botRight.c_ = (col + 2) * numPixelsInCellPerSide_;
        botRight.r_ = (row + 2) * numPixelsInCellPerSide_;
    }
    bool operator==(const HogScheme & rhs) {
        return numPixelsInCellPerSide_ ==  rhs.numPixelsInCellPerSide_ &&
        numCellsInUnitPerSide_ == rhs.numCellsInUnitPerSide_  &&
        numPixelsInBlockPerSide_ == rhs.numPixelsInBlockPerSide_ &&
        numBlocksHorizontal_ == rhs.numBlocksHorizontal_ &&
        numBlocksVertical_ == rhs.numBlocksVertical_  &&
        w_ == rhs.w_ &&
        h_ == rhs.h_;
    }
};

void computeAngleAndMagnitudeSigned( const Kg::Point2f & gradiant, AngleAndMagnitude & am);


void computeAngleAndMagnitudeUnSigned( const Kg::Point2f & gradiant, AngleAndMagnitude & am);


struct Hog {
    typedef enum {eYesGaussian, eNoGaussian} EBlockGaussianWeightingPolicy;
    typedef enum {eNoNormalization, eL2Normalization} EBlockNormalizationPolicy;
    Hog(
        const HogScheme &scheme,
        EBlockGaussianWeightingPolicy eGaussianPolicy,
        EBlockNormalizationPolicy eBlockNormalizationPolicy):
    scheme(scheme),
    eGaussianPolicy(eGaussianPolicy),
    eBlockNormalizationPolicy(eBlockNormalizationPolicy),
    gaussianWeighter(GaussianSpatialWeightForBlock(scheme.numPixelsInBlockPerSide_, scheme.numPixelsInBlockPerSide_*0.5f)){
        assert(3 <=  scheme.numPixelsInBlockPerSide_);
    }
    const HogScheme &get_scheme()const {
        return scheme;
    }
    
    void computeCell(
                     np::ndarray& imgArr,
                     int r_cell, int c_cell,
                     Kg::Point2i &topLeft,
                     Kg::StatsMaker &smAngle,
                     Kg::StatsMaker &smMag,
                     AngleHistogram &aHist,
                     bool debug =false
                     );
    
    void computeBlock(
                      np::ndarray& imgArr,
                      const AngleHistogram &aHistTemplate,
                      int r, int c,
                      Kg::StatsMaker &smAngle,
                      Kg::StatsMaker &smMag,
                      bool debug,
                      std::vector<float> &blockHist
                      );
    void computeCore_(Kg::Point2f &gradient,int r, int c, AngleAndMagnitude &am) {
        computeAngleAndMagnitudeUnSigned( gradient, am);
        if(eGaussianPolicy == eYesGaussian) {
            am.set_m( am.get_m() * gaussianWeighter.weight(r,c));
        }
    }
    bool validateHistogramResultForBlock(const AngleHistogram &aHistTemplate, const std::vector<float> &histVec);
    
    EBlockNormalizationPolicy eBlockNormalizationPolicy;
    EBlockGaussianWeightingPolicy eGaussianPolicy;
    const HogScheme scheme;
    GaussianSpatialWeightForBlock gaussianWeighter;
};

#endif //HOGC_HPP_