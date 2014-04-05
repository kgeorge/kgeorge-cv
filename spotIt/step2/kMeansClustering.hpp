/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
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
                     const std::vector<T> &items);
    
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
protected:
    static float doClusterAssignmentForAllItems(
                                                     const std::vector<T> &items,
                                                     const std::vector<T> &clusterCenters,
                                                     std::vector<int> &clusterAssignment,
                                                     std::deque<float> &squaredErrorForItems
                                                     );
    //resolve upto 4 empty clusters
    static float resolveEmptyClusters(
        std::vector<int> &numItemsInCluster,
        std::vector<int> &clusterAssignments,
        std::vector<T>   &clusterCentersBackBuffer,
        const std::vector<T>   &items,
        const std::deque<float> &squaredErrorForItems,
        int desiredNumClusters
    );
    
    void initialClusterAssignment();
protected:
    const std::vector<T> &items;
    int k;
    std::vector<int> clusterAssignments;
    std::vector<T> clusterCenters;
    std::vector<T> clusterCentersBackBuffer;
    std::vector<int> numItemsInCluster;
    //squared error for each item from current cluster assignment
    std::deque<float> squaredErrorForItems;
    //total for all items
    float lastSSEForAllItems;
    
protected:
};




template<typename T, typename TTraits>
KMeansClustering<T, TTraits>::KMeansClustering(
                                               int nDesiredNumCluster,
                                               const std::vector<T> &items):
items(items),
k(nDesiredNumCluster),
lastSSEForAllItems(0){
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
    
    initialClusterAssignment();
    
    
    float deltaError = lastSSEForAllItems;
    int emptyClusterIndices[4];

    for(int i =0; i < maxIterations && deltaError > 10.0 ; ++i) {
        
        float curSSEForAllItems = 0;
        
        numItemsInCluster.assign(k, 0);
        for(int m=0; m < k; ++m){
            clusterCentersBackBuffer[m] = T();
        }
        
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
        
        clusterCenters.swap(clusterCentersBackBuffer);
        curSSEForAllItems = doClusterAssignmentForAllItems(
                                                                items,
                                                                clusterCenters,
                                                                clusterAssignments,
                                                                squaredErrorForItems
                                                                );
        deltaError = lastSSEForAllItems - curSSEForAllItems;
        lastSSEForAllItems = curSSEForAllItems;
    }
    assert(clusterAssignments.size() == items.size());
}

template<typename T, typename TTraits>
void KMeansClustering<T, TTraits>::initialClusterAssignment(
                                                            ) {
    
    std::set<T, std::less<T> > itemsSoFarSelectedForInitialization;
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
    
    lastSSEForAllItems = doClusterAssignmentForAllItems(
                                                             items,
                                                             clusterCenters,
                                                             clusterAssignments,
                                                             squaredErrorForItems
                                                             );
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
    return curSSEForAllItems;
}

template<typename T, typename TTraits>
float KMeansClustering<T, TTraits>::resolveEmptyClusters(
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