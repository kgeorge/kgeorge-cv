
import sys
import math
import numpy as np



class StatsSampler(object):
    def __init__(self):
        self._sum=0
        self._count=0
        self._sumSq=0
        self._mn=sys.maxint
        self._mx=-sys.maxint
        self._dirty=True
        self._cachedAvg=0
        self._cachedVar=0
        pass

    def sample(self, val):
        self._sum += float(val)
        self._sumSq += float(val * val)
        self._count += 1
        self._mn = float(val) if (val < self._mn) else self._mn
        self._mx = float(val) if (val > self._mn) else self._mx
        self._dirty=True

    def compute(self):
        if self._dirty:
            self._cachedAvg = self._sum/self._count if self._count > 0 else 0
            self._cachedVar = (self._sumSq/self._count - (self._cachedAvg * self._cachedAvg)) if self._count > 0 else 0
            self._dirty = False


    def __str__(self):
        s = "{avg: %f, sd: %f, count: %d, min: %f, max: %f }" % \
            (self.avg, math.sqrt(self.var), self._count, self._mn, self._mx)
        return s

    @property
    def sum(self):
        """I'm the 'sum' property."""
        return self._sum


    @property
    def avg(self):
        """I'm the 'avg' property."""
        self.compute()
        avgRet = self._cachedAvg
        return avgRet

    @property
    def var(self):
        self.compute()
        varRet = self._cachedVar
        return varRet

    @property
    def sumSq(self):
        """I'm the 'sumSq' property."""
        return self._sumSq

    @property
    def count(self):
        """I'm the 'count' property."""
        return self._count

    @property
    def min(self):
        """I'm the 'min' property."""
        return self._mn


    @property
    def max(self):
        """I'm the 'max' property."""
        return self._mx

def main():
    s = StatsSampler()
    for i in range(10):
        s.sample(i)
    print "s=%s" % str(s)
    pass


if __name__ == "__main__":
    main()



