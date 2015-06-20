__author__ = 'kgeorge'
import re
import cStringIO
import io
from PIL import Image
import urlparse
from io import  BytesIO
from cgi import parse_header, parse_multipart
import base64
import os
import json
#import yaml
#import simplejson as json

from SimpleHTTPServer import SimpleHTTPRequestHandler
import BaseHTTPServer
formats = {
    'image/jpeg': 'JPEG',
    'image/png': 'PNG',
    'image/gif': 'GIF'
}



class CORSRequestHandler (SimpleHTTPRequestHandler):
    dataUrlPattern = re.compile('data:image/(png|jpeg);base64,(.*)$')
    def do_GET(self):
        SimpleHTTPRequestHandler.do_GET(self)

    def do_POST(self):
        try:
            _, _, path, _, query, _ = urlparse.urlparse(self.path)
            if path == '/save':
                ctype, pdict = parse_header(self.headers['content-type'])
                print "ctype", ctype
                if ctype == 'application/json':
                    length = int(self.headers['content-length'])
                    postvars = urlparse.parse_qs(
                        self.rfile.read(length),
                        keep_blank_values=1)
                    try:
                        #print postvars.keys()[0][-50:]
                        jimgdata = postvars['imagedata']
                        #imgdata = json.loads(jimgdata)
                        print type(jimgdata[0]), len(jimgdata[0])
                        #imgdata = json.loads(jimgdata[0])
                        #imgdata = json.loads(jimgdata[0]).encode('utf-8')
                        #print type(imgdata), len(imgdata), imgdata[0:50]
                        #imgdata = jimgdata.get("imagedata", "")
                        #imgdata = imgdata.encode('utf-8')
                        #imgdata = 'data:image/png;base64,' + imgdata

                        foo = base64.b64decode(imgdata.encode('utf-8'))
                        #print type(base64.b64decode(imgdata))
                        #imgdataDecode = io.BytesIO(base64.b64decode(imgdata))
                        #image = Image.open(imgdataDecode)
                        #image.save("C:/Users/kgeorge/Desktop/foo",".png")


                        #imgdata = jimgdata["imagedata"]
                        #print jimgdata.keys()
                        #imgdata = 'data:image/png;base64,' + imgdata
                        #image = Image.open(cStringIO.StringIO(base64.b64decode(imgdata)))
                    except :
                        print 'failure in reading image'
                    #imgdata = self.path.split('/')[-1]
                    #jmgdata = postvars[0]
                    #print postvars.keys()
                    #if(postvars.get('imgdata', None)):
                    #    imgdata = postvars['imgdata']
                    #    image = Image.open(cStringIO.StringIO(base64.b64decode(imgdata)))
                    #    #image.save("C:/Users/kgeorge/Desktop/foo",".jpg")

                #    print "keys", postvars.keys()

                self.send_response(200)
                self.end_headers()
                data = self.rfile.read(int(self.headers.getheader('content-length')))
                self.wfile.write(data)
                #response="aaaaaaaaaaaaaaaa"
                #self.send_response(200)
                #self.send_header("Content-type",'text/plain')
                #self.send_header("Content-Length",len(response))
                #self.end_headers()
                #length = int(self.headers.getheader('content-length'))
                #SimpleHTTPRequestHandler.do_POST(self)
                #data = self.rfile.read(length)
                #self.send_response(200, "OK")
                #self.done(200, "OK")
        except Exception as inst:
            pass

    def end_headers (self):
        self.send_header('Access-Control-Allow-Origin', '*')
        SimpleHTTPRequestHandler.end_headers(self)

if __name__ == '__main__':
    BaseHTTPServer.test(CORSRequestHandler, BaseHTTPServer.HTTPServer)

