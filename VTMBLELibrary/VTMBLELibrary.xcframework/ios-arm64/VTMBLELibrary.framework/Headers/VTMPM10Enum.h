//
//  VTMPM10Enum.h
//  VTMBLELibrary
//
//  Created by yangweichao on 2025/11/7.
//

#ifndef VTMPM10Enum_h
#define VTMPM10Enum_h

typedef enum : NSUInteger {
    VTMPM10LanguageZH,
    VTMPM10LanguageEN,
    VTMPM10LanguageIT,
    VTMPM10LanguageRU,
    VTMPM10LanguageFR,
    VTMPM10LanguageBG,
    VTMPM10LanguageKK,
    VTMPM10LanguagePL,
    VTMPM10LanguageUK,
    VTMPM10LanguageES,
    VTMPM10LanguageSK,
    VTMPM10LanguagePT,
    VTMPM10LanguageTR,
    VTMPM10LanguageDE,
    VTMPM10LanguageJP,
    VTMPM10LanguageHI,
    VTMPM10LanguageAR,
    VTMPM10LanguageNL,
} VTMPM10Language;

typedef enum : NSUInteger {
    VTMPM10ReqCaseInfoNext = 1,
    VTMPM10ReqCaseInfoResent,
    VTMPM10ReqCaseInfoAll,
    VTMPM10ReqCaseInfoUnupload,
    VTMPM10ReqCaseInfoUploaded,
    VTMPM10ReqCaseInfoTarget,
} VTMPM10ReqCaseInfo;

typedef enum : NSUInteger {
    VTMPM10ReqCaseDataZero,
    VTMPM10ReqCaseDataNext,
    VTMPM10ReqCaseDataResent,
    VTMPM10ReqCaseDataSave = 126,
    VTMPM10ReqCaseDataDelete = 127,
} VTMPM10ReqCaseData;

#endif /* VTMPM10Enum_h */
