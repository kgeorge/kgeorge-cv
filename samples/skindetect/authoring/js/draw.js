
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

    k.DrawMode = k.Mode.extend( {
        init: function(main){
            this._super("draw", main);
            this.eDrawStates = ["idle", "draw","finishDraw"];
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
            var relPos = {x: evt.stageX, y: evt.stageY};
            var newCurve = new k.Curve(relPos,  this.nextCurveName);
            this.nextCurveName += 1;

            this.stage1.addChild(newCurve);


            this.pushCurvesCollected(newCurve);

            //this.authoringArea.addEventListener("mousemove" , this.boundHandleMouseMove);
            this.authoringArea.addEventListener("stagemousemove" , this.boundHandleMouseMove);
            this.currentMode.currentState = "draw";
            this.previousPt = relPos;
            this.currentPt = relPos;


            console.log("DDDDDDDDDDDDDDD  stage mnouse down", relPos.x, relPos.y);



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
                this.previousPt = this.currentPt;
                this.currentMode.currentState = "idle";
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
            this.currentMode.currentState = "idle";
        }
    });

  }
  ());
