
(
  function() {
    var k = KgeorgeNamespace("K")
    var kUtils = KgeorgeNamespace("K.Utils")

    k.g_pickedPoints = {}


    k.Frame = Class.extend( {
        init: function(canvasId, name, imgUrl) {
            this.eStates = ["idle", "draw","finishDraw"];
            this.currentState = "idle";
            this.name = name;
            this.imgUrl = imgUrl;
            this.nextShapeIdx=0;
            this.pointsCollected=[]
            this.bMouseDown = false;
            this.previousPt = {x:0, y:0}
            this.currentPt = {x:0, y:0}

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

            this.handleMouseMove = function (evt) {
                var relPos = {x: evt.stageX, y: evt.stageY};
			    this.stage.addEventListener("pressup", this.handleMouseUp);
                this.currentPt = relPos;
			    this.stage.update();
            }.bind(this)


            this.handleMouseUp = function (evt) {
                var relPos = {x: evt.stageX, y: evt.stageY};
                //console.log("UPUPUPUPUPUPP   stage mnouse Up", relPos.x, relPos.y);
			    this.stage.addEventListener("stagemousedown", this.handleMouseDown);
			    this.stage.removeEventListener("pressup", this.handleMouseUp);
                this.stage.removeEventListener("stagemousemove" , this.handleMouseMove);
                this.currentState = "idle";
			    this.stage.update();

            }.bind(this)

            this.collectPoint = function (pt) {
                this.pointsCollected.push(pt);
            }


            this.handleMouseDoubleClick = function(evt) {
                var relPos = {x: evt.stageX, y: evt.stageY};
                //console.log("DCDCDCDCDCDCDC  stage mnouse down", relPos.x, relPos.y);
                this.stage.removeEventListener("stagemousemove" , this.handleMouseMove);
                this.currentState = "finishDraw";
            }.bind(this);

            this.handleMouseDown =  function(evt) {
                var relPos = {x: evt.stageX, y: evt.stageY};
                var lineDrawingShape = new createjs.Shape().set({x:0,y:0});
                lineDrawingShape.graphics.beginFill("red");
                this.lineDrawingShape = lineDrawingShape;
                this.stage.addChild(lineDrawingShape);
                var drawingShape = new createjs.Shape().set({x:relPos.x, y: relPos.y});
                drawingShape.snapToPixelEnabled = true;
		        this.stage.addChild(drawingShape);
                drawingShape.graphics.beginFill("blue");
                drawingShape.graphics.dc(0,0, 2);
                this.stage.addEventListener("stagemousemove" , this.handleMouseMove);
                this.currentState = "draw";
                this.previousPt = relPos;
                this.currentPt = relPos;
                this.collectPoint(relPos);
                console.log("DDDDDDDDDDDDDDD  stage mnouse down", relPos.x, relPos.y);
			    this.stage.update();
            }.bind(this);


            this.stage.addEventListener("dblclick" , this.handleMouseDoubleClick);
            this.stage.addEventListener("stagemousedown", this.handleMouseDown );
            createjs.Ticker.addEventListener("tick", this.tick.bind(this));
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

        imgOnload: function() {
            var bitmap = new createjs.Bitmap(this.img);
            bitmap.x=0;
            bitmap.y=0;
            console.log("bitmap bounds" , name, bitmap.getBounds())
            this.stage.addChild(bitmap);
            console.log("stage bounds" , name, this.stage.getBounds())


            //this.stage.addChild(this.lineDrawingShape);
            this.stage.update();
        },


        tick: function () {
            var drawColor = createjs.Graphics.getHSL(Math.random()*360, 100, 50);
            if(this.currentState == "draw") {
                var midPt = {x: (this.previousPt.x + this.currentPt.x)*0.5, y: (this.previousPt.y + this.currentPt.y)*0.5}
                this.lineDrawingShape.graphics.setStrokeStyle(2);
                this.lineDrawingShape.graphics.beginStroke(drawColor);
			    this.lineDrawingShape.graphics.moveTo(this.previousPt.x, this.previousPt.y);
			    this.lineDrawingShape.graphics.curveTo(midPt.x, midPt.y, this.currentPt.x, this.currentPt.y );
			    this.lineDrawingShape.graphics.endStroke();
                this.previousPt = this.currentPt;


                this.stage.update();
            } else if (this.currentState == "finishDraw") {
                this.previousPt = this.currentPt;
                if(this.pointsCollected.length > 2) {
                    this.currentPt = this.pointsCollected[this.pointsCollected.length -3];
                    //console.log( "FFFFFFFFFFFFF", this.currentPt, this.previousPt);
                }
                var midPt = {x: (this.previousPt.x + this.currentPt.x)*0.5, y: (this.previousPt.y + this.currentPt.y)*0.5}
                this.lineDrawingShape.graphics.setStrokeStyle(2);
                this.lineDrawingShape.graphics.beginStroke(drawColor);
			    this.lineDrawingShape.graphics.moveTo(this.previousPt.x, this.previousPt.y);
			    this.lineDrawingShape.graphics.curveTo(midPt.x, midPt.y, this.currentPt.x, this.currentPt.y );
			    this.lineDrawingShape.graphics.endStroke();
                this.currentState = "idle";

            }
            this.stage.tick();
        }
    });

    k.init = function () {
        var baseDataDir = "http://localhost:8000/demo/image"
        k.frame = new k.Frame("demoCanvasFrame", "frame",  baseDataDir + "/frm/frm3.jpg" );
    }

  }
  ());
