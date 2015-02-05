
#include "compilebl.h"
#include "../utils.h"
#include "../platform.h"
#include "../texture.h"
#include "vertexarray.h"

//#define COMPILEB_DEBUG

TexFitRow::TexFitRow()
{
	bounds[0].x = 0;
	bounds[0].y = 0;
	bounds[1].x = 0;
	bounds[1].y = 0;
}

TexFitInfo::TexFitInfo()
{
	tiletimes.x = 0;
	tiletimes.y = 0;
}

// get a power of 2 number that is big enough to hold 'lowerbound' but does not exceed 2048
int Max2Pow(int lowerbound)
{
	int twopow = 2;

	while( twopow < lowerbound && twopow < 2048 )
		twopow *= 2;

	return twopow;
}

// determine how many times a triangle tiles on a texture
Vec2i TileTimes(Vec2f* tc)
{
	Vec2f tc0 = tc[0];
	Vec2f tc1 = tc[1];
	Vec2f tc2 = tc[2];

	float minu = imin(tc0.x, imin(tc1.x, tc2.x));
	float minv = imin(tc0.y, imin(tc1.y, tc2.y));
	float maxu = imax(tc0.x, imax(tc1.x, tc2.x));
	float maxv = imax(tc0.y, imax(tc1.y, tc2.y));

	float rangeu = maxu - minu;
	float rangev = maxv - minv;

	float offu = minu - floor(minu);
	float offv = minv - floor(minv);

	float ceilrangeu = ceil(rangeu + offu);
	float ceilrangev = ceil(rangev + offv);

	/*
				float rangeu = maxu - minu;
				float rangev = maxv - minv;

				float offu = minu - floor(minu);
				float offv = minv - floor(minv);
				*/

	Vec2i tiletimes(ceilrangeu, ceilrangev);

#ifdef COMPILEB_DEBUG
	if(tiletimes.x == 11 || tiletimes.y == 11)
	{
		g_log<<"--------------11 or 11 tile times---------------"<<endl;
		g_log<<"ceilrange = "<<ceilrangeu<<","<<ceilrangev<<endl;
		g_log<<"range = "<<rangeu<<","<<rangev<<endl;
		g_log<<"std::min = "<<minu<<","<<minv<<endl;
		g_log<<"std::max = "<<maxu<<","<<maxv<<endl;
		g_log<<"[0] = "<<tc0.x<<","<<tc0.y<<endl;
		g_log<<"[1] = "<<tc1.x<<","<<tc1.y<<endl;
		g_log<<"[2] = "<<tc2.x<<","<<tc2.y<<endl;
		g_log<<"--------------11 or 11 tile times---------------"<<endl;
		g_log.flush();
	}
#endif

	return tiletimes;
}

// determine how many times a collection of triangles tiles on a texture
Vec2i TileTimes(VertexArray* va)
{
	Vec2i maxtile(0,0);

	for(int j=0; j<va->numverts; j+=3)
	{
		Vec2i tritile = TileTimes(&va->texcoords[j]);

		if(tritile.x > maxtile.x)
			maxtile.x = tritile.x;
		if(tritile.y > maxtile.y)
			maxtile.y = tritile.y;
	}

	return maxtile;
}

#if 0
void CompileBuilding(const char* fullfile, EdBuilding* bldg)
{
#ifdef COMPILEB_DEBUG
	g_log<<"compile "<<fullfile<<endl;
	g_log.flush();
#endif

	char basename[MAX_PATH+1];
	strcpy(basename, fullfile);
	StripPath(basename);
	StripExtension(basename);

	std::string fullpath = StripFile(fullfile);	// + CORRECT_SLASH;

#ifdef COMPILEB_DEBUG
	g_log<<"FULLPATH STRIPPED "<<fullpath<<endl;
	g_log.flush();
#endif

	std::string difffull = fullpath + std::string(basename) + ".jpg";
	std::string difffullpng = fullpath + std::string(basename) + ".png";
	std::string specfull = fullpath + std::string(basename) + ".spec.jpg";
	std::string normfull = fullpath + std::string(basename) + ".norm.jpg";
	std::string ownfull = fullpath + std::string(basename) + ".own.png";

	//std::string diffpath = std::string(basename) + ".jpg";
	//std::string diffpathpng = std::string(basename) + ".png";
	//std::string specpath = std::string(basename) + ".spec.jpg";
	//std::string normpath = std::string(basename) + ".norm.jpg";

	unsigned int notexindex;
	CreateTexture(notexindex, "textures/notex.jpg", false);

	bool transparency = false;

	// STEP 1:
	// i. count total triangles, excluding sides with default (notex.jpg) texture (they will not be added to the final building)
	// ii. make a std::list of unique textures used in the brush sides and see if there are any transparent ones
	// iii. calculate the std::max x,y tile extents of the texture coordinates for each triangles

	int ntris = 0;
	std::list<unsigned int> uniquetexs;
	TexFitInfo texfitinfo[TEXTURES];

#ifdef COMPILEB_DEBUG
	g_log<<"compile 1"<<endl;
	g_log.flush();
#endif

	for(auto br=bldg->m_brush.begin(); br!=bldg->m_brush.end(); br++)
	{
		for(int i=0; i<br->m_nsides; i++)
		{
			EdBrushSide* s = &br->m_sides[i];

			if(s->m_diffusem == notexindex)
				continue;

			VertexArray* va = &s->m_drawva;
			ntris += va->numverts / 3;

			bool found = false;

			auto ut=uniquetexs.begin();
			for(; ut!=uniquetexs.end(); ut++)
			{
				TexFitInfo* tfi = &texfitinfo[ *ut ];

				if(*ut == s->m_diffusem)
				{
					Vec2i tiletimes = TileTimes(va);

					if(tiletimes.x > tfi->tiletimes.x)
						tfi->tiletimes.x = tiletimes.x;
					if(tiletimes.y > tfi->tiletimes.y)
						tfi->tiletimes.y = tiletimes.y;

					found = true;
					break;
				}
			}

			if(!found)
			{
				uniquetexs.push_back(s->m_diffusem);

				TexFitInfo* tfi = &texfitinfo[ s->m_diffusem ];
				Vec2i tiletimes = TileTimes(va);
				TexFitInfo newtfi;
				tfi->tiletimes.x = tiletimes.x;
				tfi->tiletimes.y = tiletimes.y;

				if(g_texture[ s->m_diffusem ].transp)
					transparency = true;
			}
		}
	}


#ifdef COMPILEB_DEBUG
	g_log<<"compile 2"<<endl;
	g_log.flush();
#endif

	// STEP 2.
	// i. allocate the final array of all the vertices/texcoords/normals that will be written to file.
	// ii. store the vertices for now and normals for now. texcoords will be calculated later when the texture images are packed together.

	VertexArray fullva;
	fullva.alloc(ntris*3);

	int vindex = 0;

	for(auto br=bldg->m_brush.begin(); br!=bldg->m_brush.end(); br++)
	{
		for(int i=0; i<br->m_nsides; i++)
		{
			EdBrushSide* s = &br->m_sides[i];

			if(s->m_diffusem == notexindex)
				continue;

			VertexArray* va = &s->m_drawva;

			for(int j=0; j<va->numverts; j++)
			{
				fullva.vertices[vindex] = va->vertices[j];
				fullva.normals[vindex] = va->normals[j];
				vindex++;
			}
		}
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 3"<<endl;
	g_log.flush();
#endif

	// STEP 3.
	// i. load all the diffuse, specular, and normal texture images (RGB data) from the compiled std::list of unique texture indices
	// ii. save references to the texture images (RGB data) array based on the unique (diffuse) texture index

	TexRef texref[TEXTURES];

	LoadedTex** images = new LoadedTex*[uniquetexs.size()*TEX_PER_BL];

	int i=0;
	for(auto ut=uniquetexs.begin(); ut!=uniquetexs.end(); ut++, i++)
	{
		char fulldiffusepath[MAX_PATH+1];
		Texture* diffusetex = &g_texture[ *ut ];
		FullPath(diffusetex->filepath, fulldiffusepath);
		images[i] = LoadTexture(fulldiffusepath);
		TexRef* tr = &texref[ *ut ];
#ifdef COMPILEB_DEBUG
		tr->filepath = diffusetex->filepath;
#endif
		tr->diffindex = i;

#ifdef COMPILEB_DEBUG
		TexFitInfo* tfi = &texfitinfo[ *ut ];
		g_log<<"tile times "<<texref[ *ut ].filepath<<" "<<tfi->tiletimes.x<<","<<tfi->tiletimes.y<<endl;
		g_log.flush();
#endif

		i++;

		char basepath[MAX_PATH+1];
		strcpy(basepath, fulldiffusepath);
		StripExtension(basepath);

		char fullspecularpath[MAX_PATH+1];
		SpecPath(basepath, fullspecularpath);
		images[i] = LoadTexture(fullspecularpath);
		tr->specindex = i;

		i++;

		char fullnormalpath[MAX_PATH+1];
		NormPath(basepath, fullnormalpath);
		images[i] = LoadTexture(fullnormalpath);
		tr->normindex = i;

		i++;

		char fullownerpath[MAX_PATH+1];
		OwnPath(basepath, fullownerpath);
		images[i] = LoadTexture(fullownerpath);
		tr->ownindex = i;

#ifdef COMPILEB_DEBUG
		g_log<<"------"<<endl;
		if(images[tr->diffindex])
			g_log<<fulldiffusepath<<" ("<<images[tr->diffindex]->sizeX<<","<<images[tr->diffindex]->sizeY<<")"<<endl;
		if(images[tr->specindex])
			g_log<<fullspecularpath<<" ("<<images[tr->specindex]->sizeX<<","<<images[tr->specindex]->sizeY<<")"<<endl;
		if(images[tr->normindex])
			g_log<<fullnormalpath<<" ("<<images[tr->normindex]->sizeX<<","<<images[tr->normindex]->sizeY<<")"<<endl;
		g_log<<i<<endl;
		g_log<<"------"<<endl;
		g_log.flush();
#endif
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 4"<<endl;
	g_log.flush();
#endif

	// STEP 4.
	// i. make a std::list of images sorted by height from tallest to shortest

	std::list<unsigned int> heightsorted;
	int nextadd;
	int lastheight = 0;

	for(int i=0; i<uniquetexs.size(); i++)
	{
		nextadd = -1;
		lastheight = 0;

		int j=0;
		auto ut=uniquetexs.begin();
		for(; ut!=uniquetexs.end(); ut++, j++)
		{
			bool found = false;

			for(auto hs=heightsorted.begin(); hs!=heightsorted.end(); hs++)
			{
				if(*hs == *ut)
				{
					found = true;
					break;
				}
			}

			if(found)
				continue;

			TexRef* tr = &texref[ *ut ];
			LoadedTex* lt = images[ tr->diffindex ];
			TexFitInfo* tfi = &texfitinfo[ *ut ];
			int thisheight = 0;
			if(lt != NULL)	// image might not have loaded
				thisheight = lt->sizeY * tfi->tiletimes.y;

			if(thisheight >= lastheight || nextadd < 0)
			{
#ifdef COMPILEB_DEBUG
				g_log<<thisheight<<" >= "<<lastheight<<endl;
				g_log.flush();
#endif

				lastheight = thisheight;
				nextadd = (int)*ut;
			}
		}

		if(nextadd < 0)
			continue;	//shouldn't happen

		heightsorted.push_back((unsigned int)nextadd);
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 5"<<endl;
	g_log.flush();
#endif

	// STEP 5.
	// i. add textures to the right side of the row until the width of the final image is twice its height
	// ii. then increase the height and add a new row
	// iii. for each next image, check each row if the image can be added without increasing the image width
	// iii. repeat until all diffuse images have been fitted in
	// iv. calculate the scaled, translated texture coordinates for the final vertex array

	Vec2i maxdim(0, 0);
	std::list<TexFitRow> rows;

	for(auto hs=heightsorted.begin(); hs!=heightsorted.end(); hs++)
	{
		TexRef* tr = &texref[ *hs ];
		TexFitInfo* tfi = &texfitinfo[ *hs ];

		LoadedTex* lt = images[ tr->diffindex ];

#ifdef COMPILEB_DEBUG
		g_log<<"------height sorted-----"<<endl;
		g_log<<"\t "<<tr->filepath<<endl;
		g_log<<"\t ("<<lt->sizeX<<","<<lt->sizeY<<")"<<endl;
		g_log<<"------height sorted-----"<<endl;
		g_log.flush();
#endif

		//if(maxdim.x < lt->sizeX)
		//	maxdim.x = lt->sizeX;
		//if(maxdim.y < lt->sizeY)
		//	maxdim.y = lt->sizeY;

		// Add first image row.
		if(rows.size() <= 0)
		{
			rows.push_back(TexFitRow());
			TexFitRow* firstrow = &*rows.begin();
			TexFit fit;
			fit.bounds[0].x = 0;
			fit.bounds[0].y = 0;
			fit.bounds[1].x = lt->sizeX * tfi->tiletimes.x;
			fit.bounds[1].y = lt->sizeY * tfi->tiletimes.y;
			fit.texindex = *hs;
			memcpy(tfi->bounds, fit.bounds, sizeof(Vec2i)*2);

			firstrow->fits.push_back(fit);
			firstrow->bounds[0].x = 0;
			firstrow->bounds[0].y = 0;
			firstrow->bounds[1].x = lt->sizeX * tfi->tiletimes.x;
			firstrow->bounds[1].y = lt->sizeY * tfi->tiletimes.y;

			maxdim.x += lt->sizeX * tfi->tiletimes.x;
			maxdim.y += lt->sizeY * tfi->tiletimes.y;

#ifdef COMPILEB_DEBUG
			g_log<<"\tfit1 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<endl;
			g_log.flush();
#endif

			continue;
		}

		TexFitRow* addto = NULL;
		int leftest = 0;

		for(auto row=rows.begin(); row!=rows.end(); row++)
		{
			// If addto == NULL, adding it to the first row will be high enough.
			// Otherwise, we have to check if that row is tall enough for the image.
			// We want to fit on a row as far left as possible.
			if((row->bounds[1].x < leftest && row->bounds[1].y - row->bounds[0].y >= lt->sizeY * tfi->tiletimes.y) || addto == NULL)
			{
				addto = &*row;
				leftest = row->bounds[1].x;
			}
		}

		// If adding it on a row as far left as possible is still making the image more than twice as wide as it is tall,
		// add a new row, and put the image on that row.

		if(leftest + lt->sizeX > maxdim.y * 2)
		{
			TexFitRow* lastrow = &*rows.rbegin();
			TexFitRow row;

			row.bounds[0].x = 0;
			row.bounds[0].y = lastrow->bounds[1].y;
			row.bounds[1].x = lt->sizeX * tfi->tiletimes.x;
			row.bounds[1].y = row.bounds[0].y + lt->sizeY * tfi->tiletimes.y;
			maxdim.y = row.bounds[1].y;

			TexFit fit;
			fit.bounds[0].x = row.bounds[0].x;
			fit.bounds[0].y = row.bounds[0].y;
			fit.bounds[1].x = row.bounds[1].x;
			fit.bounds[1].y = row.bounds[1].y;
			fit.texindex = *hs;
			memcpy(tfi->bounds, fit.bounds, sizeof(Vec2i)*2);

			if(fit.bounds[1].x > maxdim.x)
				maxdim.x = fit.bounds[1].x;

			row.fits.push_back(fit);
			rows.push_back(row);

#ifdef COMPILEB_DEBUG
			g_log<<"\tfit2 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<endl;
			g_log.flush();
#endif

			continue;
		}

		// Added it on to the right of the row.

		TexFit fit;
		fit.bounds[0].x = addto->bounds[1].x;
		fit.bounds[0].y = addto->bounds[0].y;
		fit.bounds[1].x = fit.bounds[0].x + lt->sizeX * tfi->tiletimes.x;
		fit.bounds[1].y = addto->bounds[0].y + lt->sizeY * tfi->tiletimes.y;
		fit.texindex = *hs;
		memcpy(tfi->bounds, fit.bounds, sizeof(Vec2i)*2);

		addto->fits.push_back(fit);
		addto->bounds[1].x += lt->sizeX * tfi->tiletimes.x;

		if(addto->bounds[1].x > maxdim.x)
			maxdim.x = addto->bounds[1].x;

#ifdef COMPILEB_DEBUG
		g_log<<"\tfit3 to "<<fit.bounds[0].x<<","<<fit.bounds[0].y<<"->"<<fit.bounds[1].x<<","<<fit.bounds[1].y<<endl;
		g_log.flush();
#endif
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 6"<<endl;
	g_log.flush();
#endif

	// STEP 6.
	// i. resize images so that the combined final image is a power of 2 that doesn't exceed 2048x2048

	Vec2i finaldim( Max2Pow( maxdim.x ), Max2Pow( maxdim.y ) );
	float scalex = (float)finaldim.x / (float)maxdim.x;
	float scaley = (float)finaldim.y / (float)maxdim.y;

#ifdef COMPILEB_DEBUG
	g_log<<"dimensions = ("<<maxdim.x<<","<<maxdim.y<<")->("<<finaldim.x<<","<<finaldim.y<<")"<<endl;
	g_log.flush();
#endif

	LoadedTex* resizedimages = new LoadedTex[ uniquetexs.size()*TEX_PER_BL ];

	for(i=0; i<uniquetexs.size()*TEX_PER_BL; i++)
		resizedimages[i].data = NULL;

#ifdef COMPILEB_DEBUG
	int counter = 0;
#endif

	for(auto r=rows.begin(); r!=rows.end(); r++)
	{
		for(auto f=r->fits.begin(); f!=r->fits.end(); f++)
		{
			TexRef* tr = &texref[ f->texindex ];
			TexFitInfo* tfi = &texfitinfo[ f->texindex ];

			tfi->newdim.x = (f->bounds[1].x - f->bounds[0].x) / (float)tfi->tiletimes.x * scalex;
			tfi->newdim.y = (f->bounds[1].y - f->bounds[0].y) / (float)tfi->tiletimes.y * scaley;

#ifdef COMPILEB_DEBUG
			g_log<<"compile 6 image ("<<(f->bounds[1].x - f->bounds[0].x)<<","<<(f->bounds[1].y - f->bounds[0].y)<<")->("<<tfi->newdim.x<<","<<tfi->newdim.y<<")"<<counter<<endl;
			g_log.flush();
			counter ++;
#endif

			Vec2i newdim( ceil(tfi->newdim.x), ceil(tfi->newdim.y) );

			Resample(images[ tr->diffindex ], &resizedimages[ tr->diffindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_log<<"\t done compile 6 image diff"<<endl;
			g_log.flush();
#endif

			Resample(images[ tr->specindex ], &resizedimages[ tr->specindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_log<<"\t done compile 6 image spec"<<endl;
			g_log.flush();
#endif

			Resample(images[ tr->normindex ], &resizedimages[ tr->normindex ], newdim);

#ifdef COMPILEB_DEBUG
			g_log<<"\t done compile 6 image norm"<<endl;
			g_log.flush();
#endif

			Resample(images[ tr->ownindex ], &resizedimages[ tr->ownindex ], newdim);
		}
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7"<<endl;
	g_log.flush();
#endif

	// STEP 7.
	// i. combine all the resized images into the final image

	LoadedTex finaldiff;
	LoadedTex finalspec;
	LoadedTex finalnorm;
	LoadedTex finalown;

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7a"<<endl;
	g_log.flush();
#endif

	AllocTex(&finaldiff, finaldim.x, finaldim.y, transparency ? 4 : 3);
	AllocTex(&finalspec, finaldim.x, finaldim.y, 3);
	AllocTex(&finalnorm, finaldim.x, finaldim.y, 3);
	AllocTex(&finalown, finaldim.x, finaldim.y, 4);

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7b"<<endl;
	g_log.flush();
#endif

	for(auto r=rows.begin(); r!=rows.end(); r++)
	{
		for(auto f=r->fits.begin(); f!=r->fits.end(); f++)
		{
			TexRef* tr = &texref[ f->texindex ];
			TexFitInfo* tfi = &texfitinfo[ f->texindex ];

			tfi->bounds[0] = f->bounds[0];
			tfi->bounds[1] = f->bounds[2];

#ifdef COMPILEB_DEBUG
			g_log<<"compile 7b1"<<endl;
			g_log.flush();
#endif

			for(int x=0; x<tfi->tiletimes.x; x++)
				for(int y=0; y<tfi->tiletimes.y; y++)
				{
					Vec2i newpos;
					newpos.x = f->bounds[0].x * scalex + x * tfi->newdim.x;
					newpos.y = f->bounds[0].y * scaley + y * tfi->newdim.y;

#ifdef COMPILEB_DEBUG
					g_log<<"compile 7b2 to "<<newpos.x<<","<<newpos.y<<endl;
					g_log.flush();
#endif

					Blit( &resizedimages[ tr->diffindex ], &finaldiff, newpos );
					Blit( &resizedimages[ tr->specindex ], &finalspec, newpos );
					Blit( &resizedimages[ tr->normindex ], &finalnorm, newpos );
					Blit( &resizedimages[ tr->ownindex ], &finalown, newpos );
				}
#ifdef COMPILEB_DEBUG
			g_log<<"compile 7b3"<<endl;
			g_log.flush();
#endif
		}
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7c"<<endl;
	g_log.flush();
#endif

	for(i=0; i<uniquetexs.size()*TEX_PER_BL; i++)
	{

#ifdef COMPILEB_DEBUG
		g_log<<"compile 7c free image "<<i<<endl;
		g_log.flush();
#endif

		if(resizedimages[i].data)
			free(resizedimages[i].data);
	}

#ifdef COMPILEB_DEBUG
	g_log<<"compile 7d"<<endl;
	g_log.flush();
#endif

	delete [] resizedimages;

#ifdef COMPILEB_DEBUG
	g_log<<"compile 8"<<endl;
	g_log.flush();
#endif

	// STEP 8.
	// i. write the JPEG's/PNG's

	if(transparency)
		SavePNG(difffullpng.c_str(), &finaldiff);
	else
		SaveJPEG(difffull.c_str(), &finaldiff, 0.5f);
	SaveJPEG(specfull.c_str(), &finalspec, 0.5f);
	SaveJPEG(normfull.c_str(), &finalnorm, 0.5f);
	SavePNG(ownfull.c_str(), &finalown);

	//SavePNG(difffullpng.c_str(), &finaldiff);
	//SavePNG(specfull.c_str(), &finalspec);
	//SavePNG(normfull.c_str(), &finalnorm);

	// STEP 9.
	// i. calculate the new texcoords
	// ii. write the vertex array

	vindex = 0;

#ifdef COMPILEB_DEBUG
	unsigned int window3index;
	CreateTextureI(window3index, "textures/apartment/stuccowindow3.jpg", false);
#endif

	for(auto br=bldg->m_brush.begin(); br!=bldg->m_brush.end(); br++)
	{
		for(int i=0; i<br->m_nsides; i++)
		{
			EdBrushSide* s = &br->m_sides[i];

			if(s->m_diffusem == notexindex)
				continue;

			TexRef* tr = &texref[ s->m_diffusem ];
			LoadedTex* lt = images[ tr->diffindex ];
			TexFitInfo* tfi = &texfitinfo[ s->m_diffusem ];
			VertexArray* va = &s->m_drawva;

			for(int j=0; j<va->numverts; j+=3, vindex+=3)
			{
				//fullva.vertices[vindex] = va->vertices[j];
				//fullva.normals[vindex] = va->normals[j];

				Vec2f tc[3];
				tc[0] = va->texcoords[j + 0];
				tc[1] = va->texcoords[j + 1];
				tc[2] = va->texcoords[j + 2];

				float minu = std::min(tc[0].x, std::min(tc[1].x, tc[2].x));
				float minv = std::min(tc[0].y, std::min(tc[1].y, tc[2].y));
				float maxu = std::max(tc[0].x, std::max(tc[1].x, tc[2].x));
				float maxv = std::max(tc[0].y, std::max(tc[1].y, tc[2].y));

				float rangeu = maxu - minu;
				float rangev = maxv - minv;

				//float offu = minu - floor(minu+0.05f);
				//float offv = minv - floor(minv+0.05f);
				float offu = minu - floor(minu);
				float offv = minv - floor(minv);

				/*
				double pixelsrangeu = rangeu * tfi->newdim.x;
				double pixelsrangev = rangev * tfi->newdim.y;

				int availablerangeu = tfi->newdim.x * tfi->tiletimes.x;
				int availablerangev = tfi->newdim.y * tfi->tiletimes.y;

				//float scalerangeu = pixelsrangeu / (double)availablerangeu;
				//float scalerangev = pixelsrangev / (double)availablerangev;

				float scalerangeu = rangeu * (pixelsrangeu / (double)availablerangeu) * ((float)availablerangeu / (float)finaldim.x);
				float scalerangev = rangev * (pixelsrangev / (double)availablerangev) * ((float)availablerangev / (float)finaldim.y);

				float tcleft = (float)tfi->bounds[0].x / (float)maxdim.x;
				float tctop = (float)tfi->bounds[0].y / (float)maxdim.y;

				fullva.texcoords[vindex + 0] = Vec2f( tcleft + (offu + tc0.x - minu) * scalerangeu, tctop + (offv + tc0.y - minv) * scalerangev );
				fullva.texcoords[vindex + 1] = Vec2f( tcleft + (offu + tc1.x - minu) * scalerangeu, tctop + (offv + tc1.y - minv) * scalerangev );
				fullva.texcoords[vindex + 2] = Vec2f( tcleft + (offu + tc2.x - minu) * scalerangeu, tctop + (offv + tc2.y - minv) * scalerangev );
				*/
				//fullva.texcoords[vindex + 0] = tc0;
				//fullva.texcoords[vindex + 1] = tc1;
				//fullva.texcoords[vindex + 2] = tc2;

				for(int k=0; k<3; k++)
				{
					float uvpixel[2];

					uvpixel[0] = tc[k].x * (float)lt->sizeX;
					uvpixel[1] = tc[k].y * (float)lt->sizeY;

					uvpixel[0] = uvpixel[0] - minu*(float)lt->sizeX + offu*(float)lt->sizeX;
					uvpixel[1] = uvpixel[1] - minv*(float)lt->sizeY + offv*(float)lt->sizeY;

					uvpixel[0] += tfi->bounds[0].x;
					uvpixel[1] += tfi->bounds[0].y;

					fullva.texcoords[vindex + k].x = uvpixel[0] / (float)maxdim.x;
					fullva.texcoords[vindex + k].y = uvpixel[1] / (float)maxdim.y;

#ifdef COMPILEB_DEBUG
					if(s->m_diffusem == window3index)
					{
						g_log<<"-----window 3 face---------"<<endl;
						g_log<<"minu,v = "<<minu<<","<<minv<<endl;
						g_log<<"offu,v = "<<offu<<","<<offv<<endl;
						g_log<<"uv = "<<fullva.texcoords[vindex + k].x<<","<<fullva.texcoords[vindex + k].y<<endl;
						g_log<<"uvpixel = "<<uvpixel[0]*scalex<<","<<uvpixel[1]*scaley<<endl;
						g_log<<"-----window 3 face---------"<<endl;
						g_log.flush();
					}
#endif
				}

#ifdef COMPILEB_DEBUG
				if(fullva.texcoords[vindex + 0].x > 1.0f || fullva.texcoords[vindex + 0].y > 1.0f
						//||
						//fullva.texcoords[vindex + 1].x > 1.0f || fullva.texcoords[vindex + 1].y > 1.0f ||
						//fullva.texcoords[vindex + 2].x > 1.0f || fullva.texcoords[vindex + 2].y > 1.0f
				  )
				{

					g_log<<"------tc "<<vindex<<"-------"<<endl;
					g_log<<"initial uv = ("<<tc[0].x<<","<<tc[0].y<<"),("<<tc[1].x<<","<<tc[1].y<<"),("<<tc[2].x<<","<<tc[2].y<<")"<<endl;
					g_log<<"fits to region ("<<tfi->bounds[0].x<<","<<tfi->bounds[0].y<<")->("<<tfi->bounds[1].x<<","<<tfi->bounds[1].y<<") in image (0,0)->("<<maxdim.x<<","<<maxdim.y<<")"<<endl;
					g_log<<"minu = "<<minu<<endl;
					g_log<<"minv = "<<minv<<endl;
					g_log<<"maxu = "<<maxu<<endl;
					g_log<<"maxv = "<<maxv<<endl;
					g_log<<"rangeu = "<<rangeu<<endl;
					g_log<<"rangev = "<<rangev<<endl;
					g_log<<"tiles times "<<tfi->tiletimes.x<<"x"<<tfi->tiletimes.y<<endl;
					g_log<<"resized fit to region"\
						 " (pixels "<<tfi->bounds[0].x*scalex<<","<<tfi->bounds[0].y*scaley<<")->(pixels "<<tfi->bounds[1].x*scalex<<","<<tfi->bounds[1].y*scaley<<")"\
						 " i.e. (ratio "<<(float)tfi->bounds[0].x / (float)maxdim.x<<","<<(float)tfi->bounds[0].y / (float)maxdim.y<<")->(pixels "<<(tfi->newdim.x * tfi->tiletimes.x + tfi->bounds[0].x*scalex)<<","<<(tfi->newdim.y * tfi->tiletimes.y + tfi->bounds[0].y*scaley)<<")"\
						 " in image (0,0)->("<<finaldim.x<<","<<finaldim.y<<")"<<endl;
					//g_log<<"offu,v = "<<offu<<","<<offv<<endl;
					//g_log<<"tcleft,top = "<<tcleft<<","<<tctop<<endl;
					//g_log<<"pixelsrangeu,v = "<<pixelsrangeu<<","<<pixelsrangev<<endl;
					//g_log<<"scalerangeu,v = "<<scalerangeu<<","<<scalerangev<<endl;
					//g_log<<"availablerangeu,v = "<<availablerangeu<<","<<availablerangev<<endl;
					//g_log<<"(offu + tc0.x - minu),(offv + tc0.y - minv) = "<<(offu + tc0.x - minu)<<","<<(offv + tc0.y - minv)<<endl;
					//g_log<<"(offu + tc0.x - minu)*scalerangeu,(offv + tc0.y - minv)*scalerangev = "<<(offu + tc0.x - minu)*scalerangeu<<","<<(offv + tc0.y - minv)*scalerangev<<endl;
					g_log<<"final tc0 "<<fullva.texcoords[vindex + 0].x<<","<<fullva.texcoords[vindex + 0].y<<endl;
					g_log<<"final tc1 "<<fullva.texcoords[vindex + 1].x<<","<<fullva.texcoords[vindex + 1].y<<endl;
					g_log<<"final tc2 "<<fullva.texcoords[vindex + 2].x<<","<<fullva.texcoords[vindex + 2].y<<endl;
					g_log<<"------tc "<<vindex<<"-------"<<endl;
					g_log.flush();

				}
#endif
			}
		}
	}

	FILE* fp = fopen(fullfile, "wb");

	char tag[] = TAG_BUILDINGM;
	fwrite(tag, sizeof(char), 5, fp);

	float version = BUILDINGM_VERSION;
	fwrite(&version, sizeof(float), 1, fp);

	SaveVertexArray(fp, &fullva);

	fclose(fp);

	// Free the diffuse, specular, normal map RGB data

	if(finaldiff.data)
		free(finaldiff.data);
	if(finalspec.data)
		free(finalspec.data);
	if(finalnorm.data)
		free(finalnorm.data);
	if(finalown.data)
		free(finalown.data);

	for(int i=0; i<uniquetexs.size()*TEX_PER_BL; i++)
	{
		if(images[i])
		{
			if(images[i]->data)
			{
				free(images[i]->data);
			}

			free(images[i]);
		}
	}

	delete [] images;
}

#endif
