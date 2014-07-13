
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>
#include <boost/numpy.hpp>

#include "hogc.hpp"
#include "NumPyArrayData.hpp"


namespace bp = boost::python;
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

//since I haven't fuond a  way to write boost=python method where std::string
//can e passed as a reference
template <typename H>
std::string describeHistogram( const H & hist, bool bVerbose) {
    std::string desc;
    hist.describe(desc, bVerbose);
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

// Expose classes and methods to Python
BOOST_PYTHON_MODULE(hogc) {
    np::initialize();
	//bp::def("setArray", &setArray);
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
    
    
    
    bp::class_<HogException>("HogException", bp::init<const std::string &>())
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
    bp::def("serializeFloatVec", &serializeFloatVec);
    bp::def("copyToNpArray", &copyToNpArray);
    bp::def("makeTestFloatVec", &makeTestFloatVec);
    
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
                        bp::init<const HogScheme &,Hog::EBlockGaussianWeightingPolicy, Hog::EBlockNormalizationPolicy>())
        .def("scheme", &Hog::get_scheme, bp::return_value_policy<bp::return_by_value>())
        .def_readonly("gaussianPolicy", &Hog::eGaussianPolicy)
        .def_readonly("normalizationPolicy", &Hog::eBlockNormalizationPolicy)
        .def("computeCore", &Hog::computeCore_)
        .def("computeCell", &Hog::computeCell)
        .def("computeBlock", &Hog::computeBlock)
        .def("validateHistogramResultForBlock", &Hog::validateHistogramResultForBlock)
        ;
        
        bp::enum_<Hog::EBlockGaussianWeightingPolicy>("BlockGaussianWeightingPolicy")
        .value("YesGaussian", Hog::eYesGaussian)  //define Hog.YesGaussian
        .value("NoGaussian",  Hog::eNoGaussian) //define Hog.NoGaussian
        ;
        
        bp::enum_<Hog::EBlockNormalizationPolicy>("BlockNormalizationPolicy")
        .value("NoNormalization", Hog::eNoNormalization)  //define Hog.NoNormalization
        .value("L2Normalization",  Hog::eL2Normalization) //define Hog.L2Normalization
        ;
        
    }
}