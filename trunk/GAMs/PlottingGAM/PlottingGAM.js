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
function putRGraphLabelLine(canvasId) {
    var canv  = document.getElementById(canvasId);
    var context = canv.getContext("2d");
    context.textBaseline = "top";
    context.textAlign = "left";
    context.font = "normal 4mm sans-serif";
    context.fillText('Powered by RGraph', 0, 0);
}

function putTitleLine(canvasId, title, color) {
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


function getMaxOfArrayLine(numArray) {
	    return Math.max.apply(null, numArray);
	}




function getMinOfArrayLine(numArray) {
    return Math.min.apply(null, numArray);
}



function ArrayToStringLineExp(dataMatrix, length){
	var dataString = new Array(dataMatrix.length);
	for(var i = 0 ; i < dataMatrix.length ; i++) {
		dataString[i] = (dataMatrix[i].toExponential(3)).toString(10);
	}
	return dataString;
}

function ArrayToStringLine(dataMatrix, length){
	var dataString = new Array(dataMatrix.length);
	for(var i = 0 ; i < dataMatrix.length ; i++) {
		dataMatrix[i] = Math.round(dataMatrix[i]*length)/length;
		dataString[i] = dataMatrix[i].toString();
	}
	return dataString;
}


function plotLine(canvasId, dataMatrix, xLimits, title, xlabel, ylabel, zoom) {
    //Clear the plot every time.
    RGraph.Clear(document.getElementById(canvasId));

    var colorSequence = ['black','blue','red','green','aqua','purple','olive','fuchsia','teal','maroon','navy','lime','gray','yellow','silver'];

    //Calculate yMax and yMin from the data array.
    var yMaxArr = new Array(dataMatrix.length);
    var yMinArr = new Array(dataMatrix.length);

    for(var i = 0 ; i < dataMatrix.length ; i++) {
        yMaxArr[i]=getMaxOfArrayLine(dataMatrix[i]);
        yMinArr[i]=getMinOfArrayLine(dataMatrix[i]);
    }

    var yMax=getMaxOfArrayLine(yMaxArr);
    var yMin=getMinOfArrayLine(yMinArr);

    //Adjust bounds with the zoom.
    yMax*=(1+zoom);
    yMin*=(1-zoom);

    //Define x and y ticks.
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

    //Convert the x-label to a string array rounding at 2 decimal numbers, otherwise use the scientific mode.
    if(xLimits[1] < 0.01){
        var xLabelArrayStr = ArrayToStringExpBar(xLabelArray,2);
    }
    else{
        var xLabelArrayStr = ArrayToStringBar(xLabelArray, 100);
    }
 
    //Calculate a string array in scientific mode for y label.
    var yLabelArray= new Array(NUM_OF_YTICKS);
    var element=yMax;
    var k=0;
    while(k<NUM_OF_YTICKS){
        yLabelArray[k]=element;
        element-=(yMax-yMin)/(NUM_OF_YTICKS-1);
        k++;
    }
    var dataString=ArrayToStringLineExp(yLabelArray);

    //var xAxisPosString = 'center';

   var dataNewMatrix=new Array(dataMatrix.length+1);
    


    for(var i=0; i<(dataMatrix.length); i++){
	dataNewMatrix[i+1]=dataMatrix[i];
    }

    if(yMin < 0){
	var dataXTicks=new Array(NUM_OF_XTICKS+1);
	for(var i=0; i<=NUM_OF_XTICKS; i++){
		dataXTicks[i]=0;
	}
	dataNewMatrix[0]=dataXTicks;
    }
    else{
	dataNewMatrix[0]=[];
    }   


//alert(yLabelArray);
    //Draw the line plot.
    var line = new RGraph.Line({
                id: canvasId,
                data: dataNewMatrix,
                options: {
                    colors: colorSequence,
                    gutter: {
                        left: 100,
                        bottom: 50
                    },
		   // xaxispos: xAxisPosString,
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
                    tickmarks: ['tick','circle'],
                    labels: xLabelArray,
                 
                    //spline: true
                }
            }).draw()

    putTitleLine(canvasId+"title", title, colorSequence);

}
