//
// Copyright 2011 EFDA | European Fusion Development Agreement
//
// Licensed under the EUPL, Version 1.1 or - as soon they 
// will be approved by the European Commission - subsequent  
// versions of the EUPL (the "Licence"); 
// You may not use this work except in compliance with the 
// Licence. 
// You may obtain a copy of the Licence at: 
//  
// http://ec.europa.eu/idabc/eupl
//
// Unless required by applicable law or agreed to in 
// writing, software distributed under the Licence is 
// distributed on an "AS IS" basis, 
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
// express or implied. 
// See the Licence for the specific language governing 
// permissions and limitations under the Licence. 
//
// $Id$
//
//
function putRGraphLabel(canvasId) {
    var canv  = document.getElementById(canvasId);
    var context = canv.getContext("2d");
    context.textBaseline = "top";
    context.textAlign = "left";
    context.font = "normal 4mm sans-serif";
    context.fillText('Powered by RGraph', 0, 0);
}

function putTitle(canvasId, title, color) {
    var canvas  = document.getElementById(canvasId);
    var context = canvas.getContext("2d");
    context.textBaseline = "top";
    context.textAlign = "left";
    context.font = "normal 4mm sans-serif";
    
    var acumul = 0;
    for(var i = 0 ; i < title.length ; i++) {
	context.fillStyle = color[i];
	context.fillText(title[i], acumul, 20);
	acumul += 8*title[i].length;
	acumul += 4;
    }
}


function getMaxOfArray(numArray) {
	    return Math.max.apply(null, numArray);
	}




function getMinOfArray(numArray) {
    return Math.min.apply(null, numArray);
}



function ArrayToString(dataMatrix){
	var dataString = new Array(dataMatrix.length);
	for(var i = 0 ; i < dataMatrix.length ; i++) {
		dataString[i] = (dataMatrix[i].toExponential(3)).toString(10);
	}
	return dataString;
}




function plot(canvasId, dataMatrix, xLimits, title, xlabel, ylabel, bottomXAxisPos) {
   RGraph.Clear(document.getElementById(canvasId));

var colorSequence = ['blue','red','black','green','aqua','purple','olive','fuchsia','teal','maroon','navy','lime','gray','yellow','silver'];


    var yMaxArr = new Array(dataMatrix.length);
    var yMinArr = new Array(dataMatrix.length);

for(var i = 0 ; i < dataMatrix.length ; i++) {
	yMaxArr[i]=getMaxOfArray(dataMatrix[i]);
	yMinArr[i]=getMinOfArray(dataMatrix[i]);
 }

var yMax=getMaxOfArray(yMaxArr);
var yMin=getMinOfArray(yMinArr);

//If you want to use zoom, add it to the arguments.
//yMax*=(1+zoom);
//yMin*=(1-zoom);


var NUM_OF_XTICKS = 10;
var NUM_OF_YTICKS = 10;
    if(xLimits.length == 2) {
	var xLabelArray = new Array(NUM_OF_XTICKS+1);
	var deltaX = (xLimits[1]-xLimits[0])/NUM_OF_XTICKS;
	for(var i = 0 ; i <= NUM_OF_XTICKS ; i++) {
	    xLabelArray[i] = xLimits[0] + i*deltaX;
	}
    } else {
	xLabelArray = [];
    }


var yLabelArray= new Array(NUM_OF_YTICKS);
var element=yMax;
var k=0;
while(k<NUM_OF_YTICKS){
	yLabelArray[k]=element;
	element-=(yMax-yMin)/(NUM_OF_YTICKS-1);
	k++;
}
var dataString=ArrayToString(yLabelArray);



  var line = new RGraph.Line({
                id: canvasId,
                data: dataMatrix,
                options: {
		    colors: colorSequence,
		    gutter: {
			left: 100,
                        bottom: 50
                    },
		    ylabels: {
                        specific: dataString
		    },
		    background: {
		    	grid: {
				autofit: {
					self: true,
					numhlines: 10,
					numvlines:20
				}
			    }
		    },
		    text: {
			size: 8,
	            },
		    title: {
			xaxis: xlabel,
			yaxis: ylabel,
		    },
		    xticks: NUM_OF_XTICKS,
		    yticks: NUM_OF_YTICKS,
                    ymax: yMax,
                    ymin: yMin,
		    tickmarks: 'circle',
		    labels: xLabelArray,
                }
            }).draw()

    putTitle(canvasId+"title", title, colorSequence);




}
