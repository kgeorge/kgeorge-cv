__author__ = 'kgeorge'

from optparse import OptionParser
from PIL import Image
import os


def main():
    parser = OptionParser()
    parser.add_option("-f", "--file", dest="filename",
                  help="imagefilename", metavar="FILE")
    (options, args) = parser.parse_args()
    print options.filename

    srcBaseDir = r'/Users/kgeorge/Documents/projects/kgeorge-cv/samples/skindetect/authoring/image/'
    maskBaseDir = r'/Users/kgeorge/Downloads'
    maskFilename = os.path.splitext(options.filename)[0] + '.png'

    im = Image.open(os.path.join(srcBaseDir, options.filename))
    im2 = Image.open(os.path.join(maskBaseDir, 'skindetect-' + maskFilename))
    print im.size, im2.size
    if(im2.size[0] >= im.size[0] and im2.size[1] >= im.size[1]):
        im2 = im2.crop((0,0, im.size[0], im2.size[1]))
        #im.paste(im2, (0,0))
        im2 = im2.convert('L')
        im2 = im2.convert('1')
        pass
    elif (im2.size[0] <= im.size[0] and im2.size[1] <= im.size[1]):
        print 'mask smaller than image'
        pass
    else:
        raise IOError
    im.paste(im2, (0,0))
    maskFilename = os.path.splitext(options.filename)[0] + '_mask' + '.png'
    im.save(os.path.join(srcBaseDir, maskFilename))

    print options.filename, im.size
    print options.filename, im2.size

    pass

if __name__ == '__main__':
    main()

