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

function plot(canvasId, dataMatrix, xLimits, title, xlabel, ylabel, bottomXAxisPos) {
    RGraph.Clear(document.getElementById(canvasId));
    line = new RGraph.Line(canvasId, dataMatrix);

    var colorSequence = ['blue','red','black','green','aqua','purple','olive','fuchsia','teal','maroon','navy','lime','gray','yellow','silver'];
    line.Set('chart.colors', colorSequence);

    if(bottomXAxisPos) {
    	line.Set('chart.xaxispos', 'bottom');
    } else {
     	line.Set('chart.xaxispos', 'center');
    }
    
    // Build string for markers
    var markerString = new Array(dataMatrix.length);
    for(var i = 0 ; i < dataMatrix.length ; i++) {
	markerString[i] = 'circle';
    }

    var NUM_OF_XTICKS = 10;
    if(xLimits.length == 2) {
	var xLabelArray = new Array(NUM_OF_XTICKS+1);
	var deltaX = (xLimits[1]-xLimits[0])/NUM_OF_XTICKS;
	for(var i = 0 ; i <= NUM_OF_XTICKS ; i++) {
	    xLabelArray[i] = xLimits[0] + i*deltaX;
	}
    } else {
	xLabelArray = [];
    }

    line.Set('chart.tickmarks', markerString);
    line.Set('chart.background.grid.autofit', true);
    line.Set('chart.background.grid.autofit.numhlines', 10);
    line.Set('chart.background.grid.autofit.numvlines', 20);
    line.Set('chart.xticks', NUM_OF_XTICKS);
    line.Set('chart.yticks', 10);
    if(xLabelArray.length != 0) {
    	line.Set('chart.labels', xLabelArray);
    }
    line.Set('chart.text.size', 8);
    line.Set('chart.title.xaxis', xlabel);
    line.Set('chart.title.yaxis', ylabel);
    line.Set('chart.gutter', 75);
    line.Set('chart.zoom.mode', 'area');

    line.Draw();
    putTitle(canvasId+"title", title, colorSequence);
}
