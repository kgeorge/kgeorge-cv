import pylab as pl
import numpy as nu
import re
import random
import bisect

rcomment = re.compile(r'^\s*[#]')
rcontent=re.compile(r'\s*(?P<size>\d+)\s*[,]\s*(?P<lane>\d+)')


class HistRand(object):
    def __init__(self, items):
        assert(len(items)>0)
        h = pl.hist(items, bins=100)
        cumPdf = []
        sum = 0
        for i in  h[0]:
            sum += i
            cumPdf.append(sum)
        self.cumPdf = map(lambda x: float(x)/len(items), cumPdf)
        self.bins = h[1]
        random.seed(42)
        #print len(cumPdf), len(self.bins)
        pass

    def random(self):
        p = random.random()
        idx = bisect.bisect_left(self.cumPdf, p)
        return self.bins[idx]


def processHist(lHist):
    assert(len(lHist)>0)
    h = pl.hist(lHist, bins=100)
    normFactor = len(lHist)
    cumPdf = []
    sum = 0
    for i in  h[0]:
        sum += i
        cumPdf.append(sum)
    cumPdf = map(lambda x: float(x)/len(lHist), cumPdf)
    print cumPdf
    pass

def main():
    lane1Hist=[]
    lane2Hist=[]
    with file("./data/cars2.txt") as fp:
        for l in fp:
            l = l.strip()
            if not l:
                continue
            m = rcomment.match(l)
            if m:
                continue
            m = rcontent.match(l)

            if m:
                if int(m.group('lane')) == 1:

                    lane1Hist.append(int(m.group('size')))
                elif int(m.group('lane')) == 2:
                    lane2Hist.append(int(m.group('size')))
                else:
                    pass

        pass
    h1 = HistRand(lane1Hist)
    h2 = HistRand(lane2Hist)
    #print [ h.random() for i in range(0, 100)]
    c = "n"
    while c != "q":
        b = random.random()
        val = ""
        rval = 0
        if b < 0.5:
            rval = h1.random()
            val = "   1,   " +  str(rval)
        else:
            rval = h2.random()
            val = "   2,   " + str(rval)
        if( c!= "n"):
            dim = float(c)
            w = 300.0 * dim/rval
            h = w/2.0
            print val, "/////////////// w: ", w, "   \\\\\\\\\\\\\\ h: ", h
            pass
        c=raw_input("n/q:")
        c = c.strip()

    pass


if __name__ == "__main__":
    main()



