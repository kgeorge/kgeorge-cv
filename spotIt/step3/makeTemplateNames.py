#!/usr/bin/env python
#********************************************************************************/
#
#      author: koshy george, kgeorge2@gmail.com, copyright 2014
#      please read license.txt provided
#      free-bsd-license
#********************************************************************************/

import cv2
import numpy as np
import os
import fnmatch
import xml.etree.ElementTree as ET

gTemplateNames = [
    "scissors",   #0
    "simpleDaisy", #1
    "eye",  #2
    "ok", #3
    "candle", #4
    "bomb", #5
    "lightning", #6
    "cheese", #7
    "fire", #8
    "cactus", #9
    "tree", #10
    "sunSpark", #11
    "bulb", #12
    "seahorseDragon", #13
    "anchor", #14
    "loveHeart", #15
    "balloon", #16
    "paddleLock", #17
    "ladyBug", #18
    "questionMark", #19
    "snowFlake", #20
    "dolphin", #21
    "toyMan", #22
    "pencil", #23
    "stop", #24
    "musicalScore", #25
    "spider", #26
    "squareCat", #27
    "ghostFace", #28
    "key", #29
    "icePuddle", #30
    "carrot", #31
    "puddle", #32
    "exclamationMark", #33
    "jokerFace", #34
    "trex", #35
    "art", #36
    "apple", #37
    "crescentMoon",
    "palm",
    "chessKnight",
    "clock",
    "car",
    "snowMan",
    "waterDrop",
    "shamrock",
    "target",
    "maple",
    "lips",
    "igloo",
    "yinYang",
    "milkBottle",
    "spiderWeb",
    "toyZebra",
    "sunglass",
    "skullNBones",
    "puppy",
    "testhash4",
    "testhash5"
]

gTemplateMap = {}

def makeTemplateMap( templateNames, templateMap):
    for i in range(len(templateNames)):
        tname = templateNames[i]
        existingEntry = templateMap.get(tname, None)
        assert(existingEntry == None)
        templateMap[tname] = i
    print "processed %d templates" % (len(templateNames))

#fill the templateId and name in the xml  file containing the geometric hash for a template
def processFieFillTemplateId(fpath):
    (head, tail) = os.path.split(fpath)
    (tailName, ext) = os.path.splitext(tail)
    templateId = gTemplateMap.get(tailName, None)
    assert(templateId >= 0)
    tree = ET.parse(fpath)
    n = tree.find('SpotItHash/name')
    n.text = tailName
    data = tree.find("SpotItHash/hashTable/data")
    ntext = n.text if (n != None ) else ""
    hentry = list(data._children)
    for h in hentry:
        tes = h.find('tableEntries')
        te_list = list(tes._children)
        for te in te_list:
            tid = te.find('templateId')
            tid.text = str(templateId)
    return tree

#read an xml file containig the geometeric hash of a template and verify that te nuEntries is correctly registered
def processFileVerifyNumEntries(fpath):
    tree = ET.parse(fpath)
    n = tree.find('SpotItHash/name')
    numEntries =0;
    for ie in tree.iterfind("SpotItHash/hashTable/data/_"):
        for te in ie.iterfind("tableEntries/_"):
            numEntries += 1
    ne = tree.find("SpotItHash/hashTable/numEntries")
    numEntriesGiven = int(ne.text)
    print "given, %d, calculated: %d" % (numEntriesGiven, numEntries)

#read in the xml file cotaining the  geometric hash of a template,
#and merge it with the suppliled mergedTree
def mergeFiles(fpath, mergedTree, mergedIndices):
    tobeMergedTree = processFieFillTemplateId(fpath)
    numEntriesElement = mergedTree.find("SpotItHash/hashTable/numEntries")
    incrementNumEntriesBy = 0
    for tchildToBeMerged in tobeMergedTree.iterfind("SpotItHash/hashTable/data/_"):
        idx = tchildToBeMerged.find('index')
        sourceTemplateIdsSet  = set([ t.text for t in tchildToBeMerged.iterfind('tableEntries/_/templateId') ])
        assert(len(sourceTemplateIdsSet))
        try :
            iidx = int(idx.text)
            if(iidx in mergedIndices):
                for tchildMerged in mergedTree.iterfind("SpotItHash/hashTable/data/_"):
                    idMerged = tchildMerged.find('index')
                    try:
                        mergedIdx = int(idMerged.text)
                        if mergedIdx == iidx :
                            for tElementMerged in  tchildMerged.iterfind('tableEntries/_/templateId'):
                                assert(tElementMerged.text not in sourceTemplateIdsSet)
                            tcm = tchildMerged.find('tableEntries')
                            lengthBefore = len(tcm._children)
                            numAddedNow = len(tchildToBeMerged.find('tableEntries')._children)
                            for te in    tchildToBeMerged.iterfind('tableEntries/_'):
                                tcm.append(te)
                            lengthAfter = len(tcm._children)
                            assert(lengthAfter == (lengthBefore + numAddedNow))
                            incrementNumEntriesBy  += numAddedNow
                            break
                        else:
                            continue
                    except:
                        pass
                pass
            else:
                t2= mergedTree.find("SpotItHash/hashTable/data")
                t2._children.append(tchildToBeMerged)
                te = tchildToBeMerged.find('tableEntries')
                incrementNumEntriesBy += len(te._children)
                #add
                pass
        except TypeError:
            pass
    numEntriesElement.text = str(int(numEntriesElement.text)+ incrementNumEntriesBy)
    mergedTree.write("data/merged.xml", encoding="UTF-8", xml_declaration=True)
    print len(mergedIndices)
    pass

#merge
def fun(argdir):
    makeTemplateMap(gTemplateNames, gTemplateMap)
    print "%r" % gTemplateMap
    mergedTree = None
    mergedIndices = set()
    l = [mergedTree]
    for root,dir,files in os.walk(argdir):
        for item in fnmatch.filter(files, "*.xml"):
            (fileName, ext) = os.path.splitext(item)
            if fileName.endswith("_c"):
                continue
            if(fileName == "merged"):
                continue
            if not mergedTree:
                mergedTree = processFieFillTemplateId(os.path.join(root,item))
                mergedData = mergedTree.find("SpotItHash/hashTable/data")
                mergedChildren = list(mergedData._children)
                for mchild in mergedChildren:
                    idx = mchild.find('index')
                    try :
                        iidx = int(idx.text)
                        mergedIndices.add(iidx)
                    except TypeError:
                        pass
            else:
                mergeFiles(os.path.join(root,item), mergedTree, mergedIndices)
        pass
    pass

#fillin name and templateId
def funFillTemplateId(argdir):
    makeTemplateMap(gTemplateNames, gTemplateMap)
    print "%r" % gTemplateMap
    for root,dir,files in os.walk(argdir):
        for item in fnmatch.filter(files, "*.xml"):
            (title, ext) = os.path.splitext(item)
            if( title.endswith("_c")):
                continue
            if(item != "merged.xml"):
                processedRoot = processFieFillTemplateId(os.path.join(root,item))
                processedRoot.write(os.path.join(root,item), encoding="UTF-8", xml_declaration=True)
        pass
    pass

#verify
def funVerifyNumEntries(argdir):
    makeTemplateMap(gTemplateNames, gTemplateMap)
    processFileVerifyNumEntries(os.path.join(argdir, "apple.xml"))


if __name__ == "__main__":
    #fun("./data")
    funFillTemplateId("./data")
    pass


