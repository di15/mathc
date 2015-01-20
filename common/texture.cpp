#include "platform.h"
#include "texture.h"
#include "render/model.h"
#include "gui/gui.h"
#include "utils.h"
#include "debug.h"

Texture g_texture[TEXTURES];
std::vector<TextureToLoad> g_texLoad;

int g_texwidth;
int g_texheight;
//int g_texindex;
int g_lastLTex = -1;

char jpegBuffer[JPEG_BUFFER_SIZE];
JPEGSource   jpegSource;
FILE* g_src;
//CFile g_src;
int srcLen;


LoadedTex::LoadedTex()
{
	data = NULL;
}

void LoadedTex::destroy()
{
	if(data)
	{
		free(data);
		data = NULL;
	}
}

LoadedTex::~LoadedTex()
{
	destroy();
}

LoadedTex *LoadBMP(const char *fullpath)
{
	LoadedTex *pImage = new LoadedTex;
	FILE *pFile = NULL;

	if((pFile = fopen(fullpath, "rb")) == NULL)
	{
		char msg[MAX_PATH+1];
		sprintf(msg, "Unable to load BMP File: %s", fullpath);
		ErrMess("Error", msg);
		return NULL;
	}

	if(!pImage)
		OutOfMem(__FILE__, __LINE__);


#ifdef PLATFORM_WIN
	AUX_RGBImageRec *pBitbldg = NULL;

	// Load the bitbldg using the aux function stored in glaux.lib
	pBitbldg = auxDIBImageLoad(fullpath);

	pImage->channels = 3;
	pImage->sizeX = pBitbldg->sizeX;
	pImage->sizeY = pBitbldg->sizeY;
	pImage->data  = pBitbldg->data;

	free(pBitbldg);
#endif // PLATFORM_WIN
#ifdef PLATFORM_LINUX
	SDL_Surface *s = nullptr;
	s = SDL_LoadBMP(fullpath);

	pImage->channels = 3;
	pImage->sizeX = s->h;
	pImage->sizeY = s->w;
	pImage->data  = (unsigned char*)s->pixels; // Dunno if this right. TODO Confirm.
#endif // PLATFORM_LINUX

	/*
	int stride = pImage->channels * pBitbldg->sizeX;
	int i;
	int y2;
	int temp;

	for(int y = 0; y < pImage->sizeY/2; y++)
	{
		y2 = pImage->sizeY - y;
		// Store a pointer to the current line of pixels
		unsigned char *pLine = &(pImage->data[stride * y]);
		unsigned char *pLine2 = &(pImage->data[stride * y2]);

		// Go through all of the pixels and swap the B and R values since TGA
		// files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
		for(i = 0; i < stride; i += pImage->channels)
		{
			temp = pLine[i];
			pLine[i] = pLine2[i];
			pLine2[i] = temp;

			temp = pLine[i+1];
			pLine[i+1] = pLine2[i+1];
			pLine2[i+1] = temp;

			temp = pLine[i+2];
			pLine[i+2] = pLine2[i+2];
			pLine2[i+2] = temp;
		}
	}*/

	fclose(pFile);

	return pImage;
}

LoadedTex *LoadTGA(const char *fullpath)
{
	LoadedTex *pImageData = NULL;			// This stores our important image data
	WORD width = 0, height = 0;			// The dimensions of the image
	byte length = 0;					// The length in bytes to the pixels
	byte imageType = 0;					// The image type (RLE, RGB, Alpha...)
	byte bits = 0;						// The bits per pixel for the image (16, 24, 32)
	FILE *pFile = NULL;					// The file pointer
	int channels = 0;					// The channels of the image (3 = RGA : 4 = RGBA)
	int stride = 0;						// The stride (channels * width)
	int i = 0;							// A counter

	if((pFile = fopen(fullpath, "rb")) == NULL)
	{
		char msg[MAX_PATH+1];
		sprintf(msg, "Unable to load TGA File: %s", fullpath);
		ErrMess("Error", msg);
		return NULL;
	}

	// allocate the structure that will hold our eventual image data (must free it!)
	pImageData = new LoadedTex;

	if(!pImageData)
		OutOfMem(__FILE__, __LINE__);

	// Read in the length in bytes from the header to the pixel data
	fread(&length, sizeof(byte), 1, pFile);

	// Jump over one byte
	fseek(pFile,1,SEEK_CUR);

	// Read in the imageType (RLE, RGB, etc...)
	fread(&imageType, sizeof(byte), 1, pFile);

	// Skip past general information we don't care about
	fseek(pFile, 9, SEEK_CUR);

	// Read the width, height and bits per pixel (16, 24 or 32)
	fread(&width,  sizeof(WORD), 1, pFile);
	fread(&height, sizeof(WORD), 1, pFile);
	fread(&bits,   sizeof(byte), 1, pFile);

	// Now we move the file pointer to the pixel data
	fseek(pFile, length + 1, SEEK_CUR);

	// Check if the image is RLE compressed or not
	if(imageType != TGA_RLE)
	{
		// Check if the image is a 24 or 32-bit image
		if(bits == 24 || bits == 32)
		{
			// Calculate the channels (3 or 4) - (use bits >> 3 for more speed).
			// Next, we calculate the stride and allocate enough memory for the pixels.
			channels = bits / 8;
			stride = channels * width;
			pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

			if(!pImageData->data)
			{
				OutOfMem(__FILE__, __LINE__);
				fclose(pFile);
				return NULL;
			}

			// Load in all the pixel data line by line
			for(int y = 0; y < height; y++)
			{
				// Store a pointer to the current line of pixels
				unsigned char *pLine = &(pImageData->data[stride * y]);

				// Read in the current line of pixels
				fread(pLine, stride, 1, pFile);

				// Go through all of the pixels and swap the B and R values since TGA
				// files are stored as BGR instead of RGB (or use GL_BGR_EXT verses GL_RGB)
				for(i = 0; i < stride; i += channels)
				{
					int temp     = pLine[i];
					pLine[i]     = pLine[i + 2];
					pLine[i + 2] = temp;
				}
			}
		}
		// Check if the image is a 16 bit image (RGB stored in 1 unsigned short)
		else if(bits == 16)
		{
			unsigned short pixels = 0;
			int r=0, g=0, b=0;

			// Since we convert 16-bit images to 24 bit, we hardcode the channels to 3.
			// We then calculate the stride and allocate memory for the pixels.
			channels = 3;
			stride = channels * width;
			pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));

			if(!pImageData->data)
			{
				OutOfMem(__FILE__, __LINE__);
				fclose(pFile);
				return NULL;
			}

			// Load in all the pixel data pixel by pixel
			for(int i = 0; i < width*height; i++)
			{
				// Read in the current pixel
				fread(&pixels, sizeof(unsigned short), 1, pFile);

				// Convert the 16-bit pixel into an RGB
				b = (pixels & 0x1f) << 3;
				g = ((pixels >> 5) & 0x1f) << 3;
				r = ((pixels >> 10) & 0x1f) << 3;

				// This essentially assigns the color to our array and swaps the
				// B and R values at the same time.
				pImageData->data[i * 3 + 0] = r;
				pImageData->data[i * 3 + 1] = g;
				pImageData->data[i * 3 + 2] = b;
			}
		}
		// Else return a NULL for a bad or unsupported pixel format
		else
			return NULL;
	}
	// Else, it must be Run-Length Encoded (RLE)
	else
	{
		// Create some variables to hold the rleID, current colors read, channels, & stride.
		byte rleID = 0;
		int colorsRead = 0;
		channels = bits / 8;
		stride = channels * width;

		// Next we want to allocate the memory for the pixels and create an array,
		// depending on the channel count, to read in for each pixel.
		pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*stride*height));
		byte *pColors = ((byte*)malloc(sizeof(byte)*channels));

		if(!pImageData->data)
		{
			OutOfMem(__FILE__, __LINE__);
			fclose(pFile);
			return NULL;
		}

		// Load in all the pixel data
		while(i < width*height)
		{
			// Read in the current color count + 1
			fread(&rleID, sizeof(byte), 1, pFile);

			// Check if we don't have an encoded std::string of colors
			if(rleID < 128)
			{
				// Increase the count by 1
				rleID++;

				// Go through and read all the unique colors found
				while(rleID)
				{
					// Read in the current color
					fread(pColors, sizeof(byte) * channels, 1, pFile);

					// Store the current pixel in our image array
					pImageData->data[colorsRead + 0] = pColors[2];
					pImageData->data[colorsRead + 1] = pColors[1];
					pImageData->data[colorsRead + 2] = pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits == 32)
						pImageData->data[colorsRead + 3] = pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}
			}
			// Else, let's read in a std::string of the same character
			else
			{
				// Minus the 128 ID + 1 (127) to get the color count that needs to be read
				rleID -= 127;

				// Read in the current color, which is the same for a while
				fread(pColors, sizeof(byte) * channels, 1, pFile);

				// Go and read as many pixels as are the same
				while(rleID)
				{
					// Assign the current pixel to the current index in our pixel array
					pImageData->data[colorsRead + 0] = pColors[2];
					pImageData->data[colorsRead + 1] = pColors[1];
					pImageData->data[colorsRead + 2] = pColors[0];

					// If we have a 4 channel 32-bit image, assign one more for the alpha
					if(bits == 32)
						pImageData->data[colorsRead + 3] = pColors[3];

					// Increase the current pixels read, decrease the amount
					// of pixels left, and increase the starting index for the next pixel.
					i++;
					rleID--;
					colorsRead += channels;
				}

			}

		}
	}

	// Close the file pointer that opened the file
	fclose(pFile);

	// Flip upside-down
	int x;
	int y2;
	byte temp[4];
	for(int y=0; y<height/2; y++)
	{
		y2 = height - y - 1;

		unsigned char *pLine = &(pImageData->data[stride * y]);
		unsigned char *pLine2 = &(pImageData->data[stride * y2]);

		for(x=0; x<width*channels; x+=channels)
		{
			temp[0] = pLine[x + 0];
			temp[1] = pLine[x + 1];
			temp[2] = pLine[x + 2];
			if(bits == 32)
				temp[3] = pLine[x + 3];

			pLine[x + 0] = pLine2[x + 0];
			pLine[x + 1] = pLine2[x + 1];
			pLine[x + 2] = pLine2[x + 2];
			if(bits == 32)
				pLine[x + 3] = pLine2[x + 3];

			pLine2[x + 0] = temp[0];
			pLine2[x + 1] = temp[1];
			pLine2[x + 2] = temp[2];
			if(bits == 32)
				pLine2[x + 3] = temp[3];
		}
	}

	// Fill in our LoadedTex structure to pass back
	pImageData->channels = channels;
	pImageData->sizeX    = width;
	pImageData->sizeY    = height;

	// Return the TGA data (remember, you must free this data after you are done)
	return pImageData;
}

void DecodeJPG(jpeg_decompress_struct* cinfo, LoadedTex *pImageData)
{
	// Read in the header of the jpeg file
	jpeg_read_header(cinfo, TRUE);

	// Start to decompress the jpeg file with our compression info
	jpeg_start_decompress(cinfo);

	// Get the image dimensions and channels to read in the pixel data
	pImageData->channels = cinfo->num_components;
	pImageData->sizeX    = cinfo->image_width;
	pImageData->sizeY    = cinfo->image_height;

	// Get the row span in bytes for each row
	int rowSpan = cinfo->image_width * cinfo->num_components;

	// allocate memory for the pixel buffer
	pImageData->data = ((unsigned char*)malloc(sizeof(unsigned char)*rowSpan*pImageData->sizeY));

	if(!pImageData->data)
	{
		OutOfMem(__FILE__, __LINE__);
		return;
	}

	// Create an array of row pointers
	unsigned char** rowPtr = new unsigned char*[pImageData->sizeY];

	if(!rowPtr)
	{
		OutOfMem(__FILE__, __LINE__);
		return;
	}

	for (int i = 0; i < pImageData->sizeY; i++)
		rowPtr[i] = &(pImageData->data[i * rowSpan]);

	// Now comes the juice of our work, here we extract all the pixel data
	int rowsRead = 0;
	while (cinfo->output_scanline < cinfo->output_height)
	{
		// Read in the current row of pixels and increase the rowsRead count
		rowsRead += jpeg_read_scanlines(cinfo,
		                                &rowPtr[rowsRead], cinfo->output_height - rowsRead);
	}

	// Delete the temporary row pointers
	delete [] rowPtr;

	// Finish decompressing the data
	jpeg_finish_decompress(cinfo);
}

LoadedTex *LoadJPG(const char *fullpath)
{
	struct jpeg_decompress_struct cinfo;
	LoadedTex *pImageData = NULL;
	FILE *pFile;

	// Open a file pointer to the jpeg file and check if it was found and opened
	if((pFile = fopen(fullpath, "rb")) == NULL)
	{
		// Display an error message saying the file was not found, then return NULL
		char msg[MAX_PATH+1];
		sprintf(msg, "Unable to load JPG File: %s \n errno = %d", fullpath, errno);
		ErrMess("Error", msg);
		return NULL;
	}

	// Create an error handler
	jpeg_error_mgr jerr;

	// Have our compression info object point to the error handler address
	cinfo.err = jpeg_std_error(&jerr);

	// Initialize the decompression object
	jpeg_create_decompress(&cinfo);

	// Specify the data source (Our file pointer)
	jpeg_stdio_src(&cinfo, pFile);

	// allocate the structure that will hold our eventual jpeg data (must free it!)
	pImageData = new LoadedTex;

	if(!pImageData)
		OutOfMem(__FILE__, __LINE__);

	// Decode the jpeg file and fill in the image data structure to pass back
	DecodeJPG(&cinfo, pImageData);

	// This releases all the stored memory for reading and decoding the jpeg
	jpeg_destroy_decompress(&cinfo);

	// Close the file pointer that opened the file
	fclose(pFile);

	// Return the jpeg data (remember, you must free this data after you are done)
	return pImageData;
}

LoadedTex *LoadPNG(const char *fullpath)
{
	LoadedTex *pImageData = NULL;

	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	//int color_type, interlace_type;
	FILE *fp;

	/*
	if ((fp = fopen(relative, "rb")) == NULL)
		return NULL;
	png_byte header[8];
	fread(header, sizeof(png_byte), 8, fp);
	fclose(fp);

	g_log<<"PNG header "<<relative<<" "
		<<(int)header[0]<<","<<(int)header[1]<<","<<(int)header[2]<<","<<(int)header[3]<<","
		<<(int)header[4]<<","<<(int)header[5]<<","<<(int)header[6]<<","<<(int)header[7]<<std::endl;
	*/
	if ((fp = fopen(fullpath, "rb")) == NULL)
		return NULL;


	/* Create and initialize the png_struct
	 * with the desired error handler
	 * functions.  If you want to use the
	 * default stderr and longjump method,
	 * you can supply NULL for the last
	 * three parameters.  We also supply the
	 * the compiler header file version, so
	 * that we know if the application
	 * was compiled with a compatible version
	 * of the library.  REQUIRED
	 */

	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return NULL;
	}

	/* allocate/initialize the memory
	 * for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
		return NULL;
	}

	/* Set error handling if you are
	 * using the setjmp/longjmp method
	 * (this is the normal method of
	 * doing things with libpng).
	 * REQUIRED unless you  set up
	 * your own error handlers in
	 * the png_create_read_struct()
	 * earlier.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* Free all of the memory associated
		 * with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
		fclose(fp);
		/* If we get here, we had a
		 * problem reading the file */
		return NULL;
	}

	/* Set up the output control if
	 * you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* If we have already
	 * read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);

	/*
	 * If you have enough memory to read
	 * in the entire image at once, and
	 * you need to specify only
	 * transforms that can be controlled
	 * with one of the PNG_TRANSFORM_*
	 * bits (this presently excludes
	 * dithering, filling, setting
	 * background, and doing gamma
	 * adjustment), then you can read the
	 * entire image (including pixels)
	 * into the info structure with this
	 * call
	 *
	 * PNG_TRANSFORM_STRIP_16 |
	 * PNG_TRANSFORM_PACKING  forces 8 bit
	 * PNG_TRANSFORM_EXPAND forces to
	 *  expand a palette into RGB
	 */
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);

	pImageData = new LoadedTex;

	if(!pImageData)
		OutOfMem(__FILE__, __LINE__);

	pImageData->sizeX = png_get_image_width(png_ptr, info_ptr); //info_ptr->width;
	pImageData->sizeY = png_get_image_height(png_ptr, info_ptr); //info_ptr->height;
	//switch (info_ptr->color_type)
	switch( png_get_color_type(png_ptr, info_ptr) )
	{
	case PNG_COLOR_TYPE_RGBA:
		pImageData->channels = 4;
		break;
	case PNG_COLOR_TYPE_RGB:
		pImageData->channels = 3;
		break;
	default:
		g_log<<fullpath<<" color type "<<png_get_color_type(png_ptr, info_ptr)<<" not supported"<<std::endl;
		//std::cout << "Color type " << info_ptr->color_type << " not supported" << std::endl;
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		delete pImageData;
		return NULL;
	}

	unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	pImageData->data = (unsigned char*) malloc(row_bytes * pImageData->sizeY);

	if(!pImageData->data)
	{
		OutOfMem(__FILE__, __LINE__);
		fclose(fp);
		delete pImageData;
		return NULL;
	}

	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

	for (int i = 0; i < pImageData->sizeY; i++)
	{
		// note that png is ordered top to
		// bottom, but OpenGL expect it bottom to top
		// so the order or swapped

		memcpy((void*)(pImageData->data+(row_bytes * i)), row_pointers[i], row_bytes);
		//memcpy((void*)(pImageData->data+(row_bytes * (pImageData->sizeY-1-i))), row_pointers[i], row_bytes);
	}

	/* Clean up after the read,
	 * and free any memory allocated */
	png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
	fclose(fp);

	return pImageData;
}

bool FindTexture(unsigned int &textureidx, const char* relative)
{
	char corrected[1024];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	for(int i=0; i<TEXTURES; i++)
	{
		Texture* t = &g_texture[i];

		if(t->loaded && _stricmp(t->filepath, corrected) == 0)
		{
			//g_texindex = i;
			//texture = t->texname;
			textureidx = i;
			g_texwidth = t->width;
			g_texheight = t->height;
			return true;
		}
	}

	return false;
}

void FreeTexture(const char* relative)
{
	char corrected[MAX_PATH+1];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	for(int i=0; i<TEXTURES; i++)
	{
		Texture* t = &g_texture[i];

		if(t->loaded && _stricmp(t->filepath, corrected) == 0)
		{
			t->loaded = false;
			glDeleteTextures(1, &t->texname);
			//g_log<<"Found texture "<<filepath<<" ("<<texture<<")"<<std::endl;
			return;
		}
	}
}

void FreeTexture(int i)
{
	Texture* t = &g_texture[i];

	if(t->loaded)
	{
		t->loaded = false;
		glDeleteTextures(1, &t->texname);
	}
}

int NewTexture()
{
	for(int i=0; i<TEXTURES; i++)
		if(!g_texture[i].loaded)
			return i;

	return -1;
}

bool TextureLoaded(unsigned int texture, const char* relative, bool transp, bool clamp, bool mipmaps, unsigned int& texindex, bool reload)
{
	char corrected[1024];
	strcpy(corrected, relative);
	CorrectSlashes(corrected);

	if(!reload)
	{
		texindex = NewTexture();

		if((int)texindex < 0)
		{
			texindex = 0;	// Give a harmless texture index
			return false;
		}
	}

	//g_texindex = texindex;
	Texture* t = &g_texture[texindex];
	t->loaded = true;
	strcpy(t->filepath, corrected);
	t->texname = texture;
	t->width = g_texwidth;
	t->height = g_texheight;
	t->transp = transp;
	t->clamp = clamp;
	t->mipmaps = mipmaps;

	t->sky = false;
	t->breakable = false;
	t->passthru = false;
	t->grate = false;
	t->ladder = false;
	t->water = false;
	t->fabric = false;

	if(strstr(corrected, "^"))
		t->sky = true;
	if(strstr(corrected, "!"))
		t->breakable = true;
	if(strstr(corrected, "~"))
		t->passthru = true;
	if(strstr(corrected, "`"))
		t->grate = true;
	if(strstr(corrected, "#"))
		t->ladder = true;
	if(strstr(corrected, "$"))
		t->water = true;
	if(strstr(corrected, "@"))
		t->fabric = true;

	return true;
}

void FreeTextures()
{
	for(int i=0; i<TEXTURES; i++)
	{
		if(!g_texture[i].loaded)
			continue;

		glDeleteTextures(1, &g_texture[i].texname);
		g_texture[i].loaded = false;	// Needed to reload textures. EDIT: not.
	}
}

void FindTextureExtension(char *relative)
{
	char strJPGPath[MAX_PATH] = {0};
	char strPNGPath[MAX_PATH] = {0};
	char strTGAPath[MAX_PATH]    = {0};
	char strBMPPath[MAX_PATH]    = {0};
	FILE *fp = NULL;

	//GetCurrentDirectory(MAX_PATH, strJPGPath);

	//strcat(strJPGPath, "\\");
	FullPath("", strJPGPath);

	strcat(strJPGPath, relative);
	strcpy(strTGAPath, strJPGPath);
	strcpy(strBMPPath, strTGAPath);
	strcpy(strPNGPath, strBMPPath);

	strcat(strJPGPath, ".jpg");
	strcat(strTGAPath, ".tga");
	strcat(strBMPPath, ".bmp");
	strcat(strPNGPath, ".png");

	if((fp = fopen(strJPGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".jpg");
		return;
	}

	if((fp = fopen(strPNGPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".png");
		return;
	}

	if((fp = fopen(strTGAPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".tga");
		return;
	}

	if((fp = fopen(strBMPPath, "rb")) != NULL)
	{
		fclose(fp);
		strcat(relative, ".bmp");
		return;
	}
}

bool Load1Texture()
{
	if(g_lastLTex+1 < g_texLoad.size())
		Status(g_texLoad[g_lastLTex+1].relative);

	CheckGLError(__FILE__, __LINE__);

	if(g_lastLTex >= 0)
	{
		TextureToLoad* t = &g_texLoad[g_lastLTex];
		if(t->reload)
			CreateTexture(t->texindex, t->relative, t->clamp, t->mipmaps, t->reload);
		else
			CreateTexture(*t->ptexindex, t->relative, t->clamp, t->mipmaps, t->reload);
	}

	g_lastLTex ++;

	if(g_lastLTex >= g_texLoad.size())
	{
		g_texLoad.clear();
		return false;	// Done loading all textures
	}

	return true;	// Not finished loading textures
}

void QueueTexture(unsigned int* texindex, const char* relative, bool clamp, bool mipmaps)
{
	TextureToLoad toLoad;
	toLoad.ptexindex = texindex;
	strcpy(toLoad.relative, relative);
	toLoad.clamp = clamp;
	toLoad.reload = false;
	toLoad.mipmaps = mipmaps;

	g_texLoad.push_back(toLoad);
}

void RequeueTexture(unsigned int texindex, const char* relative, bool clamp, bool mipmaps)
{
	TextureToLoad toLoad;
	toLoad.texindex = texindex;
	strcpy(toLoad.relative, relative);
	toLoad.clamp = clamp;
	toLoad.reload = true;
	toLoad.mipmaps = mipmaps;

	g_texLoad.push_back(toLoad);
}

void ReloadTextures()
{
	for(int i=0; i<TEXTURES; i++)
	{
		Texture* t = &g_texture[i];

		if(!t->loaded)
			continue;

		std::string rel;
		rel = MakePathRelative(t->filepath);
		RequeueTexture(i, rel.c_str(), t->clamp, t->mipmaps);
	}
}

LoadedTex* LoadTexture(const char* full)
{
	if(strstr(full, ".jpg"))
	{
		return LoadJPG(full);
		//return LoadJPG2(relative);
	}
	else if(strstr(full, ".png"))
	{
		return LoadPNG(full);
	}
	else if(strstr(full, ".tga"))
	{
		return LoadTGA(full);
	}
	else if(strstr(full, ".bmp"))
	{
		return LoadBMP(full);
	}

	return NULL;
}

bool CreateTexture(unsigned int &texindex, const char* relative, bool clamp, bool mipmaps, bool reload)
{
	CheckGLError(__FILE__, __LINE__);

	if(!relative)
		return false;

	if(!reload)
		if(FindTexture(texindex, relative))
			return true;

	// Define a pointer to a LoadedTex
	LoadedTex *pImage = NULL;

	char full[1024];
	FullPath(relative, full);

	pImage = LoadTexture(full);

	// Make sure valid image data was given to pImage, otherwise return false
	if(pImage == NULL)
	{
		g_log<<"Failed to load "<<relative<<std::endl;
		g_log.flush();

		if(!reload)
			texindex = 0;	// Give a harmless texture index

		return false;
	}

	unsigned int texname;
	// Generate a texture with the associative texture ID stored in the array
	glGenTextures(1, &texname);

	CheckGLError(__FILE__, __LINE__);
	// This sets the alignment requirements for the start of each pixel row in memory.
	glPixelStorei (GL_UNPACK_ALIGNMENT, 1);
	CheckGLError(__FILE__, __LINE__);

	// Bind the texture to the texture arrays index and init the texture
	glBindTexture(GL_TEXTURE_2D, texname);

	CheckGLError(__FILE__, __LINE__);
	// Assume that the texture is a 24 bit RGB texture (We convert 16-bit ones to 24-bit)
	int textureType = GL_RGB;
	bool transp = false;

	// If the image is 32-bit (4 channels), then we need to specify GL_RGBA for an alpha
	if(pImage->channels == 4)
	{
		textureType = GL_RGBA;
		transp = true;
	}


	CheckGLError(__FILE__, __LINE__);

#if 1

	//g_log<<"mipmaps:"<<(int)mipmaps<<" :"<<relative<<std::endl;

	if(mipmaps)
	{
		glEnable(GL_TEXTURE_2D);	// ATI fix
		// Option 1: with mipmaps

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);

		if(clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizeX, pImage->sizeY, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
		glGenerateMipmap(GL_TEXTURE_2D);

		CheckGLError(__FILE__, __LINE__);
	}
	else
	{
		// Option 2: without mipmaps
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		if(clamp)
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
		}
		else
		{
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		}

		glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizeX, pImage->sizeY, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);

		CheckGLError(__FILE__, __LINE__);
	}

	CheckGLError(__FILE__, __LINE__);
#else
	// Option 3: without mipmaps linear
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexImage2D(GL_TEXTURE_2D, 0, textureType, pImage->sizeX, pImage->sizeY, 0, textureType, GL_UNSIGNED_BYTE, pImage->data);
#endif

	if(pImage)
	{
		g_texwidth = pImage->sizeX;
		g_texheight = pImage->sizeY;

		delete pImage;								// Free the image structure

		g_log<<relative<<"\n\r";
		g_log.flush();
	}

	TextureLoaded(texname, relative, transp, clamp, mipmaps, texindex, reload);

	// Return a success
	return true;
}

void RequeueTextures()
{
	FreeTextures();

	for(int i=0; i<TEXTURES; i++)
	{
		if(g_texture[i].loaded)
			RequeueTexture(i, g_texture[i].filepath, g_texture[i].clamp, g_texture[i].mipmaps);
	}

	//LoadParticles();
	//LoadProjectiles();
	//LoadTerrainTextures();
	//LoadUnitSprites();
	//BSprites();
}

void DiffPath(const char* basepath, char* diffpath)
{
	strcpy(diffpath, basepath);
	//StripExtension(diffpath);
	strcat(diffpath, ".jpg");
}

void DiffPathPNG(const char* basepath, char* diffpath)
{
	strcpy(diffpath, basepath);
	//StripExtension(diffpath);
	strcat(diffpath, ".png");
}

void SpecPath(const char* basepath, char* specpath)
{
	strcpy(specpath, basepath);
	//StripExtension(specpath);
	strcat(specpath, ".spec.jpg");
}

void NormPath(const char* basepath, char* normpath)
{
	strcpy(normpath, basepath);
	//StripExtension(normpath);
	strcat(normpath, ".norm.jpg");
}

void OwnPath(const char* basepath, char* ownpath)
{
	strcpy(ownpath, basepath);
	//StripExtension(ownpath);
	strcat(ownpath, ".team.png");
}

void AllocTex(LoadedTex* empty, int width, int height, int channels)
{
	empty->data = (unsigned char*)malloc(width * height * channels * sizeof(unsigned char));
	empty->sizeX = width;
	empty->sizeY = height;
	empty->channels = channels;

	if(!empty->data)
	{
		OutOfMem(__FILE__, __LINE__);
	}
}

void Blit(LoadedTex* src, LoadedTex* dest, Vec2i pos)
{
	if(src == NULL || src->data == NULL)
		return;

	for(int x=0; x<src->sizeX; x++)
	{
		if(x+pos.x < 0)
			continue;

		if(x+pos.x >= dest->sizeX)
			continue;

		for(int y=0; y<src->sizeY; y++)
		{
			if(y+pos.y < 0)
				continue;

			if(y+pos.y >= dest->sizeY)
				continue;

			int srcpixel = x*src->channels + y*src->channels*src->sizeX;
			int destpixel = (x+pos.x)*dest->channels + (y+pos.y)*dest->channels*dest->sizeX;

			dest->data[destpixel + 0] = src->data[srcpixel + 0];
			dest->data[destpixel + 1] = src->data[srcpixel + 1];
			dest->data[destpixel + 2] = src->data[srcpixel + 2];

			if(dest->channels > 3)
			{
				if(src->channels <= 3)
					dest->data[destpixel + 3] = 255;
				else
					dest->data[destpixel + 3] = src->data[srcpixel + 3];
			}
		}
	}
}

void SaveJPEG(const char* fullpath, LoadedTex* image, float quality)
{
	FILE *outfile;
	if ((outfile = fopen(fullpath, "wb")) == NULL)
	{
		return;
	}

	struct jpeg_compress_struct cinfo;
	struct jpeg_error_mgr       jerr;

	cinfo.err = jpeg_std_error(&jerr);
	jpeg_create_compress(&cinfo);
	jpeg_stdio_dest(&cinfo, outfile);

	cinfo.image_width      = image->sizeX;
	cinfo.image_height     = image->sizeY;
	cinfo.input_components = 3;
	cinfo.in_color_space   = JCS_RGB;

	jpeg_set_defaults(&cinfo);
	/*set the quality [0..100]  */
	jpeg_set_quality (&cinfo, 100*quality, true);
	jpeg_start_compress(&cinfo, true);

	JSAMPROW row_pointer;
	int row_stride = image->sizeX * 3;

	while (cinfo.next_scanline < cinfo.image_height)
	{
		row_pointer = (JSAMPROW) &image->data[cinfo.next_scanline*row_stride];
		jpeg_write_scanlines(&cinfo, &row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);

	fclose(outfile);

	jpeg_destroy_compress(&cinfo);
}


int SavePNG(const char* fullpath, LoadedTex* image)
{
	FILE *fp;
	png_structp png_ptr;
	png_infop info_ptr;
	//png_colorp palette;

	/* Open the file */
	fp = fopen(fullpath, "wb");
	if (fp == NULL)
		return (ERROR);

	/* Create and initialize the png_struct with the desired error handler
	 * functions.  If you want to use the default stderr and longjump method,
	 * you can supply NULL for the last three parameters.  We also check that
	 * the library version is compatible with the one used at compile time,
	 * in case we are using dynamically linked libraries.  REQUIRED.
	 */
	png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING,
	                                  (png_voidp) NULL, NULL, NULL);

	if (png_ptr == NULL)
	{
		fclose(fp);
		return (ERROR);
	}

	/* Allocate/initialize the image information data.  REQUIRED */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL)
	{
		fclose(fp);
		png_destroy_write_struct(&png_ptr,  NULL);
		return (ERROR);
	}

	int color_type = PNG_COLOR_TYPE_RGB;

	if(image->channels == 4)
		color_type = PNG_COLOR_TYPE_RGBA;

	png_set_IHDR(png_ptr, info_ptr, image->sizeX, image->sizeY, 8, color_type, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

	/* Set error handling.  REQUIRED if you aren't supplying your own
	 * error handling functions in the png_create_write_struct() call.
	 */
	if (setjmp(png_jmpbuf(png_ptr)))
	{
		/* If we get here, we had a problem writing the file */
		fclose(fp);
		png_destroy_write_struct(&png_ptr, &info_ptr);
		return (ERROR);
	}

	/* One of the following I/O initialization functions is REQUIRED */

	/* Set up the output control if you are using standard C streams */
	png_init_io(png_ptr, fp);


	png_bytep* row_pointers = (png_bytep*) malloc(sizeof(png_bytep) * image->sizeY);

	if(!row_pointers)
	{
		OutOfMem(__FILE__, __LINE__);
		return NULL;
	}

	for (int y=0; y<image->sizeY; y++)
		row_pointers[y] = (png_byte*)&image->data[y*image->sizeX*image->channels];

	png_write_info(png_ptr, info_ptr);
	png_write_image(png_ptr, row_pointers);
	png_write_end(png_ptr, NULL);

	//for (y=0; y<image->sizeY; y++)
	//   free(row_pointers[y]);
	free(row_pointers);


	/* This is the easy way.  Use it if you already have all the
	 * image info living in the structure.  You could "|" many
	 * PNG_TRANSFORM flags into the png_transforms integer here.
	 */
	//png_write_png(png_ptr, info_ptr, NULL, NULL);

	/* If you png_malloced a palette, free it here (don't free info_ptr->palette,
	 * as recommended in versions 1.0.5m and earlier of this example; if
	 * libpng mallocs info_ptr->palette, libpng will free it).  If you
	 * allocated it with malloc() instead of png_malloc(), use free() instead
	 * of png_free().
	 */
	//png_free(png_ptr, palette);
	//palette = NULL;

	/* Similarly, if you png_malloced any data that you passed in with
	 * png_set_something(), such as a hist or trans array, free it here,
	 * when you can be sure that libpng is through with it.
	 */
	//png_free(png_ptr, trans);
	//trans = NULL;
	/* Whenever you use png_free() it is a good idea to set the pointer to
	 * NULL in case your application inadvertently tries to png_free() it
	 * again.  When png_free() sees a NULL it returns without action, thus
	 * avoiding the double-free security problem.
	 */

	/* Clean up after the write, and free any memory allocated */
	png_destroy_write_struct(&png_ptr, &info_ptr);

	/* Close the file */
	fclose(fp);

	/* That's it */
	return (1);
}

void FlipImage(LoadedTex* image)
{
	int x;
	int y2;
	byte temp[4];
	int stride = image->sizeX * image->channels;

	for(int y=0; y<image->sizeY/2; y++)
	{
		y2 = image->sizeY - y - 1;

		unsigned char *pLine = &(image->data[stride * y]);
		unsigned char *pLine2 = &(image->data[stride * y2]);

		for(x = 0; x < image->sizeX * image->channels; x += image->channels)
		{
			temp[0] = pLine[x + 0];
			temp[1] = pLine[x + 1];
			temp[2] = pLine[x + 2];
			if(image->channels == 4)
				temp[3] = pLine[x + 3];

			pLine[x + 0] = pLine2[x + 0];
			pLine[x + 1] = pLine2[x + 1];
			pLine[x + 2] = pLine2[x + 2];
			if(image->channels == 4)
				pLine[x + 3] = pLine2[x + 3];

			pLine2[x + 0] = temp[0];
			pLine2[x + 1] = temp[1];
			pLine2[x + 2] = temp[2];
			if(image->channels == 4)
				pLine2[x + 3] = temp[3];
		}
	}
}

// Warning: this function modifies the image data to switch RGB to BGR
int SaveBMP(const char* fullpath, LoadedTex* image)
{
#ifdef PLATFORM_WIN
	FILE* filePtr;
	BITMAPFILEHEADER bitmapFileHeader;
	BITMAPINFOHEADER bitmapInfoHeader;
	int imageIdx;
	unsigned char tempRGB;

	filePtr = fopen(fullpath, "wb");
	if(!filePtr)
		return 0;

	bitmapFileHeader.bfSize = sizeof(BITMAPFILEHEADER);
	bitmapFileHeader.bfType = 0x4D42;
	bitmapFileHeader.bfReserved1 = 0;
	bitmapFileHeader.bfReserved2 = 0;
	bitmapFileHeader.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

	bitmapInfoHeader.biSize = sizeof(BITMAPINFOHEADER);
	bitmapInfoHeader.biPlanes = 1;
	bitmapInfoHeader.biBitCount = 24;
	bitmapInfoHeader.biCompression = BI_RGB;
	bitmapInfoHeader.biSizeImage = image->sizeX * abs(image->sizeY) * 3;
	bitmapInfoHeader.biXPelsPerMeter = 0;
	bitmapInfoHeader.biYPelsPerMeter = 0;
	bitmapInfoHeader.biClrUsed = 0;
	bitmapInfoHeader.biClrImportant = 0;
	bitmapInfoHeader.biWidth = image->sizeX;
	bitmapInfoHeader.biHeight = image->sizeY;

	for(imageIdx = 0; imageIdx < bitmapInfoHeader.biSizeImage; imageIdx += 3)
	{
		tempRGB = image->data[imageIdx];
		image->data[imageIdx] = image->data[imageIdx + 2];
		image->data[imageIdx + 2] = tempRGB;
	}

#if 0
	for(int imageIdx = image->channels * (image->sizeX - 1);
	                imageIdx < image->sizeX * image->sizeY * image->channels;
	                imageIdx += image->channels * image->sizeX)
	{
		image->data[imageIdx] = image->data[imageIdx-3];
		image->data[imageIdx+1] = image->data[imageIdx-2];
		image->data[imageIdx+2] = image->data[imageIdx-1];
	}
#endif

	fwrite(&bitmapFileHeader, 1, sizeof(BITMAPFILEHEADER), filePtr);

	fwrite(&bitmapInfoHeader, 1, sizeof(BITMAPINFOHEADER), filePtr);

	fwrite(image->data, 1, bitmapInfoHeader.biSizeImage, filePtr);

	fclose(filePtr);

#endif // PLATFORM_WIN
	return 1;
}


bool SaveRAW(const char* fullpath, LoadedTex* image)
{
	FILE* fp = fopen(fullpath, "wb");

	fwrite(image->data, image->sizeX*image->sizeY*image->channels, 1, fp);

	fclose(fp);

	return true;
}

void StreamRaw(FILE* fp, unsigned int* texname, Vec2i fullsz, Vec2i srcpos, Vec2i srcsz, Vec2i destsz)
{
	if(srcpos.x < 0)
		srcpos.x = 0;
	if(srcpos.y < 0)
		srcpos.y = 0;
	if(srcpos.x + srcsz.x > fullsz.x)
		srcsz.x = fullsz.x - srcpos.x;
	if(srcpos.y + srcsz.y > fullsz.y)
		srcsz.y = fullsz.y - srcpos.y;

	int stridex = srcsz.x / destsz.x * 3;
	int stridey = srcsz.y / destsz.y * 3 * fullsz.x;

	LoadedTex newtex;
	AllocTex(&newtex, destsz.x, destsz.y, 3);

	int i = 0;

	for(int y=srcpos.y; srcpos.y < srcpos.y+srcsz.y; y+=stridey)
		for(int x=srcpos.x; srcpos.x < srcpos.x+srcsz.x; x+=stridex)
		{
			fseek(fp, y * 3 * fullsz.x + x * 3, SEEK_SET);
			fread(&newtex.data[i], 3, 1, fp);
			i+=3;
		}

	newtex.destroy();
}

void Resample(LoadedTex* original, LoadedTex* empty, Vec2i newdim)
{
#ifdef COMPILEB_DEBUG
	g_log<<"resample...?"<<std::endl;
	g_log.flush();
#endif

	if(original == NULL || original->data == NULL || original->sizeX <= 0 || original->sizeY <= 0)
	{
#ifdef COMPILEB_DEBUG
		g_log<<"resample NULL 1"<<std::endl;
		g_log.flush();
#endif

		empty->data = NULL;

#ifdef COMPILEB_DEBUG
		g_log<<"resample NULL 2"<<std::endl;
		g_log.flush();
#endif

		empty->sizeX = 0;
		empty->sizeY = 0;

		if(original != NULL)
			empty->channels = original->channels;

		return;
	}

#ifdef COMPILEB_DEBUG
	g_log<<"resample "<<original->sizeX<<","<<original->sizeY<<" to "<<newdim.x<<","<<newdim.y<<std::endl;
	g_log.flush();
#endif

	AllocTex(empty, newdim.x, newdim.y, original->channels);

	double scaleW =  (double)newdim.x / (double)original->sizeX;
	double scaleH = (double)newdim.y / (double)original->sizeY;

	for(int cy = 0; cy < newdim.y; cy++)
	{
		for(int cx = 0; cx < newdim.x; cx++)
		{
			int pixel = cy * (newdim.x * original->channels) + cx*original->channels;
			int nearestMatch =  (int)(cy / scaleH) * original->sizeX * original->channels + (int)(cx / scaleW) * original->channels;

			empty->data[pixel    ] =  original->data[nearestMatch    ];
			empty->data[pixel + 1] =  original->data[nearestMatch + 1];
			empty->data[pixel + 2] =  original->data[nearestMatch + 2];

			if(original->channels > 3)
				empty->data[pixel + 3] =  original->data[nearestMatch + 3];
		}
	}

#ifdef COMPILEB_DEBUG
	g_log<<"\t done resample"<<std::endl;
	g_log.flush();
#endif
}