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

#if !defined(K_MEANS_CLUSTERING_H_)
#define K_MEANS_CLUSTERING_H_

#include <iostream>
#include <vector>
#include <limits>
#include <deque>
#include <functional>
#include <set>


#include "kgUtils.hpp"
#include "kMeansClustering_detail.hpp"


//defaut traits for template type
template<typename T>
struct KMeansDataElementTraits{
    
    static float getMinDist(float minDist, const T &c) {
        return minDist;
    }
    static  T &getContribution(  T &c) {
        return c;
    }
    static  bool validate( const T & c) {
        return true;
    }
    
    static const T &getContribution(const  T &c) {
        return c;
    }
    
    
    static int getContributingNumber( const T &c) {
        return 1;
    }
    
    
    static float dist(const T &lhs, const T &rhs) {
        T pdiff = lhs - rhs;
        throw std::runtime_error("not impemented");
        return 0;
    }
    
};


template<typename T, typename TTraits = KMeansDataElementTraits<T>>
class KMeansClustering {
public:
    KMeansClustering(
                     int nDesiredNumCluster,
                     const std::vector<T> &items,
                     const std::vector<T> &userSpecifiedInit);
    
    ~KMeansClustering(){}
    
    
    void doIt();
    
    
    std::vector<int> &getOutputClusterAssignments() {
        return clusterAssignments;
    }
    
    float &getOutputLastSSEForAllItems() {
        return lastSSEForAllItems;
    }
    const std::vector<int> &getNumItemsInCluster() {
        return numItemsInCluster;
    }
    const std::vector<T> &getClusterCenters() {
        return clusterCenters;
    }
protected:
    //given the cluster centers,
    //find the cluster center which is closest to each item
    static float doClusterAssignmentForAllItems(
                                                const std::vector<T> &items,
                                                const std::vector<T> &clusterCenters,
                                                std::vector<int> &clusterAssignment,
                                                std::deque<float> &squaredErrorForItems
                                                );
    //during each iteration,
    //if we end up with some current clusters with no members,
    //then call this to do a redistribution
    static void resolveEmptyClusters(
                                     std::vector<int> &numItemsInCluster,
                                     std::vector<int> &clusterAssignments,
                                     std::vector<T>   &clusterCentersBackBuffer,
                                     const std::vector<T>   &items,
                                     const std::deque<float> &squaredErrorForItems,
                                     int desiredNumClusters
                                     );
    //randomly assign one item as a
    //cluster center
    void initialClusterAssignment();
    
    void statsInitialClusterAssignment();
protected:
    //input items
    const std::vector<T> &items;
    //optional user specified initialization
    const std::vector<T> &userSpecifiedInit;
    //k in kMeans
    int k;
    //output cluster assignments
    std::vector<int> clusterAssignments;
    
    
    //transient members
    //current centroids
    std::vector<T> clusterCenters;
    //current centroids back-buffer
    std::vector<T> clusterCentersBackBuffer;
    //current number of items in each cluster
    std::vector<int> numItemsInCluster;
    //current squared error for each item from current cluster assignment
    std::deque<float> squaredErrorForItems;
    //last sum of squared error for all items
    float lastSSEForAllItems;
    
protected:
};




template<typename T, typename TTraits>
KMeansClustering<T, TTraits>::KMeansClustering(
                                               int nDesiredNumCluster,
                                               const std::vector<T> &items,
                                               const std::vector<T> &userSpecifiedInit):
items(items),
k(nDesiredNumCluster),
lastSSEForAllItems(0),
userSpecifiedInit(userSpecifiedInit){
    clusterAssignments.assign(items.size(), -1);
    clusterCenters.assign(k, T());
    clusterCentersBackBuffer.assign(k, T());
    numItemsInCluster.assign(k, 0);
    squaredErrorForItems.assign(items.size(), -1.0);
};




template<typename T, typename TTraits>
void  KMeansClustering<T, TTraits>::doIt(
) {
    const int maxIterations = 100;
    const size_t nItems = items.size();
    
    std::set<T, std::less<T> > contoursSoFarSelectedForInit2;
    
    //if number of input samples < k
    //return
    if(items.size() <= k) {
        for(int i=0; i < items.size(); ++i) {
            clusterAssignments[i] = i;
        }
        return;
    }
    
    //initialization
    initialClusterAssignment();
    
    KG_DBGOUT( statsInitialClusterAssignment() );
    
    float deltaError = lastSSEForAllItems;
    for(int i =0; i < maxIterations && deltaError > 10.0 ; ++i) {
        
        float curSSEForAllItems = 0;
        //initialize some transient variables per iteration
        numItemsInCluster.assign(k, 0);
        for(int m=0; m < k; ++m){
            clusterCentersBackBuffer[m] = T();
            KG_DBGOUT( std::cout << "low num contours" );
        }
        
        //update the cluster centers
        //use clusterCenterBackBuffer to do the calculation
        for(int j=0; j < nItems; ++j) {
            clusterCentersBackBuffer[clusterAssignments[j]] = clusterCentersBackBuffer[clusterAssignments[j]] +  TTraits::getContribution(items[j]);
            numItemsInCluster[clusterAssignments[j]] += TTraits::getContributingNumber(items[j]);
        }
        resolveEmptyClusters(
                             numItemsInCluster,
                             clusterAssignments,
                             clusterCentersBackBuffer,
                             items,
                             squaredErrorForItems,k);
        for(int m=0; m < k; ++m) {
            //all clusters should be non-empty
            assert(numItemsInCluster[m] > 0);
            clusterCentersBackBuffer[m] = clusterCentersBackBuffer[m] *  (1.0f/(float)numItemsInCluster[m]);
        }
        
        
        
        //switch back buffer to front buffer
        clusterCenters.swap(clusterCentersBackBuffer);
        
        //re-assign items to clusters
        curSSEForAllItems = doClusterAssignmentForAllItems(
                                                           items,
                                                           clusterCenters,
                                                           clusterAssignments,
                                                           squaredErrorForItems
                                                           );
        //keep track of error
        deltaError = lastSSEForAllItems - curSSEForAllItems;
        lastSSEForAllItems = curSSEForAllItems;
    }
    assert(clusterAssignments.size() == items.size());
}




template<typename T, typename TTraits>
void KMeansClustering<T, TTraits>::initialClusterAssignment(
) {
    //if the user has already specified an initialization
    //then use that
    if(userSpecifiedInit.size() == k) {
        for(int m=0; m < k; ++m) {
            clusterCenters[m] = userSpecifiedInit[m];
        }
    } else {
        std::set<T, std::less<T> > itemsSoFarSelectedForInitialization;
        //select up to k non-unique items
        for(int m=0; m < k; ++m) {
            int whichItem;
            while(true) {
                float r = 1;
                while(r >= 1.0f) {
                    r = Kg::uRand0To1();
                }
                int prospectiveItemIndex =  floor(r * items.size());
                assert( prospectiveItemIndex < items.size());
                if( itemsSoFarSelectedForInitialization.end() == itemsSoFarSelectedForInitialization.find(items[ prospectiveItemIndex ]) ) {
                    whichItem = prospectiveItemIndex;
                    itemsSoFarSelectedForInitialization.insert(items[prospectiveItemIndex]);
                    break;
                }
            }
            clusterCenters[m] = items[whichItem];
        }
    }
    //do an initial cluster assignment of items
    lastSSEForAllItems = doClusterAssignmentForAllItems(
                                                        items,
                                                        clusterCenters,
                                                        clusterAssignments,
                                                        squaredErrorForItems
                                                        );
}

template<typename T, typename TTraits>
void KMeansClustering<T, TTraits>::statsInitialClusterAssignment(
) {
    float minDistanceBetweenCentersSoFar = std::numeric_limits<float>::max();
    float maxDistanceBetweenCentersSoFar = -std::numeric_limits<float>::max();
    float sumDistanceBetweenCenters = 0.0;
    int numPairs = k*(k-1)/2;
    for(int m=0; m < k ; ++m ) {
        const T & center = clusterCenters[m];
        for(int n=m+1; n < k; ++n) {
            const T & otherCenter = clusterCenters[n];
            float curDistanceanceBetweenTwoCenters = TTraits::dist(center, otherCenter);
            minDistanceBetweenCentersSoFar = Kg::min( minDistanceBetweenCentersSoFar, curDistanceanceBetweenTwoCenters  );
            maxDistanceBetweenCentersSoFar = Kg::max( maxDistanceBetweenCentersSoFar, curDistanceanceBetweenTwoCenters  );
            sumDistanceBetweenCenters += curDistanceanceBetweenTwoCenters;
        }
    }
    float avgDistanceBetweenCenters = sumDistanceBetweenCenters  / numPairs;
    
    std::cout << "inititalization statistics: " << std::endl;
    std::cout << "minDistanceBetweenCenters: " << minDistanceBetweenCentersSoFar << std::endl;
    std::cout << "maxDistanceBetweenCenters: " << maxDistanceBetweenCentersSoFar << std::endl;
    std::cout << "avgDistanceBetweenCenters: " << avgDistanceBetweenCenters << std::endl;
}

template<typename T, typename TTraits>
float KMeansClustering<T, TTraits>::doClusterAssignmentForAllItems(
                                                                   const std::vector<T> &items,
                                                                   const std::vector<T> &clusterCenters,
                                                                   std::vector<int> &clusterAssignment,
                                                                   std::deque<float> &squaredErrorForItems
                                                                   ) {
    const size_t nItems = items.size();
    const size_t k = clusterCenters.size();
    float curSSEForAllItems = 0.0f;
    squaredErrorForItems.assign(nItems, -1.0);
    for(int j=0; j < nItems; ++j) {
        assert(TTraits::validate(items[j]));
        squaredErrorForItems[j] = 0.0f;
        float minDistSoFar = std::numeric_limits<float>::max();
        int clusterWithMinDist = -1;
        for(int m=0; m < k; ++m) {
            float curDist = TTraits::dist(clusterCenters[m], items[j]);
            if(curDist < minDistSoFar) {
                clusterWithMinDist = m;
                minDistSoFar = curDist;
            }
        }
        clusterAssignment[j] = clusterWithMinDist;
        squaredErrorForItems[j] = minDistSoFar;
        curSSEForAllItems += minDistSoFar;
    }
    
    //verification that each valid contour is assigned to a cluster
    for(int j =0; j < clusterAssignment.size(); ++j) {
        assert(clusterAssignment[j] >= 0);
    }
    
    return curSSEForAllItems;
}

template<typename T, typename TTraits>
void KMeansClustering<T, TTraits>::resolveEmptyClusters(
                                                        std::vector<int> &numItemsInCluster,
                                                        std::vector<int> &clusterAssignments,
                                                        std::vector<T>   &clusterCentersBackBuffer,
                                                        const std::vector<T>   &items,
                                                        const std::deque<float> &squaredErrorForItems,
                                                        int desiredNumClusters
                                                        ) {
    static int emptyClusterIndices[4];
    const int nItems = items.size();
    //resolve upto 4 empty  clusters
    int nextEmptyClusterIndexIndex = 0;
    emptyClusterIndices[0] = emptyClusterIndices[1] = emptyClusterIndices[2] = emptyClusterIndices[3] = -1;
    for(int m=0; m < desiredNumClusters; ++m) {
        if(numItemsInCluster[m] <= 0 && nextEmptyClusterIndexIndex < 4 ) {
            emptyClusterIndices[nextEmptyClusterIndexIndex++] = m;
        }
    }
    
    std::set<SquaredErrorAndItemIndex> orderedErrorsForItems;
    int numEmptyClusters = nextEmptyClusterIndexIndex;
    if( numEmptyClusters > 0 ) {
        for(int j=0; j < nItems; ++j) {
            SquaredErrorAndItemIndex e(squaredErrorForItems[j], j);
            orderedErrorsForItems.insert(e);
        }
        for(int j=0; j < numEmptyClusters; ++j) {
            assert(emptyClusterIndices[j] >= 0);
        }
    }
    for(int j=0; j < numEmptyClusters; ++j) {
        assert(emptyClusterIndices[j] >= 0);
        while(orderedErrorsForItems.size() > 0) {
            //maximum error entry
            auto eit  = orderedErrorsForItems.begin();
            int replacementClusterItemIndex =  eit->i_;
            int currentClusterAssignmentOfReplacementItem = clusterAssignments[replacementClusterItemIndex ];
            if(numItemsInCluster[ currentClusterAssignmentOfReplacementItem] > 1 ) {
                clusterCentersBackBuffer[ currentClusterAssignmentOfReplacementItem ] - items[replacementClusterItemIndex];
                numItemsInCluster[ currentClusterAssignmentOfReplacementItem ] -= 1;
                clusterCentersBackBuffer[emptyClusterIndices[j]] = items[replacementClusterItemIndex];
                numItemsInCluster[emptyClusterIndices[j]] = 1;
            } else {
                orderedErrorsForItems.erase(eit);
            }
        }
    }
}

#endif //K_MEANS_CLUSTERING_H_