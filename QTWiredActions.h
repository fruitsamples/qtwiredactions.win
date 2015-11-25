//////////
//
//	File:		QTWiredActions.c
//
//	Contains:	QuickTime wired sprites target support for QuickTime movies.
//
//	Written by:	Tim Monroe
//
//	Copyright:	� 2001 by Apple Computer, Inc., all rights reserved.
//
//	Change History (most recent first):
//
//	   <1>	 	02/28/01	rtm		first file; based on existing AddHTActions sample code
//	   
//////////

//////////
//
// header files
//
//////////

#ifndef _SPRITEUTILITIES_
#include "SpriteUtilities.h"
#endif

#ifndef _WIREDSPRITEUTILITIES_
#include "WiredSpriteUtilities.h"
#endif

#ifndef __ENDIANUTILITIES__
#include "EndianUtilities.h"
#endif

#include "ComApplication.h"

#if TARGET_OS_MAC
#include "MacFramework.h"
#endif

#if TARGET_OS_WIN32
#include "WinFramework.h"
#endif


//////////
//
// compiler directives
//
//////////

#define USE_WIRED_UTILITIES					1


//////////
//
// constants
//
//////////

#define kIDOne								1
#define kIDTwo								2

#define kIndexZero							0
#define kIndexOne							1
#define kIndexTwo							2
#define kZeroDataLength						0

#define kWidth320							320
#define kWidth200							200
#define kHeight240							240
#define kHeight100							100
#define kHeight50							50

#define kTrackVolumeZero					0

#define kTimeScale600						600

#define kTrackStartTimeZero					0
#define kMediaStartTimeZero					0

#define kTextBoxInset						2

#define kSize48								48
#define kFacePlain							0

// the following should be in a public header file somewhere, but they're not....
#define kHyperTextTextAtomType				FOUR_CHAR_CODE('htxt')
#define kTextWiredObjectsAtomType			FOUR_CHAR_CODE('wtxt')
#define kHyperTextItemAtomType				FOUR_CHAR_CODE('htxt')
#define kRangeStart							FOUR_CHAR_CODE('strt')
#define kRangeEnd							FOUR_CHAR_CODE('end ')

#define kKeyEntryDisabled					0
#define kKeyEntryDirect						1
#define kKeyEntryScript						2

// resource ID for string resource containing application's name
#define kAppNameResID						1000
#define kAppNameResIndex					1

// prompt strings for file-put dialog
#define kHTSavePrompt						"Save Hypertext Movie As:"
#define kHTSaveFileName						"HyperText.mov"
#define kMDSavePrompt						"Save Memory Display Movie As:"
#define kMDSaveFileName						"Memory.mov"


//////////
//
// constants for QuickTime icon sprite and its movie
//
//////////

// sizes of the sprite tracks
#define kIconSpriteTrackWidth				300
#define kIconSpriteTrackHeight				150

// PICT resource IDs
#define kOldQTIconID						200
#define kNewQTIconID						201
#define kNewQTIconDownID					202

// sprite atom IDs	
#define kQTIconSpriteAtomID					1

#define kOldQTIconSpriteAtomID				1
#define kNewQTIconSpriteAtomID				2

// image indices
#define kQTIconImageIndex					1

#define kOldQTIconImageIndex				1
#define kNewQTIconImageIndex				2

#define kSpriteMediaTimeScale				600
#define kSpriteMediaFrameDurationIcon		1000

#define kIdleOffset							2

// sprite track variable IDs
#define kXMoveVarIDNew						2000
#define kYMoveVarIDNew						2100

#define kXMoveVarIDOld						2001
#define kYMoveVarIDOld						2101

#define kXMoveVarID							2000
#define kYMoveVarID							2100

#define kSpriteHitID						3000

#define kXMoveVarIDCur						5000
#define kYMoveVarIDCur						5001
#define kSpriteIDCur						5002

#define kXTempVarID							4000
#define kYTempVarID							4001
#define kTempSpriteID						4002

#define kMyCustomEventType					FOUR_CHAR_CODE('myEv')
#define kMasterSpriteID						kNewQTIconSpriteAtomID

#define kSpriteSavePrompt					"Save New Sprite Movie As:"
#define kSpriteSaveMovieFileName			"WiredSprite.mov"

#define kTrackFocusCanEditFlag				FOUR_CHAR_CODE('kedt')


//////////
//
// function prototypes
//
//////////

OSErr							QTWired_CreateTextMovie (FSSpec *theFSSpec);
static OSErr					QTWired_CreateHyperTextActionContainer (QTAtomContainer *theActions);
static OSErr					QTWired_AddActionsToSample (Handle theSample, QTAtomContainer theActions, SInt32 theAtomExtType);
OSErr							QTWired_AddActionsToTextMovie (FSSpec *theFSSpec, UInt16 theMenuItem);
OSErr							QTWired_CreateMemoryDisplayMovie (FSSpec *theFSSpec);
static OSErr					QTWired_CreateMemoryDisplayActionContainer (QTAtomContainer *theActions);
void							QTWired_SetTextTrackProperties (Media theMedia, UInt32 theIdleFrequency, Boolean isEditable);

OSErr							QTWired_CreateWiredIconMovie (UInt16 theMenuItem);
OSErr							QTWired_AddBouncingIconSpriteToMedia (Media theMedia);
OSErr							QTWired_AddSideBounceToSprite (QTAtomContainer theSprite, QTAtomType theSide, float theLimit, QTAtomType theTest, QTAtomID theVariableID);
OSErr							QTWired_AddCollidingIconsSpritesToMedia (Media theMedia);
OSErr							QTWired_AddCollisionLogicToSprite (QTAtomContainer theSprite);
OSErr							QTWired_AddCornerCollisionLogicToSprite (QTAtomContainer theSprite, UInt32 theHorizCorner, UInt32 theVertCorner);
OSErr							QTWired_AddCustomEventSpritesToMedia (Media theMedia);
void							QTWired_SetTrackProperties (Media theMedia, UInt32 theIdleFrequency);
static OSErr					QTWired_AddNegateVariableAction (QTAtomContainer theSprite, QTAtom theParentAtom, QTAtomID theVariableID);
static OSErr					QTWired_AddTranslateByVariablesAction (QTAtomContainer theSprite, QTAtom theParentAtom, QTAtomID theXVariableID, QTAtomID theYVariableID, QTAtom *theActionAtom);
static OSErr					QTWired_AddGetValueOfVarIDAction (QTAtomContainer theSprite, QTAtom theParentAtom, QTAtomID theTempVariableID, QTAtomID theVarID);

