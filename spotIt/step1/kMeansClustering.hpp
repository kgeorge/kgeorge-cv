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
#include <unordered_set>
#include <set>

#include "kgUtils.hpp"


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
                     const std::vector<bool> &shouldProcess);
    
    ~KMeansClustering(){}
    
    
    void doIt();
    
    
    std::vector<int> &getOutputClusterAssignments() {
        return clusterAssignments;
    }
    
    float &getOutputLastSSEForAllItems() {
        return lastSSEForAllItems;
    }
protected:
    static float doClusterAssignmentForAllDataPoints(
                                                     const std::vector<T> &items,
                                                     const std::vector<bool> &shouldProcess,
                                                     const std::vector<T> &clusterCenters,
                                                     std::vector<int> &clusterAssignment,
                                                     std::deque<float> &squaredErrorForItems
                                                     );
    
    void initialClusterAssignment(
                                  const std::deque<int> &validDataIndices
                                  );
protected:
    const std::vector<T> &items;
    int k;
    std::vector<int> clusterAssignments;
    std::vector<T> clusterCenters;
    std::vector<T> clusterCentersBackBuffer;
    const std::vector<bool> &shouldProcess;
    std::vector<int> numItemsInCluster;
    //squared error for each item from current cluster assignment
    std::deque<float> squaredErrorForItems;
    //total for all items
    float lastSSEForAllItems;
};


template<typename T, typename TTraits>
KMeansClustering<T, TTraits>::KMeansClustering(
                                               int nDesiredNumCluster,
                                               const std::vector<T> &items,
                                               const std::vector<bool> &shouldProcess):
items(items),
k(nDesiredNumCluster),
shouldProcess(shouldProcess),
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
    const size_t nDataPoints = items.size();
    
    
    std::unordered_set<int> contoursSoFarSelectedForInit;
    std::set<T, std::less<T> > contoursSoFarSelectedForInit2;
    std::deque<int> validDataIndices;
    
    int numValidData=0;
    for(int i=0; i < items.size(); ++i) {
        if(shouldProcess[i]){
            validDataIndices.push_back(i);
            numValidData++;
        }
    }
    //if numValidData <= k
    //return
    int iNextCluster=0;
    if(numValidData <= k) {
        for(int i=0; i < items.size(); ++i) {
            if(shouldProcess[i]) {
                clusterAssignments[i] = iNextCluster++;
            }
        }
        return;
    }
    
    initialClusterAssignment(validDataIndices);
    
    
    float deltaError = lastSSEForAllItems;
    for(int i =0; i < maxIterations && deltaError > 10.0 ; ++i) {
        
        float curSSEForAllItems = 0;
        
        numItemsInCluster.assign(k, 0);
        for(int m=0; m < k; ++m){
            clusterCentersBackBuffer[m] = T();
        }
        
        for(int j=0; j < nDataPoints; ++j) {
            if(!shouldProcess[j]) {
                continue;
            }
            assert(items[j].nPointsInContour > 0);
            clusterCentersBackBuffer[clusterAssignments[j]] = clusterCentersBackBuffer[clusterAssignments[j]] +  TTraits::getContribution(items[j]);
            numItemsInCluster[clusterAssignments[j]] += TTraits::getContributingNumber(items[j]);
        }
        
        for(int m=0; m < k; ++m) {
            if(numItemsInCluster[m] <= 0) {
                KG_DBGOUT( std::cout <<  "cluster is empty: " << m <<  std::endl );
            }
        }
        
        clusterCenters.swap(clusterCentersBackBuffer);
        
        curSSEForAllItems = doClusterAssignmentForAllDataPoints(
                                                                items,
                                                                shouldProcess,
                                                                clusterCenters,
                                                                clusterAssignments,
                                                                squaredErrorForItems
                                                                );
        deltaError = lastSSEForAllItems - curSSEForAllItems;
        lastSSEForAllItems = curSSEForAllItems;
    }
}

template<typename T, typename TTraits>
void KMeansClustering<T, TTraits>::initialClusterAssignment(
                                                            const std::deque<int> &validDataIndices
                                                            ) {
    
    std::set<T, std::less<T> > dataSoFarSelected;
    for(int m=0; m < k; ++m) {
        int whichData;
        while(true) {
            float r = 1;
            while(r >= 1.0f) {
                r = Kg::uRand0To1();
            }
            int randomDataIndexIndex =  floor(r * validDataIndices.size());
            assert( randomDataIndexIndex < validDataIndices.size());
            int prospectiveDataIndex = validDataIndices[randomDataIndexIndex];
            if( dataSoFarSelected.end() == dataSoFarSelected.find(items[prospectiveDataIndex]) ) {
                whichData = prospectiveDataIndex;
                dataSoFarSelected.insert(items[prospectiveDataIndex]);
                break;
            }
        }
        clusterCenters[m] = items[whichData];
    }
    
    lastSSEForAllItems = doClusterAssignmentForAllDataPoints(
                                                             items,
                                                             shouldProcess,
                                                             clusterCenters,
                                                             clusterAssignments,
                                                             squaredErrorForItems
                                                             );
}
template<typename T, typename TTraits>
float KMeansClustering<T, TTraits>::doClusterAssignmentForAllDataPoints(
                                                                        const std::vector<T> &items,
                                                                        const std::vector<bool> &shouldProcess,
                                                                        const std::vector<T> &clusterCenters,
                                                                        std::vector<int> &clusterAssignment,
                                                                        std::deque<float> &squaredErrorForItems
                                                                        ) {
    const size_t nDataPoints = items.size();
    const size_t k = clusterCenters.size();
    float curSSEForAllItems = 0.0f;
    squaredErrorForItems.assign(nDataPoints, -1.0);
    for(int j=0; j < nDataPoints; ++j) {
        if(!shouldProcess[j]) {
            continue;
        }
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

#endif //K_MEANS_CLUSTERING_H_