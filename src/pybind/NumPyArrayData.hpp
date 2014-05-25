#if !defined(NUM_PY_ARRAY_DATA_H_)
#define NUM_PY_ARRAY_DATA_H_

#include <boost/python.hpp>
#include <boost/numpy.hpp>
#include <stdexcept>
#include <sstream>

namespace bp = boost::python;
namespace np = boost::numpy;

template< typename T >
class NumPyArrayData {
    public:
    NumPyArrayData<T>(const np::ndarray &arr){
        np::dtype dtype = arr.get_dtype();
        np::dtype dtype_expected = np::dtype::get_builtin<T>();
        if(dtype != dtype_expected) {
            std::stringstream ss;
            ss << "NumPyArrayData: Unexpected data type (" << bp::extract<const char*>(dtype.attr("__str__")()) << ") received";
            ss << "Expected " << bp::extract<const char*>(dtype_expected.attr("__str__")());
            throw std::runtime_error(ss.str().c_str());
        }
        data_ = arr.get_data();
		strides_ = arr.get_strides();


    }

	T* data()
	{
		return reinterpret_cast<T*>(data_);
	}

	const Py_intptr_t* strides()
	{
		return strides_;
	}

	// 1D array access
	inline T& operator()(int i)	{
		return *reinterpret_cast<T*>(data_ + i*strides_[0]);
	}

	// 1D array access
	inline T& operator()(int i, int j){
		return *reinterpret_cast<T*>(data_ + i*strides_[0] + j * strides_[1]);
	}

	// 1D array access
	inline T& operator()(int i, int j, int k)	{
		return *reinterpret_cast<T*>(data_ + i*strides_[0] + j * strides_[1] + k * strides_[2]);
	}

		char *  data_;
		const Py_intptr_t* strides_;
};
#endif //NUM_PY_ARRAY_DATA_H_