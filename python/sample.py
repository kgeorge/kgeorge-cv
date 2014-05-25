import hello_ext
import numpy as nu




def main():
    #print hello_ext.greet()
    a = nu.arange(5, dtype=nu.int32 )
    print a.dtype
    hello_ext.setArray(a)
    pass


if __name__ == "__main__":
    main()



