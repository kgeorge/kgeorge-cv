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


#include "kgCommonTraits.hpp"
#include "kgUtils.hpp"


template< typename T, typename LSHashEntry>
struct HashTableValue {
    std::deque< std::pair<T, LSHashEntry> > data;
    void clear() {
        data.clear();
    }
};


template<typename LSHashEntry>
struct StatsForLSHashEntry {

    void operator()(const LSHashEntry &rhs) {


    }

    std::ostream & output( std::ostream &o) {
        return o;
    }
};

template<typename T, typename LSHashEntry,  int numFields, int numBuckets,  typename TTraits = KgLocalitySensitiveHash_Traits< T >  >
struct LocalitySensitiveHash {
    typedef typename T::value_type I;
    typedef typename AppropriateNonIntegralType<I>::value_type K;
    static constexpr K zero = static_cast<K>(0);
    static constexpr K one = static_cast<K>(1);
    LocalitySensitiveHash(K w, K minRange, K maxRange):
        w(w),
        minRange(minRange),
        maxRange(maxRange),
        oneByW(one/w),
        gen(42),
        ndist(std::normal_distribution<>(zero, one)) {
        resize_(w, minRange, maxRange);
    }
    LocalitySensitiveHash():
        w(1),
        oneByW(one/w),
        gen(42),
        minRange(-1),
        maxRange(1),
        ndist(std::normal_distribution<>(zero, one)) {
        resize_(w, minRange, maxRange);
    }

    int findIndex( int ret[numBuckets][numFields], int iBucket) {
        assert(numFields == 3);
        int r = ret[iBucket][2] * nSizePerField * nSizePerField + ret[iBucket][1] * nSizePerField + ret[iBucket][0];
        return r;
    }

    void computeHashTableIndexCore(const T &arg, int retVal[numBuckets] ) {
        K temp;
        K numSizeBy2 = nSizePerField/2;
        int hashTableIndex=-1;
        for(int i=0; i < numBuckets; ++i) {
            for(int j=0; j < numFields; ++j) {
                temp = TTraits::dot(a[i][j], arg) + b[i][j];
                K tempVal = floor(temp * oneByW);
                tempStorage[i][j] = Kg::clamp(tempVal, -numSizeBy2, numSizeBy2);
                assert(tempStorage[i][j] >= -numSizeBy2 && tempStorage[i][j] <= numSizeBy2);
                tempStorage[i][j] += numSizeBy2;
                assert(tempStorage[i][j] > 0);
            }
            retVal[i] = findIndex(tempStorage, i);
        }
    }

    void advanceDiffIndex( int diffIndex[numFields]) {

    for(int i=0; i < numFields; ++i) {
        if(diffIndex[i] < 1) {
            diffIndex[i] += 1;
            return;
        } else {
            diffIndex[i] = -1;
        }
     }
    }


    void computeHashTableIndexCoreQ(const T &arg, std::vector< std::pair<int, int> > &retVal ) {
        K temp;
       int tempStorage2[numBuckets][numFields];
        K numSizeBy2 = nSizePerField/2;
        for(int i=0; i < numBuckets; ++i) {
            for(int j=0; j < numFields; ++j) {
                temp = TTraits::dot(a[i][j], arg) + b[i][j];
                K tempVal = floor(temp * oneByW);
                tempStorage[i][j] = Kg::clamp(tempVal, -numSizeBy2, numSizeBy2);
                assert(tempStorage[i][j] >= -numSizeBy2 && tempStorage[i][j] <= numSizeBy2);
                tempStorage[i][j] += numSizeBy2;
                assert(tempStorage[i][j] > 0);
            }
            int hashTableIndex = findIndex(tempStorage, i);
            assert(hashTableIndex < hashTable[i].size());
            retVal.push_back(std::pair<int, int>(i, hashTableIndex));
        }
    }


    void insertHashEntryCore( const T &arg, const LSHashEntry &entry, int hashTableIndices[numBuckets] ) {
        for(int i=0; i < numBuckets; ++i) {
            int hIndex = hashTableIndices[i];
            HashTableValue<T, LSHashEntry> &hashTableVal = hashTable[i][hIndex];
            auto hit = hashTableVal.data.begin();
            int numEntriesOfSameValue =0;
            bool found = false;
            std::pair<T, LSHashEntry> he(arg, entry);
            for(hit = hashTableVal.data.begin(); hit != hashTableVal.data.end(); ++hit) {

                if( *hit ==  he) {
                    hit->second.count++;
                    numEntriesOfSameValue++;
                    found = true;
                }
            }
            if(!found) {
                std::pair<T, LSHashEntry> hashEntry( arg, entry);
                hashEntry.second.count++;
                hashTableVal.data.push_back( hashEntry );

                ++numEntries;
                assert(count() == numEntries);
                if(((numEntries % 100) == 0) && (numEntries >= 100)) {
                    //stats();
                }
            }
            assert( numEntriesOfSameValue <= 1);
        }
    }

    void index( const T &arg, const LSHashEntry & entry, int hashTableIndicesReturned[numBuckets]) {
        computeHashTableIndexCore(arg, hashTableIndicesReturned);
        insertHashEntryCore(arg, entry, hashTableIndicesReturned);
    }

    void computeHashTableIndex(const T &arg, int hashTableIndicesReturned[numBuckets] ) {
        computeHashTableIndexCore(arg, hashTableIndicesReturned);
    }


    //std::map<const LSHashEntry *, int, std::less<LSHashEntry*> >
    //void query( const T &arg,  const LSHashEntry & entry, std::map<const LSHashEntry*, int> &templateMatch) {
    void query( const T &arg, std::deque< std::pair<const T*, const LSHashEntry*> > &ret) {
        int queryResults[numBuckets];
        computeHashTableIndexCore(arg, queryResults);
        //int hashTableIndex =  computeHashTableIndexCore(arg);
        for(int i=0; i < numBuckets; ++i) {
            int hashTableIndex = queryResults[i];
            assert(hashTableIndex < hashTable[i].size());
            HashTableValue<T, LSHashEntry> &hashTableVal = hashTable[i][hashTableIndex];
            auto hit = hashTableVal.data.begin();
            for(hit = hashTableVal.data.begin(); hit != hashTableVal.data.end(); ++hit) {
                const std::pair<T, LSHashEntry> &he = *hit;
                if(norm(he.first-arg) < 0.1) {
                    std::pair<const T*, const LSHashEntry*> entry(NULL, NULL);
                    ret.push_back( entry );
                    std::pair<const T*, const LSHashEntry*> & entryRef = ret.back();
                    entryRef.first = &he.first;
                    entryRef.second = &he.second;
                }
            }
        }
    }
    
    
    void stats() const {
#if 0
        std::map<int, int> histogram;
        for(auto hashTableValIt = hashTable.begin(); hashTableValIt != hashTable.end(); ++hashTableValIt ) {
            int n2 = 0;
            for(auto hoit = hashTableValIt->data.begin(); hoit!= hashTableValIt->data.end(); ++hoit ) {
                n2 += hoit->second.count;
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
#endif
    }

    void clear() {
        for(int i=0; i < numBuckets; ++i) {
            for(auto hit = hashTable[i].begin(); hit != hashTable[i].end(); ++hit ) {
                hit->clear();
            }
        }
        numEntries=0;
    }


    int count() {
        int nEntries = 0;
        for(int i=0; i < numBuckets; ++i) {
            for(int j=0; j < hashTable.size(); ++j) {
                const HashTableValue< T, LSHashEntry> &htValue = hashTable[i][j];
                nEntries += htValue.data.size();
            }
        }
        return nEntries;
    }

    void serialize(cv::FileStorage &fs) const{
        assert(fs.isOpened());
        fs << "{";
        fs << "description" << "geometric hash";
        fs << "numFields" << numFields;
        fs << "minRange" << minRange;
        fs << "maxRange" << maxRange;
        fs << "windowSize" << w;
        fs << "numEntries" << numEntries;
        fs << "data" << "[";
        fs << "{";
        for(int i=0; i < numBuckets; ++i) {
        fs << "[";
            for(int j=0; j < hashTable[i].size(); ++j)  {
                const HashTableValue<T, LSHashEntry> &htValue = hashTable[i][j];
                if (htValue.data.size() > 0) {
                    fs << "{";
                    fs << "index" << i;
                    fs << "tableEntries" << "[";
                    for(int j=0; j < htValue.data.size(); ++j) {
                        const std::pair<T, LSHashEntry> &entry = htValue.data[i];
                        fs << "{";
                        fs << "hashArg" << entry.first;
                        fs << "data" << entry.second;
                        fs << "}";
                    }
                    fs << "]";
                    fs << "}";

                }
            }
        }
        fs << "]";
        fs << "}";
        fs << "]";
        fs << "}";
    }

    void unSerialize(const cv::FileNode  &fn) {
        
        const std::string description( (std::string)fn["description"]);
        int nFields_2 = (int)fn["numFields"];
        K minRange = (K)fn["minRange"];
        K maxRange = (K)fn["maxRange"];
        int numEntriesReported = (int)fn["numEntries"];
        K w = (K)fn["windowSize"];
        resize_(w, minRange, maxRange);
        StatsForLSHashEntry<LSHashEntry> stats;
        
        cv::FileNode data = fn["data"];                         // Read string sequence - Get node
        if (data.type() != cv::FileNode::SEQ)
        {
            std::cerr << "data is not a sequence! FAIL" << std::endl;
            throw std::runtime_error("data is not a sequence! FAIL");
        }
        
        cv::FileNodeIterator it = data.begin(), it_end = data.end(); // Go through the node
        int whichHashTable=0;
        for (; it != it_end; ++it, ++whichHashTable) {
            const cv::FileNode &thisTable = *it;
            if (thisTable.type() != cv::FileNode::SEQ)
            {
                std::cerr << "data is not a sequence! FAIL" << std::endl;
                throw std::runtime_error("data is not a sequence! FAIL");
            }

            cv::FileNodeIterator it_2 = thisTable.begin(), it_2_end = thisTable.end(); // Go through the node
            for(; it_2 != it_2_end; ++it_2) {

                const cv::FileNode &thisNode = *it_2;



                int index = (int) thisNode["index"];
                const cv::FileNode &tableEntries = thisNode["tableEntries"];
                if (tableEntries.type() != cv::FileNode::SEQ)
                {
                    std::cerr << "tableEntries is not a sequence! FAIL" << std::endl;
                    throw std::runtime_error("tableEntries is not a sequence! FAIL");
                }

                cv::FileNodeIterator it_3 = tableEntries.begin(), it_3_end = tableEntries.end(); // Go through the node
         
            
                for (; it_3 != it_3_end; ++it_3) {
                    const cv::FileNode & thisEntry = *it_3;
                    std::pair<T, LSHashEntry> entry;
                    hashTable[whichHashTable][index].data.push_back(entry);
                    std::pair<T, LSHashEntry> &entryRef = hashTable[whichHashTable][index].data.back();
                    thisEntry["hashArg"] >> entryRef.first;
                    thisEntry["data"] >> entryRef.second;

                    stats(entryRef.second);
                    ++numEntries;
                }
            }
        }
        assert(numEntriesReported == numEntries);
        stats.output(std::cout);
    }

    protected:
     void resize_( K w_arg, K minRange_arg, K maxRange_arg ) {

        clear();
        assert( minRange_arg == -(maxRange_arg)); //right now this has to be likethis
        w = w_arg;
        assert(w > zero);
        assert(maxRange_arg > minRange_arg);
        maxRange = maxRange_arg;
        minRange = minRange_arg;
        nSizePerField = ceil((maxRange - minRange)/w);

        assert(nSizePerField > 1);

        oneByW = one / w;
        nSizeAllFields = nSizePerField*nSizePerField*nSizePerField;
        gen = std::mt19937(42);
        udist = std::uniform_real_distribution<K>(zero, w);
        for(int i=0;   i < numBuckets; ++i) {
            hashTable[i].resize(nSizeAllFields);
            for(int j=0; j < numFields; ++j) {
                a[i][j] = TTraits::gen(ndist, gen);
                b[i][j] = udist(gen);
            }
        }
     }
    public:
    std::normal_distribution<> ndist;
    std::uniform_real_distribution<K> udist;
    std::mt19937 gen;
    T a[numBuckets][numFields];
    K b[numBuckets][numFields];
    K w;
    K oneByW;
    K minRange;
    K maxRange;
    int tempStorage[numBuckets][numFields];
    int nSizePerField;
    int nSizeAllFields;
    std::deque<HashTableValue< T,  LSHashEntry > > hashTable[ numBuckets ];
    int numEntries;
};

#endif //KG_LOCALITY_SENSITIVE_HASH_