
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



    k.ConnectMode = k.Mode.extend( {
        init: function(main){
            this._super("connect", main);
            this.resetSelection();
        },
        resetSelection: function() {

                //this.currentMode.currentCurvesSelected
            this.currentCurvesSelected=[]
        },
        //-----------------------------------------------------------------------
        handleKeyDown : function(evt) {
            evt = evt || window.event;
            this.keyState[evt.keyCode] = evt;
        },
        //-----------------------------------------------------------------------
        handleKeyUp : function(evt) {
            evt = evt || window.event;
            delete this.keyState[evt.keyCode];
        },
        //-----------------------------------------------------------------------
        //Mouse/keyboard handlers
        //-----------------------------------------------------------------------
        handleMouseMove: function (evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            this.authoringArea.addEventListener("pressup", this.boundHandleMouseUp);
            this.currentPt = relPos;
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
            this.currentMode.currentState = "idle";
            this.stage1.update();

        },

        handleMouseDown : function(evt) {
            var currentCurve = undefined;
            var relPos = {x: evt.stageX, y: evt.stageY};
            for(var i=0; i < this.collectedCurves.length; i += 1) {
                currentCurve = this.collectedCurves[i];
                var currentCurveShape = currentCurve.getChildByName("curve");
                var localPos = currentCurveShape.globalToLocal(relPos.x, relPos.y);

                var bHit = currentCurveShape.hitTest(localPos.x, localPos.y);
                if(bHit) {
                    currentCurve.setPointVisibility(true);
                    console.log("hit curve:, ", currentCurve.name);
                    bNoDuplicate = true;
                    for(var j=0; j < this.currentMode.currentCurvesSelected.length; j += 1) {
                        if(this.currentMode.currentCurvesSelected[j] ==  currentCurve) {
                            bNoDuplicate = false;
                            break;
                        }
                    }
                    if(bNoDuplicate) {
                        this.currentMode.currentCurvesSelected.push(currentCurve);
                    }
                    break;
                }
            }
            this.currentMode.currentState = "draw";
            this.authoringArea.addEventListener("stagemousemove" , this.boundHandleMouseMove);
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
        switchFrom: function(previousMode){
            if(previousMode){
                previousMode.cleanBeforeSwitch();
            }
            this.keyState = {};
            this.currentMode.resetSelection();

                //this.currentMode.currentCurvesSelected
            this.currentMode.currentState = "idle";
        },


        //-----------------------------------------------------------------------
        consolidateCurves: function() {
            if(this.currentCurvesSelected.length <= 0) {
                return;
            }
            var bAnyCurveClosed = false;
            for(var i=0;  !bAnyCurveClosed && i < this.currentCurvesSelected.length; i += 1) {
                bAnyCurveClosed = this.currentCurvesSelected[i].bClosedCurve;
            }
            if( bAnyCurveClosed ) {
                console.log("cannot consolidate curves since some selected curve could be closed");
            }

            var motherCurve =  this.currentCurvesSelected[0];
            if( this.currentCurvesSelected.length > 1) {
                for(var i=1; i < this.currentCurvesSelected.length; i += 1) {
                    var didConsolidated = this.consolidateTwoCurves(motherCurve, this.currentCurvesSelected[i] );
                    if(didConsolidated) {
                        var indexOfConsolidatedCurve = this.main.collectedCurves.indexOf(this.currentCurvesSelected[i]);
                        kUtils.assert((indexOfConsolidatedCurve > -1), "error logic")
                        this.main.collectedCurves.splice(indexOfConsolidatedCurve, 1);
                        this.main.stage1.removeChild( this.currentCurvesSelected[i]);
                    }
                }
            } else {
                motherCurve.bClosedCurve = true;
            }
            motherCurve.redrawAllPoints("blue");
            var bFill = true;
            var motherCurveCurve = motherCurve.getChildByName("curve");
            if(motherCurveCurve.alpha == 0.5) {
                bFill = false;
            }
            motherCurve.redrawCurveShapeGraphics(bFill);
        },
        consolidateTwoCurves: function(mother, curveToBeConsolidated) {

            var pointsContainerMother =  mother.getChildByName("points");
            var pointsContainerSubject =  curveToBeConsolidated.getChildByName("points");
            //assert  pointsContainerMother.children.length > 0
            //assert  pointsContainerSubject.children.length > 0

            var lastPointOfMother =  mother.absolutePos (pointsContainerMother.children.slice(-1)[0]);
            var firstPointOfSubject = curveToBeConsolidated.absolutePos ( pointsContainerSubject.children[0] );
            var firstPointOfMother = mother.absolutePos( pointsContainerMother.children[0] );
            var lastPointOfSubject = curveToBeConsolidated.absolutePos( pointsContainerSubject.children.slice(-1)[0] );
            var sqrDistLastFirst = k.sqrDistBetweenPts( lastPointOfMother ,firstPointOfSubject);
            var sqrDistFirstLast =   k.sqrDistBetweenPts(firstPointOfMother, lastPointOfSubject);
            var sqrDistFirstFirst =   k.sqrDistBetweenPts(firstPointOfMother, firstPointOfSubject);
            var sqrDistLastLast =   k.sqrDistBetweenPts(lastPointOfMother, lastPointOfSubject);
            var minSqrDistance = Math.min(  sqrDistLastFirst,   sqrDistFirstLast , sqrDistFirstFirst, sqrDistLastLast  );
            if(  minSqrDistance > k.minSqrdDistanceToDistinguishPts2) {
                return false;
            }

            if( minSqrDistance == sqrDistLastFirst ) {
                for(var i=0; i < pointsContainerSubject.children.length; i += 1) {
                    var currentPtPos =  curveToBeConsolidated.absolutePos({x: pointsContainerSubject.children[i].x, y:  pointsContainerSubject.children[i].y});
                    currentPtPos = mother.relativePos(currentPtPos);
                    pointsContainerSubject.children[i].x = currentPtPos.x;
                    pointsContainerSubject.children[i].y = currentPtPos.y;
                    pointsContainerMother.children.push(pointsContainerSubject.children[i]);
                }
            }

            else if( minSqrDistance == sqrDistFirstLast ) {
                for(var i=pointsContainerSubject.children.length-1; i >= 0; i -= 1) {
                    var currentPtPos =  curveToBeConsolidated.absolutePos({x: pointsContainerSubject.children[i].x, y: pointsContainerSubject.children[i].y});
                    currentPtPos = mother.relativePos(currentPtPos);
                    pointsContainerSubject.children[i].x = currentPtPos.x;
                    pointsContainerSubject.children[i].y = currentPtPos.y;
                    pointsContainerMother.children.unshift(pointsContainerSubject.children[i]);
                }

            } else if ( minSqrDistance == sqrDistFirstFirst  ) {

                for(var i=0; i < pointsContainerSubject.children.length; i += 1) {
                    var currentPtPos =  curveToBeConsolidated.absolutePos({x: pointsContainerSubject.children[i].x, y: pointsContainerSubject.children[i].y});
                    currentPtPos = mother.relativePos(currentPtPos);
                    pointsContainerSubject.children[i].x = currentPtPos.x;
                    pointsContainerSubject.children[i].y = currentPtPos.y;
                    pointsContainerMother.children.unshift(pointsContainerSubject.children[i]);
                }

            } else if(  minSqrDistance == sqrDistLastLast ) {
                for(var i=pointsContainerSubject.children.length-1; i >= 0; i -= 1) {
                    var currentPtPos =  curveToBeConsolidated.absolutePos({x: pointsContainerSubject.children[i].x, y: pointsContainerSubject.children[i].y});
                    currentPtPos = mother.relativePos(currentPtPos);
                    pointsContainerSubject.children[i].x = currentPtPos.x;
                    pointsContainerSubject.children[i].y = currentPtPos.y;
                    pointsContainerMother.children.push(pointsContainerSubject.children[i]);
                }
            }
            return true;
        },

        tick: function() {
            if(this.currentMode.currentState == "finishDraw") {


                var curvesConsolidated ="";
                for(var k= 0; k < this.currentMode.currentCurvesSelected.length; k += 1) {
                    curvesConsolidated += this.currentMode.currentCurvesSelected[k].name + ", ";
                }
                console.log("consolidating cyjurves...", curvesConsolidated);
                this.currentMode.consolidateCurves();
                this.currentMode.currentState = "idle"
            }
        }
    });


  }
  ());
