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
function putRGraphLabelBar(canvasId) {
    var canv  = document.getElementById(canvasId);
    var context = canv.getContext("2d");
    context.textBaseline = "top";
    context.textAlign = "left";
    context.font = "normal 4mm sans-serif";
    context.fillText('Powered by RGraph', 0, 0);
}

function putTitleBar(canvasId, title, color) {
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


function getMaxOfArrayBar(numArray) {
	    return Math.max.apply(null, numArray);
	}




function getMinOfArrayBar(numArray) {
    return Math.min.apply(null, numArray);
}



function ArrayToStringExpBar(dataMatrix, resolution){
	var dataString = new Array(dataMatrix.length);
	for(var i = 0 ; i < dataMatrix.length ; i++) {
		dataString[i] = (dataMatrix[i].toExponential(resolution)).toString(10);
	}
	return dataString;
}

function ArrayToStringBar(dataMatrix, length){
	var dataString = new Array(dataMatrix.length);
	for(var i = 0 ; i < dataMatrix.length ; i++) {
		dataMatrix[i] = Math.round(dataMatrix[i]*length)/length;
		dataString[i] = dataMatrix[i].toString();
	}
	return dataString;
}


function plotBar(canvasId, dataMatrix, xLimits, title, xlabel, ylabel, bottomXAxisPos, zoom) {

    //Clear the plot every time.
    RGraph.Clear(document.getElementById(canvasId));
    var colorSequence = ['blue','red','black','green','aqua','purple','olive','fuchsia','teal','maroon','navy','lime','gray','yellow','silver'];
    
    //Define x and y ticks.
    var NUM_OF_XTICKS = 10;
    var NUM_OF_YTICKS = 10;

    //Calculate values for each tick
    if(xLimits.length == 2) {
	var xLabelArray = new Array(dataMatrix.length*(NUM_OF_XTICKS+1));
	var deltaX = (xLimits[1]-xLimits[0])/NUM_OF_XTICKS;
	var cnt=0;
	for(var j =0; j < dataMatrix.length; j++){
	    for(var i = 0 ; i <= NUM_OF_XTICKS ; i++) {
	        xLabelArray[cnt] = xLimits[0] + i*deltaX;
		    cnt++;
	    }
	}
    } else {
	xLabelArray = [];
    }

    var xLabelArrayStr;
    //Convert the x-label to a string array rounding at 2 decimal numbers, otherwise use the scientific mode.
    if(xLimits[1] < 0.01){
	var xLabelArrayStr = ArrayToStringExpBar(xLabelArray,2);
    }
    else{
	var xLabelArrayStr = ArrayToStringBar(xLabelArray, 100);
    }


    //Calculate yMax and yMin from the data array.
    var yMaxArr = new Array(dataMatrix.length);
    var yMinArr = new Array(dataMatrix.length);

    for(var i = 0 ; i < dataMatrix.length ; i++) {
        yMaxArr[i]=getMaxOfArrayBar(dataMatrix[i]);
	yMinArr[i]=getMinOfArrayBar(dataMatrix[i]);
    }

    var yMax=getMaxOfArrayBar(yMaxArr);
    var yMin=getMinOfArrayBar(yMinArr);

    //Adjust bounds with the zoom.
    yMax*=(1+zoom);
    yMin*=(1-zoom);

    //Calculate a string array in scientific mode for y label.
    var yLabelArray= new Array(NUM_OF_YTICKS);
    var element=yMax;
    var k=0;
    while(k<NUM_OF_YTICKS){
	yLabelArray[k]=element;
	element-=(yMax-yMin)/(NUM_OF_YTICKS-1);
	k++;
    }
    var dataString=ArrayToStringExpBar(yLabelArray,3);

    //Draw the bar plot.
    var bar = new RGraph.Bar({
                id: canvasId,
                data: dataMatrix,
                options: {
		gutter: {
			left: 100,
                        bottom: 50
                    },
                    //labels: xLabelArray,
                    colors: {
			self: ['yellow'],
			//sequential: false
		    },
		    ylabels: {
                        specific: dataString
		    },
		    labels: xLabelArrayStr,
                    hmargin: 15,
                    //bevel: true,
 		    ymax: yMax,
                    ymin: yMin,
                    strokestyle: 'black'
                }                
            }).draw()
    putTitleLine(canvasId+"title", title, colorSequence);
}
