
(
  function() {
    var k = KgeorgeNamespace("K")
    var KUtils = KgeorgeNamespace("K.Utils")

    k.g_pickedPoints = {}

    k.sqrDistBetweenPts= function(ptA, ptB) {
             var sqrDiff = (ptA.x - ptB.x)*(ptA.x - ptB.x) + (ptA.y - ptB.y)*(ptA.y - ptB.y);
             return sqrDiff
        }
    k.minSqrdDistanceToDistinguishPts = 50;
     k.minSqrdDistanceToDistinguishPts2 = 2000;
    k.Frame = Class.extend( {
        init: function(canvasId, toolbarId,  name, baseDirUrl, srcImgFilename) {
            createjs.Ticker.setFPS(40);
            this.eModes = {"draw" : new k.DrawMode(this), "edit": new k.EditMode(this), "connect": new k.ConnectMode(this),  "save": new k.SaveMode(this)};
            this.currentMode = undefined;
            this.name = name;
            this.baseDirUrl = baseDirUrl;
            this.srcImgFilename = srcImgFilename
            this.collectedCurves = []
            this.srcImgScale = 1.0
            this.previousPt = {x:0, y:0}
            this.currentPt = {x:0, y:0}
            this.keyState = {}
            this.nextCurveName = 0;
            this.canvasId = canvasId;
            this.toolbarId = toolbarId;
            var frameCanvasElement = document.getElementById(canvasId);
            this.stage1 = new createjs.Stage( frameCanvasElement );
            this.stage1.mouseMoveOutside = false;

            var toolbarCanvasElement =  document.getElementById(toolbarId);
            this.stage2 = new createjs.Stage( toolbarCanvasElement );


            this.loader = new createjs.LoadQueue(true, null, true);

            this.loader.addEventListener("fileloaderror",  function(ev){
                console.log("error: 00000000000000");
            });
            this.loader.addEventListener("error",  function(ev){
                console.log("error: ~~~~~~~~~~~~~");
            });
            this.loader.addEventListener("complete",  this.loaderOnComplete.bind(this));
            this.loader.addEventListener("fileload", this.loaderOnLoad.bind(this));
            this.stage1.enableMouseOver(10);


            this.boundHandleMouseDoubleClick = this.handleMouseDoubleClick.bind(this);
            this.boundHandleMouseDown = this.handleMouseDown.bind(this);
            this.boundHandleMouseUp = this.handleMouseUp.bind(this);
            this.boundHandleMouseMove = this.handleMouseMove.bind(this);
            this.boundHandleKeyDown  = this.handleKeyDown.bind(this);
            this.boundHandleKeyUp = this.handleKeyUp.bind(this);

            createjs.Ticker.addEventListener("tick", this.tick.bind(this));
            window.onkeydown =this.boundHandleKeyDown;
            window.onkeyup = this.boundHandleKeyUp;
            this.wireupToolbar();
        },

        getDataUrl: function(dataType) {
            if(dataType == 'img') {
                return this.baseDirUrl + this.srcImgFilename;
            }
        },

        changeMode: function(modeName) {
            if(modeName && this.currentMode != this.eModes[modeName]) {
                var previousMode = this.currentMode;
                this.currentMode = this.eModes[modeName];
                this.currentMode.boundSwitchFrom(previousMode);
                console.log("mode changes to, ", modeName);
            }
        },
        wireupToolbar: function() {
            var modeNames = Object.keys(this.eModes);
            var radioButtonGroup = new k.RadioButtonGroup(modeNames);
            radioButtonGroup.x = 5;
            radioButtonGroup.y = 2;
            radioButtonGroup.addEventListener("click", function(evt){
                    this.changeMode(evt.target.label);
            }.bind(this))


            this.stage2.addChild(radioButtonGroup);

            this.stage2.update();
            var firstButton = radioButtonGroup.getButton(modeNames[0]);
            var firstButtonDispatchEvent = new createjs.Event("click");
            firstButtonDispatchEvent.target = firstButton;
            firstButton.dispatchEvent( firstButtonDispatchEvent);
            this.stage2.update();
            this.changeMode(modeNames[0]);

        },
        loaderOnLoad : function(ev) {
            /*
            switch(ev.item.type) {
                case createjs.LoadQueue.IMAGE:
                    console.log("load event ", ev.item.type);
                    break;
                case createjs.LoadQueue.TEXT:
                    console.log("load event ", ev.item.type);
                    break;
            }*/
        },

        loaderOnComplete : function(ev) {
            var imgResult = this.loader.getResult(67);
            var img = document.createElement('img');
            img.crossOrigin = "Anonymous";
            img.src = imgResult.src;

            var bitmap = new createjs.Bitmap(img);
            bitmap.x=0;
            bitmap.y=0;

            var frameCanvasElement = document.getElementById(this.canvasId);
            var scaleX = frameCanvasElement.width/bitmap.image.width;
            var scaleY = (frameCanvasElement.height)/bitmap.image.height;
            var scale = scaleX;
            if(scaleY < scaleX) {
                scale = scaleY;
            }
            bitmap.scaleX = scale;
            bitmap.scaleY = scale;
            this.srcImgScale = scale;
            console.log("frameCanvasElement" ,  frameCanvasElement.width, frameCanvasElement.height)
            console.log("bitmap bounds" , name, bitmap.getBounds(), scale)
            var bitmapContainer =new createjs.Container();

            bitmapContainer.name = "imageToEdit";
            bitmapContainer.addChild(bitmap);


            this.stage1.addChild(bitmapContainer);
            console.log("stage bounds" , name, this.stage1.getBounds())



            this.authoringArea = this.stage1;
            this.authoringArea.mouseEnabled = true;

            this.authoringArea.addEventListener("dblclick" , this.boundHandleMouseDoubleClick );
            this.authoringArea.addEventListener("mousedown", this.boundHandleMouseDown );

            this.stage1.update();
        },

        loadItems: function() {
            var imgLoadItem = new createjs.LoadItem().set({
                src:  this.getDataUrl('img'),
                crossOrigin:true,
                type: createjs.LoadQueue.IMAGE,
                id:67
            });

            this.loader.loadManifest([imgLoadItem]);
        },

        updateImageVisibility: function(bVisible) {
            var bitmapImage = this.stage1.getChildByName("imageToEdit");
            bitmapImage.visible =  bVisible;
        },

        //-----------------------------------------------------------------------
        //Mouse/keyboard handlers
        //-----------------------------------------------------------------------
        handleMouseMove: function (evt) {
            this.currentMode.boundHandleMouseMove(evt);
        },
        //-----------------------------------------------------------------------
        handleMouseUp : function (evt) {
            this.currentMode.boundHandleMouseUp(evt);
        },
        //-----------------------------------------------------------------------
        handleMouseDoubleClick : function(evt) {
            this.currentMode.boundHandleMouseDoubleClick(evt);
        },
        //-----------------------------------------------------------------------
        handleMouseDown : function(evt) {
            this.currentMode.boundHandleMouseDown(evt);
        },
        //-----------------------------------------------------------------------
        handleKeyDown : function(evt) {
            this.currentMode.boundHandleKeyDown(evt);
        },
        //-----------------------------------------------------------------------
        handleKeyUp : function(evt) {
            this.currentMode.boundHandleKeyUp(evt);
        },

        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //State maintainance
        //-----------------------------------------------------------------------
        pushCurvesCollected: function(curveShape) {
            this.currentMode.pushCurvesCollected(curveShape);
        },
        //-----------------------------------------------------------------------
        popCurvesCollected: function() {
            this.currentMode.popCurvesCollected();
        },
        //-----------------------------------------------------------------------
        stackTopCurvesCollected: function() {
            return this.currentMode.stackTopCurvesCollected();
        },
        //-----------------------------------------------------------------------
        removeLastCurveShapeFromStage: function() {

            var lastCurveShape = this.stackTopCurvesCollected();
            if (lastCurveShape !== undefined) {
                var thisParent = lastCurveShape.parent;
                thisParent.removeChild(lastCurveShape);

                this.popCurvesCollected();
            }
        },
        changeExt: function(str, newExt){
            return str.substring(0, str.lastIndexOf(".")) + newExt
        },

        addSuffixToFilename: function(str, suffix){
            var lastDot = str.lastIndexOf(".");
            return str.substring(0, lastDot) + suffix + str.substring(lastDot);
        },
        save: function() {
                var stageBounds = this.stage1.getBounds();
                this.updateImageVisibility(false);
                this.stage1.update();
                var bFill =true;
                var fillColor="white";
                var fillAlpha = 1.0;
                for(var k=0; k < this.collectedCurves.length; k +=1) {
                    var currentCurve = this.collectedCurves[k];
                    currentCurve.redrawCurveShapeGraphics(bFill, fillAlpha, fillColor);
                }
                this.stage1.update();
                var frameCanvasElement = document.getElementById(this.canvasId);
                var srcImgFilename = this.changeExt(this.srcImgFilename, ".png");
                var canvasCopy = document.createElement("canvas");
                var copyContext = canvasCopy.getContext("2d");
                canvasCopy.width = stageBounds.width/this.srcImgScale;
                canvasCopy.height = stageBounds.height/this.srcImgScale;
                if(this.srcImgScale > 0) {
                    copyContext.drawImage(frameCanvasElement, 0, 0, stageBounds.width, stageBounds.height, 0, 0, canvasCopy.width, canvasCopy.height);
                }

                canvasCopy.toBlob(function(blob) {
                    saveAs(blob, "skindetect-" + srcImgFilename );
                });

                for(var k=0; k < this.collectedCurves.length; k +=1) {
                    var currentCurve = this.collectedCurves[k];
                    currentCurve.redrawCurveShapeGraphics(bFill);
                }
        },
        //-----------------------------------------------------------------------

        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //tick
        //-----------------------------------------------------------------------

        tick: function () {
            this.currentMode.boundTick();
            this.stage1.update();
            this.stage1.tick();
            this.stage2.update();
            this.stage2.tick();
        }
    });

    K.ViolaJonesFrame = K.Frame.extend({
        init: function(canvasId, toolbarId,  name, baseDirUrl, srcImgFilename) {
            this._super(canvasId, toolbarId,  name, baseDirUrl, srcImgFilename);
            console.log('!!!!!@@@@!!!!', this.getDataUrl('txt'));
        },

        getDataUrl: function(dataType) {
            if(dataType == 'txt') {
                txtFile = this.changeExt(this.srcImgFilename, '.txt');
                return this.baseDirUrl + txtFile;
            } else {
                return this._super(dataType);
            }
        },

        loadItems: function() {
            var imgLoadItem = new createjs.LoadItem().set({
                src:  this.getDataUrl('img'),
                crossOrigin:true,
                type: createjs.LoadQueue.IMAGE,
                id:67
            });

            var txtLoadItem = new createjs.LoadItem().set({
                src:  this.getDataUrl('txt'),
                crossOrigin:true,
                type: createjs.LoadQueue.TEXT,
                id:89
            });

            this.loader.loadManifest([imgLoadItem, txtLoadItem]);
        },

        loaderOnComplete : function(ev) {
            this._super(ev);
            var txtResult = this.loader.getResult(89);
            var rbounds = this.parseText(txtResult);
            var imgToEdit = this.stage1.getChildByName("imageToEdit");
            var bitmap = imgToEdit.getChildAt(0);

            var scaleX = bitmap.scaleX;
            var scaleY = bitmap.scaleY;


            var faceRect = new createjs.Shape();
            faceRect.graphics.setStrokeStyle(2,"square").beginStroke("#ff0000");
            faceRect.graphics.rect(rbounds.x * scaleX, rbounds.y * scaleY, rbounds.w *scaleX, rbounds.h * scaleY);

            faceRect.x_pos = 0;
            faceRect.y_pos = 0;
            faceRect.setBounds(rbounds.x * scaleX, rbounds.y * scaleY, rbounds.w *scaleX, rbounds.h * scaleY);
            faceRect.name = "faceRect";


            imgToEdit.addChild(faceRect);
            this.stage1.update();
        },

        parseText: function(txt) {
            vtxt = txt.split('\n');
            if(vtxt.length > 1) {
                var regexp_expected = /\w+:\s+(\d+),\s+(\d+),\s+(\d+),\s+(\d+)/;
                var dims_recovered = regexp_expected.exec(vtxt[1]);
                if(dims_recovered.length > 4) {
                    var x = parseInt(dims_recovered[1])
                    var y = parseInt(dims_recovered[2])
                    var w = parseInt(dims_recovered[3])
                    var h = parseInt(dims_recovered[4])
                    return {x: x, y: y, w: w, h:h}
                }
                console.log(dims_recovered);
            }
        },
        save: function() {
                var stageBounds = this.stage1.getBounds();
                this.updateImageVisibility(false);
                this.stage1.update();



                var imgToEdit = this.stage1.getChildByName("imageToEdit");
                var faceRect = imgToEdit.getChildByName("faceRect");
                var faceRectBounds = faceRect.getBounds();

                var bitmap = imgToEdit.getChildAt(0);

                var scaleX = bitmap.scaleX;
                var scaleY = bitmap.scaleY;

                console.log('*********', faceRect.getBounds());

                var bFill =true;
                var fillColor="white";
                var fillAlpha = 1.0;
                for(var k=0; k < this.collectedCurves.length; k +=1) {
                    var currentCurve = this.collectedCurves[k];
                    currentCurve.redrawCurveShapeGraphics(bFill, fillAlpha, fillColor);
                }
                this.stage1.update();
                var frameCanvasElement = document.getElementById(this.canvasId);
                var srcImgFilename = this.changeExt(this.srcImgFilename, ".png");
                var canvasCopy = document.createElement("canvas");
                var copyContext = canvasCopy.getContext("2d");
                canvasCopy.width = faceRectBounds.width/scaleX;
                canvasCopy.height = faceRectBounds.height/scaleY;
                if(this.srcImgScale > 0) {
                    copyContext.drawImage(frameCanvasElement, faceRectBounds.x, faceRectBounds.y, faceRectBounds.width, faceRectBounds.height, 0, 0, canvasCopy.width, canvasCopy.height);
                }


                var filePart_splits = srcImgFilename.split("/");
                var filePart = filePart_splits[filePart_splits.length - 1  ];
                if (filePart.match(/_vj.png/)) {
                    fileNameToSave = this.addSuffixToFilename(filePart,"_mask");
                    canvasCopy.toBlob(function(blob) {
                        saveAs(blob, fileNameToSave );
                    });
                }

                for(var k=0; k < this.collectedCurves.length; k +=1) {
                    var currentCurve = this.collectedCurves[k];
                    currentCurve.redrawCurveShapeGraphics(bFill);
                }
        }
    });


    K.init = function (query) {
        var sourceImgFilename = query.sourceImgFilename || "f0.jpg"
        var baseDatadir = query.baseDatadir || "skindetect/authoring/image"
        baseDatadir = "http://localhost:8000/" + baseDatadir + "/";
        //K.frame = new K.ViolaJonesFrame("demoCanvasFrame", "demoToolbar", "frame",  baseDatadir, sourceImgFilename );
        K.frame = new K.Frame("demoCanvasFrame", "demoToolbar", "frame",  baseDatadir, sourceImgFilename );
        K.frame.loadItems();
    }

  }
  ());
