//-----------------------------------------------------------------------------
// V12 Engine
// 
// Copyright (c) 2001 GarageGames.Com
// Portions Copyright (c) 2001 by Sierra Online, Inc.
//-----------------------------------------------------------------------------

#ifndef _TYPEVALIDATORS_H_
#define _TYPEVALIDATORS_H_

class TypeValidator
{
   public:
   S32 fieldIndex;

   // prints a console error message, prefaces it with:
   // className objectName (objectId) - invalid value for fieldName: msg
   void consoleError(SimObject *object, const char *format, ...);

   // validateType is called for each assigned value on the field this
   // validator is attached to
   virtual void validateType(SimObject *object, void *typePtr) = 0;
};


// Floating point min/max range validator

class FRangeValidator : public TypeValidator
{
   F32 minV, maxV;
public:
   FRangeValidator(F32 minValue, F32 maxValue)
	{
		minV = minValue;
		maxV = maxValue;
	}
   void validateType(SimObject *object, void *typePtr);
};

// signed integer min/max range validator

class IRangeValidator : public TypeValidator
{
   S32 minV, maxV;
public:
   IRangeValidator(S32 minValue, S32 maxValue)
	{
		minV = minValue;
		maxV = maxValue;
	}
   void validateType(SimObject *object, void *typePtr);
};

// scaled integer field validator - !note! should
// NOT be used on a field that gets exported - 
// field is only converted once on initial assignment

class IRangeValidatorScaled : public TypeValidator
{
   S32 minV, maxV;
   S32 factor;
public:
   IRangeValidatorScaled(S32 scaleFactor, S32 minValueScaled, S32 maxValueScaled)
	{
		minV = minValueScaled;
		maxV = maxValueScaled;
		factor = scaleFactor;
	}
   void validateType(SimObject *object, void *typePtr);
};

#endif