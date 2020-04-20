
#include "OpenGEX.h"

using namespace OGEX;


// TODO: could be a fun metaprogramming exercise to pull these from OpenDDL.h
const char *DescribeError( ODDL::DataResult errorCode )
{
    switch (errorCode) {

        case kDataSyntaxError:          return "kDataSyntaxError: The syntax is invalid."; break;
        case kDataIdentifierEmpty:      return "kDataIdentifierEmpty: No identifier was found where one was expected."; break;
        case kDataIdentifierIllegalChar:return "kDataIdentifierIllegalChar: An identifier contains an illegal character."; break;
        case kDataStringInvalid:        return "kDataStringInvalid: A string literal is invalid."; break;
        case kDataStringIllegalChar:    return "kDataStringIllegalChar: string literal contains an illegal character."; break;
        case kDataStringIllegalEscape:  return "kDataStringIllegalEscape: A string literal contains an illegal escape sequence."; break;
        case kDataStringEndOfFile:      return "kDataStringEndOfFile: The end of file was reached inside a string literal."; break;
        case kDataCharIllegalChar:      return "kDataCharIllegalChar: A character literal contains an illegal character."; break;
        case kDataCharIllegalEscape:    return "kDataCharIllegalEscape: A character literal contains an illegal escape sequence."; break;
        case kDataCharEndOfFile:        return "kDataCharEndOfFile: The end of file was reached inside a character literal."; break;
        case kDataBoolInvalid:          return "kDataBoolInvalid: A boolean value is not 'true' or 'false'."; break;
        case kDataTypeInvalid:          return "kDataTypeInvalid: A data type value does not name a primitive type."; break;
        case kDataIntegerOverflow:      return "kDataIntegerOverflow: An integer value lies outside the range of representable values for the number of bits in its underlying type."; break;
        case kDataFloatOverflow:        return "kDataFloatOverflow: A hexadecimal or binary literal used to represent a floating-point value contains more bits than the underlying type."; break;
        case kDataFloatInvalid:         return "kDataFloatInvalid: A floating-point literal has an invalid format."; break;
        case kDataReferenceInvalid:     return "kDataReferenceInvalid: A reference uses an invalid syntax."; break;
        case kDataStructUndefined:      return "kDataStructUndefined: An undefined structure type was encountered."; break;
        case kDataStructNameExists:     return "kDataStructNameExists: Structure name is equal to a previously used structure name."; break;
        case kDataPropertySyntaxError:  return "kDataPropertySyntaxError: A property list contains a syntax error."; break;
        case kDataPropertyUndefined:    return "kDataPropertyUndefined: An undefined property was encountered. This error is generated when the $@Structure::ValidateProperty@$ function returns false."; break;
        case kDataPropertyInvalidType:  return "kDataPropertyInvalidType: A property has specified an invalid type. This error is generated if the $@Structure::ValidateProperty@$ function does not specify a recognized data type."; break;
        case kDataPrimitiveSyntaxError: return "kDataPrimitiveSyntaxError: A primitive data structure contains a syntax error."; break;
        case kDataPrimitiveIllegalArraySize: return "kDataPrimitiveIllegalArraySize: A primitive data array size is too large."; break;
        case kDataPrimitiveInvalidFormat: return "kDataPrimitiveInvalidFormat: A primitive data structure contains data in an invalid format."; break;
        case kDataPrimitiveArrayUnderSize: return "kDataPrimitiveArrayUnderSize: A primitive array contains too few elements."; break;
        case kDataPrimitiveArrayOverSize:   return "kDataPrimitiveArrayOverSize: A primitive array contains too many elements."; break;
        case kDataInvalidStructure:     return "kDataInvalidStructure: A structure contains a substructure of an invalid type, or a structure of an invalid type appears at the top level of the file. This error is generated when either the $@Structure::ValidateSubstructure@$ function or $@DataDescription::ValidateTopLevelStructure@$ function returns false."; break;

        // ------- OpenGEX format errors 
        case kDataOpenGexInvalidUpDirection:        return "kDataOpenGexInvalidUpDirection"; break;
        case kDataOpenGexInvalidForwardDirection:   return "kDataOpenGexInvalidForwardDirection"; break;
        case kDataOpenGexInvalidTranslationKind:    return "kDataOpenGexInvalidTranslationKind "; break;
        case kDataOpenGexInvalidRotationKind:       return "kDataOpenGexInvalidRotationKind"; break;
        case kDataOpenGexInvalidScaleKind:          return "kDataOpenGexInvalidScaleKind"; break;
        case kDataOpenGexDuplicateLod:              return "kDataOpenGexDuplicateLod"; break;
        case kDataOpenGexMissingLodSkin:            return "kDataOpenGexMissingLodSkin"; break;
        case kDataOpenGexMissingLodMorph:           return "kDataOpenGexMissingLodMorph"; break;
        case kDataOpenGexDuplicateMorph:            return "kDataOpenGexDuplicateMorph"; break;
        case kDataOpenGexUndefinedLightType:        return "kDataOpenGexUndefinedLightType"; break;
        case kDataOpenGexUndefinedCurve:            return "kDataOpenGexUndefinedCurve"; break;
        case kDataOpenGexUndefinedAtten:            return "kDataOpenGexUndefinedAtten"; break;
        case kDataOpenGexDuplicateVertexArray:      return "kDataOpenGexDuplicateVertexArray"; break;
        case kDataOpenGexPositionArrayRequired:     return "kDataOpenGexPositionArrayRequired"; break;
        case kDataOpenGexVertexCountUnsupported:    return "kDataOpenGexVertexCountUnsupported"; break;
        case kDataOpenGexIndexValueUnsupported:     return "kDataOpenGexIndexValueUnsupported"; break;
        case kDataOpenGexIndexArrayRequired:        return "kDataOpenGexIndexArrayRequired"; break;
        case kDataOpenGexVertexCountMismatch:       return "kDataOpenGexVertexCountMismatch"; break;
        case kDataOpenGexBoneCountMismatch:         return "kDataOpenGexBoneCountMismatch"; break;
        case kDataOpenGexBoneWeightCountMismatch:   return "kDataOpenGexBoneWeightCountMismatch"; break;
        case kDataOpenGexInvalidBoneRef:            return "kDataOpenGexInvalidBoneRef"; break;
        case kDataOpenGexInvalidObjectRef:          return "kDataOpenGexInvalidObjectRef"; break;
        case kDataOpenGexInvalidMaterialRef:        return "kDataOpenGexInvalidMaterialRef"; break;
        case kDataOpenGexMaterialIndexUnsupported:  return "kDataOpenGexMaterialIndexUnsupported"; break;
        case kDataOpenGexDuplicateMaterialRef:      return "kDataOpenGexDuplicateMaterialRef"; break;
        case kDataOpenGexMissingMaterialRef:        return "kDataOpenGexMissingMaterialRef"; break;
        case kDataOpenGexTargetRefNotLocal:         return "kDataOpenGexTargetRefNotLocal"; break;
        case kDataOpenGexInvalidTargetStruct:       return "kDataOpenGexInvalidTargetStruct"; break;
        case kDataOpenGexInvalidKeyKind:            return "kDataOpenGexInvalidKeyKind"; break;
        case kDataOpenGexInvalidCurveType:          return "kDataOpenGexInvalidCurveType"; break;
        case kDataOpenGexKeyCountMismatch:          return "kDataOpenGexKeyCountMismatch"; break;
        case kDataOpenGexEmptyKeyStructure:         return "kDataOpenGexEmptyKeyStructure"; break;

        default: return "Unknown Error: An Unknown error was encountered."; break;
    }
}
