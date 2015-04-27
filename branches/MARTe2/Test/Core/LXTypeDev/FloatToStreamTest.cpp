/* Copyright 2015 F4E | European Joint Undertaking for
 * ITER and the Development of Fusion Energy ('Fusion for Energy')
 *
 * Licensed under the EUPL, Version 1.1 or - as soon they
 will be approved by the European Commission - subsequent
 versions of the EUPL (the "Licence");
 * You may not use this work except in compliance with the
 Licence.
 * You may obtain a copy of the Licence at:
 *
 * http: //ec.europa.eu/idabc/eupl
 *
 * Unless required by applicable law or agreed to in
 writing, software distributed under the Licence is
 distributed on an "AS IS" basis,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 express or implied.
 * See the Licence
 permissions and limitations under the Licence.
 *
 * $Id:$
 *
 **/

#include "GeneralDefinitions.h"
#include "FloatToStreamTest.h"
#include "StringTestHelper.h"
#include "StreamTestHelper.h"
#include "stdio.h"



bool FloatToStreamTest::TestFixedPoint(){
	MyStream thisStream;
	float sbit32=-1.1234567;
	FormatDescriptor format;
	const char* pformat;
	
	pformat="- 20.8f";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|-1.1234567|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();	 

	
	pformat="- 11.4f";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|-1.123|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();


	sbit32=112345.67;//112345,...
 	pformat=" 10.8f";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|112345.67|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();

	double sbit64=12345.9999;
	
	pformat=" 10.8f";


	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit64, format);
	
	printf("\n|12346|%s|, %f\n",thisStream.Buffer(), sbit64);
	
	thisStream.Clear();
		
	sbit64=999999.55556;

	pformat=" 20.15f"; //4 zeri

	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit64, format);
	
	printf("\n|999999.555560000|%s|, %f\n",thisStream.Buffer(), sbit64);
	
	thisStream.Clear();

	pformat=" 20.6f"; //4 zeri

	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit64, format);
	
	printf("\n|6|%s|, %f\n",thisStream.Buffer(), sbit64);
	
	thisStream.Clear();


	pformat=" 20.1f";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit64, format);
	
	printf("\n|1|%s|, %f\n",thisStream.Buffer(), sbit64);
	
	thisStream.Clear();


	return True;

}	

bool FloatToStreamTest::TestExponential(){
	MyStream thisStream;
	float sbit32=-11.234567;
	FormatDescriptor format;
	const char* pformat;
	
	pformat="- 20.8e";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|-1.1234567E+1|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();	 

	
	pformat="- 11.4e";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|-1.123E+1|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();


	sbit32=112345.67;//112345,...
 	pformat=" 10.8e";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|1.1234567E+5|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();

	double sbit64=12345.9999;
	
	pformat=" 10.8e";


	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit64, format);
	
	printf("\n|1.2346E+4|%s|, %f\n",thisStream.Buffer(), sbit64);
	
	thisStream.Clear();
		
 	__float128 sbit128=999999.55556;

	pformat=" 20.15e"; //4 zeri

	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit128, format);
	
	printf("\n|9.99999555560000E+5|%s|, %f\n",thisStream.Buffer(), sbit128);
	
	thisStream.Clear();

	pformat=" 20.6e"; //4 zeri

	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit128, format);
	
	printf("\n|6|%s|, %f\n",thisStream.Buffer(), sbit128);
	
	thisStream.Clear();


	pformat=" 20.1e";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit128, format);
	
	printf("\n|1|%s|, %f\n",thisStream.Buffer(), sbit128);
	
	thisStream.Clear();


	return True;

}



bool FloatToStreamTest::TestEngeneering(){
	MyStream thisStream;
	float sbit32=-11.234567;
	FormatDescriptor format;
	const char* pformat;
	
	pformat="- 20.8E";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|-11.234567|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();	 

	
	sbit32*=100;	
	pformat="- 11.4E";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|-1.123E+3|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();


	sbit32=112345.67;//112345,...
 	pformat=" 10.8E";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit32, format);
	
	printf("\n|112.34567E+3|%s|, %f\n",thisStream.Buffer(), sbit32);
	
	thisStream.Clear();

	double sbit64=12345.9999;
	
	pformat=" 10.8E";


	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit64, format);
	
	printf("\n|12.346E+3|%s|, %f\n",thisStream.Buffer(), sbit64);
	
	thisStream.Clear();
		
 	__float128 sbit128=999999.55556;

	pformat=" 20.15E"; //4 zeri

	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit128, format);
	
	printf("\n|999.999555560000E+3|%s|, %f\n",thisStream.Buffer(), sbit128);
	
	thisStream.Clear();

	pformat=" 20.6E"; //4 zeri

	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit128, format);
	
	printf("\n|6|%s|, %f\n",thisStream.Buffer(), sbit128);
	
	thisStream.Clear();


	pformat=" 20.1E";
	format.InitialiseFromString(pformat);
	FloatToStream(thisStream, sbit128, format);
	
	printf("\n|1|%s|, %f\n",thisStream.Buffer(), sbit128);
	
	thisStream.Clear();


	return True;

}
