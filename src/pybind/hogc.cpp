
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/numpy.hpp>

#include "kgeorge_cv/kgUtils.hpp"
#include "NumPyArrayData.hpp"

//#include <numpy/arrayobject.h>
#include <iostream>

namespace bp = boost::python;
namespace np = boost::numpy;

// Functions to demonstrate extraction
void setArray( np::ndarray& arr ) {
    //assert(arr.);
    NumPyArrayData<unsigned char> arr_data(arr);
    Py_intptr_t const *pShape =   arr.get_shape();
    std::cout << "num dims: " << arr.get_nd();
    std::cout << "shape: " ;
    for(int i=0; i < arr.get_nd(); ++i) {
        std::cout << *(pShape + i) << " ";
    }
    std::cout << std::endl;
	// Need to <extract> array elements because their type is unknown
	std::cout << "First array item: " << arr_data(0,0,0) << std::endl;
}


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
std::string describeStatsMaker( const Kg::StatsMaker & sm) {
    std::string desc;
    sm.describe_(desc);
    return desc;
}



//since I haven't fuond a  way to write boost=python method where std::string
//can e passed as a reference
template <typename H>
std::string describeHistogram( const H & hist, bool bVerbose) {
    std::string desc;
    hist.describe(desc, bVerbose);
    return desc;
}

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


struct Hog {
    typedef enum {eYesGaussian, eNoGaussian} EBlockGaussianWeightingPolicy;
    Hog(
        const HogScheme &scheme,
        EBlockGaussianWeightingPolicy eGaussianPolicy):
        scheme(scheme),
        eGaussianPolicy(eGaussianPolicy),
        gaussianWeighter(GaussianSpatialWeightForBlock(scheme.numPixelsInBlockPerSide_, scheme.numPixelsInBlockPerSide_*0.5f)){
            assert(3 <=  scheme.numPixelsInBlockPerSide_);
        }
    const HogScheme &get_scheme()const {
        return scheme;
    }

    void computeBlock(
        np::ndarray& imgArr,
        int r, int c,
        Kg::StatsMaker &smAngle,
        Kg::StatsMaker &smMag,
        AngleHistogram &aHist,
        bool debug =false
    ){
        assert(scheme.validateImage( imgArr ));
        Kg::Point2i topLeft, botRight;
        scheme.computeIndexLimits( r, c, topLeft, botRight );

        NumPyArrayData<unsigned char> imgData(imgArr);

        for(int r=topLeft.r_ + 1; r < botRight.r_-1; ++r ){
            for(int c=topLeft.c_ + 1; c < botRight.c_-1; ++c) {
                Kg::Point2f p = Kg::Point2f(
                    (imgData(r+1,c) - imgData(r-1,c)) * 0.5,
                    (imgData(r,c+1) - imgData(r,c-1)) * 0.5
                );
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


    void computeCore_(Kg::Point2f &gradient,int r, int c, AngleAndMagnitude &am) {
        computeAngleAndMagnitudeUnSigned( gradient, am);
        if(eGaussianPolicy == eYesGaussian) {
            am.set_m( am.get_m() * gaussianWeighter.weight(r,c));
        }
    }

    EBlockGaussianWeightingPolicy eGaussianPolicy;
    const HogScheme scheme;
    GaussianSpatialWeightForBlock gaussianWeighter;
};

// Expose classes and methods to Python
BOOST_PYTHON_MODULE(hogc) {
    np::initialize();
	bp::def("setArray", &setArray);
	bp::def("gammaCorrect", &gammaCorrect,  bp::args("imgArr", "gamma", "reverse"));

	bp::def("computeAngleAndMagnitudeSigned", &computeAngleAndMagnitudeSigned);
    bp::def("computeAngleAndMagnitudeUnSigned", &computeAngleAndMagnitudeUnSigned);

	bp::class_<Kg::Point2i>("Point2i", bp::init<>())
	    .def(bp::init<int, int>())
	    .add_property("r", &Kg::Point2i::get_r, &Kg::Point2i::set_r)
        .add_property("c", &Kg::Point2i::get_c, &Kg::Point2i::set_c)
	;

	bp::class_<Kg::Point2f>("Point2f", bp::init<>())
	    .def(bp::init<float, float>())
	    .add_property("r", &Kg::Point2f::get_r, &Kg::Point2f::set_r)
        .add_property("c", &Kg::Point2f::get_c, &Kg::Point2f::set_c)
	;


	bp::class_<AngleAndMagnitude>("AngleAndMagnitude", bp::init<>())
	    .def(bp::init<float, float>())
	    .add_property("a", &AngleAndMagnitude::get_a, &AngleAndMagnitude::set_a)
        .add_property("m", &AngleAndMagnitude::get_m, &AngleAndMagnitude::set_m)
	;

	bp::class_<Kg::StatsMaker>("StatsMaker", bp::init<const std::string &>())
	    .def_readonly("name", &Kg::StatsMaker::name)
	    .def_readonly("nSamples", &Kg::StatsMaker::nSamples)
	    .def_readonly("mean", &Kg::StatsMaker::mean)
	    .def_readonly("variance", &Kg::StatsMaker::variance)
	    .def_readonly("minVal", &Kg::StatsMaker::minVal)
	    .def_readonly("maxVal", &Kg::StatsMaker::maxVal)
	    .def("addSample", &Kg::StatsMaker::addSample)
	;
    bp::def("describeStatsMaker", &describeStatsMaker);


    bp::class_<std::vector<int>>("IntVec")
        .def(bp::vector_indexing_suite<std::vector<int> >())
        ;

    bp::class_<Kg::Histogram1fi>("Histogram1fi", bp::init<int, float, float>())
	    .def_readonly("nBins", &Kg::Histogram1fi::nBins)
	    .def_readonly("nSamples", &Kg::Histogram1fi::nSamples)
	    .def_readonly("minSample", &Kg::Histogram1fi::minSample)
	    .def_readonly("maxSample", &Kg::Histogram1fi::maxSample)
	    .def("hist", &Kg::Histogram1fi::get_hist, bp::return_value_policy<bp::return_by_value>())
	    .def("addSample",  &Kg::Histogram1fi::addSample)
    ;
    bp::def("describeHistogram1fi", &describeHistogram<Kg::Histogram1fi>, bp::args("hist", "verbose"));



    bp::class_<std::vector<float>>("FloatVec")
        .def(bp::vector_indexing_suite<std::vector<float> >())
        ;

    bp::class_<Histogram1ff>("Histogram1ff", bp::init<int, float, float>())
	    .def_readonly("nBins", &Histogram1ff::nBins)
	    .def_readonly("nSamples", &Histogram1ff::nSamples)
	    .def_readonly("minSample", &Histogram1ff::minSample)
	    .def_readonly("maxSample", &Histogram1ff::maxSample)
	    .def("hist", &Histogram1ff::get_hist, bp::return_value_policy<bp::return_by_value>())
	    .def("addSample",  &Histogram1ff::addSample)
    ;
    bp::def("describeHistogram1ff", &describeHistogram<Histogram1ff>, bp::args("hist", "verbose"));

    {
        bp::scope inAngleHistogram =
            bp::class_<AngleHistogram, bp::bases<Histogram1ff> >("AngleHistogram", bp::init<int, float, float, AngleHistogram::EContributionPolicy, AngleHistogram::EInterpolationPolicy>())
                .def("addSample",  &AngleHistogram::addSample)
                .def_readonly("eContributionPolicy", &AngleHistogram::eContributionPolicy)
                .def_readonly("eInterpolationPolicy", &AngleHistogram::eInterpolationPolicy)
            ;
            bp::enum_<AngleHistogram::EContributionPolicy>("ContributionPolicy")
                .value("One", AngleHistogram::eOne)  //define AngleHistogram.One
                .value("Magnitude", AngleHistogram::eMagnitude) //define AngleHistogram.Magnitude
            ;

            bp::enum_<AngleHistogram::EInterpolationPolicy>("InterpolationPolicy")
                .value("NoInterpolation", AngleHistogram::eNoInterpolation) ////define AngleHistogram.NoInterpolation
            .value("YesInterpolation", AngleHistogram::eYesInterpolation) ////define AngleHistogram.YesInterpolation
            ;
    }



    bp::class_<HogScheme>("HogScheme", bp::init<int, int>())
    .def("validateImage", &HogScheme::validateImage)
    .def("computeIndexLimits", &HogScheme::computeIndexLimits)
    .add_property("numPixelsInCellPerSide", &HogScheme::get_numPixelsInCellPerSide, &HogScheme::set_numPixelsInCellPerSide)
    .add_property("numCellsInUnitPerSide", &HogScheme::get_numCellsInUnitPerSide, &HogScheme::set_numCellsInUnitPerSide)
    .add_property("numPixelsInBlockPerSide", &HogScheme::get_numPixelsInBlockPerSide, &HogScheme::set_numPixelsInBlockPerSide)
    .add_property("numBlocksHorizontal", &HogScheme::get_numBlocksHorizontal, &HogScheme::set_numBlocksHorizontal)
    .add_property("numBlocksVertical", &HogScheme::get_numBlocksVertical, &HogScheme::set_numBlocksVertical)
    .def(bp::self == bp::self);


    bp::class_<GaussianSpatialWeightForBlock>("GaussianSpatialWeightForBlock", bp::init<int, float>())
        .def_readonly("sigma", &GaussianSpatialWeightForBlock::sigma)
        .def_readonly("blockWidth", &GaussianSpatialWeightForBlock::blockWidth)
        .def_readonly("blockHeight", &GaussianSpatialWeightForBlock::blockHeight)
        .def("lut", &GaussianSpatialWeightForBlock::get_lut, bp::return_value_policy<bp::return_by_value>())
        .def("weight", &GaussianSpatialWeightForBlock::weight)
    ;

    {
        bp::scope inAngleHistogram =
            bp::class_<Hog>("Hog",
                bp::init<const HogScheme &,Hog::EBlockGaussianWeightingPolicy>())
                .def("scheme", &Hog::get_scheme, bp::return_value_policy<bp::return_by_value>())
                .def_readonly("gaussianPolicy", &Hog::eGaussianPolicy)
                .def("computeCore", &Hog::computeCore_)
                .def("computeBlock", &Hog::computeBlock)
            ;

            bp::enum_<Hog::EBlockGaussianWeightingPolicy>("BlockGaussianWeightingPolicy")
                .value("YesGaussian", Hog::eYesGaussian)  //define Hog.YesGaussian
                .value("NoGaussian",  Hog::eNoGaussian) //define Hog.NoGaussian
            ;

    }
}