
#include <boost/python.hpp>
#include <boost/python/suite/indexing/vector_indexing_suite.hpp>

#include "hogc.hpp"


namespace bp = boost::python;



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