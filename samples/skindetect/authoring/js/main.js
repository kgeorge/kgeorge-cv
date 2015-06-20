
(
  function() {
    var k = KgeorgeNamespace("K")
    var kUtils = KgeorgeNamespace("K.Utils")

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
            this.imgUrl = baseDirUrl + srcImgFilename
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


            var loader = new createjs.LoadQueue(true, null, true);

            loader.addEventListener("fileloaderror",  function(ev){
                console.log("error: 00000000000000");
            });
            loader.addEventListener("error",  function(ev){
                console.log("error: ~~~~~~~~~~~~~");
            });
            loader.addEventListener("complete",  function(ev){
                console.log("complete", "88888888888888");
            });
            loader.addEventListener("fileload", this.loaderImgOnLoad.bind(this));
            var loadItem = new createjs.LoadItem().set({
                src:  this.imgUrl,
                crossOrigin:true,
                type: createjs.LoadQueue.IMAGE
            });
            loader.loadFile(loadItem);
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
        loaderImgOnLoad: function(ev) {
            console.log("load error~~~~~~~~~~~~")

            var img = document.createElement('img');
            img.crossOrigin = "Anonymous";
            img.src = ev.result.src;

            var bitmap = new createjs.Bitmap(img);
            bitmap.x=0;
            bitmap.y=0;

            var frameCanvasElement = document.getElementById(this.canvasId);
            var scaleX = frameCanvasElement.width/bitmap.image.width;
            var scaleY = frameCanvasElement.height/bitmap.image.height;
            var scale = scaleX;
            if(scaleY < scaleX) {
                scale = scaleY;
            }
            bitmap.scaleX = scale;
            bitmap.scaleY = scale;
            this.srcImgScale = scale;
            console.log("bitmap bounds" , name, bitmap.getBounds())
            var bitmapContainer =new createjs.Container();

            bitmapContainer.name = "imageToEdit";
            bitmapContainer.addChild(bitmap);


            this.stage1.addChild(bitmapContainer);
            console.log("stage bounds" , name, this.stage1.getBounds())
            //this.stage1.addChild(this.lineDrawingShape);



            this.authoringArea = this.stage1;
            this.authoringArea.mouseEnabled = true;

            this.authoringArea.addEventListener("dblclick" , this.boundHandleMouseDoubleClick );
            //this.authoringArea.addEventListener("stagemousedown", this.boundHandleMouseDown );
            this.authoringArea.addEventListener("mousedown", this.boundHandleMouseDown );

            this.stage1.update();

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
            //this.collectedCurves.push(curveShape);
        },
        //-----------------------------------------------------------------------
        popCurvesCollected: function() {
            this.currentMode.popCurvesCollected();
            //if(this.collectedCurves.length > 0) {
            //    this.collectedCurves.pop();
            //}
        },
        //-----------------------------------------------------------------------
        stackTopCurvesCollected: function() {
            return this.currentMode.stackTopCurvesCollected();
            /*
            if(this.collectedCurves.length > 0) {
                return this.collectedCurves[this.collectedCurves.length-1];
            } else {
                return undefined;
            }*/
        },
        //-----------------------------------------------------------------------
        removeLastCurveShapeFromStage: function() {

            var lastCurveShape = this.stackTopCurvesCollected();
            if (lastCurveShape !== undefined) {
                var thisParent = lastCurveShape.parent;
                thisParent.removeChild(lastCurveShape);
                //this.stage1.removeChild(lastCurveShape);
                this.popCurvesCollected();
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

    k.init = function (query) {
        var sourceImgFilename = query.sourceImgFilename || "f17.jpg"
        var baseDataDir = "http://localhost:8000/samples/skindetect/authoring/image/"
        k.frame = new k.Frame("demoCanvasFrame", "demoToolbar", "frame",  baseDataDir, sourceImgFilename );
    }

  }
  ());
