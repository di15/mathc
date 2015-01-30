
#include "../utils.h"
#include "shadow.h"
#include "shader.h"
#include "../math/3dmath.h"
#include "model.h"
#include "particle.h"
#include "projectile.h"
#include "../debug.h"
#include "billboard.h"
#include "../platform.h"
#include "../window.h"
#include "../math/vec3f.h"
#include "../math/matrix.h"
#include "model.h"
#include "../sim/simdef.h"
#include "model.h"
#include "../math/camera.h"
#include "../window.h"
#include "../../game/gmain.h"

unsigned int g_depth = -1;
unsigned int g_rbDepth;
unsigned int g_fbDepth;

//#define LIGHT_MAX_D		(1000.0f * 100.0f)
//#define LIGHT_MIN_D		1.0f

Vec3f g_lightoff(-MAX_DISTANCE/4, MAX_DISTANCE/4, MAX_DISTANCE/4);
//Vec3f g_lightoff(-LIGHT_MAX_D/5, LIGHT_MAX_D/3, LIGHT_MAX_D/4);
Vec3f g_lightpos;	//(-MAX_DISTANCE/2, MAX_DISTANCE/5, MAX_DISTANCE/3);
Vec3f g_lighteye;	//(-MAX_DISTANCE/2+1.0f/2.0f, MAX_DISTANCE/3-1.0f/3.0f, MAX_DISTANCE/3-1.0f/3.0f);
Vec3f g_lightup(0,1,0);

Matrix g_lightproj;
Matrix g_lightview;
Matrix g_caminvmv;  //camera inverse modelview
Matrix g_lightmat;
Matrix g_cammodelview;
Matrix g_camproj;
Matrix g_camview;
Matrix g_cammvp;

#if 0
void (*DrawSceneDepthFunc)() = NULL;
void (*DrawSceneFunc)(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float lightpos[3], float lightdir[3]) = NULL;
#endif

Vec3f g_viewInter;

void InitShadows()
{
	glGenTextures(1, &g_depth);
	glBindTexture(GL_TEXTURE_2D, g_depth);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);

#if 1
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16, DEPTH_SIZEX, DEPTH_SIZEY, 0, GL_RGBA, GL_UNSIGNED_SHORT, 0);
#else
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, DEPTH_SIZEX, DEPTH_SIZEY, 0, GL_RGBA, GL_UNSIGNED_SHORT, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, DEPTH_SIZEX, DEPTH_SIZEY, 0, GL_RED, GL_UNSIGNED_SHORT, 0);
#endif
	glBindTexture(GL_TEXTURE_2D, 0);

	glGenRenderbuffers(1, &g_rbDepth);
	glBindRenderbuffer(GL_RENDERBUFFER_EXT, g_rbDepth);
	glRenderbufferStorageEXT(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, DEPTH_SIZEX, DEPTH_SIZEY);
	glBindRenderbuffer(GL_RENDERBUFFER, 0);

	glGenFramebuffers(1, &g_fbDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, g_fbDepth);
	glFramebufferTexture2DEXT(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, g_depth, 0);
	glFramebufferRenderbufferEXT(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, g_rbDepth);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

/*
Doesn't work in all cases but good enough for its purpose in shadow mapping.
*/
void Inverse(Matrix* dstm, Matrix srcm)
{
	float dst[16];
	const float* src = srcm.m_matrix;

	dst[0] = src[0];
	dst[1] = src[4];
	dst[2] = src[8];
	dst[3] = 0.0;
	dst[4] = src[1];
	dst[5] = src[5];
	dst[6]  = src[9];
	dst[7] = 0.0;
	dst[8] = src[2];
	dst[9] = src[6];
	dst[10] = src[10];
	dst[11] = 0.0;
	dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
	dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
	dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
	dst[15] = 1.0;

	dstm->set(dst);
}

/*
More robust inverse matrix function.
*/
bool Inverse2(Matrix mat, Matrix &invMat)
{
	double inv[16], det;
	int i;

	const float* m = mat.m_matrix;

	inv[0] = m[5]  * m[10] * m[15] -
	         m[5]  * m[11] * m[14] -
	         m[9]  * m[6]  * m[15] +
	         m[9]  * m[7]  * m[14] +
	         m[13] * m[6]  * m[11] -
	         m[13] * m[7]  * m[10];

	inv[4] = -m[4]  * m[10] * m[15] +
	         m[4]  * m[11] * m[14] +
	         m[8]  * m[6]  * m[15] -
	         m[8]  * m[7]  * m[14] -
	         m[12] * m[6]  * m[11] +
	         m[12] * m[7]  * m[10];

	inv[8] = m[4]  * m[9] * m[15] -
	         m[4]  * m[11] * m[13] -
	         m[8]  * m[5] * m[15] +
	         m[8]  * m[7] * m[13] +
	         m[12] * m[5] * m[11] -
	         m[12] * m[7] * m[9];

	inv[12] = -m[4]  * m[9] * m[14] +
	          m[4]  * m[10] * m[13] +
	          m[8]  * m[5] * m[14] -
	          m[8]  * m[6] * m[13] -
	          m[12] * m[5] * m[10] +
	          m[12] * m[6] * m[9];

	inv[1] = -m[1]  * m[10] * m[15] +
	         m[1]  * m[11] * m[14] +
	         m[9]  * m[2] * m[15] -
	         m[9]  * m[3] * m[14] -
	         m[13] * m[2] * m[11] +
	         m[13] * m[3] * m[10];

	inv[5] = m[0]  * m[10] * m[15] -
	         m[0]  * m[11] * m[14] -
	         m[8]  * m[2] * m[15] +
	         m[8]  * m[3] * m[14] +
	         m[12] * m[2] * m[11] -
	         m[12] * m[3] * m[10];

	inv[9] = -m[0]  * m[9] * m[15] +
	         m[0]  * m[11] * m[13] +
	         m[8]  * m[1] * m[15] -
	         m[8]  * m[3] * m[13] -
	         m[12] * m[1] * m[11] +
	         m[12] * m[3] * m[9];

	inv[13] = m[0]  * m[9] * m[14] -
	          m[0]  * m[10] * m[13] -
	          m[8]  * m[1] * m[14] +
	          m[8]  * m[2] * m[13] +
	          m[12] * m[1] * m[10] -
	          m[12] * m[2] * m[9];

	inv[2] = m[1]  * m[6] * m[15] -
	         m[1]  * m[7] * m[14] -
	         m[5]  * m[2] * m[15] +
	         m[5]  * m[3] * m[14] +
	         m[13] * m[2] * m[7] -
	         m[13] * m[3] * m[6];

	inv[6] = -m[0]  * m[6] * m[15] +
	         m[0]  * m[7] * m[14] +
	         m[4]  * m[2] * m[15] -
	         m[4]  * m[3] * m[14] -
	         m[12] * m[2] * m[7] +
	         m[12] * m[3] * m[6];

	inv[10] = m[0]  * m[5] * m[15] -
	          m[0]  * m[7] * m[13] -
	          m[4]  * m[1] * m[15] +
	          m[4]  * m[3] * m[13] +
	          m[12] * m[1] * m[7] -
	          m[12] * m[3] * m[5];

	inv[14] = -m[0]  * m[5] * m[14] +
	          m[0]  * m[6] * m[13] +
	          m[4]  * m[1] * m[14] -
	          m[4]  * m[2] * m[13] -
	          m[12] * m[1] * m[6] +
	          m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
	         m[1] * m[7] * m[10] +
	         m[5] * m[2] * m[11] -
	         m[5] * m[3] * m[10] -
	         m[9] * m[2] * m[7] +
	         m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
	         m[0] * m[7] * m[10] -
	         m[4] * m[2] * m[11] +
	         m[4] * m[3] * m[10] +
	         m[8] * m[2] * m[7] -
	         m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
	          m[0] * m[7] * m[9] +
	          m[4] * m[1] * m[11] -
	          m[4] * m[3] * m[9] -
	          m[8] * m[1] * m[7] +
	          m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
	          m[0] * m[6] * m[9] -
	          m[4] * m[1] * m[10] +
	          m[4] * m[2] * m[9] +
	          m[8] * m[1] * m[6] -
	          m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
	{
		//ErrMess("d", "De 0");
		return false;
	}

	det = 1.0 / det;

	float invOut[16];
	for (i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;

	invMat.set(invOut);

	return true;
}

void Transpose(Matrix mat, Matrix &transpMat)
{
	const float* m = mat.m_matrix;
	float transp[16];

	int j;
	int index_1;
	int index_2;

	for (int i = 0; i < 4; i++)
	{
		for (j = 0; j < 4;  j++)
		{
			index_1 = i * 4 + j;
			index_2 = j * 4 + i;
			transp[index_1] = m[index_2];
		}
	}

	transpMat.set(transp);
}

void RenderToShadowMap(Matrix projection, Matrix viewmat, Matrix modelmat, Vec3f focus, Vec3f lightpos, void (*drawscenedepthfunc)())
{
	//return;

	g_camproj = projection;
	g_camview = viewmat;
	//g_cammodelview = modelview;

	Matrix oldproj = g_camproj;
	Matrix oldview = g_camview;
	Matrix oldmvp = g_cammvp;

	CheckGLError(__FILE__, __LINE__);

#if 1
	glDisable(GL_CULL_FACE);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);

	glBindFramebuffer(GL_FRAMEBUFFER, g_fbDepth);
	glViewport(0, 0, DEPTH_SIZEX, DEPTH_SIZEY);

	glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glEnable(GL_POLYGON_OFFSET_FILL);
	//glPolygonOffset(2.0, 500.0);
	//glPolygonOffset(10.0, 2500.0);
	glPolygonOffset(1.0, 250.0);

	CheckGLError(__FILE__, __LINE__);
#endif

	g_lightpos = lightpos;
	g_lighteye = focus;

	//return;

#if 0
	g_log<<"lposd "<<g_lightpos.x<<","<<g_lightpos.y<<","<<g_lightpos.z<<std::endl;
	g_log<<"leyed "<<g_lighteye.x<<","<<g_lighteye.y<<","<<g_lighteye.z<<std::endl;
#endif

	//g_lighteye = Vec3f(0,0,0);
	//g_lightpos = g_lighteye + g_lightoff;

	float zoom = Magnitude(g_lightoff) / Magnitude(g_lighteye - g_lightpos);

#define LIGHT_SCALE		3
//#define LIGHT_SCALE		1

	//g_lightproj = PerspProj(90.0, 1.0, 30.0, 10000.0);
	g_lightproj = OrthoProj(
	                      -PROJ_RIGHT*LIGHT_SCALE/zoom,
	                      PROJ_RIGHT*LIGHT_SCALE/zoom,
	                      PROJ_RIGHT*LIGHT_SCALE/zoom,
	                      -PROJ_RIGHT*LIGHT_SCALE/zoom,
	                      MIN_DISTANCE,
	                      MAX_DISTANCE/zoom/1.0f);

	//g_lightpos = RotateAround(g_lightpos, g_lighteye, DEGTORAD((g_simframe%360)), 0, 0, 1);
#if 1
	g_lightview = gluLookAt2(
	                      g_lightpos.x, g_lightpos.y, g_lightpos.z,
	                      //timelightpos.x, timelightpos.y, timelightpos.z,
	                      g_lighteye.x, g_lighteye.y, g_lighteye.z,
	                      g_lightup.x, g_lightup.y, g_lightup.z);
#else
	g_lightview = LookAt(
	                      g_lightpos.x, g_lightpos.y, g_lightpos.z,
	                      //timelightpos.x, timelightpos.y, timelightpos.z,
	                      g_lighteye.x, g_lighteye.y, g_lighteye.z,
	                      g_lightup.x, g_lightup.y, g_lightup.z);
#endif

	CheckGLError(__FILE__, __LINE__);

#if 1
	UseS(SHADER_DEPTH);
	glUniformMatrix4fv(g_shader[SHADER_DEPTH].m_slot[SSLOT_PROJECTION], 1, 0, g_lightproj.m_matrix);
	glUniformMatrix4fv(g_shader[SHADER_DEPTH].m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
	glUniformMatrix4fv(g_shader[SHADER_DEPTH].m_slot[SSLOT_VIEWMAT], 1, 0, g_lightview.m_matrix);
	glUniform4f(g_shader[SHADER_DEPTH].m_slot[SSLOT_COLOR], 1, 1, 1, 1);

	g_camproj = g_lightproj;
	g_camview = g_lightview;

	Matrix mvp;
	mvp.set(g_lightproj.m_matrix);
	mvp.postmult2(g_lightview);


	CheckGLError(__FILE__, __LINE__);

	if(drawscenedepthfunc != NULL)
		drawscenedepthfunc();

	CheckGLError(__FILE__, __LINE__);

	EndS();

	CheckGLError(__FILE__, __LINE__);

	glDisable(GL_POLYGON_OFFSET_FILL);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glViewport(0, 0, g_width, g_height);
	glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
	glEnable(GL_CULL_FACE);

	g_camproj = oldproj;
	g_camview = oldview;
	g_cammvp = oldmvp;
#endif

	CheckGLError(__FILE__, __LINE__);
}

void UseShadow(int shader, Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float lightpos[3], float lightdir[3])
{
	UseS(shader);
	Shader* s = &g_shader[g_curS];
	glUniformMatrix4fv(s->m_slot[SSLOT_PROJECTION], 1, 0, projection.m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
	glUniformMatrix4fv(s->m_slot[SSLOT_VIEWMAT], 1, 0, viewmat.m_matrix);
	//glUniformMatrix4fv(s->m_slot[SSLOT_NORMALMAT], 1, 0, modelviewinv.m_matrix);
	//glUniformMatrix4fv(s->m_slot[SSLOT_INVMODLVIEWMAT], 1, 0, modelviewinv.m_matrix);
	glUniform4f(s->m_slot[SSLOT_COLOR], 1, 1, 1, 1);
	//glEnableVertexAttribArray(s->m_slot[SSLOT_POSITION]);
	//glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD0]);
	//glEnableVertexAttribArray(s->m_slot[SSLOT_TEXCOORD1]);
	//if(s->m_slot[SSLOT_NORMAL] != -1)   glEnableVertexAttribArray(s->m_slot[SSLOT_NORMAL]);

	//glUniformMatrix4fv(s->m_slot[SSLOT_LIGHTMATRIX], 1, false, g_lightmat);
	glUniformMatrix4fv(s->m_slot[SSLOT_LIGHTMATRIX], 1, false, g_lightmat.m_matrix);

	glUniform3f(s->m_slot[SSLOT_LIGHTPOS], lightpos[0], lightpos[1], lightpos[2]);
	glUniform3f(s->m_slot[SSLOT_SUNDIRECTION], lightdir[0], lightdir[1], lightdir[2]);
	//glUniform1f(s->m_slot[SSLOT_MAXELEV], g_maxelev);

#if 0
	g_log<<"sun "<<lightdir[0]<<","<<lightdir[1]<<","<<lightdir[2]<<std::endl;
#endif
}

void RenderShadowedScene(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelview, void (*drawscenefunc)(Matrix projection, Matrix viewmat, Matrix modelmat, Matrix modelviewinv, float lightpos[3], float lightdir[3]))
{
	//glViewport(0, 0, g_width, g_height);
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClearColor(1.0, 1.0, 1.0, 1.0);

	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//gluPerspective(FIELD_OF_VIEW, double(g_width) / double(g_height), MIN_DISTANCE, MAX_DISTANCE);
	//g_camproj = PerspProj(FIELD_OF_VIEW, double(g_width) / double(g_height), MIN_DISTANCE, MAX_DISTANCE);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	//c->Look();

	//glGetFloatv(GL_PROJECTION_MATRIX, g_camproj);
	g_camproj = projection;
	//glGetFloatv(GL_MODELVIEW_MATRIX, g_cammodelview);
	g_cammodelview = modelview;

	// Do non-shadowed drawing here
	//DrawSkyBox(c->LookPos());

	Inverse(&g_caminvmv, modelview);

	// We need to change the clip-space coordinates from range [-1,1] to [0,1] for texture mapping
	g_lightmat.reset();
#if 1
	float trans[] = { 0.5f, 0.5f, 0.5f };
	g_lightmat.translation(trans);
	float scalef[] = { 0.5f, 0.5f, 0.5f };
	Matrix scalem;
	scalem.scale(scalef);
	g_lightmat.postmult2(scalem);
#endif
	g_lightmat.postmult2(g_lightproj);
	g_lightmat.postmult2(g_lightview);
	//g_lightmat.postmult(g_caminvmv);

	Matrix modelviewinv;
	Inverse2(modelview, modelviewinv);
	Transpose(modelviewinv, modelviewinv);

	const float* mv = g_cammodelview.m_matrix;
	float lightpos[3];
	lightpos[0] = mv[0] * g_lightpos.x + mv[4] * g_lightpos.y + mv[8] * g_lightpos.z + mv[12];
	lightpos[1] = mv[1] * g_lightpos.x + mv[5] * g_lightpos.y + mv[9] * g_lightpos.z + mv[13];
	lightpos[2] = mv[2] * g_lightpos.x + mv[6] * g_lightpos.y + mv[10] * g_lightpos.z + mv[14];

	Vec3f lightdir;
	//lightdir[0] = g_lighteye.x - g_lightpos.x;
	//lightdir[1] = g_lighteye.y - g_lightpos.y;
	//lightdir[2] = g_lighteye.z - g_lightpos.z;
#if 0
	g_log<<"lpos "<<g_lightpos.x<<","<<g_lightpos.y<<","<<g_lightpos.z<<std::endl;
	g_log<<"leye "<<g_lighteye.x<<","<<g_lighteye.y<<","<<g_lighteye.z<<std::endl;
#endif
	lightdir = g_lightpos - g_lighteye;
#if 0
	g_log<<"sun pres "<<lightdir.x<<","<<lightdir.y<<","<<lightdir.z<<std::endl;
#endif
	lightdir = Normalize(lightdir);
#if 0
	g_log<<"sun norm "<<lightdir.x<<","<<lightdir.y<<","<<lightdir.z<<std::endl;
#endif

	if(drawscenefunc != NULL)
		//drawscenefunc(projection, viewmat, modelmat, modelviewinv, lightpos, (float*)&lightdir);
		drawscenefunc(projection, viewmat, modelmat, modelviewinv, lightpos, (float*)&lightdir);
	//DrawSceneFunc(projection, viewmat, modelmat, modelviewinv, (float*)&g_lightpos, lightdir);

	//TurnOffShader();
	glBindTexture(GL_TEXTURE_2D, 0);
	glActiveTexture(GL_TEXTURE0);
	/*
	if(g_mode == APPMODE_EDITOR)
	{
		UseS(COLOR3D);
		glUniformMatrix4fv(g_shader[SHADER_COLOR3D].m_slot[SSLOT_PROJECTION], 1, 0, projection.m_matrix);
		glUniformMatrix4fv(g_shader[SHADER_COLOR3D].m_slot[SSLOT_MODELMAT], 1, 0, modelmat.m_matrix);
		glUniformMatrix4fv(g_shader[SHADER_COLOR3D].m_slot[SSLOT_VIEWMAT], 1, 0, viewmat.m_matrix);
		glUniform4f(g_shader[SHADER_COLOR3D].m_slot[SSLOT_COLOR], 0, 1, 0, 1);
		glEnableVertexAttribArray(g_shader[SHADER_COLOR3D].m_slot[SSLOT_POSITION]);
		glEnableVertexAttribArray(g_shader[SHADER_COLOR3D].m_slot[SSLOT_NORMAL]);
		DrawTileSq();
	}*/

	TurnOffShader();
	//c->Look();
}


