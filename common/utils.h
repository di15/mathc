#ifndef UTILS_H
#define UTILS_H

#include "platform.h"

#define CORRECT_SLASH '/'

#define ARRSZ(a)	(sizeof(a)/sizeof(a[0]))

extern std::ofstream g_log;

const std::string DateTime();
const std::string Time();
const std::string FileDateTime();
void FullPath(const char* filename, char* full);
std::string MakePathRelative(const char* full);
void ExePath(char* exepath);
std::string StripFile(std::string filepath);
void StripPathExtension(const char* n, char* o);
void StripExtension(char* filepath);
void StripPath(char* filepath);
void OpenLog(const char* filename, int version);
float StrToFloat(const char *s);
int HexToInt(const char* s);
int StrToInt(const char *s);
void CorrectSlashes(char* corrected);
void BackSlashes(char* corrected);
void ErrMess(const char* title, const char* message);
void InfoMess(const char* title, const char* message);
void WarnMess(const char* title, const char* message);
void OutOfMem(const char* file, int line);

#ifndef PLATFORM_WIN

unsigned long long timeGetTime();
unsigned long long GetTickCount();
unsigned long long GetTickCount64();
void Sleep(int ms);

#endif

inline float fmax(const float a, const float b)
{
	return (((a)>(b))?(a):(b));
}

inline float fmin(const float a, const float b)
{
	return (((a)<(b))?(a):(b));
}

inline int imax(const int a, const int b)
{
	return (((a)>(b))?(a):(b));
}

inline int imin(const int a, const int b)
{
	return (((a)<(b))?(a):(b));
}

inline int iabs(int x)
{
	//return x & INT_MAX;
	return x & 0x7FFFFFFF;
}

//deterministic ceil
inline int iceil(const int num, const int denom)
{
	if(denom  == 0)
		return 0;

	int div = num / denom;
	const int mul = div * denom;
	const int rem = num - mul;

	if(rem > 0)
		div += 1;

	return div;
}

std::string iform(int n);

void ListFiles(const char* fullpath, std::list<std::string>& files);

#endif	//UTILS_H
