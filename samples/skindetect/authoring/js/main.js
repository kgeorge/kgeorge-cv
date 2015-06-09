
(
  function() {
    var k = KgeorgeNamespace("K")
    var kUtils = KgeorgeNamespace("K.Utils")

    k.g_pickedPoints = {}

    k.sqrDistBetweenPts= function(ptA, ptB) {
             var sqrDiff = (ptA.x - ptB.x)*(ptA.x - ptB.x) + (ptA.y - ptB.y)*(ptA.y - ptB.y);
             return sqrDiff
        }
    k.minSqrdDistanceToDistinguishPts = 0.1


    k.Frame = Class.extend( {
        init: function(canvasId, toolbarId,  name, imgUrl) {
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
            this.canvasId = canvasId;
            this.toolbarId = toolbarId;

            var frameCanvasElement = document.getElementById(canvasId);
            this.stage = new createjs.Stage( frameCanvasElement );
            this.stage.mouseMoveOutside = false;
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


            //this.stage.addEventListener("dblclick" , this.boundHandleMouseDoubleClick );
            //this.stage.addEventListener("stagemousedown", this.boundHandleMouseDown );
            //this.stage.addEventListener("mousedown", this.boundHandleMouseDown );
            createjs.Ticker.addEventListener("tick", this.tick.bind(this));
            window.onkeydown =this.boundHandleKeyDown;
            window.onkeyup = this.boundHandleKeyUp;

            _.extend(createjs.Container.prototype, {
                absolutePos: function(relPos) {
                    return {x:relPos.x + this.startPoint.x, y: relPos.y + this.startPoint.y};
                },
                relativePos: function(absPos) {
                     return {x:absPos.x - this.startPoint.x, y: absPos.y - this.startPoint.y};
                },
                addPointToDraw: function(previousPt, currentPt){
                    var distBetPts = k.sqrDistBetweenPts( previousPt , currentPt );
                    if(distBetPts < k.minSqrdDistanceToDistinguishPts) {
                        return;
                    }
                    var midPt = {x: (previousPt.x + currentPt.x)*0.5, y: (previousPt.y + currentPt.y)*0.5}
                    midPt = this.relativePos(midPt);
                    var previousPtRelPos = this.relativePos(previousPt);
                    var currentPtRelPos = this.relativePos(currentPt);
                    var curveShape = this.getChildByName("curve");
                    curveShape.graphics.setStrokeStyle(2);
                    curveShape.graphics.beginStroke(this.drawColor);
			        curveShape.graphics.moveTo(previousPtRelPos.x, previousPtRelPos.y);
			        curveShape.graphics.curveTo(midPt.x, midPt.y, currentPtRelPos.x, currentPtRelPos.y );
			        curveShape.graphics.endStroke();
			        this.collectedPoints.push( currentPtRelPos);
			    }

            });
        },
        wireupToolbar: function() {


        },
        loaderImgOnLoad: function(ev) {
            console.log("load error~~~~~~~~~~~~")

            var img = document.createElement('img');
            img.crossOrigin = "Anonymous";
            img.src = ev.result.src;

            var bitmap = new createjs.Bitmap(img);
            bitmap.name = "imageToEdit";
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
            console.log("bitmap bounds" , name, bitmap.getBounds())
            var bitmapContainer =new createjs.Container();
            bitmapContainer.addChild(bitmap);

            this.stage.addChild(bitmapContainer);
            console.log("stage bounds" , name, this.stage.getBounds())
            //this.stage.addChild(this.lineDrawingShape);



            this.authoringArea = this.stage;
            this.authoringArea.mouseEnabled = true;

            this.authoringArea.addEventListener("dblclick" , this.boundHandleMouseDoubleClick );
            //this.authoringArea.addEventListener("stagemousedown", this.boundHandleMouseDown );
            this.authoringArea.addEventListener("mousedown", this.boundHandleMouseDown );

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
            console.log("MMMMMVVVVV   stage mnouse Up", relPos.x, relPos.y);
            this.authoringArea.addEventListener("pressup", this.boundHandleMouseUp);
            this.currentPt = relPos;
            var currentCurve = this.stackTopCurvesCollected();
           // currentCurve.collectedPoints.push(currentCurve.relativePos(relPos));
            this.pointsCollected.push(relPos);
            this.stage.update();
        },
        //-----------------------------------------------------------------------
        handleMouseUp : function (evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            //console.log("UPUPUPUPUPUPP   stage mnouse Up", relPos.x, relPos.y);
            //this.authoringArea.addEventListener("stagemousedown", this.boundHandleMouseDown);
            this.authoringArea.addEventListener("mousedown", this.boundHandleMouseDown);
            this.authoringArea.removeEventListener("pressup", this.boundHandleMouseUp);
            this.authoringArea.removeEventListener("stagemousemove", this.boundHandleMouseMove);
            //this.authoringArea.removeEventListener("mousemove", this.boundHandleMouseMove);
            this.currentState = "idle";
            this.stage.update();

        },
        //-----------------------------------------------------------------------
        handleMouseDoubleClick : function(evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            //console.log("DCDCDCDCDCDCDC  stage mnouse down", relPos.x, relPos.y);
            this.authoringArea.removeEventListener("stagemousemove" , this.boundHandleMouseMove);
            //this.authoringArea.removeEventListener("mousemove" , this.boundHandleMouseMove);
            this.currentState = "finishDraw";

        },
        //-----------------------------------------------------------------------
        handleMouseDown : function(evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};


            var container = new createjs.Container();
            container.set({x:relPos.x, y: relPos.y});
            container.startPoint = relPos;
            container.drawColor = createjs.Graphics.getHSL(Math.random()*360, 100, 50);
            container.name = this.nextCurveName;
            this.nextCurveName += 1;


            var lineDrawingShape = new createjs.Shape().set({x:0,y:0});
            lineDrawingShape.graphics.beginFill("red");
            lineDrawingShape.name = "curve";
            container.addChild(lineDrawingShape);



            var startPoint = new createjs.Shape().set({x:0, y: 0});
            startPoint.snapToPixelEnabled = true;
            container.addChild(startPoint);
            startPoint.graphics.beginFill("blue");
            startPoint.graphics.dc(0,0, 2);
            startPoint.name = "startPoint";

            this.stage.addChild(container);


            this.pushCurvesCollected(container);

            //this.authoringArea.addEventListener("mousemove" , this.boundHandleMouseMove);
            this.authoringArea.addEventListener("stagemousemove" , this.boundHandleMouseMove);
            this.currentState = "draw";
            this.previousPt = relPos;
            this.currentPt = relPos;


            container.collectedPoints = [];
            container.collectedPoints.push(container.relativePos(relPos));
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
            var indicesToConsolidate = [];
            for(var i = this.curvesCollected.length; i > 0;  i -= 1) {
                var curveIndex = i-1;
                var currentCurve = this.curvesCollected[curveIndex];
                var collectedPointsCurrent = currentCurve.collectedPoints;
                var firstPointOfCurrentCurve = undefined;
                if(collectedPointsCurrent.length > 0) {
                    firstPointOfCurrentCurve = currentCurve.absolutePos(collectedPointsCurrent[0]);
                }
                indicesToConsolidate.unshift(curveIndex);
                if(i <= 1) {
                    break;
                }

                var previousCurve = this.curvesCollected[curveIndex-1];
                var collectedPointsPreviousCurve = previousCurve.collectedPoints;
                var lastPointOfPreviousCurve = undefined;
                if(collectedPointsPreviousCurve.length > 0) {
                    lastPointOfPreviousCurve = previousCurve.absolutePos(collectedPointsPreviousCurve[collectedPointsPreviousCurve.length-1]);
                }
                console.log( "CCCCCCCCCCCCCCCCCCCCC", "name: ", currentCurve.name, "  numPoints: ", currentCurve.collectedPoints.length, "  currentIndex: ",  curveIndex, " previousIndex: ",  curveIndex-1, " distance: ", k.sqrDistBetweenPts(firstPointOfCurrentCurve,  lastPointOfPreviousCurve));

                if(k.sqrDistBetweenPts(firstPointOfCurrentCurve,  lastPointOfPreviousCurve) > 500) {
                    break;
                }
            }
            var motherCurve = this.curvesCollected[indicesToConsolidate[0]];
            for(var i=1; i < indicesToConsolidate.length; i += 1) {
                var curveToConsolidate = this.curvesCollected[indicesToConsolidate[i]];
                this.consolidateTwoCurves(motherCurve, curveToConsolidate );
            }
            //assert indicesToConsolidate.length > = 2
            this.curvesCollected.splice(indicesToConsolidate[1], indicesToConsolidate.length-1)
            for(var i=this.curvesCollected.length-1; i >= 0; i -= 1) {
                var currentCurve = this.curvesCollected[i];
                console.log( "CCCCCCCCCCCCCCCCCCCCC", "name: ", currentCurve.name, "  numPoints: ", currentCurve.collectedPoints.length, " firstPt: ", currentCurve.absolutePos(currentCurve.collectedPoints[0]), " lastPt: ",  currentCurve.absolutePos(currentCurve.collectedPoints[currentCurve.collectedPoints.length-1]));
            }
            this.stage.update();
        },

        consolidateTwoCurves: function(mother, curveToBeConsolidated) {
            //assert mother.collectedPoints.length > 0;
            var previousPt = mother.absolutePos(mother.collectedPoints[mother.collectedPoints.length-1]);
            for(var i=0; i < curveToBeConsolidated.collectedPoints.length; i += 1) {
                var currentPt =  curveToBeConsolidated.absolutePos(curveToBeConsolidated.collectedPoints[i]);
                mother.addPointToDraw(previousPt, currentPt);
                mother.collectedPoints.push(mother.relativePos(currentPt));
                previousPt = currentPt;
            }
            this.stage.removeChild(curveToBeConsolidated);
        },
        //-----------------------------------------------------------------------

        //-----------------------------------------------------------------------
        //-----------------------------------------------------------------------
        //tick
        //-----------------------------------------------------------------------

        tick: function () {
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

                var lastCurve = this.stackTopCurvesCollected();
                if(lastCurve) {
                    //assert lastCurve.collectedPoints.length >= 1
                    var previousPt = lastCurve.absolutePos(lastCurve.collectedPoints[lastCurve.collectedPoints.length-1]);
                    lastCurve.addPointToDraw(previousPt, this.currentPt);
                }
                this.previousPt = this.currentPt;
            } else if (this.currentState == "finishDraw") {
                this.previousPt = this.currentPt;
                this.consolidateConsecutveCurves();
                /*if(this.pointsCollected.length > 2) {
                    this.currentPt = this.pointsCollected[this.pointsCollected.length -3];
                    console.log( "FFFFFFFFFFFFF", this.currentPt, this.previousPt);
                }
                var lastCurve = this.stackTopCurvesCollected();
                if(lastCurve) {
                    lastCurve.addPointToDraw(this.previousPt, this.currentPt);
                }*/
                this.currentState = "idle";
            }

            this.stage.update();
            this.stage.tick();
        }
    });

    k.init = function () {
        var baseDataDir = "http://localhost:8000/samples/skindetect/authoring/image"
        k.frame = new k.Frame("demoCanvasFrame", "demoToolbar", "frame",  baseDataDir + "/f16.jpg" );
    }

  }
  ());
