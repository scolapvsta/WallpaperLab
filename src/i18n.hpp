#pragma once
#include <string>
namespace lab {
int languageCount();
const char* languageCode(int index);
const char* languageName(int index);
int languageIndex(const std::string& code);
void setLanguage(int index);
int currentLanguage();
const char* translate(const char* key);
}
