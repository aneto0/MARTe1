/*
 * Copyright 2011 EFDA | European Fusion Development Agreement
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they 
   will be approved by the European Commission - subsequent  
   versions of the EUPL (the "Licence"); 
 * You may not use this work except in compliance with the 
   Licence. 
 * You may obtain a copy of the Licence at: 
 *  
 * http://ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in 
   writing, software distributed under the Licence is 
   distributed on an "AS IS" basis, 
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either 
   express or implied. 
 * See the Licence for the specific language governing 
   permissions and limitations under the Licence. 
 *
 * $Id$
 *
**/

/**
 * @file
 * @brief Basic support for SVG graphics
 */
#ifndef SVGGRAPH_H_
#define SVGGRAPH_H_

#include "StreamInterface.h"
#include "LinkedListHolder.h"
#include "LinkedListable.h"
#include "FString.h"

enum SVGGColor {
    SVGBlack,
    SVGBlue,
    SVGGreen,
    SVGRed,
    SVGYellow,
    SVGWhite
};

class SVGGElementInterface {
public:
    virtual void Draw(float XFactor, float YFactor, StreamInterface &out) = 0;

    void SetColor(SVGGColor color, StreamInterface &out){
        switch (color) {
            case SVGBlack:{
                out.Printf("black");
            }break;
            case SVGBlue:{
                out.Printf("blue");
            }break;
            case SVGGreen:{
                out.Printf("green");
            }break;
            case SVGRed:{
                out.Printf("red");
            }break;
            case SVGYellow:{
                out.Printf("yellow");
            }break;
            case SVGWhite:{
                out.Printf("white");
            }break;
        }
    }
};

enum SVGGPointType {
    SVGPointDot,
    SVGPointCross,
    SVGPointStar
};

class SVGGPoint : public SVGGElementInterface, public LinkedListable {
public:
    float                         x;
    float                         y;
    SVGGPointType                 pointType;
    int                           size;
    SVGGColor                     color;

public:
    virtual void Draw(float XFactor, float YFactor, StreamInterface &out) {
        out.Printf("<circle cx=\"%f\" cy=\"%f\" r=\"%d\" style=\"fill: ", x*XFactor, 50.0 - y*YFactor, size);
        SetColor(color, out);         
        out.Printf("\"/>\n");
    }
};





enum SVGGLineType {
    SVGLinePlain,
    SVGLineDotted,
    SVGLineDashed,
};

class SVGGLine : public SVGGElementInterface, public LinkedListable {
public:
    float                         x1;
    float                         y1;
    float                         x2;
    float                         y2;
    int                           lineWidth;
    SVGGLineType                  lineType;
    SVGGColor                     color;

public:
    virtual void Draw(float XFactor, float YFactor, StreamInterface &out) {
        out.Printf("<line x1=\"%f\" y1=\"%f\" x2=\"%f\" y2=\"%f\" stroke=\"", x1*XFactor, 50.0 - y1*YFactor, x2*XFactor, 50.0 - y2*YFactor);
        SetColor(color, out); 
        out.Printf("\" stroke-width=\"%d\" ", lineWidth);

        switch (lineType) {
            case SVGLinePlain: {} break;
            case SVGLineDotted: {
                out.Printf("stroke-dasharray=\"%d\"", lineWidth);
            } break;
            case SVGLineDashed: {
                out.Printf("stroke-dasharray=\"%d\"", 2*lineWidth);
            } break;
        }

        out.Printf("/>\n");
    }
};

class SVGGRectangle : public SVGGElementInterface, public LinkedListable {
public:
    float                         x;
    float                         y;
    float                         width;
    float                         height;
    int                           lineWidth;
    SVGGLineType                  lineType;
    SVGGColor                     lineColor;
    SVGGColor                     areaColor;

public:
    virtual void Draw(float XFactor, float YFactor, StreamInterface &out) {
        out.Printf("<rect x=\"%f\" y=\"%f\" width=\"%f\" height=\"%f\" ", x*XFactor, 50.0 - y*YFactor, width*XFactor, height*YFactor);
        out.Printf("style=\"fill:");
        SetColor(areaColor, out);
        out.Printf(";stroke:");        
        SetColor(lineColor, out);
        out.Printf(";stroke-width:%d;", lineWidth);

        switch (lineType) {
            case SVGLinePlain: {} break;
            case SVGLineDotted: {
                out.Printf(";stroke-dasharray=%d", lineWidth);
            } break;
            case SVGLineDashed: {
                out.Printf(";stroke-dasharray=%d", 2*lineWidth);
            } break;
        }

        out.Printf("\"/>\n");
    }
};


class SVGGraph : public LinkedListHolder {
private:
    float                               maxX;
    float                               maxY;
public:
    int                                 xSize;
    int                                 ySize;

    bool                                grid;
    float                               gridDensity;

    bool                                axes;


public:
    SVGGraph() {
        this->Reset();

        xSize               = 500;
        ySize               = 500;

        grid                = False;
        gridDensity         = 1.0;

        axes                = True;

        maxX                = -1e16;
        maxY                = -1e16;
    }

    ~SVGGraph() {
        LinkedListable* e = this->ListExtract();
        while ( e != NULL ) {
            delete e;
            e = this->ListExtract();
        }
    }

    void AddPoint(float xCoord, float yCoord, SVGGPointType type, SVGGColor color, int lineWidth) {
        SVGGPoint* p = new SVGGPoint;

        p->x          = xCoord;
        p->y          = yCoord;
        p->pointType  = type;
        p->size       = lineWidth;
        p->color      = color;

        if (fabs(xCoord) > maxX) maxX = fabs(xCoord);
        if (fabs(yCoord) > maxY) maxY = fabs(yCoord);

        this->ListAdd(p);
    }

    void AddLine(float x1, float y1, float x2, float y2, SVGGLineType lineType, SVGGColor color, int lineWidth) {
        SVGGLine* l = new SVGGLine;

        l->x1         = x1;
        l->y1         = y1;
        l->x2         = x2;
        l->y2         = y2;
        l->lineType   = lineType;
        l->lineWidth  = lineWidth;
        l->color      = color;

        if (fabs(x1) > maxX) maxX = fabs(x1);
        if (fabs(x2) > maxX) maxX = fabs(x2);
        if (fabs(y1) > maxY) maxY = fabs(y1);
        if (fabs(y2) > maxY) maxY = fabs(y2);

        this->ListAdd(l);
    }

    void AddRectangle(float x, float y, float width, float height, SVGGLineType lineType, SVGGColor areaColor, SVGGColor lineColor, int lineWidth) {
        SVGGRectangle* l = new SVGGRectangle;

        l->x         = x;
        l->y         = y;
        l->width     = width;
        l->height    = height;
        l->lineType  = lineType;
        l->lineWidth = lineWidth;
        l->lineColor = lineColor;
        l->areaColor = areaColor;

        if (fabs(x) > maxX) maxX = fabs(x);
        if (fabs(width) > maxX) maxX = fabs(width);
        if (fabs(y) > maxY) maxY = fabs(y);
        if (fabs(height) > maxY) maxY = fabs(height);

        this->ListAdd(l);
    }

    void Draw(StreamInterface &out) {

        float XFactor = 100.0 / maxX;
        float YFactor = 100.0 / maxY / 2;


        out.Printf("<?xml version=\"1.0\" encoding=\"iso-8859-1\" standalone=\"no\"?>\n");
        out.Printf("<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.0//EN\" \"http://www.w3.org/TR/SVG/DTD/svg10.dtd\">\n");
        out.Printf("<svg xmlns:xlink=\"http://www.w3.org/1999/xlink\" xmlns=\"http://www.w3.org/2000/svg\" width=\"%d\" height=\"%d\" viewBox=\"0 0 120 120\">\n", xSize, ySize);
        out.Printf("<g transform=\"translate(10,10)\">\n");

        if(grid) {
            float f = 0.0;
            for (f=0.0; f<=100.0; f+=gridDensity) {
                // X
                out.Printf("<line x1=\"%f\" y1=\"0\" x2=\"%f\" y2=\"100\" stroke=\"gray\" stroke-width=\"0.1\"/>\n", f, f);
                // Y
                out.Printf("<line x1=\"0\" y1=\"%f\" x2=\"100\" y2=\"%f\" stroke=\"gray\" stroke-width=\"0.1\"/>\n", f, f);
            }
        }

        if(axes) {
            out.Printf("<line x1=\"0\" y1=\"0\" x2=\"0\" y2=\"100\" stroke=\"black\" stroke-width=\"1\"/>\n");
            out.Printf("<line x1=\"0\" y1=\"50\" x2=\"100\" y2=\"50\" stroke=\"black\" stroke-width=\"1\"/>\n");

            // X arrow
            out.Printf("<path d=\"M 100 48.5 L 100 51.5 L 102 50 L 100 48.5\" fill=\"black\" stroke=\"black\"/>\n");
            // Y arrow
            out.Printf("<path d=\"M -1.5 0 L 1.5 0 L 0 -2 L -1.5 0\" fill=\"black\" stroke=\"black\"/>\n");

            // Labels
            out.Printf("<text x=\"2\" y=\"53\" text-anchor=\"start\" font-family=\"monospace\" font-size=\"2\">0</text>\n");
            out.Printf("<text x=\"100\" y=\"53\" text-anchor=\"end\" font-family=\"monospace\" font-size=\"2\">%.1e</text>\n", maxX);

            out.Printf("<text x=\"2\" y=\"0\" text-anchor=\"start\" font-family=\"monospace\" font-size=\"2\">%.1e</text>\n", maxY);
            out.Printf("<text x=\"2\" y=\"100\" text-anchor=\"start\" font-family=\"monospace\" font-size=\"2\">%.1e</text>\n", -maxY);

            // Add labels to axes
            float f = 0.0;
            for (f=20.0; f<=80.0; f+=20.0) {
                out.Printf("<text x=\"%f\" y=\"53\" text-anchor=\"middle\" font-family=\"monospace\" font-size=\"2\">%.1e</text>\n", f, f/XFactor);
                out.Printf("<text x=\"2\" y=\"%f\" text-anchor=\"start\" font-family=\"monospace\" font-size=\"2\">%.1e</text>\n", f, (50.0 - f)/YFactor);
            }


        }


        int i;
        for (i=0; i<this->ListSize(); i++) {
            SVGGElementInterface* e = dynamic_cast<SVGGElementInterface*>(this->ListPeek(i));
            e->Draw(XFactor, YFactor, out);
        }
        out.Printf("</g>\n");
        out.Printf("</svg>\n");
    }

};

#endif /* SVGGRAPH_H_ */
