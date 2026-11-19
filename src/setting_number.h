#pragma once
#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

// Accept a full finite number; the UI owns display units and storage scaling.
static inline bool LabParseSettingNumber(const char* text,double lo,double hi,double* value){
 if(!text||strlen(text)>=64)return false;
 char input[64];strcpy(input,text);for(char* p=input;*p;p++)if(*p==',')*p='.';
 char* end;double number=strtod(input,&end);if(end==input||!isfinite(number))return false;
 while(isspace((unsigned char)*end))end++;if(*end)return false;
 *value=fmax(lo,fmin(hi,number));return true;
}
