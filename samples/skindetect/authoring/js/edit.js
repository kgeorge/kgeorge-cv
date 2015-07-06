
(
  function() {
    var K = KgeorgeNamespace("K")
    var KUtils = KgeorgeNamespace("K.Utils")

    K.g_pickedPoints = {}

    K.sqrDistBetweenPts= function(ptA, ptB) {
             var sqrDiff = (ptA.x - ptB.x)*(ptA.x - ptB.x) + (ptA.y - ptB.y)*(ptA.y - ptB.y);
             return sqrDiff
        }
    K.minSqrdDistanceToDistinguishPts = 50;
     K.minSqrdDistanceToDistinguishPts2 = 2000;
    K.EditMode = K.Mode.extend( {
        init: function(main){
            this._super("edit", main);
            this.eDrawStates = ["idle", "draw","finishDraw", "move"];
            this.selectedCurve = undefined;
            this.editCurves = []
            this.intersectingPoints = []
        },
        //-----------------------------------------------------------------------
        //Mouse/keyboard handlers
        //-----------------------------------------------------------------------
        handleMouseMove: function (evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            this.authoringArea.addEventListener("pressup", this.boundHandleMouseUp);
            this.currentPt = relPos;
            var currentCurve = this.stackTopCurvesCollected();
           // currentCurve.collectedPoints.push(currentCurve.relativePos(relPos));
            this.stage1.update();
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
            this.currentMode.currentState = "idle";
            this.stage1.update();

        },
        //-----------------------------------------------------------------------
        handleMouseDoubleClick : function(evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            //console.log("DCDCDCDCDCDCDC  stage mnouse down", relPos.x, relPos.y);
            this.authoringArea.removeEventListener("stagemousemove" , this.boundHandleMouseMove);
            //this.authoringArea.removeEventListener("mousemove" , this.boundHandleMouseMove);
            this.currentMode.currentState = "finishDraw";

        },
        //-----------------------------------------------------------------------
        handleMouseDown : function(evt) {
            if(this.currentMode.currentState == "idle") {
                var currentCurve = undefined;
                var relPos = {x: evt.stageX, y: evt.stageY};
                var bHit = false;
                for(var i=0; i < this.collectedCurves.length; i += 1) {
                    currentCurve = this.collectedCurves[i];
                    var currentCurveShape = currentCurve.getChildByName("curve");
                    var localPos = currentCurveShape.globalToLocal(relPos.x, relPos.y);

                    bHit = currentCurveShape.hitTest(localPos.x, localPos.y);
                    if(bHit) {
                        currentCurve.setPointVisibility(true);
                        console.log("hit curve:, ", currentCurve.name);
                        this.currentMode.selectedCurve = currentCurve;
                        this.currentMode.currentState = "draw";
                        break;
                    }
                }
                if(!bHit) {
                    this.currentMode.currentState = "move";
                this.previousPt = relPos;
                this.currentPt = relPos;
                }
            } else if(this.currentMode.currentState == "draw") {

                var relPos = {x: evt.stageX, y: evt.stageY};
                var newCurve = new K.Curve(relPos,  this.nextCurveName);
                this.nextCurveName += 1;

                var editContainer = this.stage1.getChildByName("edit");
                if(!editContainer) {
                    editContainer = new createjs.Container();
                    editContainer.name = "edit";
                    this.stage1.addChild(editContainer);
                }


                editContainer.addChild(newCurve);


                this.pushCurvesCollected(newCurve);


                this.currentMode.currentState = "draw";
                this.previousPt = relPos;
                this.currentPt = relPos;


                console.log("DDDDDDDDDDDDDDD  stage mnouse down", relPos.x, relPos.y);
            }  else if(this.currentMode.currentState == "move") {

                this.previousPt = relPos;
                this.currentPt = relPos;

            }

                //this.authoringArea.addEventListener("mousemove" , this.boundHandleMouseMove);
            this.authoringArea.addEventListener("stagemousemove" , this.boundHandleMouseMove);

            this.stage1.update();
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
        tick: function() {
            if (Object.keys(this.keyState).length > 0 ) {
                //keycode for z == 9-, Ctrl == 17
                if(this.keyState[90]) {
                    if(this.keyState[17] || this.keyState[90].ctrlKey) {
                        this.removeLastCurveShapeFromStage();
                        this.keyState = {}
                    }
                }
            }
            if(this.currentMode.currentState == "draw") {

                var lastCurve = this.stackTopCurvesCollected();
                if(lastCurve) {
                    //assert lastCurve.collectedPoints.length >= 1
                    var previousPt = lastCurve.absolutePos(lastCurve.collectedPoints[lastCurve.collectedPoints.length-1]);
                    lastCurve.addPointToDraw(previousPt, this.currentPt);
                }
                this.previousPt = this.currentPt;
            } else if (this.currentMode.currentState == "finishDraw") {
                this.currentMode.consolidateCurves();
                var editCurve = this.stackTopCurvesCollected();
                this.intersectingPoints =  this.currentMode.selectedCurve.findIntersectingPointsWithAnotherCurve(editCurve);
                this.currentMode.selectedCurve.redrawSomePoints( "red", {x:0, y:0}, this.intersectingPoints);
                this.currentMode.currentState = "move"
            } else if (this.currentMode.currentState == "move") {
            if(this.intersectingPoints && this.currentPt && this.previousPt) {
                var deltaPos = {x: this.currentPt.x - this.previousPt.x, y: this.currentPt.y - this.previousPt.y};
                this.currentMode.selectedCurve.redrawSomePoints("blue", deltaPos, this.intersectingPoints);
                var bFill = false;
                this.currentMode.selectedCurve.redrawCurveShapeGraphics(bFill);
                var editCurve = this.stackTopCurvesCollected();
                editCurve.moveCurve(deltaPos);
            }
                this.previousPt = this.currentPt;
            }
        },
        switchFrom: function(previousMode){
            if(previousMode){
                previousMode.cleanBeforeSwitch();
            }
            this.keyState = {};
            if(previousMode && (previousMode.modename == "edit" || previousMode.modename == "connect")) {
                for(var i=0; i < this.collectedCurves.length; i += 1) {
                    var currentCurve = this.collectedCurves[i];
                    currentCurve.setPointVisibility(false);
                }
            }
            this.currentMode.selectedCurve = undefined;
            this.currentMode.currentState = "idle";
        },
        //-----------------------------------------------------------------------
        pushCurvesCollected: function(curveShape) {
            this.editCurves.push(curveShape);
        },
        //-----------------------------------------------------------------------
        popCurvesCollected: function() {
            if(this.editCurves.length > 0) {
                this.editCurves.pop();
            }
        },
        //-----------------------------------------------------------------------
        stackTopCurvesCollected: function() {
            if(this.editCurves.length > 0) {
                return this.editCurves[this.editCurves.length-1];
            } else {
                return undefined;
            }
        },
        cleanBeforeSwitch: function() {
            this.editCurves = [];
            var editCurve = this.main.stage1.getChildByName("edit");
            this.main.stage1.removeChild(editCurve);
            if(this.selectedCurve) {
                this.selectedCurve.setPointVisibility(false);
            }
            this.currentState = "idle";
            this.intersectingPoints = [];
        },

        //-----------------------------------------------------------------------
        consolidateCurves: function() {
            var motherCurve =  this.stackTopCurvesCollected();// ;
            motherCurve.bClosedCurve = true;
            motherCurve.redrawAllPoints("blue");
            var bFill = false;
            motherCurve.redrawCurveShapeGraphics(bFill);
        }
    });



  }
  ());
