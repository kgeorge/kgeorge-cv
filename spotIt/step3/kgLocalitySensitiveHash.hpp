/********************************************************************************/
//
//      author: koshy george, kgeorge2@gmail.com, copyright 2014
//      please read license.txt provided
//      free-bsd-license
/********************************************************************************/
#if !defined(KG_LOCALITY_SENSITIVE_HASH_)
#define KG_LOCALITY_SENSITIVE_HASH_

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


template< typename LSHashEntry>
struct HashTableValue {
    std::deque< LSHashEntry > data;
    void clear() {
        data.clear();
    }
};


template<typename T, typename LSHashEntry,  int numBuckets, typename TTraits = KgLocalitySensitiveHash_Traits< T >  >
struct LocalitySensitiveHash {
    typedef typename T::value_type I;
    typedef typename AppropriateNonIntegralType<I>::value_type K;
    static constexpr K zero = static_cast<K>(0);
    static constexpr K one = static_cast<K>(1);
    static constexpr int numQueryIndices = Kg::Pow<3, numBuckets>::value;
    LocalitySensitiveHash(K w, K minRange, K maxRange):
        w(w),
        minRange(minRange),
        maxRange(maxRange),
        oneByW(one/w),
        gen(42),
        ndist(std::normal_distribution<>(zero, one)) {
        resize(w, minRange, maxRange);
    }
    LocalitySensitiveHash():
        w(1),
        oneByW(one/w),
        gen(42),
        minRange(-1),
        maxRange(1),
        ndist(std::normal_distribution<>(zero, one)) {
        resize(w, minRange, maxRange);
    }

    int findIndex( int ret[numBuckets]) {
        assert(numBuckets == 3);
        //please rewrite this function in a loop
        //for nBuckets != 3
        return ret[2] * nSizePerBucket * nSizePerBucket + ret[1] * nSizePerBucket + ret[0];
    }

    int computeHashTableIndexCore(const T &arg ) {
        K temp;
        K numSizeBy2 = nSizePerBucket/2;
        for(int i=0; i < numBuckets; ++i) {
            temp = TTraits::dot(a[i], arg) + b[i];
            tempStorage[i] = floor(temp * oneByW);
            assert(tempStorage[i] >= -numSizeBy2 && tempStorage[i] <= numSizeBy2);
            tempStorage[i] += numSizeBy2;
            assert(tempStorage[i] > 0);
        }
        int hashTableIndex = findIndex(tempStorage);
        assert(hashTableIndex < hashTable.size());
        return hashTableIndex;
    }

    void advanceDiffIndex( int diffIndex[numBuckets]) {

    for(int i=0; i < numBuckets; ++i) {
        if(diffIndex[i] < 1) {
            diffIndex[i] += 1;
            return;
        } else {
            diffIndex[i] = -1;
        }
     }
    }


    void computeHashTableIndexCoreQ(const T &arg, int retVal [numQueryIndices] ) {
        K temp;
       int tempStorage2[numBuckets];
        K numSizeBy2 = nSizePerBucket/2;
        for(int i=0; i < numBuckets; ++i) {
            temp = TTraits::dot(a[i], arg) + b[i];
            tempStorage[i] = floor(temp * oneByW);
            assert(tempStorage[i] >= -numSizeBy2 && tempStorage[i] <= numSizeBy2);
            tempStorage[i] += numSizeBy2;
            assert(tempStorage[i] > 0);
        }
        int diffIndex [numBuckets];
        for(int i=0; i < numBuckets; ++i) {
            diffIndex[i] = -1;
        }
        for(int j=0; j < numQueryIndices; ++j) {
            for(int i=0; i < numBuckets; ++i ) {
                tempStorage2[i] = tempStorage[i] + diffIndex[i];
            }
            int hashTableIndex = findIndex(tempStorage2);
            assert(hashTableIndex < hashTable.size());
            retVal[j] = hashTableIndex;
            advanceDiffIndex( diffIndex);
        }
    }


    int index( const T &arg, const LSHashEntry & entry) {
        int hashTableIndex =  computeHashTableIndexCore(arg);
        HashTableValue<LSHashEntry> &hashTableVal = hashTable[hashTableIndex];
        auto hit = hashTableVal.data.begin();
        int numEntriesOfSameValue =0;
        bool found = false;
        for(hit = hashTableVal.data.begin(); hit != hashTableVal.data.end(); ++hit) {
            if( *hit == entry ) {
                hit->count++;
                numEntriesOfSameValue++;
                found = true;
            }
        }
        if(!found) {
            LSHashEntry entry_2(entry);
            entry_2.count++;
            hashTableVal.data.push_back(entry_2);

            ++numEntries;
            assert(count() == numEntries);
            if(((numEntries % 100) == 0) && (numEntries >= 100)) {
                //stats();
            }
        }
        assert( numEntriesOfSameValue <= 1);
        return hashTableIndex ;
    }
    //std::map<const LSHashEntry *, int, std::less<LSHashEntry*> >
    //void query( const T &arg,  const LSHashEntry & entry, std::map<const LSHashEntry*, int> &templateMatch) {
    void query( const T &arg,  const LSHashEntry & entry, std::map<const LSHashEntry *, int, std::less<LSHashEntry*> > &templateMatch) {
        int neighboringIndices[numQueryIndices];
        computeHashTableIndexCoreQ(arg, neighboringIndices);
        //int hashTableIndex =  computeHashTableIndexCore(arg);
        for(int i=0; i < numQueryIndices; ++i) {
            int hashTableIndex = neighboringIndices[i];
            HashTableValue<LSHashEntry> &hashTableVal = hashTable[hashTableIndex];
            auto hit = hashTableVal.data.begin();
            for(hit = hashTableVal.data.begin(); hit != hashTableVal.data.end(); ++hit) {
                assert(hit->templateId >=0);
                auto tmatch = templateMatch.find(&(*hit));
                if(tmatch == templateMatch.end()) {
                    templateMatch.insert(std::pair<const LSHashEntry*, int>(&(*hit), hit->count));
                } else {
                    tmatch->second += hit->count;
                }
            }
        }
    }
    
    
    void stats() const {
        std::map<int, int> histogram;
        for(auto hashTableValIt = hashTable.begin(); hashTableValIt != hashTable.end(); ++hashTableValIt ) {
            int n2 = 0;
            for(auto hoit = hashTableValIt->data.begin(); hoit!= hashTableValIt->data.end(); ++hoit ) {
                n2 += hoit->count;
            }
            n2 /= 10;
            auto histIt = histogram.find(n2);
            if (histIt == histogram.end()) {
                histogram.insert( std::pair<int, int>(n2, 1));
            } else {
                histIt->second += 1;
            }
        }
        std::cout << "histogram of hashTbale" << std::endl;
        for(auto mit = histogram.begin(); mit != histogram.end(); ++mit) {
            std::cout << mit->first << ", " << mit->second << std::endl;
        }
    }

    void clear() {
        for(auto hit = hashTable.begin(); hit != hashTable.end(); ++hit ) {
            hit->clear();
        }
        numEntries=0;
    }


    int count() {
        int nEntries = 0;
        for(int i=0; i < hashTable.size(); ++i) {
            const HashTableValue<LSHashEntry> &htValue = hashTable[i];
            nEntries += htValue.data.size();
        }
        return nEntries;

    }
    void serialize(cv::FileStorage &fs) const{
        assert(fs.isOpened());
        fs << "{";
        fs << "description" << "geometric hash";
        fs << "numBuckets" << numBuckets;
        fs << "minRange" << minRange;
        fs << "maxRange" << maxRange;
        fs << "windowSize" << w;
        fs << "numEntries" << numEntries;
        fs << "data" << "[";
        for(int i=0; i < hashTable.size(); ++i) {
            const HashTableValue<LSHashEntry> &htValue = hashTable[i];
            if (htValue.data.size() > 0) {
                fs << "{";
                fs << "index" << i;
                fs << "tableEntries" << "[";
                for(int j=0; j < htValue.data.size(); ++j) {
                    const LSHashEntry &he = htValue.data[j];
                    fs << he;
                }
                fs << "]";
                fs << "}";

            }
        }
        fs << "]";
        fs << "}";
    }

    void unSerialize(const cv::FileNode  &fn) {
        
        const std::string description( (std::string)fn["description"]);
        int nBuckets_2 = (int)fn["numBuckets"];
        K minRange = (K)fn["minRange"];
        K maxRange = (K)fn["maxRange"];
        int numEntriesReported = (int)fn["numEntries"];
        K w = (K)fn["windowSize"];
        resize(w, minRange, maxRange);
        
        cv::FileNode data = fn["data"];                         // Read string sequence - Get node
        if (data.type() != cv::FileNode::SEQ)
        {
            std::cerr << "data is not a sequence! FAIL" << std::endl;
            throw std::runtime_error("data is not a sequence! FAIL");
        }
        
        cv::FileNodeIterator it = data.begin(), it_end = data.end(); // Go through the node

        for (; it != it_end; ++it) {
            const cv::FileNode &thisNode = *it;
            int index = (int) thisNode["index"];
            const cv::FileNode &tableEntries = thisNode["tableEntries"];
            if (tableEntries.type() != cv::FileNode::SEQ)
            {
                std::cerr << "tableEntries is not a sequence! FAIL" << std::endl;
                throw std::runtime_error("tableEntries is not a sequence! FAIL");
            }

            cv::FileNodeIterator it_2 = tableEntries.begin(), it_2_end = tableEntries.end(); // Go through the node
         
            
            for (; it_2 != it_2_end; ++it_2) {
                LSHashEntry he;
                (*it_2) >> he;
                hashTable[index].data.push_back(he);
                ++numEntries;
            }
        }
        assert(numEntriesReported == numEntries);
    }

    protected:
     void resize( K w_arg, K minRange_arg, K maxRange_arg ) {

        clear();
        assert( minRange_arg == -(maxRange_arg)); //right now this has to be likethis
        w = w_arg;
        assert(w > zero);
        assert(maxRange_arg > minRange_arg);
        maxRange = maxRange_arg;
        minRange = minRange_arg;
        nSizePerBucket = ceil((maxRange - minRange)/w);

        assert(nSizePerBucket > 1);

        oneByW = one / w;
        nSizeAllBuckets = nSizePerBucket*nSizePerBucket*nSizePerBucket;
        gen = std::mt19937(42);
        udist = std::uniform_real_distribution<K>(zero, w);
        hashTable.resize(nSizeAllBuckets);
        for(int i=0; i < numBuckets; ++i) {
            a[i] = TTraits::gen(ndist, gen);
            b[i] = udist(gen);
        }
     }
    public:
    std::normal_distribution<> ndist;
    std::uniform_real_distribution<K> udist;
    std::mt19937 gen;
    T a[numBuckets];
    K b[numBuckets];
    K w;
    K oneByW;
    K minRange;
    K maxRange;
    int tempStorage[numBuckets];
    int nSizePerBucket;
    int nSizeAllBuckets;
    std::deque<HashTableValue< LSHashEntry > > hashTable;
    int numEntries;
};

#endif //KG_LOCALITY_SENSITIVE_HASH_