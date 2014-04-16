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
    tree = ET.parse(fpath)
    n = tree.find('SpotItHash/name')
    ntext = n.text if (n != None ) else ""
    print "%s, %r" % (fpath, ntext)
    #tree.write("foo.xml", encoding="utf-8", xml_declaration=True)

    #raise "HiHI"
    pass

def fun(argdir):
    makeTemplateMap(templateNames, templateMap)
    print "%r" % templateMap
    for root,dir,files in os.walk(argdir):
        for item in fnmatch.filter(files, "*.xml"):
            processFile(os.path.join(root,item))
            pass
        pass

if __name__ == "__main__":
    fun("./data")
    pass


