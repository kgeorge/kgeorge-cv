
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


    Curve = function(relPos, name) {
        this.Container_constructor();
        this.setup(relPos, name);
    }

    var c = createjs.extend(Curve, createjs.Container);
    c.setup = function(relPos, name) {
        var container = this;
        container.set({x:relPos.x, y: relPos.y});
        container.startPoint = relPos;
        container.drawColor = createjs.Graphics.getHSL(Math.random()*360, 100, 50);
        container.name = name;



        var lineDrawingShape = new createjs.Shape().set({x:0,y:0});
        lineDrawingShape.graphics.beginFill("red");
        lineDrawingShape.name = "curve";
        container.addChild(lineDrawingShape);

        var pointsContainer = new createjs.Container();
        pointsContainer.name = "points";
        container.addChild(pointsContainer);

        this.createPointShape_Aux(this.relativePos(relPos));
        this.collectedPoints = [];
        this.collectedPoints.push(this.relativePos(relPos));
        this.bClosedCurve = false;

    }

    c.absolutePos = function(relPos) {
        return {x:relPos.x + this.startPoint.x, y: relPos.y + this.startPoint.y};
    }
    c.relativePos= function(absPos) {
         return {x:absPos.x - this.startPoint.x, y: absPos.y - this.startPoint.y};
    }
    c.addPointToDraw= function(previousPt, currentPt){
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
        this.createPointShape_Aux(currentPtRelPos);
        this.collectedPoints.push( currentPtRelPos);
    }

    c.createPointShape_Aux=function(relPos) {
        var point = new createjs.Shape();
        point.set({x:relPos.x, y:relPos.y});
        point.snapToPixelEnabled = true;
        point.graphics.beginFill("blue");
        point.graphics.dc(0,0, 2);
        point.visible = false;
        var pointsContainer =  this.getChildByName("points");
        pointsContainer.addChild(point);
    }

    c.setPointVisibility= function(b) {
        var pointsContainer =  this.getChildByName("points");
        for(var k=0; k < pointsContainer.children.length; k += 1) {
            var currentPoint = pointsContainer.children[k];
            currentPoint.visible = b;
        }
    }


    c.hitTestOnPoints = function(globalPos) {
        var pointIndexHit = undefined;
        var pointsContainer =  this.getChildByName("points");
        for(var k=0; k < pointsContainer.children.length; k += 1) {
            var currentPoint = pointsContainer.children[k];
            var localPos = currentPoint.globalToLocal(globalPos.x, globalPos.y);
            var bHit = currentPoint.hitTest(localPos.x, localPos.y);
            if(bHit) {
                pointIndexHit = k;
                break;
            }
        }
        return pointIndexHit;
    }

    c.redrawPoint = function(ptShape, color) {
        ptShape.graphics.clear();
        ptShape.graphics.beginFill(color);
        ptShape.graphics.dc(0,0, 2);
    }


    c.redrawAllPoints = function(color) {

        var pointsContainer =  this.getChildByName("points");
        for(var k=0; k < pointsContainer.children.length; k += 1) {
            var ptShape = pointsContainer.children[k];
            ptShape.graphics.clear();
            ptShape.graphics.beginFill(color);
            ptShape.graphics.dc(0,0, 2);
        }
    }
    c.setSelectionOfPoint = function(kThPoint, bSelect) {
        if(kThPoint) {
            var pointsContainer =  this.getChildByName("points");
            var pointSelected = pointsContainer.children[kThPoint];
            var color = (bSelect) ? "cyan" : "blue";
            this.redrawPoint(pointSelected, color);
        }
    }

    c.selectRangeOfPoints = function(lowerIndex, higherIndex, bSelected) {
        if(lowerIndex && higherIndex) {
            for(var i=lowerIndex; i <= higherIndex; i += 1) {
                this.setSelectionOfPoint(i, bSelected);
            }
        }
    }
    c.applyDeltaToPoints = function(deltaPt, lowIndex, highIndex) {
        var pointsContainer =  this.getChildByName("points");
        for(var k=lowIndex; k <= highIndex; k += 1) {
            var currentPoint = pointsContainer.children[k];
            currentPoint.x = currentPoint.x + deltaPt.x;
            currentPoint.y = currentPoint.y + deltaPt.y;
            this.redrawPoint(currentPoint, "cyan");
        }

        var coreCurveShape = this.getChildByName("curve");

    }

    c.redrawCurveShapeGraphics = function(bFill) {
        //assuming points is uptodate
        var curveShape = this.getChildByName("curve");
        var pointsContainer =  this.getChildByName("points");
        var previousPt = pointsContainer.children[0];
        if(!this.bClosedCurve) {
            curveShape.graphics.clear();

            curveShape.graphics.setStrokeStyle(2);
            curveShape.graphics.beginStroke(this.drawColor);
            for(var k=0; k < pointsContainer.children.length; k += 1) {
                var currentPt = pointsContainer.children[k];
                var midPt = {x: (previousPt.x + currentPt.x)*0.5, y: (previousPt.y + currentPt.y)*0.5}
                curveShape.graphics.moveTo(previousPt.x, previousPt.y);
                curveShape.graphics.curveTo(midPt.x, midPt.y, currentPt.x, currentPt.y );
                previousPt = currentPt;
            }
            curveShape.graphics.endStroke();
        } else {
            if(this.bClosedCurve) {
                if(bFill) {
                    curveShape.graphics.clear();

                    curveShape.graphics.beginFill(this.drawColor);
                    curveShape.graphics.moveTo(previousPt.x, previousPt.y);
                    for(var k=1; k < pointsContainer.children.length; k += 1) {
                        var currentPt = pointsContainer.children[k];
                        var midPt = {x: (previousPt.x + currentPt.x)*0.5, y: (previousPt.y + currentPt.y)*0.5}

                        curveShape.graphics.curveTo(midPt.x, midPt.y, currentPt.x, currentPt.y );
                        previousPt = currentPt;
                    }


                    previousPt = pointsContainer.children.slice(-1)[0];
                    currentPt = pointsContainer.children[0];
                    var midPt = {x: (previousPt.x + currentPt.x)*0.5, y: (previousPt.y + currentPt.y)*0.5}
                    curveShape.graphics.curveTo(midPt.x, midPt.y, currentPt.x, currentPt.y );
                    curveShape.graphics.endFill();
                    curveShape.alpha = 0.5
                    } else {

                    curveShape.graphics.clear();
                    curveShape.graphics.setStrokeStyle(2);
                    curveShape.graphics.beginStroke(this.drawColor);
                    curveShape.graphics.moveTo(previousPt.x, previousPt.y);
                    for(var k=1; k < pointsContainer.children.length; k += 1) {
                        var currentPt = pointsContainer.children[k];
                        var midPt = {x: (previousPt.x + currentPt.x)*0.5, y: (previousPt.y + currentPt.y)*0.5}
                        curveShape.graphics.curveTo(midPt.x, midPt.y, currentPt.x, currentPt.y );
                        previousPt = currentPt;
                    }
                    previousPt = pointsContainer.children.slice(-1)[0];
                    currentPt = pointsContainer.children[0];
                    var midPt = {x: (previousPt.x + currentPt.x)*0.5, y: (previousPt.y + currentPt.y)*0.5}
                    curveShape.graphics.curveTo(midPt.x, midPt.y, currentPt.x, currentPt.y );
                    curveShape.graphics.endStroke();
                    curveShape.alpha = 1.0
                }
            }
        }
    }


    k.Curve = createjs.promote(Curve, "Container");



    k.Mode = Class.extend( {
        init: function(modename, main){
            this.main = main;
            this.modename = modename;
            this.boundHandleMouseDoubleClick = this.handleMouseDoubleClick.bind(main);
            this.boundHandleMouseDown = this.handleMouseDown.bind(main);
            this.boundHandleMouseUp = this.handleMouseUp.bind(main);
            this.boundHandleMouseMove = this.handleMouseMove.bind(main);
            this.boundHandleKeyDown  = this.handleKeyDown.bind(main);
            this.boundHandleKeyUp = this.handleKeyUp.bind(main);
            this.boundTick = this.tick.bind(main);
            this.boundSwitchFrom = this.switchFrom.bind(main);
            this.currentState = "idle";
        },
        handleMouseDoubleClick: function(evt) {


        },
        handleMouseDown: function(evt) {

        },
        handleMouseUp: function(evt) {

        },
        handleMouseMove: function(evt) {

        },
        handleKeyDown: function(evt) {
        },
        handleKeyUp: function(evt) {

        },
        tick: function() {
             console.log("edit mode tick");
        },
        switchFrom: function(previousMode) {
            this.currentState = "idle";
        },

        //-----------------------------------------------------------------------
        pushCurvesCollected: function(curveShape) {
            this.main.collectedCurves.push(curveShape);
        },
        //-----------------------------------------------------------------------
        popCurvesCollected: function() {
            if(this.main.collectedCurves.length > 0) {
                this.main.collectedCurves.pop();
            }
        },
        //-----------------------------------------------------------------------
        stackTopCurvesCollected: function() {
            if(this.main.collectedCurves.length > 0) {
                return this.main.collectedCurves[this.main.collectedCurves.length-1];
            } else {
                return undefined;
            }
        },
        cleanBeforeSwitch: function() {
            this.currentState = "idle";
        }

    })

  }
  ());
