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
    "scissors",
    "simpleDaisy",
    "eye",
    "ok",
    "candle",
    "bomb",
    "lightning",
    "cheese",
    "fire",
    "cactus",
    "tree",
    "sunSpark",
    "bulb",
    "seahorseDragon",
    "anchor",
    "loveHeart",
    "balloon",
    "paddleLock",
    "ladyBug",
    "questionMark",
    "snowFlake",
    "dolphin",
    "toyMan",
    "pencil",
    "stop",
    "musicalScore",
    "spider",
    "squareCat",
    "ghostFace",
    "key",
    "icePuddle",
    "carrot",
    "puddle",
    "exclamationMark",
    "jokerFace",
    "trex",
    "art",
    "apple",
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
    "puppy"
]

gTemplateMap = {}

def makeTemplateMap( templateNames, templateMap):
    for i in range(len(templateNames)):
        tname = templateNames[i]
        existingEntry = templateMap.get(tname, None)
        assert(existingEntry == None)
        templateMap[tname] = i
    print "processed %d templates" % (len(templateNames))

def processFile(fpath):
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

    print "%s, %r, %d" % (fpath, ntext, len(data._children))
    tree.write("foo.xml", encoding="utf-8", xml_declaration=True)

    #raise "HiHI"
    pass

def fun(argdir):
    makeTemplateMap(gTemplateNames, gTemplateMap)
    print "%r" % gTemplateMap
    for root,dir,files in os.walk(argdir):
        for item in fnmatch.filter(files, "*.xml"):
            processFile(os.path.join(root,item))
            pass
        pass

if __name__ == "__main__":
    fun("./data")
    pass


