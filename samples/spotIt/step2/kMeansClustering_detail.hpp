/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/

#if !defined(K_MEANS_CLUSTERING_DETAIL_H_)
#define K_MEANS_CLUSTERING_DETAIL_H_

#include <functional>

#include "kgUtils.hpp"

struct SquaredErrorAndItemIndex {
    float e_;
    int i_;
    SquaredErrorAndItemIndex():e_(0),i_(-1){}
    SquaredErrorAndItemIndex(float e, int i):e_(e),i_(i){}
};



//order the element according to the hue color
template<>
struct std::less<SquaredErrorAndItemIndex> : std::binary_function<SquaredErrorAndItemIndex, SquaredErrorAndItemIndex, bool> {
    bool operator()(const SquaredErrorAndItemIndex &l, const SquaredErrorAndItemIndex &r) {
        return l.e_ > r.e_;
    }
};




#endif //K_MEANS_CLUSTERING_DETAIL_H_