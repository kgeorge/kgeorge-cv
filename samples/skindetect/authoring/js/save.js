
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



    k.SaveMode = k.Mode.extend( {
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


        tick: function() {
            if(this.currentMode.currentState == "finishDraw") {

                this.updateImageVisibility(false);
                this.stage1.update();
                var frameCanvasElement = document.getElementById(this.canvasId);
                //var data = frameCanvasElement.toDataURL("image/png").replace("image/png", "image/octet-stream");
                //window.location.href = data;
                var srcImgFilename = this.srcImgFilename;
                var canvasCopy = document.createElement("canvas");
                var copyContext = canvasCopy.getContext("2d");
                canvasCopy.width = frameCanvasElement.width/this.srcImgScale;
                canvasCopy.height = frameCanvasElement.height/this.srcImgScale;
                if(this.srcImgScale > 0) {
                    copyContext.drawImage(frameCanvasElement, 0, 0, frameCanvasElement.width, frameCanvasElement.height, 0, 0, canvasCopy.width, canvasCopy.height);
                }

                canvasCopy.toBlob(function(blob) {
                    saveAs(blob, "skindetect-" + srcImgFilename );
                });
                //Canvas2Image.saveAsPNG(frameCanvasElement);
                this.currentMode.currentState = "idle"
            } else {
                this.updateImageVisibility(true);

            }
        }
    });


  }
  ());
