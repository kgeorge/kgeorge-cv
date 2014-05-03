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
#if !defined(KG_LEAST_SQUARES_TRANSFORM_H_)
#define KG_LEAST_SQUARES_TRANSFORM_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <unordered_set>
#include <set>
#include <string>
#include <functional>


#include "kgUtils.hpp"
#include "kgKernel.hpp"
#include <Eigen/Core>
#include <Eigen/Dense>




template<typename T, typename TTraits = KgLeastSquaresTransform_Traits< T >  >
struct LeastSquaresTransform {
    typedef typename T::value_type I;
    typedef typename AppropriateNonIntegralType<I>::value_type K;
    static constexpr K zero = static_cast<K>(0);
    static constexpr K one = static_cast<K>(1);
    static const int numColumnElements =  TTraits::numElementsInT + 1;

    LeastSquaresTransform(){}
    void operator()(
    const Eigen::MatrixXf &A,
    const Eigen::MatrixXf &B,
    Eigen::MatrixXf &Transform){
        assert(A.rows() == B.rows());
        assert(A.cols() == numColumnElements);
        assert(B.cols() == numColumnElements);

        Transform.resize(numColumnElements, numColumnElements);
        /*
        Transform.col(0) = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(B.col(0));
        Transform.col(1) = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(B.col(1));
        Transform.col(2) = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(B.col(2));
        */
        Transform = A.jacobiSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(B);
    }
};

#endif //KG_LEAST_SQUARES_TRANSFORM_H_