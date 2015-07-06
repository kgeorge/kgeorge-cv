
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



    K.SaveMode = K.Mode.extend( {
        init: function(main){
            this._super("save", main);
            this.resetSelection();
        },
        resetSelection: function() {
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
            this.currentMode.currentState = "draw";
            this.authoringArea.addEventListener("stagemousemove" , this.boundHandleMouseMove);
            this.stage1.update();
        },
        //-----------------------------------------------------------------------
        handleMouseDoubleClick : function(evt) {
            var relPos = {x: evt.stageX, y: evt.stageY};
            //console.log("DCDCDCDCDCDCDC  stage mnouse down", relPos.x, relPos.y);
            this.authoringArea.removeEventListener("stagemousemove" , this.boundHandleMouseMove);
            var frameCanvasElement = document.getElementById(this.canvasId);
            var jsonImg = JSON.stringify(frameCanvasElement.toDataURL());
            //var idata =  frameCanvasElement.toDataURL('image/png');
            //var idata =  idata.replace('data:image/png;base64,', '');
            //console.log(idata.substring(idata.length-30));
            //var idata =  frameCanvasElement.toDataURL('image/png')
            //console.log(idata);
            //$.post('/save', params, function (data) { /* ... */ })
            /*$.ajax ({
                type: 'POST',
                url: '/save',
                data: { "imagedata" : jsonImg },
                contentType: 'application/json; charset=utf-8',
                dataType: 'json',
                success: function (msg) {
                    alert("Done, Picture Uploaded.");
                }
            });*/
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
        cleanBeforeSwitch: function() {
            var test = this.main.stage1.getChildByName("test");
            this.main.stage1.removeChild(test);
            this.currentState = "idle";
        },

        test: function() {
            if(this.main.collectedCurves.length <= 0) {

            }
            var currentCurve = this.main.collectedCurves[0];
            var cRel = currentCurve.centroidRelative();
            var cAbs = currentCurve.absolutePos(cRel);

            var testContainer = this.main.stage1.getChildByName("test");
            if(!testContainer) {
                testContainer = new createjs.Container();
                testContainer.name = "test";
                this.main.stage1.addChild(testContainer);
            }
            var centroidShape = new createjs.Shape();
            centroidShape.x = cAbs.x;
            centroidShape.y = cAbs.y;
            centroidShape.graphics.clear();
            centroidShape.graphics.beginFill("red");
            centroidShape.graphics.dc(0,0, 4);
            centroidShape.graphics.endFill();
            testContainer.addChild(centroidShape);
            this.main.stage1.update();

            currentCurve.inside(cRel);

            console.log("~~~~~~~~~~~~~ test",  currentCurve.inside(cRel));
            currentCurve.normalizeCurveDirection();
            console.log("~~~~~~~~~~~~~ test",  currentCurve.inside(cRel));


        },




        tick: function() {

            if(this.currentMode.currentState == "draw") {
                //this.currentMode.test();
                this.currentMode.currentState = "idle"
            } else if(this.currentMode.currentState == "finishDraw") {
                this.save();
                this.currentMode.currentState = "idle"
            } else {
                this.updateImageVisibility(true);

            }
        }
    });


  }
  ());
