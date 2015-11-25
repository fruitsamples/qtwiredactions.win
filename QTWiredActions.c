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
//	   <2>	 	03/28/01	rtm		begun adding advanced sprite code
//	   <1>	 	03/10/01	rtm		first file; based on existing AddHTActions sample code
//	   
//
//////////

//////////
//
// header files
//
//////////

#include "QTWiredActions.h"


//////////
//
// QTWired_CreateTextMovie 
// Create a movie with a text track.
//
//////////

OSErr QTWired_CreateTextMovie (FSSpec *theFSSpec)
{
	short			myResRefNum = -1;
	short			myResID = movieInDataForkResID;
	Movie			myMovie = NULL;
	Track			myTrack = NULL;
	Media			myMedia = NULL;
	MediaHandler	myHandler = NULL;
	Rect			myTextBox;
	RGBColor		myTextColor =	{0x0000, 0x0000, 0x0000};
	RGBColor		myBackColor =	{0x8888, 0x8888, 0x8888};
	RGBColor		myHiliteColor =	{0x0000, 0x0000, 0x0000};
	long			myDisplayFlags = 0;
	short			myHiliteStart = 0;
	short			myHiliteEnd = 0;
	TimeValue		myNewMediaTime;
	TimeValue		myScrollDelay = 0;
#if TARGET_OS_MAC
	char			myText[] = "\rPlease take me to Apple or CNN";
#else
	char			myText[] = "\nPlease take me to Apple or CNN";
#endif
	long			myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile | newMovieActive;
	OSType			myType = FOUR_CHAR_CODE('none');
	OSErr			myErr = noErr;
		
	//////////
	//
	// create a new text movie
	//
	//////////

	myErr = CreateMovieFile(theFSSpec, FOUR_CHAR_CODE('TVOD'), 0, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;
	
	// select the "no-interface" movie controller
	myType = EndianU32_NtoB(myType);
	SetUserDataItem(GetMovieUserData(myMovie), &myType, sizeof(myType), kUserDataMovieControllerType, 1);

	myTrack = NewMovieTrack(myMovie, FixRatio(kWidth320, 1), FixRatio(kHeight240, 1), kTrackVolumeZero);	
	myMedia = NewTrackMedia(myTrack, TextMediaType, kTimeScale600, NULL, 0);
	if ((myTrack == NULL) || (myMedia == NULL))
		goto bail;
	
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	myHandler = GetMediaHandler(myMedia);
	if (myHandler == NULL)
		goto bail;

	//////////
	//
	// add a text sample to the movie
	//
	//////////

	MacSetRect(&myTextBox, 0, 0, kWidth320, kHeight240);
	MacInsetRect(&myTextBox, kTextBoxInset, kTextBoxInset);
	
	myErr = (OSErr)TextMediaAddTextSample(	myHandler,
											myText,
											strlen(myText),
											kFontIDTimes,
											kSize48,
											kFacePlain,
											&myTextColor,
											&myBackColor,
											teCenter,
											&myTextBox,
											myDisplayFlags,
											myScrollDelay,
											myHiliteStart,
											myHiliteEnd,
											&myHiliteColor,
											kTimeScale600,
											&myNewMediaTime);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
		
	// add the media to the track, at time 0
	myErr = InsertMediaIntoTrack(myTrack, kTrackStartTimeZero, myNewMediaTime, kTimeScale600, fixed1);
	if (myErr != noErr)
		goto bail;
	
	// add the movie resource
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);
	
bail:
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
	return(myErr);
}


//////////
//
// QTWired_CreateHyperTextActionContainer
// Return, through the theActions parameter, an atom container that contains some hypertext actions.
//
//////////

static OSErr QTWired_CreateHyperTextActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	QTAtom			myHyperTextAtom = 0;
	QTAtom			myWiredObjectsAtom = 0;
	long			mySelStart1 = 19;
	long			mySelEnd1 = 24;
	long			mySelStart2 = 28;
	long			mySelEnd2 = 31;
	long			myValue;
	short			myIndex;
	char			myAppleURL[] = "www.apple.com";
	char			myCNNURL[] = "www.cnn.com";
	ModifierTrackGraphicsModeRecord		myGraphicsModeRecord;
	OSErr			myErr = noErr;
	
	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	// create a wired objects atom
	myErr = QTInsertChild(*theActions, kParentAtomIsContainer, kTextWiredObjectsAtomType, kIndexOne, kIndexZero, kZeroDataLength, NULL, &myWiredObjectsAtom);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// add a hypertext link to the wired objects atom: ID 1
	//
	//////////
	
	myErr = QTInsertChild(*theActions, myWiredObjectsAtom, kHyperTextItemAtomType, kIDOne, kIndexZero, kZeroDataLength, NULL, &myHyperTextAtom);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelStart1);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeStart, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelEnd1);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeEnd, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	// add an event atom to the hypertext atom
	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, myHyperTextAtom, kQTEventMouseClick, kActionGoToURL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexOne, strlen(myAppleURL) + 1, &myAppleURL, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, myHyperTextAtom, kQTEventMouseEnter, kActionTextTrackSetHyperTextColor, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myIndex = EndianS16_NtoB(1);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexOne, sizeof(myIndex), &myIndex, NULL);
	if (myErr != noErr)
		goto bail;
		
	myGraphicsModeRecord.graphicsMode = EndianS32_NtoB(ditherCopy);
	myGraphicsModeRecord.opColor.red = EndianS16_NtoB(0x9999);
	myGraphicsModeRecord.opColor.green = EndianS16_NtoB(0x0000);
	myGraphicsModeRecord.opColor.blue = EndianS16_NtoB(0xcccc);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexTwo, sizeof(myGraphicsModeRecord), &myGraphicsModeRecord, NULL);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, myHyperTextAtom, kQTEventMouseExit, kActionTextTrackSetHyperTextColor, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myIndex = EndianS16_NtoB(1);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexOne, sizeof(myIndex), &myIndex, NULL);
	if (myErr != noErr)
		goto bail;
		
	myGraphicsModeRecord.graphicsMode = EndianS32_NtoB(ditherCopy);
	myGraphicsModeRecord.opColor.red = EndianS16_NtoB(0x0000);
	myGraphicsModeRecord.opColor.green = EndianS16_NtoB(0x0000);
	myGraphicsModeRecord.opColor.blue = EndianS16_NtoB(0xffff);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexTwo, sizeof(myGraphicsModeRecord), &myGraphicsModeRecord, NULL);
	if (myErr != noErr)
		goto bail;
		
	//////////
	//
	// add a hypertext link to the wired objects atom: ID 2
	//
	//////////

	myErr = QTInsertChild(*theActions, myWiredObjectsAtom, kHyperTextItemAtomType, kIDTwo, kIndexZero, kZeroDataLength, NULL, &myHyperTextAtom);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelStart2);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeStart, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	myValue = EndianS32_NtoB(mySelEnd2);
	myErr = QTInsertChild(*theActions, myHyperTextAtom, kRangeEnd, kIDOne, kIndexZero, sizeof(long), &myValue, NULL);
	if (myErr != noErr)
		goto bail;
	
	// add an event atom to the hypertext atom
	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, myHyperTextAtom, kQTEventMouseClick, kActionGoToURL, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexOne, strlen(myCNNURL) + 1, &myCNNURL, NULL);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, myHyperTextAtom, kQTEventMouseEnter, kActionTextTrackSetHyperTextColor, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myIndex = EndianS16_NtoB(2);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexOne, sizeof(myIndex), &myIndex, NULL);
	if (myErr != noErr)
		goto bail;
		
	myGraphicsModeRecord.graphicsMode = EndianS32_NtoB(ditherCopy);
	myGraphicsModeRecord.opColor.red = EndianS16_NtoB(0x9999);
	myGraphicsModeRecord.opColor.green = EndianS16_NtoB(0x0000);
	myGraphicsModeRecord.opColor.blue = EndianS16_NtoB(0xcccc);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexTwo, sizeof(myGraphicsModeRecord), &myGraphicsModeRecord, NULL);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, myHyperTextAtom, kQTEventMouseExit, kActionTextTrackSetHyperTextColor, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	myIndex = EndianS16_NtoB(2);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexOne, sizeof(myIndex), &myIndex, NULL);
	if (myErr != noErr)
		goto bail;
		
	myGraphicsModeRecord.graphicsMode = EndianS32_NtoB(ditherCopy);
	myGraphicsModeRecord.opColor.red = EndianS16_NtoB(0x0000);
	myGraphicsModeRecord.opColor.green = EndianS16_NtoB(0x0000);
	myGraphicsModeRecord.opColor.blue = EndianS16_NtoB(0xffff);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, kIndexTwo, sizeof(myGraphicsModeRecord), &myGraphicsModeRecord, NULL);

bail:
	return(myErr);
}	


//////////
//
// QTWired_AddActionsToSample
// Add the specified atom container to the end of the specified media sample.
//
// Text actions are stored at the end of the sample as a normal text atom extension.
//
//////////

static OSErr QTWired_AddActionsToSample (Handle theSample, QTAtomContainer theActions, SInt32 theAtomExtType)
{
	Ptr			myPtr = NULL;
	long		myHandleLength;
	long		myContainerLength;
	long		myNewLength;
	OSErr		myErr = noErr;

	if ((theSample == NULL) || (theActions == NULL))
		return(paramErr);

	myHandleLength = GetHandleSize(theSample);
	myContainerLength = GetHandleSize((Handle)theActions);
	
	myNewLength = (long)(sizeof(long) + sizeof(OSType) + myContainerLength);
			
	SetHandleSize(theSample, (myHandleLength + myNewLength));
	myErr = MemError();		
	if (myErr != noErr)
		goto bail;
	
	HLock(theSample);
	
	// get a pointer to the beginning of the new block of space added to the sample
	// by the previous call to SetHandleSize; we need to format that space as a text
	// atom extension
	myPtr = *theSample + myHandleLength;
	
	// set the length of the text atom extension
	*(long *)myPtr = EndianS32_NtoB((long)(sizeof(long) + sizeof(OSType) + myContainerLength));
	myPtr += (sizeof(long));
	
	// set the type of the text atom extension
	*(OSType *)myPtr = EndianS32_NtoB(theAtomExtType);
		
	myPtr += (sizeof(OSType));
	
	// set the data of the text atom extension;
	// we assume that this data is already in big-endian format
	HLock((Handle)theActions);
	BlockMove(*theActions, myPtr, myContainerLength);
	
	HUnlock((Handle)theActions);
	HUnlock(theSample);

bail:
	return(myErr);
}


//////////
//
// QTWired_AddActionsToTextMovie
// Add some actions to the specified text movie; theMenuItem specifies which actions to add.
//
//////////

OSErr QTWired_AddActionsToTextMovie (FSSpec *theFSSpec, UInt16 theMenuItem)
{
	short							myResID = 0;
	short							myResRefNum = -1;
	Movie							myMovie = NULL;
	Track							myTrack = NULL;
	Media							myMedia = NULL;
	TimeValue						myTrackOffset;
	TimeValue						myMediaTime;
	TimeValue						mySampleDuration;
	TimeValue						mySelectionDuration;
	TimeValue						myNewMediaTime;
	TextDescriptionHandle			myTextDesc = NULL;
	Handle							mySample = NULL;
	short							mySampleFlags;
	Fixed 							myTrackEditRate;
	QTAtomContainer					myActions = NULL;	
	OSErr							myErr = noErr;

	//////////
	//
	// open the movie file and get the first text track from the movie
	//
	//////////
	
	// open the movie file for reading and writing
	myErr = OpenMovieFile(theFSSpec, &myResRefNum, fsRdWrPerm);
	if (myErr != noErr)
		goto bail;

	myErr = NewMovieFromFile(&myMovie, myResRefNum, &myResID, NULL, newMovieActive, NULL);
	if (myErr != noErr)
		goto bail;
		
	// find first text track in the movie
	myTrack = GetMovieIndTrackType(myMovie, kIndexOne, TextMediaType, movieTrackMediaType);
	if (myTrack == NULL)
		goto bail;
	
	//////////
	//
	// get first media sample in the text track
	//
	//////////
	
	myMedia = GetTrackMedia(myTrack);
	if (myMedia == NULL)
		goto bail;
	
	myTrackOffset = GetTrackOffset(myTrack);
	myMediaTime = TrackTimeToMediaTime(myTrackOffset, myTrack);

	// allocate some storage to hold the sample description for the text track
	myTextDesc = (TextDescriptionHandle)NewHandle(4);
	if (myTextDesc == NULL)
		goto bail;

	mySample = NewHandle(0);
	if (mySample == NULL)
		goto bail;

	myErr = GetMediaSample(myMedia, mySample, 0, NULL, myMediaTime, NULL, &mySampleDuration, (SampleDescriptionHandle)myTextDesc, NULL, 1, NULL, &mySampleFlags);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add actions to the first media sample
	//
	//////////
	
	switch (theMenuItem) {
		case IDM_MAKE_HYPERTEXT_MOVIE:
			// create an action container for hypertext actions
			myErr = QTWired_CreateHyperTextActionContainer(&myActions);
			if (myErr != noErr)
				goto bail;
			
			// add hypertext actions to sample
			myErr = QTWired_AddActionsToSample(mySample, myActions, kHyperTextTextAtomType);
			if (myErr != noErr)
				goto bail;
			break;
			
		case IDM_MAKE_MEM_DISPLAY_MOVIE:
			// create an action container for wired actions
			myErr = QTWired_CreateMemoryDisplayActionContainer(&myActions);
			if (myErr != noErr)
				goto bail;
			
			// add actions to sample
			myErr = QTWired_AddActionsToSample(mySample, myActions, kQTEventType);
			if (myErr != noErr)
				goto bail;
			break;
			
		default:
			myErr = paramErr;
			goto bail;
	}	
		
	//////////
	//
	// replace sample in media
	//
	//////////
	
	myTrackEditRate = GetTrackEditRate(myTrack, myTrackOffset);
	if (GetMoviesError() != noErr)
		goto bail;

	GetTrackNextInterestingTime(myTrack, nextTimeMediaSample | nextTimeEdgeOK, myTrackOffset, fixed1, NULL, &mySelectionDuration);
	if (GetMoviesError() != noErr)
		goto bail;

	myErr = DeleteTrackSegment(myTrack, myTrackOffset, mySelectionDuration);
	if (myErr != noErr)
		goto bail;
		
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	myErr = AddMediaSample(	myMedia,
							mySample,
							0,
							GetHandleSize(mySample),
							mySampleDuration,
							(SampleDescriptionHandle)myTextDesc, 
							1,
							mySampleFlags,
							&myNewMediaTime);
	if (myErr != noErr)
		goto bail;
	
	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	myErr = InsertMediaIntoTrack(myTrack, myTrackOffset, myNewMediaTime, mySelectionDuration, myTrackEditRate);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// update the movie resource
	//
	//////////
	
	myErr = UpdateMovieResource(myMovie, myResRefNum, myResID, NULL);
	if (myErr != noErr)
		goto bail;
	
	// close the movie file
	myErr = CloseMovieFile(myResRefNum);
		
bail:
	if (myActions != NULL)
		(void)QTDisposeAtomContainer(myActions);
	
	if (mySample != NULL)
		DisposeHandle(mySample);		
	
	if (myTextDesc != NULL)
		DisposeHandle((Handle)myTextDesc);		
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);	

	return(myErr);
}


//////////
//
// QTWired_CreateMemoryDisplayMovie 
// Create a movie that displays the available application memory in a text track.
//
//////////

OSErr QTWired_CreateMemoryDisplayMovie (FSSpec *theFSSpec)
{
	short			myResRefNum = -1;
	short			myResID = movieInDataForkResID;
	Movie			myMovie = NULL;
	Track			myTrack = NULL;
	Media			myMedia = NULL;
	MediaHandler	myHandler = NULL;
	Rect			myTextBox;
	RGBColor		myTextColor =	{0x0000, 0x0000, 0x0000};
	RGBColor		myBackColor =	{0x8888, 0x8888, 0x8888};
	RGBColor		myHiliteColor =	{0x0000, 0x0000, 0x0000};
	long			myDisplayFlags = 0;
	short			myHiliteStart = 0;
	short			myHiliteEnd = 0;
	TimeValue		myNewMediaTime;
	TimeValue		myScrollDelay = 0;
	char			myText[] = "0";
	long			myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile | newMovieActive;
	OSType			myType = FOUR_CHAR_CODE('none');
	OSErr			myErr = noErr;
		
	//////////
	//
	// create a new text movie
	//
	//////////

	myErr = CreateMovieFile(theFSSpec, FOUR_CHAR_CODE('TVOD'), 0, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;
	
	// select the "no-interface" movie controller
	myType = EndianU32_NtoB(myType);
	SetUserDataItem(GetMovieUserData(myMovie), &myType, sizeof(myType), kUserDataMovieControllerType, 1);

	myTrack = NewMovieTrack(myMovie, FixRatio(kWidth200, 1), FixRatio(kHeight50, 1), kTrackVolumeZero);	
	myMedia = NewTrackMedia(myTrack, TextMediaType, kTimeScale600, NULL, 0);
	if ((myTrack == NULL) || (myMedia == NULL))
		goto bail;
	
	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	myHandler = GetMediaHandler(myMedia);
	if (myHandler == NULL)
		goto bail;

	//////////
	//
	// add a text sample to the movie
	//
	//////////

	MacSetRect(&myTextBox, 0, 0, kWidth200, kHeight50);
	MacInsetRect(&myTextBox, kTextBoxInset, kTextBoxInset);
	
	myErr = (OSErr)TextMediaAddTextSample(	myHandler,
											myText,
											strlen(myText),
											kFontIDTimes,
											kSize48,
											kFacePlain,
											&myTextColor,
											&myBackColor,
											teCenter,
											&myTextBox,
											myDisplayFlags,
											myScrollDelay,
											myHiliteStart,
											myHiliteEnd,
											&myHiliteColor,
											kTimeScale600,
											&myNewMediaTime);
	if (myErr != noErr)
		goto bail;

	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
		
	// add the media to the track, at time 0
	myErr = InsertMediaIntoTrack(myTrack, kTrackStartTimeZero, myNewMediaTime, kTimeScale600, fixed1);
	if (myErr != noErr)
		goto bail;
	
	// set the text track properties
	QTWired_SetTextTrackProperties(myMedia, 120, false);
	
	// add the movie resource
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, NULL);
	
bail:
	if (myResRefNum != -1)
		CloseMovieFile(myResRefNum);
	
	if (myMovie != NULL)
		DisposeMovie(myMovie);
	
	return(myErr);
}


//////////
//
// QTWired_CreateMemoryDisplayActionContainer
// Return, through the theActions parameter, an atom container that contains some edit-text actions.
//
//////////

static OSErr QTWired_CreateMemoryDisplayActionContainer (QTAtomContainer *theActions)
{
	QTAtom			myEventAtom = 0;
	QTAtom			myActionAtom = 0;
	QTAtom			myParamAtom = 0;
	short			myEditState;
	UInt32			myPos;
	QTAtom			myParameterAtom = 0;
	QTAtom			myExpressionAtom = 0;
	QTAtom			myOperandAtom = 0;
	OSErr			myErr = noErr;
	
	myErr = QTNewAtomContainer(theActions);
	if (myErr != noErr)
		goto bail;
	
	// add an event atom that enables text editing;
	// in theory we could do this just once in a frame-loaded event, but it doesn't seem to impact performance here
	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, kParentAtomIsContainer, kQTEventIdle, kActionTextTrackSetEditable, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myEditState = EndianS16_NtoB(kKeyEntryScript);
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, 1, sizeof(myEditState), &myEditState, NULL);
	if (myErr != noErr)
		goto bail;

	// add an event atom that displays the amount of application memory currently free
	myErr = WiredUtils_AddQTEventAndActionAtoms(*theActions, kParentAtomIsContainer, kQTEventIdle, kActionTextTrackPasteText, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	// first parameter: the text to be pasted
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, 1, 0, NULL, &myParameterAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(*theActions, myParameterAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(*theActions, myExpressionAtom, kOperandAtomType, 1, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(*theActions, myOperandAtom, kOperandFreeMemory, 1, 1, 0, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	// second parameter: selection range begin: 0
	myPos = EndianU32_NtoB(0);	
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, 2, sizeof(myPos), &myPos, NULL);
	if (myErr != noErr)
		goto bail;
		
	// third parameter: selection range end: 0xffff		
	myPos = EndianU32_NtoB(0xffff);	
	myErr = WiredUtils_AddActionParameterAtom(*theActions, myActionAtom, 3, sizeof(myPos), &myPos, NULL);

bail:
	return(myErr);
}	


//////////
//
// QTWired_SetTextTrackProperties
// Set the track properties for the specified text media.
//
//////////

void QTWired_SetTextTrackProperties (Media theMedia, UInt32 theIdleFrequency, Boolean isEditable)
{
	QTAtomContainer		myTrackProperties;
	Boolean				myBoolean;
	UInt32				myFrequency;
	OSErr				myErr = noErr;
		
	myErr = QTNewAtomContainer(&myTrackProperties);
	if (myErr == noErr) {
		// tell the movie controller that this text track has actions
		myBoolean = true;
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyHasActions, 1, 1, sizeof(myBoolean), &myBoolean, NULL);
	
		// tell the text track to generate QTIdleEvents
		myFrequency = EndianU32_NtoB(theIdleFrequency);
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyQTIdleEventsFrequency, 1, 1, sizeof(myFrequency), &myFrequency, NULL);

		// set the text track to be editable or not, as determined by the isEditable parameter
		QTInsertChild(myTrackProperties, 0, kTrackFocusCanEditFlag, 1, 1, sizeof(isEditable), &isEditable, NULL);

		SetMediaPropertyAtom(theMedia, myTrackProperties);
		
		QTDisposeAtomContainer(myTrackProperties);
	}
}
	

//////////
//
// QTWired_CreateWiredIconMovie
// Create a wired sprite movie containing one or more icons with the specified behavior.
//
//////////

OSErr QTWired_CreateWiredIconMovie (UInt16 theMenuItem)
{
	Movie					myMovie = NULL;
	Track					myTrack = NULL;
	Media					myMedia = NULL;
	FSSpec					myFile;
	Boolean					myIsSelected = false;
	Boolean					myIsReplacing = false;	
	Fixed					myHeight = 0;
	Fixed					myWidth = 0;
	StringPtr 				myPrompt = QTUtils_ConvertCToPascalString(kSpriteSavePrompt);
	StringPtr 				myFileName = QTUtils_ConvertCToPascalString(kSpriteSaveMovieFileName);
	long					myFlags = createMovieFileDeleteCurFile | createMovieFileDontCreateResFile;
	OSType					myType = FOUR_CHAR_CODE('none');
	short					myResRefNum = 0;
	short					myResID = movieInDataForkResID;
	OSErr					myErr = noErr;

	//////////
	//
	// create a new movie file
	//
	//////////

	// prompt the user for the destination file name
	QTFrame_PutFile(myPrompt, myFileName, &myFile, &myIsSelected, &myIsReplacing);
	myErr = myIsSelected ? noErr : userCanceledErr;
	if (!myIsSelected)
		goto bail;

	// create a movie file for the destination movie
	myErr = CreateMovieFile(&myFile, FOUR_CHAR_CODE('TVOD'), smSystemScript, myFlags, &myResRefNum, &myMovie);
	if (myErr != noErr)
		goto bail;
	
	// select the "no-interface" movie controller
	myType = EndianU32_NtoB(myType);
	SetUserDataItem(GetMovieUserData(myMovie), &myType, sizeof(myType), kUserDataMovieControllerType, 1);
	
	//////////
	//
	// create the sprite track and media
	//
	//////////
	
	myWidth = Long2Fix(kIconSpriteTrackWidth);
	myHeight = Long2Fix(kIconSpriteTrackHeight);

	myTrack = NewMovieTrack(myMovie, myWidth, myHeight, kNoVolume);
	myMedia = NewTrackMedia(myTrack, SpriteMediaType, kSpriteMediaTimeScale, NULL, 0);

	myErr = BeginMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add the appropriate samples to the sprite media
	//
	//////////
	
	switch (theMenuItem) {
		case IDM_MAKE_BOUNCE_ICON_MOVIE:
			myErr = QTWired_AddBouncingIconSpriteToMedia(myMedia);
			break;

		case IDM_MAKE_COLLIDE_ICONS_MOVIE:
			myErr = QTWired_AddCollidingIconsSpritesToMedia(myMedia);
			break;
		
//		case IDM_MAKE_CUSTOM_EVENT_MOVIE:
//			myErr = QTWired_AddCustomEventSpritesToMedia(myMedia);
//			break;
		
		default:
			myErr = paramErr;
			break;
	}

	if (myErr != noErr)
		goto bail;
	
	myErr = EndMediaEdits(myMedia);
	if (myErr != noErr)
		goto bail;
	
	// add the media to the track
	InsertMediaIntoTrack(myTrack, 0, 0, GetMediaDuration(myMedia), fixed1);
		
	//////////
	//
	// set the sprite track properties
	//
	//////////
	
	QTWired_SetTrackProperties(myMedia, 1);
	
	//////////
	//
	// add the movie resource to the movie file
	//
	//////////
	
	myErr = AddMovieResource(myMovie, myResRefNum, &myResID, myFile.name);
		
bail:
	if (myResRefNum != 0)
		CloseMovieFile(myResRefNum);

	if (myMovie != NULL)
		DisposeMovie(myMovie);
		
	free(myPrompt);
	free(myFileName);

	return(myErr);
}


//////////
//
// QTWired_AddBouncingIconSpriteToMedia
// Build the key frame for the bouncing icon sprite movie.
//
//////////

OSErr QTWired_AddBouncingIconSpriteToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	QTAtom					myEventAtom = 0;
	QTAtom					myActionAtom = 0;
	QTAtom					myNewParamAtom = 0;
	QTAtom					myExpressionAtom = 0;
	QTAtom					myOperandAtom = 0;
	QTAtom					myOperandTypeAtom = 0;
	RGBColor				myKeyColor;
	short					isVisible, myIndex;
	FixedPoint				myPoint;
	OSErr					myErr = noErr;
	
	//////////
	//
	// create a key frame sample containing the sprite images
	//
	//////////

	// create a new, empty key frame sample
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;

	myKeyColor.red = myKeyColor.green = myKeyColor.blue = 0xffff;		// white
	
	myPoint.x = 0;
	myPoint.y = 0;

	// add images to the key frame sample
	myErr = SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kNewQTIconID, &myKeyColor, 1, &myPoint, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add frame-loaded events that initialize the two sprite track variables to 1
	//
	//////////
	
	myErr = WiredUtils_AddSpriteTrackSetVariableAction(mySample, kParentAtomIsContainer, kQTEventFrameLoaded, kXMoveVarID, kIdleOffset, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddSpriteTrackSetVariableAction(mySample, kParentAtomIsContainer, kQTEventFrameLoaded, kYMoveVarID, kIdleOffset, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add the initial sprite properties to the key frame sample
	//
	//////////

	myErr = QTNewAtomContainer(&mySpriteData);
	if (myErr != noErr)
		goto bail;

	// the QT icon sprite
	isVisible		= true;
	myIndex			= kQTIconImageIndex;
	
	myErr = SpriteUtils_SetSpriteData(mySpriteData, NULL, &isVisible, NULL, &myIndex, NULL, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add idle event actions to the sprite:
	// (1) translate by the current x and y offsets
	// (2) if the left side of the sprite is less than 1 or the right side is greater than the track width,
	//     negate the x offset and move the sprite back to the edge
	// (3) if the top side of the sprite is less than 1 or the bottom side is greater than the track height,
	//     negate the y offset and move the sprite back to the edge
	//
	//////////
	
	//////////
	//
	// (1) translate by the current x and y offsets
	//
	//////////
	
	myErr = WiredUtils_AddQTEventAtom(mySpriteData, kParentAtomIsContainer, kQTEventIdle, &myEventAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTWired_AddTranslateByVariablesAction(mySpriteData, myEventAtom, kXMoveVarID, kYMoveVarID, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// (2) and (3): if we hit a side, bounce back
	//
	//////////

	myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsLeft, 1, kOperatorLessThan, kXMoveVarID);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsRight, kIconSpriteTrackWidth, kOperatorGreaterThan, kXMoveVarID);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsTop, 1, kOperatorLessThan, kYMoveVarID);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsBottom, kIconSpriteTrackHeight, kOperatorGreaterThan, kYMoveVarID);
	if (myErr != noErr)
		goto bail;
		
	// add the sprite to the key frame sample
	myErr = SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kQTIconSpriteAtomID);
	if (myErr != noErr)
		goto bail;

	myErr = SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationIcon, true, NULL);	

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);
	
	return(myErr);
}


//////////
//
// QTWired_AddSideBounceToSprite
// If the specified sprite side is beyond the limit, bounce back.
//
//////////

OSErr QTWired_AddSideBounceToSprite (QTAtomContainer theSprite, QTAtomType theSide, float theLimit, QTAtomType theTest, QTAtomID theVariableID)
{
	QTAtom				myActionAtom = 0;
	QTAtom				myNewActionAtom = 0;
	QTAtom				myExpressionAtom = 0;
	QTAtom				myParamAtom = 0;
	QTAtom				myNewParamAtom = 0;
	QTAtom				myOperatorAtom = 0;
	QTAtom				myOperandAtom = 0;
	QTAtom				myOperandTypeAtom = 0;
	QTAtom				myConditionalAtom = 0;
	QTAtom				myActionListAtom = 0;
	Boolean				myBoolean;
	OSErr				myErr = noErr;
	
	// add an idle event "if" action
	myErr = WiredUtils_AddQTEventAndActionAtoms(theSprite, kParentAtomIsContainer, kQTEventIdle, kActionCase, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	// add a parameter atom to the kActionCase action atom; this will serve as a parent to hold the expression and action atoms
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kFirstParam, 0, NULL, &myNewParamAtom);
	if (myErr != noErr)
		goto bail;
	
	//////////
	//
	// if the side of the sprite is less/greater than the specified limit...
	//
	//////////

	myErr = WiredUtils_AddConditionalAtom(theSprite, myNewParamAtom, 1, &myConditionalAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myConditionalAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddOperatorAtom(theSprite, myExpressionAtom, theTest, &myOperatorAtom);
	if (myErr != noErr)
		goto bail;
		
	// first operand: the specified side of the sprite
	myErr = QTInsertChild(theSprite, myOperatorAtom, kOperandAtomType, 1, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myOperandAtom, theSide, 1, 1, 0, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	// second operand: the specified limit
	myErr = WiredUtils_AddOperandAtom(theSprite, myOperatorAtom, kOperandConstant, 2, NULL, theLimit);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// ...translate back to the edge...
	//
	//////////

	myErr = WiredUtils_AddActionListAtom(theSprite, myConditionalAtom, &myActionListAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionAtom(theSprite, myActionListAtom, kActionSpriteTranslate, &myNewActionAtom);
	if (myErr != noErr)
		goto bail;
	
	// add parameters to the translate action: Fixed x, Fixed y, Boolean isAbsolute
	// first parameter: theLimit - the side of sprite
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myNewActionAtom, kFirstParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myParamAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myExpressionAtom, kOperatorAtomType, kOperatorSubtract, 1, 0, NULL, &myOperatorAtom);
	if (myErr != noErr)
		goto bail;

	// first operand: theLimit
	myErr = WiredUtils_AddOperandAtom(theSprite, myOperatorAtom, kOperandConstant, 1, NULL, theLimit);
	if (myErr != noErr)
		goto bail;

	// second operand: the side of sprite
	myErr = QTInsertChild(theSprite, myOperatorAtom, kOperandAtomType, 2, 2, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myOperandAtom, theSide, 1, 1, 0, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	// second parameter: 0
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myNewActionAtom, kSecondParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myParamAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddOperandAtom(theSprite, myExpressionAtom, kOperandConstant, 1, NULL, 0);
	if (myErr != noErr)
		goto bail;
	
	// third parameter: false
	myBoolean = false;
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myNewActionAtom, kThirdParam, sizeof(myBoolean), &myBoolean, NULL);

	//////////
	//
	// ...and negate the x or y offset
	//
	//////////

	myErr = QTWired_AddNegateVariableAction(theSprite, myActionListAtom, theVariableID);
		
bail:
	return(myErr);
}


//////////
//
// QTWired_AddCollidingIconsSpritesToMedia
// Build the key frame for the colliding icons sprite movie.
//
//////////

OSErr QTWired_AddCollidingIconsSpritesToMedia (Media theMedia)
{
	QTAtomContainer			mySample = NULL;
	QTAtomContainer			mySpriteData = NULL;
	QTAtom					myEventAtom = 0;
	QTAtom					myActionAtom = 0;
	QTAtom					myNewParamAtom = 0;
	QTAtom					myExpressionAtom = 0;
	QTAtom					myOperandAtom = 0;
	QTAtom					myOperandTypeAtom = 0;
	RGBColor				myKeyColor;
	short					isVisible, myIndex, myLayer;
	FixedPoint				myPoint;
	short					myCount;
	Point					myLocation;
	OSErr					myErr = noErr;
	
	//////////
	//
	// create a key frame sample containing the sprite images
	//
	//////////

	// create a new, empty key frame sample
	myErr = QTNewAtomContainer(&mySample);
	if (myErr != noErr)
		goto bail;

	myKeyColor.red = myKeyColor.green = myKeyColor.blue = 0xffff;		// white
	
	myPoint.x = 0;
	myPoint.y = 0;

	// add images to the key frame sample
	myErr = SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kOldQTIconID, &myKeyColor, kOldQTIconSpriteAtomID, &myPoint, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = SpriteUtils_AddPICTImageToKeyFrameSample(mySample, kNewQTIconID, &myKeyColor, kNewQTIconSpriteAtomID, &myPoint, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// add frame-loaded events that initialize the sprite track variables
	//
	//////////
	
	myErr = WiredUtils_AddSpriteTrackSetVariableAction(mySample, kParentAtomIsContainer, kQTEventFrameLoaded, kXMoveVarIDNew, kIdleOffset, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddSpriteTrackSetVariableAction(mySample, kParentAtomIsContainer, kQTEventFrameLoaded, kYMoveVarIDNew, kIdleOffset, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddSpriteTrackSetVariableAction(mySample, kParentAtomIsContainer, kQTEventFrameLoaded, kXMoveVarIDOld, -kIdleOffset, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddSpriteTrackSetVariableAction(mySample, kParentAtomIsContainer, kQTEventFrameLoaded, kYMoveVarIDOld, kIdleOffset, 0, NULL, 0);
	if (myErr != noErr)
		goto bail;
	
	for (myCount = 0; myCount <= 1; myCount++) {
		
		myErr = QTNewAtomContainer(&mySpriteData);
		if (myErr != noErr)
			goto bail;

		//////////
		//
		// add the initial sprite properties to the key frame sample
		//
		//////////
		
		isVisible		= true;
		myIndex			= kOldQTIconImageIndex + myCount;
		myLayer			= myCount;
		
		myLocation.h	= 100 * myCount;
		myLocation.v	= 50 * myCount;
				
		myErr = SpriteUtils_SetSpriteData(mySpriteData, &myLocation, &isVisible, &myLayer, &myIndex, NULL, NULL, NULL);
		if (myErr != noErr)
			goto bail;

		//////////
		//
		// add idle event actions to the sprite:
		// (1) translate by the current x and y offsets
		// (2) if the left side of the sprite is less than 1 or the right side is greater than the track width,
		//     negate the x offset and move the sprite back to the edge
		// (3) if the top side of the sprite is less than 1 or the bottom side is greater than the track height,
		//     negate the y offset and move the sprite back to the edge
		// (4) if we hit another sprite, recoil
		//
		//////////
		
		//////////
		//
		// (1) translate by the current x and y offsets
		//
		//////////
	
		myErr = WiredUtils_AddQTEventAtom(mySpriteData, kParentAtomIsContainer, kQTEventIdle, &myEventAtom);
		if (myErr != noErr)
			goto bail;
			
		myErr = QTWired_AddTranslateByVariablesAction(mySpriteData, myEventAtom, kXMoveVarID + myCount, kYMoveVarID + myCount, NULL);
		if (myErr != noErr)
			goto bail;

		//////////
		//
		// (2) and (3): if we hit a side, bounce back
		//
		//////////

		myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsLeft, 1, kOperatorLessThan, kXMoveVarID + myCount);
		if (myErr != noErr)
			goto bail;
			
		myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsRight, kIconSpriteTrackWidth, kOperatorGreaterThan, kXMoveVarID + myCount);
		if (myErr != noErr)
			goto bail;
			
		myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsTop, 1, kOperatorLessThan, kYMoveVarID + myCount);
		if (myErr != noErr)
			goto bail;
			
		myErr = QTWired_AddSideBounceToSprite(mySpriteData, kOperandSpriteBoundsBottom, kIconSpriteTrackHeight, kOperatorGreaterThan, kYMoveVarID + myCount);
		if (myErr != noErr)
			goto bail;
			
		//////////
		//
		// (4) if we hit another sprite, recoil
		//
		//////////
		
		if (myCount == 1)
			QTWired_AddCollisionLogicToSprite(mySpriteData);
		
		// add the sprite to the key frame sample
		myErr = SpriteUtils_AddSpriteToSample(mySample, mySpriteData, kOldQTIconSpriteAtomID + myCount);
		if (myErr != noErr)
			goto bail;
	}

//	myErr = SpriteUtils_AddSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationIcon, true, NULL);	
	myErr = SpriteUtils_AddCompressedSpriteSampleToMedia(theMedia, mySample, kSpriteMediaFrameDurationIcon, true, zlibDataCompressorSubType, NULL);

bail:	
	if (mySample != NULL)
		QTDisposeAtomContainer(mySample);

	if (mySpriteData != NULL)
		QTDisposeAtomContainer(mySpriteData);
	
	return(myErr);
}


//////////
//
// QTWired_AddCollisionLogicToSprite
// If the specified sprite hits another sprite, have both recoil in the appropriate directions.
//
//////////

OSErr QTWired_AddCollisionLogicToSprite (QTAtomContainer theSprite)
{
	OSErr				myErr = noErr;

	myErr = QTWired_AddCornerCollisionLogicToSprite(theSprite, kOperandSpriteBoundsLeft, kOperandSpriteBoundsTop);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTWired_AddCornerCollisionLogicToSprite(theSprite, kOperandSpriteBoundsRight, kOperandSpriteBoundsTop);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTWired_AddCornerCollisionLogicToSprite(theSprite, kOperandSpriteBoundsLeft, kOperandSpriteBoundsBottom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTWired_AddCornerCollisionLogicToSprite(theSprite, kOperandSpriteBoundsRight, kOperandSpriteBoundsBottom);
	
bail:
	return(myErr);	
}


//////////
//
// QTWired_AddCornerCollisionLogicToSprite
// If the corner of the specified sprite hits another sprite, have both recoil in the appropriate directions.
//
// set variable kSpriteHitID to the value of kOperandSpriteTrackSpriteIDAtPoint[theHorizCorner,theVertCorner];
// if value of kSpriteHitID is equal to ID of other sprite,
//		if xdir[1] != xdir[2] then
//			xdir[1] = -xdir[1]
//			xdir[2] = -xdir[2]
//		if ydir[1] != ydir[2] then
//			ydir[1] = -ydir[1]
//			ydir[2] = -ydir[2]
//
// Keep in mind that calling kOperandSpriteTrackSpriteIDAtPoint with the parameters set to the corner of a sprite 
// will always return the ID of that sprite, UNLESS some other sprite at that location has a lower layer.
// 
//////////

OSErr QTWired_AddCornerCollisionLogicToSprite (QTAtomContainer theSprite, UInt32 theHorizCorner, UInt32 theVertCorner)
{
	QTAtom				myActionAtom = 0;
	QTAtom				myNewActionAtom = 0;
	QTAtom				myExpressionAtom = 0;
	QTAtom				myNewExpressionAtom = 0;
	QTAtom				myParamAtom = 0;
	QTAtom				myNewParamAtom = 0;
	QTAtom				myOperatorAtom = 0;
	QTAtom				myNewOperatorAtom = 0;
	QTAtom				myOperandAtom = 0;
	QTAtom				myNewOperandAtom = 0;
	QTAtom				myOperandTypeAtom = 0;
	QTAtom				myNewOperandTypeAtom = 0;
	QTAtom				myConditionalAtom = 0;
	QTAtom				myNewConditionalAtom = 0;
	QTAtom				myActionListAtom = 0;
	QTAtom				myNewActionListAtom = 0;
	QTAtomID			myVariableID;
	OSErr				myErr = noErr;
	
	//////////
	//
	// set variable kSpriteHitID to the value of kOperandSpriteTrackSpriteIDAtPoint[theHorizCorner, theVertCorner]
	//
	//////////

	myErr = WiredUtils_AddQTEventAndActionAtoms(theSprite, kParentAtomIsContainer, kQTEventIdle, kActionSpriteTrackSetVariable, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	// add parameters to the set variable action: variable ID (QTAtomID) and value (float)
	myVariableID = EndianU32_NtoB(kSpriteHitID);
	myErr = QTInsertChild(theSprite, myActionAtom, kActionParameter, 0, (short)kFirstParam, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myActionAtom, kActionParameter, 0, (short)kSecondParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myParamAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myOperandAtom, kOperandSpriteTrackSpriteIDAtPoint, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	// add parameters to kOperandSpriteTrackSpriteIDAtPoint operand;
	// first param: theHorizCorner
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myOperandTypeAtom, kFirstParam, 0, NULL, &myNewParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myNewParamAtom, &myNewExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myNewExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myNewOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myNewOperandAtom, theHorizCorner, 1, 1, 0, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	// second param: theVertCorner
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myOperandTypeAtom, kSecondParam, 0, NULL, &myNewParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myNewParamAtom, &myNewExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myNewExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myNewOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myNewOperandAtom, theVertCorner, 1, 1, 0, NULL, NULL);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// if value of variable kSpriteHitID is ID of other sprite (kOldQTIconSpriteAtomID)...
	//
	//////////

	// add an idle event "if" action
	myErr = WiredUtils_AddQTEventAndActionAtoms(theSprite, kParentAtomIsContainer, kQTEventIdle, kActionCase, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	// add a parameter atom to the kActionCase action atom
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kFirstParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddConditionalAtom(theSprite, myParamAtom, 1, &myConditionalAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myConditionalAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddOperatorAtom(theSprite, myExpressionAtom, kOperatorEqualTo, &myOperatorAtom);
	if (myErr != noErr)
		goto bail;
		
	// first operand: the value of kSpriteHitID variable
	myErr = QTInsertChild(theSprite, myOperatorAtom, kOperandAtomType, 1, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(kSpriteHitID);
	myErr = QTInsertChild(theSprite, myOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;

	// second operand: the sprite ID
	myErr = WiredUtils_AddOperandAtom(theSprite, myOperatorAtom, kOperandConstant, 2, NULL, (float)kOldQTIconSpriteAtomID);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// ...and if the two sprites are moving in different horizontal directions...
	//
	//////////

	myErr = WiredUtils_AddActionListAtom(theSprite, myConditionalAtom, &myActionListAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionAtom(theSprite, myActionListAtom, kActionCase, &myNewActionAtom);
	if (myErr != noErr)
		goto bail;

	// add a parameter atom to the kActionCase action atom
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myNewActionAtom, kFirstParam, 0, NULL, &myNewParamAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddConditionalAtom(theSprite, myNewParamAtom, 1, &myNewConditionalAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myNewConditionalAtom, &myNewExpressionAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddOperatorAtom(theSprite, myNewExpressionAtom, kOperatorNotEqualTo, &myNewOperatorAtom);
	if (myErr != noErr)
		goto bail;
		
	// first operand: the value of kXMoveVarIDNew
	myErr = QTInsertChild(theSprite, myNewOperatorAtom, kOperandAtomType, 1, 1, 0, NULL, &myNewOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myNewOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myNewOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(kXMoveVarIDNew);
	myErr = QTInsertChild(theSprite, myNewOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;

	// second operand: the value of kXMoveVarIDOld
	myErr = QTInsertChild(theSprite, myNewOperatorAtom, kOperandAtomType, 2, 2, 0, NULL, &myNewOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myNewOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myNewOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(kXMoveVarIDOld);
	myErr = QTInsertChild(theSprite, myNewOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
		
	// ...adjust the direction of travel...
	myErr = WiredUtils_AddActionListAtom(theSprite, myNewConditionalAtom, &myNewActionListAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTWired_AddNegateVariableAction(theSprite, myNewActionListAtom, kXMoveVarIDNew);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTWired_AddNegateVariableAction(theSprite, myNewActionListAtom, kXMoveVarIDOld);
	if (myErr != noErr)
		goto bail;

	//////////
	//
	// ...and if the two sprites are moving in different vertical directions...
	//
	//////////

	myErr = WiredUtils_AddActionAtom(theSprite, myActionListAtom, kActionCase, &myNewActionAtom);
	if (myErr != noErr)
		goto bail;

	// add a parameter atom to the kActionCase action atom
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myNewActionAtom, kFirstParam, 0, NULL, &myNewParamAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddConditionalAtom(theSprite, myNewParamAtom, 1, &myNewConditionalAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myNewConditionalAtom, &myNewExpressionAtom);
	if (myErr != noErr)
		goto bail;
		
	myErr = WiredUtils_AddOperatorAtom(theSprite, myNewExpressionAtom, kOperatorNotEqualTo, &myNewOperatorAtom);
	if (myErr != noErr)
		goto bail;
		
	// first operand: the value of kYMoveVarIDNew
	myErr = QTInsertChild(theSprite, myNewOperatorAtom, kOperandAtomType, 1, 1, 0, NULL, &myNewOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myNewOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myNewOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(kYMoveVarIDNew);
	myErr = QTInsertChild(theSprite, myNewOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;

	// second operand: the value of kYMoveVarIDOld
	myErr = QTInsertChild(theSprite, myNewOperatorAtom, kOperandAtomType, 2, 2, 0, NULL, &myNewOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myNewOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myNewOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(kYMoveVarIDOld);
	myErr = QTInsertChild(theSprite, myNewOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
		
	// ...adjust the direction of travel...
	myErr = WiredUtils_AddActionListAtom(theSprite, myNewConditionalAtom, &myNewActionListAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTWired_AddNegateVariableAction(theSprite, myNewActionListAtom, kYMoveVarIDNew);
	if (myErr != noErr)
		goto bail;
		
	myErr = QTWired_AddNegateVariableAction(theSprite, myNewActionListAtom, kYMoveVarIDOld);

bail:	
	return(myErr);
}
	
	
//////////
//
// QTWired_SetTrackProperties
// Set the track properties for the specified sample sprite media.
//
//////////

void QTWired_SetTrackProperties (Media theMedia, UInt32 theIdleFrequency)
{
	QTAtomContainer		myTrackProperties;
	RGBColor			myBackgroundColor;
	Boolean				hasActions;
	UInt32				myFrequency;
	OSErr				myErr = noErr;
		
	// add a background color to the sprite track
	myBackgroundColor.red = EndianU16_NtoB(0xffff);
	myBackgroundColor.green = EndianU16_NtoB(0xffff);
	myBackgroundColor.blue = EndianU16_NtoB(0xffff);
	
	myErr = QTNewAtomContainer(&myTrackProperties);
	if (myErr == noErr) {
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyBackgroundColor, 1, 1, sizeof(myBackgroundColor), &myBackgroundColor, NULL);

		// tell the movie controller that this sprite track has actions
		hasActions = true;
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyHasActions, 1, 1, sizeof(hasActions), &hasActions, NULL);
	
		// tell the sprite track to generate QTIdleEvents
		myFrequency = EndianU32_NtoB(theIdleFrequency);
		QTInsertChild(myTrackProperties, 0, kSpriteTrackPropertyQTIdleEventsFrequency, 1, 1, sizeof(myFrequency), &myFrequency, NULL);

		SetMediaPropertyAtom(theMedia, myTrackProperties);
		
		QTDisposeAtomContainer(myTrackProperties);
	}
}
	

//////////
//
// QTWired_AddNegateVariableAction
// Add an action atom that negates the specified sprite track variable.
//
//////////

static OSErr QTWired_AddNegateVariableAction (QTAtomContainer theSprite, QTAtom theParentAtom, QTAtomID theVariableID)
{
	QTAtom				myActionAtom = 0;
	QTAtom				myExpressionAtom = 0;
	QTAtom				myParamAtom = 0;
	QTAtom				myOperatorAtom = 0;
	QTAtom				myOperandAtom = 0;
	QTAtom				myOperandTypeAtom = 0;
	QTAtomID			myVariableID;
	OSErr				myErr = paramErr;

	if ((theSprite == NULL) || (theParentAtom == 0))
		goto bail;

	myErr = WiredUtils_AddActionAtom(theSprite, theParentAtom, kActionSpriteTrackSetVariable, &myActionAtom);
	if (myErr != noErr)
		goto bail;
	
	// add parameters to the set variable action: variable ID (QTAtomID) and value (float)
	myVariableID = EndianU32_NtoB(theVariableID);
	myErr = QTInsertChild(theSprite, myActionAtom, kActionParameter, 0, (short)kFirstParam, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myActionAtom, kActionParameter, 0, (short)kSecondParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myParamAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myExpressionAtom, kOperatorAtomType, kOperatorNegate, 1, 0, NULL, &myOperatorAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myOperatorAtom, kOperandAtomType, 0, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(theVariableID);
	myErr = QTInsertChild(theSprite, myOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);

bail:
	return(myErr);
}
	

//////////
//
// QTWired_AddTranslateByVariablesAction
// Add an action atom that translates the specified sprite relatively by an amount specified by two variables.
//
//////////

static OSErr QTWired_AddTranslateByVariablesAction (QTAtomContainer theSprite, QTAtom theParentAtom, QTAtomID theXVariableID, QTAtomID theYVariableID, QTAtom *theActionAtom)
{
	QTAtom				myActionAtom = 0;
	QTAtom				myExpressionAtom = 0;
	QTAtom				myParamAtom = 0;
	QTAtom				myOperatorAtom = 0;
	QTAtom				myOperandAtom = 0;
	QTAtom				myOperandTypeAtom = 0;
	QTAtomID			myVariableID;
	Boolean				myBoolean;
	OSErr				myErr = paramErr;

	if (theSprite == NULL)
		goto bail;

	// add a translate action atom to the specified parent atom
	myErr = WiredUtils_AddActionAtom(theSprite, theParentAtom, kActionSpriteTranslate, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	// add parameters to the translate action: Fixed x, Fixed y, Boolean isAbsolute

	// first parameter: get value of variable theXVariableID
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kFirstParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myParamAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(theXVariableID);
	myErr = QTInsertChild(theSprite, myOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
	
	// second parameter: get value of variable theYVariableID
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kSecondParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myParamAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;
	
	myErr = QTInsertChild(theSprite, myOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(theYVariableID);
	myErr = QTInsertChild(theSprite, myOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;
	
	// third parameter: false (for relative translation)
	myBoolean = false;
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kThirdParam, sizeof(myBoolean), &myBoolean, NULL);
	
bail:
	if (theActionAtom != NULL)
		*theActionAtom = myActionAtom;

	return(myErr);
}
	

//////////
//
// QTWired_AddGetValueOfVarIDAction
// Add an action atom that sets theTempVariableID to the value in the variable whose ID is the value of theVarID.
//
//////////

static OSErr QTWired_AddGetValueOfVarIDAction (QTAtomContainer theSprite, QTAtom theParentAtom, QTAtomID theTempVariableID, QTAtomID theVarID)
{
	QTAtom				myActionAtom = 0;
	QTAtom				myExpressionAtom = 0;
	QTAtom				myNewExpressionAtom = 0;
	QTAtom				myParamAtom = 0;
	QTAtom				myNewParamAtom = 0;
	QTAtom				myOperandAtom = 0;
	QTAtom				myNewOperandAtom = 0;
	QTAtom				myOperandTypeAtom = 0;
	QTAtom				myNewOperandTypeAtom = 0;
	QTAtomID			myVariableID;
	OSErr				myErr = paramErr;

	if (theSprite == NULL)
		goto bail;

#if 0
	{
		Str255			myString;
		
		NumToString(theTempVariableID, myString);
		
		myErr = WiredUtils_AddActionAtom(theSprite, theParentAtom, kActionDebugStr, &myActionAtom);
		if (myErr != noErr)
			goto bail;
			
		// add a target atom as the first parameter
		myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kFirstParam, myString[0], &myString, NULL);
		if (myErr != noErr)
			goto bail;
	}
#endif

	myErr = WiredUtils_AddActionAtom(theSprite, theParentAtom, kActionSpriteTrackSetVariable, &myActionAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(theTempVariableID);
	myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kFirstParam, sizeof(myVariableID), &myVariableID, NULL);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theSprite, myActionAtom, kSecondParam, 0, NULL, &myParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myParamAtom, &myExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myOperandAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddActionParameterAtom(theSprite, myOperandTypeAtom, kFirstParam, 0, NULL, &myNewParamAtom);
	if (myErr != noErr)
		goto bail;

	myErr = WiredUtils_AddExpressionContainerAtomType(theSprite, myNewParamAtom, &myNewExpressionAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myNewExpressionAtom, kOperandAtomType, 0, 1, 0, NULL, &myNewOperandAtom);
	if (myErr != noErr)
		goto bail;

	myErr = QTInsertChild(theSprite, myNewOperandAtom, kOperandSpriteTrackVariable, 1, 1, 0, NULL, &myNewOperandTypeAtom);
	if (myErr != noErr)
		goto bail;

	myVariableID = EndianU32_NtoB(theVarID);
	myErr = QTInsertChild(theSprite, myNewOperandTypeAtom, kActionParameter, 1, 1, sizeof(myVariableID), &myVariableID, NULL);

bail:
	return(myErr);
}
	


