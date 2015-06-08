
(
  function() {
    var k = KgeorgeNamespace("K")
    var kUtils = KgeorgeNamespace("K.Utils")

    k.g_pickedPoints = {}


    k.Frame = Class.extend( {
        init: function(canvasId, name, imgUrl) {
            createjs.Ticker.setFPS(40);
            this.eStates = ["idle", "draw","finishDraw"];
            this.currentState = "idle";
            this.name = name;
            this.imgUrl = imgUrl;
            this.nextShapeIdx=0;
            this.pointsCollected=[]
            this.bMouseDown = false;
            this.previousPt = {x:0, y:0}
            this.currentPt = {x:0, y:0}
            this.curvesCollected = []
            this.keyState = {}
            this.nextCurveName = 0;

            var frameCanvasElement = document.getElementById(canvasId);
            this.stage = new createjs.Stage( frameCanvasElement );
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
                src:  imgUrl,
                crossOrigin:true,
                type: createjs.LoadQueue.IMAGE
            });
            loader.loadFile(loadItem);
            this.stage.enableMouseOver(10);


            this.boundHandleMouseDoubleClick = this.handleMouseDoubleClick.bind(this);
            this.boundHandleMouseDown = this.handleMouseDown.bind(this);
            this.boundHandleMouseUp = this.handleMouseUp.bind(this);
            this.boundHandleMouseMove = this.handleMouseMove.bind(this);
            this.boundHandleKeyDown  = this.handleKeyDown.bind(this);
            this.boundHandleKeyUp = this.handleKeyUp.bind(this);


            this.stage.addEventListener("dblclick" , this.boundHandleMouseDoubleClick );
            this.stage.addEventListener("stagemousedown", this.boundHandleMouseDown );
            createjs.Ticker.addEventListener("tick", this.tick.bind(this));
            window.onkeydown =this.boundHandleKeyDown;
            window.onkeyup = this.boundHandleKeyUp;
        },
        loaderImgOnLoad: function(ev) {
            console.log("load error~~~~~~~~~~~~")

            var img = document.createElement('img');
            img.crossOrigin = "Anonymous";
            img.src = ev.result.src;

            var bitmap = new createjs.Bitmap(img);
            bitmap.x=0;
            bitmap.y=0;
            console.log("bitmap bounds" , name, bitmap.getBounds())
            this.stage.addChild(bitmap);
            console.log("stage bounds" , name, this.stage.getBounds())
            //this.stage.addChild(this.lineDrawingShape);
            this.stage.update();

        },
        /*
        imgOnload: function() {
            var bitmap = new createjs.Bitmap(this.img);
            bitmap.x=0;
            bitmap.y=0;
            console.log("bitmap bounds" , name, bitmap.getBounds())
            this.stage.addChild(bitmap);
            console.log("stage bounds" , name, this.stage.getBounds())


            //this.stage.addChild(this.lineDrawingShape);
            this.stage.update();
        },*/

        //-----------------------------------------------------------------------
        //Mouse/keyboard handlers
        //-----------------------------------------------------------------------
        handleMouseMove: function (evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            this.stage.addEventListener("pressup", this.boundHandleMouseUp);
            this.currentPt = relPos;
            var currentCurve = this.stackTopCurvesCollected();
            currentCurve.collectedPoints.push(relPos);
            this.pointsCollected.push(relPos);
            this.stage.update();
        },
        //-----------------------------------------------------------------------
        handleMouseUp : function (evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            //console.log("UPUPUPUPUPUPP   stage mnouse Up", relPos.x, relPos.y);
            this.stage.addEventListener("stagemousedown", this.boundHandleMouseDown);
            this.stage.removeEventListener("pressup", this.boundHandleMouseUp);
            this.stage.removeEventListener("stagemousemove", this.boundHandleMouseMove);
            this.currentState = "idle";
            this.stage.update();

        },
        //-----------------------------------------------------------------------
        handleMouseDoubleClick : function(evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            //console.log("DCDCDCDCDCDCDC  stage mnouse down", relPos.x, relPos.y);
            this.stage.removeEventListener("stagemousemove" , this.boundHandleMouseMove);
            this.currentState = "finishDraw";
        },
        //-----------------------------------------------------------------------
        handleMouseDown : function(evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};


            var container = new createjs.Container();
            container.name = this.nextCurveName;
            this.nextCurveName += 1;


            var lineDrawingShape = new createjs.Shape().set({x:0,y:0});
            lineDrawingShape.graphics.beginFill("red");
            lineDrawingShape.name = "curve";
            container.addChild(lineDrawingShape);



            var startPoint = new createjs.Shape().set({x:relPos.x, y: relPos.y});
            startPoint.snapToPixelEnabled = true;
            container.addChild(startPoint);
            startPoint.graphics.beginFill("blue");
            startPoint.graphics.dc(0,0, 2);
            startPoint.name = "startPoint";

            this.stage.addChild(container);


            this.pushCurvesCollected(container);

            this.stage.addEventListener("stagemousemove" , this.boundHandleMouseMove);
            this.currentState = "draw";
            this.previousPt = relPos;
            this.currentPt = relPos;


            container.collectedPoints = [];
            container.collectedPoints.push(relPos);
            console.log("DDDDDDDDDDDDDDD  stage mnouse down", relPos.x, relPos.y);


            this.pointsCollected.push(relPos);

            this.stage.update();
        },
        //-----------------------------------------------------------------------
        handleKeyDown : function(evt) {

        },
        //-----------------------------------------------------------------------
        handleKeyUp : function(evt) {
            evt = evt || window.event;
            console.log("KKKKKKKKKKKKKKKKKK", evt.keyCode);
            this.keyState[evt.keyCode] = evt;
        },

        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //State maintainance
        //-----------------------------------------------------------------------
        pushCurvesCollected: function(curveShape) {
            this.curvesCollected.push(curveShape);
        },
        //-----------------------------------------------------------------------
        popCurvesCollected: function() {
            if(this.curvesCollected.length > 0) {
                this.curvesCollected.pop();
            }
        },
        //-----------------------------------------------------------------------
        stackTopCurvesCollected: function() {
            if(this.curvesCollected.length > 0) {
                return this.curvesCollected[this.curvesCollected.length-1];
            } else {
                return undefined;
            }
        },
        //-----------------------------------------------------------------------
        removeLastCurveShapeFromStage: function() {
            var lastCurveShape = this.stackTopCurvesCollected();
            if (lastCurveShape !== undefined) {
                this.stage.removeChild(lastCurveShape);
                this.popCurvesCollected();
            }
        },
        //-----------------------------------------------------------------------
        consolidateConsecutveCurves: function() {
            for(var i = this.curvesCollected.length; i > 0;  i -= 1) {
                if(i > 1) {
                    var curveIndex = i-1;
                    var currentCurve = this.curvesCollected[curveIndex];
                    var collectedPointsCurrent = currentCurve.collectedPoints;
                    var firstPointOfCurrentCurve = undefined;
                    if(collectedPointsCurrent.length > 0) {
                        firstPointOfCurrentCurve = collectedPointsCurrent[0];
                    }


                    var previousCurve = this.curvesCollected[curveIndex-1];
                    var collectedPointsPreviousCurve = previousCurve.collectedPoints;
                    var lastPointOfPreviousCurve = undefined;
                    if(collectedPointsPreviousCurve.length > 0) {
                        lastPointOfPreviousCurve = collectedPointsPreviousCurve[collectedPointsPreviousCurve.length-1];
                    }
                    console.log( "CCCCCCCCCCCCCCCCCCCCC", "name: ", currentCurve.name, "  numPoints: ", currentCurve.collectedPoints.length, "  currentIndex: ",  curveIndex, " previousIndex: ",  curveIndex-1, " distance: ", this.distBetweenPts(firstPointOfCurrentCurve,  lastPointOfPreviousCurve));
                }
            }
        },
        //-----------------------------------------------------------------------
        distBetweenPts: function(ptA, ptB) {
             var sqrDiff = (ptA.x - ptB.x)*(ptA.x - ptB.x) + (ptA.y - ptB.y)*(ptA.y - ptB.y);
             return sqrDiff
        },
        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //tick
        //-----------------------------------------------------------------------

        tick: function () {
            var drawColor = createjs.Graphics.getHSL(Math.random()*360, 100, 50);
            if (Object.keys(this.keyState).length > 0 ) {
                //keycode for z == 9-, Ctrl == 17
                if(this.keyState[90]) {
                    if(this.keyState[17] || this.keyState[90].ctrlKey) {
                        this.removeLastCurveShapeFromStage();
                        this.keyState = {}
                    }
                }
            }
            if(this.currentState == "draw") {
                var midPt = {x: (this.previousPt.x + this.currentPt.x)*0.5, y: (this.previousPt.y + this.currentPt.y)*0.5}
                var lastCurve = this.stackTopCurvesCollected();
                if(lastCurve) {
                    var curveShape = lastCurve.getChildByName("curve");
                    curveShape.graphics.setStrokeStyle(2);
                    curveShape.graphics.beginStroke(drawColor);
			        curveShape.graphics.moveTo(this.previousPt.x, this.previousPt.y);
			        curveShape.graphics.curveTo(midPt.x, midPt.y, this.currentPt.x, this.currentPt.y );
			        curveShape.graphics.endStroke();
                }

                this.previousPt = this.currentPt;


            } else if (this.currentState == "finishDraw") {
                this.previousPt = this.currentPt;
                this.consolidateConsecutveCurves();

                if(this.pointsCollected.length > 2) {
                    this.currentPt = this.pointsCollected[this.pointsCollected.length -3];
                    console.log( "FFFFFFFFFFFFF", this.currentPt, this.previousPt);
                }
                var midPt = {x: (this.previousPt.x + this.currentPt.x)*0.5, y: (this.previousPt.y + this.currentPt.y)*0.5};
                var lastCurve = this.stackTopCurvesCollected();
                if(lastCurve) {
                    var curveShape = lastCurve.getChildByName("curve");
                    curveShape.graphics.setStrokeStyle(2);
                    curveShape.graphics.beginStroke(drawColor);
			        curveShape.graphics.moveTo(this.previousPt.x, this.previousPt.y);
			        curveShape.graphics.curveTo(midPt.x, midPt.y, this.currentPt.x, this.currentPt.y );
			        curveShape.graphics.endStroke();
                }
                this.currentState = "idle";
            }

            this.stage.update();
            this.stage.tick();
        }
    });

    k.init = function () {
        var baseDataDir = "http://localhost:8000/samples/skindetect/authoring/image"
        k.frame = new k.Frame("demoCanvasFrame", "frame",  baseDataDir + "/frm/frm3.jpg" );
    }

  }
  ());
