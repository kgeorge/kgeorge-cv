
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
        this.bPointsVisible = false;
        this.bNormalizedCurveDirection = false;

    }

    c.absolutePos = function(relPos) {
        return {x:relPos.x + this.startPoint.x, y: relPos.y + this.startPoint.y};
    }
    c.relativePos= function(absPos) {
         return {x:absPos.x - this.startPoint.x, y: absPos.y - this.startPoint.y};
    }
    c.centroidRelative = function() {
        var centroid = undefined;
        var pointsContainer =  this.getChildByName("points");
        if( pointsContainer.children.length < 1) {
            return undefined;
        }

        centroid = {x:0, y:0};
        for(var k=0; k < pointsContainer.children.length; k += 1) {
            var currentPoint = pointsContainer.children[k];
            centroid =  { x: (centroid.x + currentPoint.x) , y : (centroid.y + currentPoint.y)};
        }
        centroid = {x : centroid.x/(pointsContainer.children.length), y: centroid.y/(pointsContainer.children.length)};
        return centroid;
    }

    c.isToTheLeftOf = function (pt, lineStart, lineEnd) {
        var ptWrtLineStart = {x: (pt.x -lineStart.x), y: (pt.y - lineStart.y)}
        var lineEndWrtLineStart = { x: (lineEnd.x - lineStart.x), y : (lineEnd.y - lineStart.y)}
        var x1y1 = ptWrtLineStart;
        var x2y2 = lineEndWrtLineStart;
        var  deter = x1y1.x * x2y2.y - x1y1.y * x2y2.x;
        return deter > 0;
    }

    c.inside = function(pt) {
        var temporaryCoordSysChange = function(relPt) {
            return {x: -relPt.x, y: relPt.y};
        }
        var evaluateLine = function(m,c, pt){
            return (pt.y - m * pt.x - c);
        }

        var pointsContainer =  this.getChildByName("points");
        if( pointsContainer.children.length < 1) {
            return undefined;
        }
        var nPointsInLine = pointsContainer.children.length;
        var bIsSoFarToTheLeftOf = true;
        var m=0;
        var c =0;
        var ptTempCoord = temporaryCoordSysChange(pt);
        var originTempCoord = temporaryCoordSysChange( {x: -this.x, y: -this.y})
        KUtils.assert((ptTempCoord.x != originTempCoord.x), "c.isinside");
        if(ptTempCoord.x != originTempCoord.x) {
            m = (ptTempCoord.y - originTempCoord.y)/(ptTempCoord.x - originTempCoord.x);
            c = ptTempCoord. y - m * ptTempCoord.x;
        }
        var numHitsWithPtLine =Number(0);
        var thsOrigEval = -this.y -m * this.x -c;
        for(var k=0; bIsSoFarToTheLeftOf &&  k < nPointsInLine; k += 1) {
            var lineStart = temporaryCoordSysChange(pointsContainer.children[k]);
            var lineEnd = temporaryCoordSysChange(pointsContainer.children[(k+1) % nPointsInLine]);
            var lineStartEval = evaluateLine(m, c, lineStart);
            var lineEndEval = evaluateLine(m, c, lineEnd);
            var sqrDistBetLineEnds = K.sqrDistBetweenPts(lineStart, lineEnd);
            var bHitsLine = false;
            if(sqrDistBetLineEnds > 0) {
                if(lineStartEval <= 0 && lineEndEval >= 0 || lineStartEval >= 0 && lineEndEval <= 0 )  {
                    bHitsLine = true;
                } else {
                    KUtils.assert(((lineStartEval < 0 && lineEndEval < 0) || (lineStartEval > 0 && lineEndEval > 0 )), "c.inside2");
                }
            } else {
                if (lineStartEval == 0 || lineEndEval ==0) {
                    bHitsLine = true;
                }
            }
            if(bHitsLine) {
                var delta = { x: ptTempCoord.x - originTempCoord.x, y:  ptTempCoord.y - originTempCoord.y};
                KUtils.assert((mDash * delta.x - delta.y) != 0, "insinside3");
                var t = -1.0;
                if(lineEnd.x != lineStart.x) {
                    var mDash = (lineEnd.y  - lineStart.y)/(lineEnd.x - lineStart.x);
                    var cDash = lineEnd.y - mDash * lineEnd.x;
                    t = (originTempCoord.y - mDash * originTempCoord.x - cDash) / (mDash * delta.x - delta.y );
                }  else {
                    t = (lineEnd.x - originTempCoord.x)/(ptTempCoord.x - originTempCoord.x);
                }
                if(t >= 0 && t <= 1) {
                    numHitsWithPtLine += 1;
                }

            }
        }
        //numHitsWithPtLine is odd
        return  numHitsWithPtLine > 0 && (numHitsWithPtLine%2) > 0;
    },

    c.normalizeCurveDirection = function() {
        var centroidRel = this.centroidRelative();
        var isCenterInside = this.inside(centroidRel);
        if( !isCenterInside ) {
            //reverse direction
            var pointsContainer =  this.getChildByName("points");
            var newPointsContainer = new createjs.Container();
            newPointsContainer.name = "points";
            for(var k=pointsContainer.children.length-1; k >= 0; k -= 1) {
                var currentPoint = pointsContainer.children[k];
                pointsContainer.removeChild(currentPoint);
                newPointsContainer.addChild(currentPoint);
            }
            this.removeChild(pointsContainer);
            this.addChild(newPointsContainer);
        }
        this.bNormalizedCurveDirection = true;
    }


    c.addPointToDraw= function(previousPt, currentPt){
        var distBetPts = K.sqrDistBetweenPts( previousPt , currentPt );
        if(distBetPts < K.minSqrdDistanceToDistinguishPts) {
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

        if(this.hasHoles()) {
            var holes = this.getChildByName("holes");
            for(var k=0; k < holes.children.length; k += 1) {
                var holeChildCurve = holes.children[k];
                holeChildCurve.setPointVisibility(b);
            }
        }
        this.bPointsVisible = b;
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
        if(this.hasHoles()) {
            var holes = this.getChildByName("holes");
            for(var k=0; k < holes.children.length; k += 1) {
                var holeChildCurve = holes.children[k];
                holeChildCurve.redrawAllPoints(color);
            }
        }
    }

    c.redrawSomePoints = function(color, deltaPos, range) {
        if(this.hasHoles()) {
            console.log("curve has holes");
            return;
        }
        deltaPos = deltaPos || {x:0, y:0}
        var pointsContainer =  this.getChildByName("points");
        for(var k=0; k < pointsContainer.children.length; k += 1) {
            if(range && range.indexOf(k) < 0) {
                continue;
            }
            var ptShape = pointsContainer.children[k];
            ptShape.x += deltaPos.x;
            ptShape.y += deltaPos.y;
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

    },

    c.moveCurve = function( deltaPos ) {
        if(this.hasHoles()) {
            console.log("curve has holes");
            return;
        }

        var curveShape = this.getChildByName("curve");
        var pointsContainer =  this.getChildByName("points")

        for(var k=0; k < pointsContainer.children.length; k += 1) {
            var currentPoint = pointsContainer.children[k];
            currentPoint.x += deltaPos.x;
            currentPoint.y += deltaPos.y;
        }
        this.redrawAllPoints("blue");
        this.redrawCurveShapeGraphics(false);
    }

    c.addChildCurve = function(childCurve) {
        if(childCurve.hasHoles()) {
            console.log("child curve has holes");
            return;
        }
        var chPointsContainer =  childCurve.getChildByName("points");
        for(var k=0; k < chPointsContainer.children.length; k += 1) {
            var currentPt = chPointsContainer.children[k];
            var newPos = childCurve.absolutePos(currentPt);
            newPos = this.relativePos(newPos);
            currentPt.x = newPos.x;
            currentPt.y = newPos.y;

        }
        var holes = this.getChildByName("holes");
        if(!holes) {
            holes = new createjs.Container();
            holes.name = "holes";
            this.addChild(holes);
        }
        childCurve.parent.removeChild(childCurve);
        childCurve.x =0;
        childCurve.y =0;
        holes.addChild(childCurve);
    }

    c.hasHoles = function() {
        var holes = this.getChildByName("holes");
        return holes != undefined && holes != null;

    }
    c.isNormalized = function() {
        var centroidRel = this.centroidRelative();
        var isCenterInside = this.inside(centroidRel);
        return isCenterInside;
    }


    c.redrawCurveShapeGraphics = function(bFill, alphaForBFill, coloForBFill) {
        coloForBFill = coloForBFill || this.drawColor;
        alphaForBFill = alphaForBFill || 0.5;
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

                    curveShape.graphics.beginFill(coloForBFill);
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




                    if(this.hasHoles()) {
                        var holes = this.getChildByName("holes");
                        for(var k=0; k < holes.children.length; k += 1) {
                            var holeChildCurve = holes.children[k];
                            var curveShapeHole =  holeChildCurve.getChildByName("curve");
                            var pointsContainerHole =  holeChildCurve.getChildByName("points");
                            previousPt = pointsContainerHole.children.slice(-1)[0];
                            var nChildPoints = pointsContainerHole.children.length;
                            curveShape.graphics.moveTo(previousPt.x, previousPt.y);
                            for(var j=nChildPoints-1; j >= 0; j -= 1) {
                                var currentPt = pointsContainerHole.children[j];
                                var midPt = {x: (previousPt.x + currentPt.x)*0.5, y: (previousPt.y + currentPt.y)*0.5};
                                curveShape.graphics.curveTo(midPt.x, midPt.y, currentPt.x, currentPt.y );
                                previousPt = currentPt;
                            }
                        }
                    }


                    curveShape.graphics.endFill();
                    curveShape.alpha = alphaForBFill;
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
                    if(this.hasHoles()) {
                        var holes = this.getChildByName("holes");
                        for(var k=0; k < holes.children.length; k += 1) {
                            var holeChildCurve = holes.children[k];
                            holeChildCurve.redrawCurveShapeGraphics(false);

                        }
                    }
                }

            }
        }
    }

    c.findIntersectingPointsWithAnotherCurve = function(other) {
        //assume other is normaizedCurveDirection

        var pointsContainer =  this.getChildByName("points");
        var intersectingPointsWithAnotherCurve = [];
        for(var k=0; k < pointsContainer.children.length; k += 1) {
            var currentPoint = pointsContainer.children[k];
            currentPoint = this.absolutePos(currentPoint);
            currentPoint = other.relativePos(currentPoint);
            if(other.inside(currentPoint)) {
                intersectingPointsWithAnotherCurve.push(k)
            }
        }
        return intersectingPointsWithAnotherCurve;
    }


    K.Curve = createjs.promote(Curve, "Container");



    K.Mode = Class.extend( {
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
