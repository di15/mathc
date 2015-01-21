#include "utils.h"
#include "platform.h"
#include "window.h"

std::ofstream g_log;

const std::string DateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	return buf;
}

const std::string Time()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%X", &tstruct);

	return buf;
}

const std::string FileDateTime()
{
	time_t     now = time(0);
	struct tm  tstruct;
	char       buf[80];
	tstruct = *localtime(&now);
	// Visit http://www.cplusplus.com/reference/clibrary/ctime/strftime/
	// for more information about date/time format
	strftime(buf, sizeof(buf), "%Y-%m-%d %X", &tstruct);

	for(int i=0; i<strlen(buf); i++)
		if(buf[i] == ':')
			buf[i] = '-';

	return buf;
}


void OpenLog(const char* filename, int version)
{
	char fullpath[MAX_PATH+1];
	FullPath(filename, fullpath);
	g_log.close();
	g_log.open(fullpath, std::ios_base::out);
	g_log<<DateTime()<<std::endl;
	g_log<<"Version "<<version<<std::endl<<std::endl;
	g_log.flush();
}

std::string MakePathRelative(const char* full)
{
	char full2c[MAX_PATH+1];
	strcpy(full2c, full);
	CorrectSlashes(full2c);
	std::string full2(full2c);
	char exepath[MAX_PATH+1];
	ExePath(exepath);
	CorrectSlashes(exepath);

	//g_log<<"exepath: "<<exepath<<std::endl;
	//g_log<<"fulpath: "<<full<<std::endl;

	std::string::size_type pos = full2.find(exepath, 0);

	if(pos == std::string::npos)
	{
		return full2;
	}

	//g_log<<"posposp: "<<pos<<std::endl;

	std::string sub = std::string( full2 ).substr(strlen(exepath)+1, strlen(full)-strlen(exepath)-1);

	//g_log<<"subpath: "<<sub<<std::endl;

	return sub;
}

std::string StripFile(std::string filepath)
{
	int lastof = filepath.find_last_of("/\\");
	if(lastof < 0)
		lastof = strlen(filepath.c_str());
	else
		lastof += 1;

	std::string stripped = filepath.substr(0, lastof);
	return stripped;
}

void StripPath(char* filepath)
{
	std::string s0(filepath);
	size_t sep = s0.find_last_of("\\/");
	std::string s1;

	if (sep != std::string::npos)
		s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	strcpy(filepath, s1.c_str());
}

void StripExtension(char* filepath)
{
	std::string s1(filepath);

	size_t dot = s1.find_last_of(".");
	std::string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(filepath, s2.c_str());
}

void StripPathExtension(const char* n, char* o)
{
	std::string s0(n);
	size_t sep = s0.find_last_of("\\/");
	std::string s1;

	if (sep != std::string::npos)
		s1 = s0.substr(sep + 1, s0.size() - sep - 1);
	else
		s1 = s0;

	size_t dot = s1.find_last_of(".");
	std::string s2;

	if (dot != std::string::npos)
		s2 = s1.substr(0, dot);
	else
		s2 = s1;

	strcpy(o, s2.c_str());
}

#ifndef PLATFORM_IOS
void ExePath(char* exepath)
{
#if 1
#ifdef PLATFORM_WIN
	//char buffer[MAX_PATH+1];
	GetModuleFileName(NULL, exepath, MAX_PATH+1);
	//std::string::size_type pos = std::string( buffer ).find_last_of( "\\/" );
	//std::string strexepath = std::string( buffer ).substr( 0, pos);
	//strcpy(exepath, strexepath.c_str());
#else
	char szTmp[32];
	//char buffer[MAX_PATH+1];
	sprintf(szTmp, "/proc/%d/exe", getpid());
	int bytes = std::min((int)readlink(szTmp, exepath, MAX_PATH+1), MAX_PATH);
	if(bytes >= 0)
		exepath[bytes] = '\0';
	//std::string strexepath = StripFile(std::string(buffer));
	//strcpy(exepath, strexepath.c_str());
#endif
#else
	char *base_path = SDL_GetBasePath();
#if 0
	if (base_path) {
        data_path = SDL_strdup(base_path);
        SDL_free(base_path);
    } else {
        data_path = SDL_strdup("./");
    }
#endif
	if(base_path)
	{
		strcpy(exepath, base_path);
		SDL_free(base_path);
	}
#endif
}
#endif

void FullPath(const char* filename, char* full)
{
	char exepath[MAX_PATH+1];
	ExePath(exepath);
	std::string path = StripFile(exepath);

	//char full[MAX_PATH+1];
	sprintf(full, "%s", path.c_str());

	char c = full[ strlen(full)-1 ];
	if(c != '\\' && c != '/')
		strcat(full, "\\");
	//strcat(full, "/");

	strcat(full, filename);
	CorrectSlashes(full);
}

float StrToFloat(const char *s)
{
	if(s[0] == '\0')
		return 1.0f;

	float x;
	std::istringstream iss(s);
	iss >> x;

	if(_isnan(x))
		x = 1.0f;

	return x;
}

int HexToInt(const char* s)
{
	int x;
	std::stringstream ss;
	ss << std::hex << s;
	ss >> x;
	return x;
}

int StrToInt(const char *s)
{
	int x;
	std::istringstream iss(s);
	iss >> x;
	return x;
}

void CorrectSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '\\')
			corrected[i] = '/';
}

void BackSlashes(char* corrected)
{
	int strl = strlen(corrected);
	for(int i=0; i<strl; i++)
		if(corrected[i] == '/')
			corrected[i] = '\\';
}

void ErrMess(const char* title, const char* message)
{
	SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, message, NULL);
	SDL_ShowCursor(false);
}

void InfoMess(const char* title, const char* message)
{
	SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, title, message, NULL);
	SDL_ShowCursor(false);
}

void WarnMess(const char* title, const char* message)
{
	SDL_ShowCursor(true);
	SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, title, message, NULL);
	SDL_ShowCursor(false);
}

void OutOfMem(const char* file, int line)
{
	char msg[2048];
	sprintf(msg, "Failed to allocate memory in %s on line %d.", file, line);
	ErrMess("Out of memory", msg);
	g_quit = true;
}

void ListFiles(const char* fullpath, std::list<std::string>& files)
{
	DIR *dp;
	struct dirent *dirp;

	if((dp  = opendir(fullpath)) == NULL)
	{
		g_log << "Error opening " << fullpath << std::endl;
		return;
	}

	while ((dirp = readdir(dp)) != NULL)
	{
		if(dirp->d_type != DT_REG)
			continue;

		files.push_back(std::string(dirp->d_name));
	}

	closedir(dp);
	return;
}

//add thousands separators
std::string iform(int n)
{
	std::string s;

	char first[32];
	sprintf(first, "%d", n);

	for(int i=strlen(first)-3; i>=0; i-=3)
	{
		if(first[i] == '-' || i == 0 || first[i-1] == '-')
			break;

		int newl = strlen(first)+1;

		for(int j=newl; j>=i; j--)
		{
			first[j+1] = first[j];
		}

		first[i] = ',';
	}

	s = first;

	return s;
}

#ifndef PLATFORM_WIN

static unsigned long long g_starttick = -1;
unsigned long long timeGetTime()
{
	return GetTickCount();
}

unsigned long long GetTickCount()
{
	if(g_starttick < 0)
		g_starttick = GetTickCount64();

	return (long)(GetTickCount64() - g_starttick);
}

unsigned long long GetTickCount64()
{
	return SDL_GetTicks();
}

void Sleep(int ms)
{
	SDL_Delay(ms);
}
#endif
