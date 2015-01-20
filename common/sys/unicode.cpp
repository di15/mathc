#include "unicode.h"
#include "../utils.h"
#include "utf8.h"


#define ERR(x) {g_log<<x<<std::endl; g_log.flush(); exit(1);}

// http://foolab.org/node/773
#if 0
int main(int argc, char *argv[])
{
	FILE *fp;
	struct stat buf;
	int x;
	unsigned char *buff;
	unsigned int *unicode = NULL;

	if (argc != 2)
	{
		ERR("Usage unicode <file>");
	}

	x = stat(argv[1], &buf);
	if (x != 0)
	{
		perror("stat");
		exit(1);
	}
	buff = (unsigned char *)malloc(buf.st_size +1);
	if (!buff)
		ERR("Not enough memory!");

	fp = fopen(argv[1], "r");
	if (!fp)
	{
		perror("`");
		exit(1);
	}
	x = fread(buff, 1, buf.st_size, fp);
	if (x != buf.st_size)
	{
		perror("fread");
		exit(1);
	}
	buff[x] = '\0';
	fclose(fp);
	if (ValidateUTF8(buff, x))
	{
		unicode = ToUTF32(buff, x);
		dump_unicode_string(unicode);
		//      dump_unicode(buff, x);
	}
	else
	{
		ERR("Invalid UTF-8 strings.");
	}
	//  printf("%s",buff);
	delete [] buff;
	buff = ToUTF8(unicode);
	if (unicode)
		free(unicode);
	printf("%s\n", buff);
	delete [] buff;
	return 0;
}
#endif

int ValidateUTF8(char unsigned *buff, int len)
{
	int x;
	for (x = 0; x < len; x++)
	{
		if ((unsigned char)(buff[x]) > 0xfd)
		{
			char msg[128];
			sprintf(msg, "Byte %i is invalid\n", x);
			InfoMess(msg, msg);
			return 0;
		}
	}
	return 1;
}

int UTF8Len(unsigned char ch)
{
	int l;
	unsigned char c = ch;
	c >>= 3;
	// 6 => 0x7e
	// 5 => 0x3e
	if (c == 0x1e)
	{
		l = 4;
	}
	else
	{
		c >>= 1;
		if (c == 0xe)
		{
			l = 3;
		}
		else
		{
			c >>= 1;
			if (c == 0x6)
			{
				l = 2;
			}
			else
			{
				l = 1;
			}
		}
	}
	return l;
}

int UTF32Len(const unsigned int* ustr)
{
	int i=0;

	for(; ustr[i]; i++)
		;

	return i;
}

#if 0
unsigned int *ToUTF32(const unsigned char *utf8, int len)
{
	const unsigned char *p = utf8;
	unsigned int ch;
	int x = 0;
	int l;
	unsigned int *result = new unsigned int [len+1];
	unsigned int *r = result;
	if (!result)
	{
		OutOfMem(__FILE__, __LINE__);
	}
	while (*p)
	{
		l = UTF8Len(*p);

		switch (l)
		{
		case 4:
			ch = (*p ^ 0xf0);
			break;
		case 3:
			ch = (*p ^ 0xe0);
			break;
		case 2:
			ch = (*p ^ 0xc0);
			break;
		case 1:
			ch = *p;
			break;
		default:
			//g_log<<"Len: "<<l<<std::endl;
			break;
		}
		++p;

		//if(p)	//polyf fix
		{
			int y;
			for (y = l; y > 1; y--)
			{
				ch <<= 6;
				ch |= (*p ^ 0x80);
				++p;
			}
			//print_char(x,l,ch);
		}

		x += l;
		*r = ch;
		r++;

		//polyf fix
		//if(!p)
		//	break;
	}
	*r = 0x0;
	return result;
}
#endif

// http://www.gamedev.ru/projects/forum/?id=152527&page=18
unsigned char *raw2utf8(unsigned ch)
{
	int i = 0;
	static unsigned char buffer[9];
	while(ch)
	{
		if(ch & 0xff000000)
		{
			buffer[i] = (ch & 0xff000000) >> 24;
			i++;
		}
		ch <<= 8;
	}
	buffer[i] = 0;
	return buffer;
}

#if 0
unsigned char *ToUTF8_2(const unsigned int *unicode)
{
	unsigned char *utf8 = NULL;
	const unsigned int *s = unicode;
	unsigned char *u;
	unsigned int ch;
	int x = 0;
	while (*s)
	{
		++s;
		++x;
	}
	if (x == 0)
	{
		return NULL;
	}
	utf8 = new unsigned char [x*4];
	if (!utf8)
		ERR("Out of memory");

	s = unicode;
	u = utf8;
	x = 0;

	while (*s)
	{
		ch = *s;
#if 0
		if (*s < 0x80)
		{
			x = 1;
			*u = *s;
			u++;
		}
		else if (*s < 0x800)
		{
			x = 2;
			*u = 0xc0 | (ch >> 6);
			u++;
		}
		else if (*s < 0x10000)
		{
			x = 3;
			*u = 0xe0 | (ch >> 12);
			u++;
		}
		else if (*s < 0x200000)
		{
			x = 4;
			*u = 0xf0 | (ch >> 18);
			u++;
		}
		if (x > 1)
		{
			int y;
			for (y = x; y > 1; y--)
			{
				/*
				unsigned int mask = 0x3f << ((y-2)*6);
				*u = 0x80 | (ch & mask);
				*/
				*u = 0x80 | (ch & (0x3f << ((y-2)*6)));
				++u;
			}
		}
#else
		while(ch)
		{
			if(ch & 0xff000000)
			{
				*u = (ch & 0xff000000) >> 24;
				++u;
			}
			ch <<= 8;
		}
#endif
		++s;
	}

	//polyf fix
	*u = 0;

	return utf8;
}

unsigned char *ToUTF8(const unsigned int *unicode)
{
	unsigned char *utf8 = NULL;
	const unsigned int *s = unicode;
	unsigned char *u;
	unsigned int ch;
	int x = 0;
	while (*s)
	{
		++s;
		++x;
	}
	if (x == 0)
	{
		return NULL;
	}
	utf8 = new unsigned char [x*4];
	if (!utf8)
		ERR("Out of memory");

	s = unicode;
	u = utf8;
	x = 0;

	while (*s)
	{
		ch = *s;
		if (*s < 0x80)
		{
			x = 1;
			*u = *s;
			u++;
		}
		else if (*s < 0x800)
		{
			x = 2;
			*u = 0xc0 | (ch >> 6);
			u++;
		}
		else if (*s < 0x10000)
		{
			x = 3;
			*u = 0xe0 | (ch >> 12);
			u++;
		}
		else if (*s < 0x200000)
		{
			x = 4;
			*u = 0xf0 | (ch >> 18);
			u++;
		}
		if (x > 1)
		{
			int y;
			for (y = x; y > 1; y--)
			{
				/*
				unsigned int mask = 0x3f << ((y-2)*6);
				*u = 0x80 | (ch & mask);
				*/
				*u = 0x80 | (ch & (0x3f << ((y-2)*6)));
				++u;
			}
		}
		++s;
	}
	return utf8;
}
#endif

unsigned char *ToUTF8(const unsigned int *unicode)
{
	int utf32len = UTF32Len(unicode);
	int utf8len = utf32len * 4;
	unsigned char* utf8 = new unsigned char [ utf8len + 1 ];
	utf8len = wchar_to_utf8(unicode, utf32len, (char*)utf8, utf8len, 0);
	utf8[utf8len] = 0;
	return utf8;
}

unsigned int *ToUTF32(const unsigned char *utf8)
{
	int utf8len = strlen((char*)utf8);
	unsigned int* utf32 = new unsigned int [ utf8len + 1 ];
	int utf32len = utf8_to_wchar((char*)utf8, utf8len, utf32, utf8len, 0);
	utf32[utf32len] = 0;
	return utf32;
}

#if 0
void dump_unicode(unsigned char *buff, int len)
{
	unsigned int *result = ToUTF32(buff, len);
	dump_unicode_string(result);
	delete [] result;
}

void dump_unicode_string(unsigned int *str)
{
	unsigned int *s = str;
	while (*s)
	{
		char msg[128];
		sprintf(msg, "%li %lx\n", *s, *s);
		g_log<<msg<<std::endl;
		s++;
	}
	//printf("\n");
	g_log<<std::endl;
}

void print_char(int pos, int len, unsigned int ch)
{
	char utf8[5];
	for(int i=0; i<4; i++)
		utf8[i] = ((char*)&ch)[i];
	utf8[4] = 0;
	//printf("Character: %i\tLength: %i\tUTF-32(hex): %lx\tUTF-32(dec): %li\n", pos, len, ch, ch);
	g_log<<"Character: "<<pos<<"\tLength: "<<len<<"\tUTF-8: "<<utf8<<"\tUTF-32(dec): "<<ch<<std::endl;
}
#endif