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

bool FloatToStreamTest::TestFixedPoint() {
    MyStream thisStream;
    float sbit32 = -1.1234567;
    FormatDescriptor format;
    const char* pformat;

    pformat = "- 12.8f";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("-1.1234567  ", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    pformat = "- 12.4f";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("-1.123      ", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    sbit32 = 112345.67; //112345,...
    pformat = " 12.8f";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("   112345.67", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    double sbit64 = 12345.9999;

    pformat = " 10.8f";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);

    if (!StringTestHelper::Compare(" 12346.000", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    sbit64 = 999999.55556;

    pformat = " 17.15f"; //4 zeri

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);

    if (!StringTestHelper::Compare(" 999999.555560000", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    pformat = "10.6f"; //4 zeri

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);

    if (!StringTestHelper::Compare("999999", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    pformat = "- 10.1f";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);

    if (!StringTestHelper::Compare("999999    ", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    pformat = " 5.10f";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);

    if (!StringTestHelper::Compare("    ?", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    float inf = 1.0 / 0.0;

    pformat = "10.10f";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, inf, format);

    if (!StringTestHelper::Compare("+Inf", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();
    double ninf = -1.0 / 0.0;

    pformat = " 10.10f";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, ninf, format);
    if (!StringTestHelper::Compare("      -Inf", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    pformat = "- 1.10f";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, ninf, format);

    if (!StringTestHelper::Compare("?", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    __float128 nan = 0.0 / 0.0;

    pformat = "- 10.10f";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, nan, format);

    if (!StringTestHelper::Compare("NaN       ", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    pformat = " 0.10f";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, nan, format);

    if (!StringTestHelper::Compare("NaN", thisStream.Buffer())) {
        return False;
    }

    thisStream.Clear();

    pformat = " 2.10f";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, nan, format);

    if (!StringTestHelper::Compare(" ?", thisStream.Buffer())) {
        return False;
    }

    return True;

}

bool FloatToStreamTest::TestExponential() {
    MyStream thisStream;
    float sbit32 = -11.234567;
    FormatDescriptor format;
    const char* pformat;

    pformat = "- 15.8e";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    //Left alignment padded	
    if (!StringTestHelper::Compare("-1.1234567E+1  ", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Left alignment, clip the number without rounding (the next is 4).
    pformat = "- 9.4e";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("-1.123E+1", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Right Alignment with round up (the next is 6)
    sbit32 = 112345.67; //112345,...
    pformat = " 10.8e";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("1.12346E+5", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Round up and zero added because the chosen precision.
    double sbit64 = -12345.9999;
    pformat = " 13.8e";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);

    if (!StringTestHelper::Compare("-1.2346000E+4", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //In case of round up overflow not correct the number.
    __float128 sbit128 = 999999.55556;
    pformat = ".6e"; //4 zeri

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("9.99999E+5", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Impossible to obtain ? with this print mode
    pformat = "- 2.2e";

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("? ", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    return True;

}

bool FloatToStreamTest::TestEngeneering() {
    MyStream thisStream;
    float sbit32 = -11.234567;
    FormatDescriptor format;
    const char* pformat;

    pformat = ".9E";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    //Precision lost (with float this happens frequently)!
    if (!StringTestHelper::Compare("-11.2345671", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Left Align without correction (the next is 4)	
    sbit32 *= 100; //1123.4567	
    pformat = "- 10.4E";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("-1.123E+3 ", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Right align padded with round up correction (the next is 5)
    sbit32 = 112345.67; //112345,...
    pformat = " 10.8E";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("112.346E+3", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Round up and zero added because the chosen precision.
    double sbit64 = 12345.9999;
    pformat = " 10.9E";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);

    if (!StringTestHelper::Compare("12.3460E+3", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Round up and right aligned padd.
    __float128 sbit128 = 999999.55556;

    pformat = " 20.10E"; //4 zeri

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("      999.9995556E+3",
                                   thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Automatic size definition
    sbit128 = -9.99999e9;

    pformat = " .3E"; //4 zeri

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("-9.99E+9", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Clip the precision because the size
    sbit128 *= 10; //1e10
    pformat = "6.4E";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("-99E+9", thisStream.Buffer())) {
        return False;
    }

    //? the size is less than the minimum required
    sbit128 *= 10; //-999E+9 min_size=7
    thisStream.Clear();
    pformat = "6.2E";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("?", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    return True;
}

bool FloatToStreamTest::TestCompact() {
    MyStream thisStream;
    float sbit32 = -11.234567;
    FormatDescriptor format;
    const char* pformat;

    pformat = ".9g";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    //Precision lost (with float this happens frequently)!
    //Print as fixed point
    if (!StringTestHelper::Compare("-11.2345671", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Print as fixed point	
    sbit32 *= 100;	//1123.4567	
    pformat = "- 10.4g";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);

    if (!StringTestHelper::Compare("-1123     ", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //print as compact
    sbit32 = 112345670;	//112345,...
    pformat = " 6.8g";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit32, format);
    if (!StringTestHelper::Compare("112.3M", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //print as compact and padded
    double sbit64 = 12345e-12;
    pformat = " 10.3g";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit64, format);
    if (!StringTestHelper::Compare("     12.3n", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Print as exponential
    __float128 sbit128 = 9.99e16;

    pformat = " 5.5g"; //4 zeri

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("9E+16", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //Big number, left padded
    sbit128 = -9.99999e9;

    pformat = "-7.3g"; //4 zeri

    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("-9.99G ", thisStream.Buffer())) {
        return False;
    }
    thisStream.Clear();

    //? the size is too short also for the exponential form.
    sbit128 = 9e4; //1e10
    pformat = "1.4g";
    format.InitialiseFromString(pformat);
    FloatToStream(thisStream, sbit128, format);

    if (!StringTestHelper::Compare("?", thisStream.Buffer())) {
        return False;
    }

    return True;
}
