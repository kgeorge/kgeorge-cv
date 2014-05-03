/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
/*
Copyright (c) 2014, kgeorge
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the {organization} nor the names of its
  contributors may be used to endorse or promote products derived from
  this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
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