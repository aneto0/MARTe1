#if !defined (BASIC_TYPE_ENUMERATED)
#define BASIC_TYPE_ENUMERATED


/** the types of data in the field type of BasicTypeDescriptor*/
enum BasicTypeEnumerated{

    /** An integer   pointer = intxx * */
    BTDSignedInteger         = 0,

    /** An integer   pointer = uintxx * */
    BTDUnsignedInteger       = 1,

    /** standard float   pointer = float32 or float64 * */
    BTDFloat                 = 2,

/** pointer to a zero terminated string   pointer = const char * */
    BTDCCString              = 3,

/** pointer to a pointer to a zero terminated string that has been allocated using malloc 
    pointer = char ** 
    BTConvert will free and re allocate memory 
*/
    BTDPCString              = 4,

/** pointer to an array of characters of size specified in size field 
    pointer = char[] 
    string will be 0 terminated and (size-1) terminated
    string is not raed only - can be written to up to size-1
*/
    BTDCArray                = 5,

/** BString class, 
    size field is meaningless
     pointer = BString *
    it is a pointer to a single BString
*/
    BTDBString               = 6,

/** FString class, size field is meaningless
    void * is FString *
    it is a pointer to a single FString
    the parts will be separated using the
    character specified in the format field  */

//MAYBE OBSOLETE 
    BTDFString               = 7,

/** BString class, size field is meaningless
    void * is BString *
    it is an array of BStrings matching the
    input size  */

//MAYBE OBSOLETE 
    BTDSTBStringArray        = 8,

/** FString class, size field is meaningless
    void * is FString *
    it is an array of FStrings matching the
    input size  */

//MAYBE OBSOLETE 
    BTDFStringArray          = 9,

/** StreamInterface class, size field is meaningless */
    BTDStream                = 10,

    /** Pointers */
    BTDPointer               = 11,

    /** A signed integer of size in bits   pointer = intxx * */
    BTDSignedBitSet          = 12,

    /** An unsigned integer of size  pointer = uintxx * */
    BTDUnSignedBitSet        = 13,

    /** Not a BTDType */
    BTDNone                  = 15

};



#endif
