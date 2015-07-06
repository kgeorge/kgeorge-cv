
(
  function() {

    var K = KgeorgeNamespace("K")

    Button = function(label, color) {
        this.Container_constructor();
        this.label = label;
        this.color = color;

        this.setup();
    }
    var p = createjs.extend(Button, createjs.Container);
    p.setup = function() {
        var text = new createjs.Text(this.label, "20px Arial", "#000");
        text.textBaseline = "top";
	    text.textAlign = "center";

	    var width = text.getMeasuredWidth()+30;
	    var height = text.getMeasuredHeight()+20;
	    text.x = width/2;
	    text.y = 2;
	    var uncheckedColor = "gray";
	    var checkedColor = "red";
	    var background = new createjs.Shape();
	    background.graphics.beginFill(uncheckedColor).drawRoundRect(0,0,width,height,10);
	    this.addChild(background, text);
	    this.on("click", this.handleClick);
	    this.on("rollover", this.handleRollOver);
	    this.on("rollout", this.handleRollOver);
	    this.cursor = "pointer";
        this.bChecked = false;
	    this.mouseChildren = false;

        this.getWidth = function() {
            return width;
        }
        this.setCheckedState = function(b) {
            if(b != this.bChecked) {
                var color = uncheckedColor;
                if(b) {
                    color = checkedColor;
                }
                background.graphics.clear();
	            background.graphics.beginFill(color).drawRoundRect(0,0,width,height,10);
	            this.bChecked = b;
            }
        }
	    this.offset = Math.random()*10;
	    this.count = 0;
    }

    p.handleClick = function (event) {
        //alert("You clicked on a button: "+this.label);
    } ;

    p.handleRollOver = function(event) {
        //this.alpha = event.type == "rollover" ? 0.4 : 1;
    };


    K.Button = createjs.promote(Button, "Container");




    RadioButtonGroup = function(labelsArray) {
        this.Container_constructor();
        this.labelsArray = labelsArray;
        this.buttons=[]

        this.setup();
    }
    var q = createjs.extend(RadioButtonGroup, createjs.Container);

    q.setup = function() {
        var nextXPos = 0;
        var nextYPos = 0;
        for(var i=0; i < this.labelsArray.length; ++i) {
            var newB = new K.Button(this.labelsArray[i], "gray");
            newB.bChecked = false;
            newB.set({x: nextXPos, y: nextYPos });
            nextXPos += newB.getWidth() + 5;
            this.addChild(newB);
            newB.addEventListener("click",function(evt){
                console.log("button click, ", this.label);
                evt.bubbles = true;
                for(var k =0; k < this.parent.children.length; k += 1) {
                    if(this != this.parent.children[k]) {
                        this.parent.children[k].setCheckedState(false);
                    }
                }
                this.setCheckedState(true);
            }.bind(newB));
         }
    }

    q.getButton= function(labelName) {
        for(var k=0; k < this.labelsArray.length; k += 1) {
            if(labelName == this.labelsArray[k]) {
                return this.children[k];
            }
        }
    }


    K.RadioButtonGroup = createjs.promote(RadioButtonGroup, "Container");


  }
  ());
