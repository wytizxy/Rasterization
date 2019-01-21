/*  
    smooth.c
    Nate Robins, 1998

    Model viewer program.  Exercises the glm library.
*/


#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdarg.h>
#include <GL/glut.h>
#include "gltb.h"
#include "glm.h"
#include "dirent32.h"

#pragma comment( linker, "/entry:\"mainCRTStartup\"" )  // set the entry point to be main()

#define DATA_DIR "data/"

char*      model_file = NULL;		/* name of the object file */
GLuint     model_list = 0;		    /* display list for object */
GLMmodel*  model;			        /* glm model data structure */
GLfloat    scale;			        /* original scale factor */
GLfloat    smoothing_angle = 90.0;	/* smoothing angle */
GLfloat    weld_distance = 0.00001;	/* epsilon for welding vertices */
GLboolean  facet_normal = GL_FALSE;	/* draw with facet normal? */
GLboolean  bounding_box = GL_FALSE;	/* bounding box on? */
GLboolean  performance = GL_FALSE;	/* performance counter on? */
GLboolean  stats = GL_FALSE;		/* statistics on? */
GLuint     material_mode = 0;		/* 0=none, 1=color, 2=material */
GLint      entries = 0;			    /* entries in model menu */
GLdouble   pan_x = 0.0;
GLdouble   pan_y = 0.0;				/* vector in 3D version */
GLdouble   pan_z = 0.0;
GLboolean  rstrz = GL_FALSE;		/* rendering by rasterization */
GLfloat data[512][512][3];			/* data in rasterization */


#if defined(_WIN32)
#include <sys/timeb.h>
#define CLK_TCK 1000
#else
#include <limits.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/times.h>
#endif
float
elapsed(void)
{
    static long begin = 0;
    static long finish, difference;
    
#if defined(_WIN32)
    static struct timeb tb;
    ftime(&tb);
    finish = tb.time*1000+tb.millitm;
#else
    static struct tms tb;
    finish = times(&tb);
#endif
    
    difference = finish - begin;
    begin = finish;
    
    return (float)difference/(float)CLK_TCK;
}

void
shadowtext(int x, int y, char* s) 
{
    int lines;
    char* p;
    
    glDisable(GL_DEPTH_TEST);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glOrtho(0, glutGet(GLUT_WINDOW_WIDTH), 
        0, glutGet(GLUT_WINDOW_HEIGHT), -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
    glColor3ub(0, 0, 0);
    glRasterPos2i(x+1, y-1);
    for(p = s, lines = 0; *p; p++) {
        if (*p == '\n') {
            lines++;
            glRasterPos2i(x+1, y-1-(lines*18));
        }
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glColor3ub(0, 128, 255);
    glRasterPos2i(x, y);
    for(p = s, lines = 0; *p; p++) {
        if (*p == '\n') {
            lines++;
            glRasterPos2i(x, y-(lines*18));
        }
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
    }
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
    glEnable(GL_DEPTH_TEST);
}

void
lists(void)
{
    GLfloat ambient[] = { 0.2, 0.2, 0.2, 1.0 };
    GLfloat diffuse[] = { 0.8, 0.8, 0.8, 1.0 };
    GLfloat specular[] = { 0.0, 0.0, 0.0, 1.0 };
    GLfloat shininess = 65.0;
    
    glMaterialfv(GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv(GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv(GL_FRONT, GL_SPECULAR, specular);
    glMaterialf(GL_FRONT, GL_SHININESS, shininess);
    
    if (model_list)
        glDeleteLists(model_list, 1);
    
    /* generate a list */
    if (material_mode == 0) { 
        if (facet_normal)
            model_list = glmList(model, GLM_FLAT);
        else
            model_list = glmList(model, GLM_SMOOTH);
    } else if (material_mode == 1) {
        if (facet_normal)
            model_list = glmList(model, GLM_FLAT | GLM_COLOR);
        else
            model_list = glmList(model, GLM_SMOOTH | GLM_COLOR);
    } else if (material_mode == 2) {
        if (facet_normal)
            model_list = glmList(model, GLM_FLAT | GLM_MATERIAL);
        else
            model_list = glmList(model, GLM_SMOOTH | GLM_MATERIAL);
    }
}

void
init(void)
{
    gltbInit(GLUT_LEFT_BUTTON);
    
    /* read in the model */
    model = glmReadOBJ(model_file);
    scale = glmUnitize(model);
    glmFacetNormals(model);
    glmVertexNormals(model, smoothing_angle);
    
    if (model->nummaterials > 0)
        material_mode = 2;
    
    /* create new display lists */
    lists();
    
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glLightModeli(GL_LIGHT_MODEL_TWO_SIDE, GL_TRUE);
    
    glEnable(GL_DEPTH_TEST);
    
    glEnable(GL_CULL_FACE);
}

void
reshape(int width, int height)
{
    gltbReshape(width, height);
    
    glViewport(0, 0, width, height);
    
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0, (GLfloat)height / (GLfloat)width, 1.0, 128.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslatef(0.0, 0.0, -3.0);
}
void
outputdata(GLfloat data[512][512][3])		
{
	FILE *fp;
	int x, y;
	fp = fopen("data.txt","w");			//open file user.txt
	if(fp==NULL)
    {
        printf("File cannot open! " );
        exit(0); 
    }

	for (y = 0; y < 512; y++)
	{
		for(x = 0; x < 512; x++)
		{
			fprintf(fp,"%d", data[y][x][2]);	//write data into file
		}
		fprintf(fp,"%d", '\n');
	}
	fclose(fp);								//close file
}

void
rstrzDrawLine(int x10, int y10, int x20, int y20)
{
	int x1,y1,x2,y2;						//three vertexes
	x1 = x10;
	y1 = y10;
	x2 = x20;
	y2 = y20;								//initialize three points
	int y = 0;
	int x = 0;
	int xr, yr;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	if (dx >= dy)                                     //forward by dx
	{
		if (x1 > x2)
		{
			//swap(x1, x2);
			xr = x1;
			x1 = x2;
			x2 = x1;
			//swap(y1, y2);
			yr = y1;
			y1 = y2;
			y2 = yr;
		}
		int A = y1 - y2;
		int B = x2 - x1;
		if(y2 >= y1)                                  //slope in [0,1]
		{
			int d = (A << 1) + B;                     //f(x+1,y+0.5)*2
			int upIncrement = (A + B) << 1;           //when choose up one, the increase of d
			int downIncrement = A << 1;               //when choose down one, the increase of d
			for (x = x1,y = y1; x <= x2; ++x)
			{
				data[y][x][0] = 1;
				if (d < 0){                           //midpoint under the line, choose the up one
					d += upIncrement;
					++y;
				}
				else  								
				{
					d += downIncrement;			
				}
			}
		}
		else                                        //slope in [-1,0)
		{
			int d = (A << 1) - B;                 
			int upIncrement = A << 1;         
			int downIncrement = (A - B) << 1;                   
			for (int x = x1,y = y1;x <= x2;++x)
			{
				data[y][x][0] = 1;
				if (d < 0){                               
					d += upIncrement;
				}
				else
				{
					d += downIncrement;
					--y;
				}
			}
		}   
	}
	else
	{
		if (y1 > y2)
		{
			//swap(x1, x2);
			xr = x1;
			x1 = x2;
			x2 = x1;
			//swap(y1, y2);
			yr = y1;
			y1 = y2;
			y2 = yr;
		}
		int A = x1 - x2;
		int B = y2 - y1;
		if (x2 >= x1)
		{
			int d = (A << 1) + B;                 //f(x+0.5,y+1)*2, Ay+Bx+C=0
			int upIncrement = (A + B) << 1;       
			int downIncrement = A << 1;           
			for (int x = x1, y = y1; y <= y2; ++y)
			{
				data[y][x][0] = 1;
				if (d < 0){                       
					d += upIncrement;
					++x;
				}
				else
				{
					d += downIncrement;
				}
			}
		}
		else
		{
			int d = (A << 1) - B;                 
			int upIncrement = A << 1;         
			int downIncrement = (A - B) << 1;           
			for (int x = x1, y = y1; y <= y2; ++y)
			{
				data[y][x][0] = 1;
				if (d < 0){                       
					d += upIncrement;
				}
				else
				{
					d += downIncrement;
					--x;
				}
			}
		}
	}
}

int
intersection(int x1, int y1, int x2, int y2,		//x1y1 x2y2 are the vertex
			int x11, int y11, int x22, int y22)		//x11y11 x22y22 are two pixel
{
	float k, b;
	k = (y1 - y2) / (x1 - x2);
	b = (x1*y2 - x2*y1) / (x1 - x2);			//!!!!!maybe this function is wrong, abandon it
	float A1, A2;
	A1 = k*x11 + b - y11;
	A2 = k*x22 + b - y22;
	if(A1*A2 < 0 && ((x11 < x1 && x11 > x2) || (x11 > x1 && x11 < x2))
			&& ((y11 < y1 && y11 > y2) || (y11 > y1 && y11 < y2)))
	{
		return 1;
	}
	else
	{
		return 0;
	}
} 

void
drawTriangle(int x1, int y1, int x2, int y2, int x3, int y3)
{
	rstrzDrawLine(x1, y1, x2, y2);
	rstrzDrawLine(x1, y1, x3, y3);
	rstrzDrawLine(x2, y2, x3, y3);			//switch 2 and 3 will cause mistakes.
	/*rstrzDrawLine(x2, y2, x1, y1);
	rstrzDrawLine(x3, y3, x1, y1);
	rstrzDrawLine(x3, y3, x2, y2);*/			//draw twice to prevent mistake

	int xstart, xend, x, y;							//scanline from y = 0 to y = 511
	for (y = 0; y < 512; y++)
	{
		for (x = 0; x < 512; x++)
		{
			if(data[y][x][0] == 1)
			{
				xstart = x;
				x++;
				while(data[y][x][0] == 0 && x < 512)
				{
					x++;
				}
				if(data[y][x][0] == 1)					//fill color
				{
					xend = x;
					for (int t = xstart; t <= xend; t++)
					{
						data[y][t][0] = 1;
					}
					x = 511;
				}
			}
		}
		xstart = 0;
		xend = 0;
	}
}

void
rasterization(GLMmodel* Model)			//my rasterization func_rasterization
{
	int x, y;
	GLdouble x1, y1, z1, x2, y2, z2, x3, y3, z3;
	GLdouble model[4*4];
    GLdouble proj[4*4];
    GLint view[4];
	glGetDoublev(GL_MODELVIEW_MATRIX, model);
	glGetDoublev(GL_PROJECTION_MATRIX, proj);
	glGetIntegerv(GL_VIEWPORT, view);
	int index;
	GLMtriangle tempTri;

	for(int tri = 0; tri < Model->numtriangles; tri++)
	{
		tempTri = *(Model->triangles + tri);
		index = tempTri.vindices[0];
		gluProject(Model->vertices[3 * index], Model->vertices[3 * index + 1], Model->vertices[3 * index + 2],
	    	model, proj, view,
	    	&x1, &y1, &z1);
		index = tempTri.vindices[1];
		gluProject(Model->vertices[3 * index], Model->vertices[3 * index + 1], Model->vertices[3 * index + 2],
	    	model, proj, view,
	    	&x2, &y2, &z2);
		index = tempTri.vindices[2];
		gluProject(Model->vertices[3 * index], Model->vertices[3 * index + 1], Model->vertices[3 * index + 2],
		    model, proj, view,
		    &x3, &y3, &z3);
		drawTriangle(x1,y1,x2,y2,x3,y3);
		//drawTriangle(10,50,100,400,200,300);
	}

	/*gluProject((GLdouble)x, (GLdouble)y, 0.0,
	    	model, proj, view,
	    	&Model->vertices[3*Model->triangles->vindices[0]], &Model->vertices[3*Model->triangles->vindices[0]+1], &Model->vertices[3*Model->triangles->vindices[0]+2]);
	*/
	FILE *fp;
	fp = fopen("data1.txt","w");			//open file user.txt
	fprintf(fp,"%d", x);
	fprintf(fp,"%d", '!');
	fprintf(fp,"%d", y);
	fclose(fp);

	glClearColor(0, 0, 0, 1);				//background color set
	glClear(GL_COLOR_BUFFER_BIT);			//background clean
	glDrawPixels(512, 512, GL_RGB, GL_FLOAT, data);
	glutSwapBuffers();					//swap buffers
	glutMainLoop();
}

void
rasterization2(GLMmodel* model, GLuint mode)			//my rasterization func_drawTriangle
{
	GLuint x1,y1,x2,y2,x3,y3;						//three vertexes
	x1 = 10;
	y1 = 50;
	x2 = 200;
	y2 = 400;
	x3 = 150;
	y3 = 100;
	rstrzDrawLine(x1, y1, x2, y2);
	rstrzDrawLine(x1, y1, x3, y3);
	rstrzDrawLine(x3, y3, x2, y2);

	int x, y,touch,xstart,xend;							//scanline from y = 0 to y = 511
	touch = 0;
	int edgeCnt;					//check intesect times
	for (y = 0; y < 512; y++)
	{
		edgeCnt = 0;
		for (x = 0; x < 512; x++)
		{
			/*if(intersection(x1, y1, x2, y2, x, y, x+1, y+1) == 1
				 || intersection(x2, y2, x3, y3, x, y, x+1, y+1) == 1
				 || intersection(x1, y1, x3, y3, x, y, x+1, y+1) == 1)		//something wrong with the original way to detect intersection
			{
				edgeCnt ++;			//draw the pixel when edgeCnt is odd
			}
			if(edgeCnt % 2)
			{
				data[y][x][2] = 1;
			}*/
			if(data[y][x][2] == 1)
			{
				xstart = x;
				x++;
				while(data[y][x][2] == 0)
				{
					x++;
				}
				if(data[y][x][2] == 1)					//fill color
				{
					xend = x;
					x = 511;
					for (int t = xstart; t <= xend; t++)
					{
						data[y][t][2] = 1;
					}
				}
			}
		}
	}
	
	//outputdata(data);						//output data to a .txt file
	glClearColor(0, 0, 0, 1);				//background color set
	glClear(GL_COLOR_BUFFER_BIT);			//background clean
	glDrawPixels(512, 512, GL_RGB, GL_FLOAT, data);
	glutSwapBuffers();					//swap buffers
	glutMainLoop();
}
void
rasterization1(GLuint model_list)			//my rasterization func_drawLine
{
	int x1,y1,x2,y2;						//two end points
	x1 = 10;
	y1 = 50;
	x2 = 200;
	y2 = 400;								//initialize two point
	int y = 0;
	int x = 0;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	if (dx >= dy)                                     //forward by dx
	{
		if (x1 > x2)
		{
			swap(x1, x2);
			swap(y1, y2);
		}
		int A = y1 - y2;
		int B = x2 - x1;
		if(y2 >= y1)                                  //slope in [0,1]
		{
			int d = (A << 1) + B;                     //f(x+1,y+0.5)*2
			int upIncrement = (A + B) << 1;           //when choose up one, the increase of d
			int downIncrement = A << 1;               //when choose down one, the increase of d
			for (x = x1,y = y1; x <= x2; ++x)
			{
				data[y][x][2] = 1;
				if (d < 0){                           //midpoint under the line, choose the up one
					d += upIncrement;
					++y;
				}
				else  								
				{
					d += downIncrement;			
				}
			}
		}
		else                                        //slope in [-1,0)
		{
			int d = (A << 1) - B;                 
			int upIncrement = A << 1;         
			int downIncrement = (A - B) << 1;                   
			for (int x = x1,y = y1;x <= x2;++x)
			{
				data[y][x][2] = 1;
				if (d < 0){                               
					d += upIncrement;
				}
				else
				{
					d += downIncrement;
					--y;
				}
			}
		}   
	}
	else
	{
		if (y1 > y2)
		{
			swap(x1, x2);
			swap(y1, y2);
		}
		int A = x1 - x2;
		int B = y2 - y1;
		if (x2 >= x1)
		{
			int d = (A << 1) + B;                 //f(x+0.5,y+1)*2, Ay+Bx+C=0
			int upIncrement = (A + B) << 1;       
			int downIncrement = A << 1;           
			for (int x = x1, y = y1; y <= y2; ++y)
			{
				data[y][x][2] = 1;
				if (d < 0){                       
					d += upIncrement;
					++x;
				}
				else
				{
					d += downIncrement;
				}
			}
		}
		else
		{
			int d = (A << 1) - B;                 
			int upIncrement = A << 1;         
			int downIncrement = (A - B) << 1;           
			for (int x = x1, y = y1; y <= y2; ++y)
			{
				data[y][x][2] = 1;
				if (d < 0){                       
					d += upIncrement;
				}
				else
				{
					d += downIncrement;
					--x;
				}
			}
		}
	}
	//outputdata(data);						//output data to a .txt file
	glClearColor(0, 0, 0, 1);				//background color set
	glClear(GL_COLOR_BUFFER_BIT);			//background clean
	glDrawPixels(512, 512, GL_RGB, GL_FLOAT, data);
	glutSwapBuffers();					//swap buffers
	glutMainLoop();
}

void
rasterization0(GLuint model_list)			//my rasterization func
{
	int y = 0;
	int x = 0;
	x = 256;
	for (int y = 0; y < 512; y++)			//test basic drawPixel func
	{
		data[y][x][0] = 0;
		data[y][x][1] = 0;
		data[y][x][2] = 1;
	}
	//outputdata(data);						//output data to a .txt file
	glClearColor(0, 0, 0, 1);				//background color set
	glClear(GL_COLOR_BUFFER_BIT);			//background clean
	glDrawPixels(512, 512, GL_RGB, GL_FLOAT, data);
	glutSwapBuffers();					//swap buffers
	glutMainLoop();
}

#define NUM_FRAMES 5
void
display(void)
{
    static char s[256], t[32];
    static char* p;
    static int frames = 0;
    
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    glPushMatrix();
    
    glTranslatef(pan_x, pan_y, 0.0);
    
    gltbMatrix();
    
	if(rstrz)		/* glmDraw() performance test */  //when y is pressed, start rstrz.
	{
		rasterization(model);					//my rasterization func
		/*if (material_mode == 0) { 
	        if (facet_normal)
	            glmDraw(model, GLM_FLAT);
	        else
	            glmDraw(model, GLM_SMOOTH);
	    } else if (material_mode == 1) {
	        if (facet_normal)
	            glmDraw(model, GLM_FLAT | GLM_COLOR);
	        else
	            glmDraw(model, GLM_SMOOTH | GLM_COLOR);
	    } else if (material_mode == 2) {
	        if (facet_normal)
	            glmDraw(model, GLM_FLAT | GLM_MATERIAL);
	        else
	            glmDraw(model, GLM_SMOOTH | GLM_MATERIAL);
	    }*/
	}    
	else
	{
		glCallList(model_list);		//the original display call
	}
    
    glDisable(GL_LIGHTING);			//turn off GL func, using current lighting to calculate color
    if (bounding_box) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glEnable(GL_CULL_FACE);
        glColor4f(1.0, 0.0, 0.0, 0.25);
        glutSolidCube(2.0);
        glDisable(GL_BLEND);
    }
    
    glPopMatrix();
    
    if (stats) {
        /* XXX - this could be done a _whole lot_ faster... */
        int height = glutGet(GLUT_WINDOW_HEIGHT);
        glColor3ub(0, 0, 0);
        sprintf(s, "%s\n%d vertices\n%d triangles\n%d normals\n"
            "%d texcoords\n%d groups\n%d materials",
            model->pathname, model->numvertices, model->numtriangles, 
            model->numnormals, model->numtexcoords, model->numgroups,
            model->nummaterials);
        shadowtext(5, height-(5+18*1), s);
    }
    
    /* spit out frame rate. */
    frames++;
    if (frames > NUM_FRAMES) {
        sprintf(t, "%g fps", frames/elapsed());
        frames = 0;
    }
    if (performance) {
        shadowtext(5, 5, t);
    }
    
    glutSwapBuffers();
    glEnable(GL_LIGHTING);
}

void
keyboard(unsigned char key, int x, int y)
{
    GLint params[2];
    
    switch (key) {
    case 'h':
        printf("help\n\n");
        printf("w         -  Toggle wireframe/filled\n");
        printf("c         -  Toggle culling\n");
        printf("n         -  Toggle facet/smooth normal\n");
        printf("b         -  Toggle bounding box\n");
        printf("r         -  Reverse polygon winding\n");
        printf("m         -  Toggle color/material/none mode\n");
        printf("p         -  Toggle performance indicator\n");
        printf("s/S       -  Scale model smaller/larger\n");
        printf("t         -  Show model stats\n");
        printf("o         -  Weld vertices in model\n");
        printf("+/-       -  Increase/decrease smoothing angle\n");
        printf("W         -  Write model to file (out.obj)\n");
        printf("y		  -  Use rasterization algorithm\n");
		printf("q/escape  -  Quit\n\n");
        break;
        
    case 't':
        stats = !stats;
        break;
        
    case 'p':
        performance = !performance;
        break;
        
    case 'm':
        material_mode++;
        if (material_mode > 2)
            material_mode = 0;
        printf("material_mode = %d\n", material_mode);
        lists();
        break;
        
    case 'd':							//delete??
        glmDelete(model);
        init();
        lists();
        break;
        
    case 'w':
        glGetIntegerv(GL_POLYGON_MODE, params);
        if (params[0] == GL_FILL)
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        else
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        break;
        
    case 'c':
        if (glIsEnabled(GL_CULL_FACE))
            glDisable(GL_CULL_FACE);
        else
            glEnable(GL_CULL_FACE);
        break;
        
    case 'b':
        bounding_box = !bounding_box;
        break;
        
    case 'n':
        facet_normal = !facet_normal;
        lists();
        break;
        
    case 'r':
        glmReverseWinding(model);
        lists();
        break;
        
    case 's':
        glmScale(model, 0.8);
        lists();
        break;
        
    case 'S':
        glmScale(model, 1.25);
        lists();
        break;
        
    case 'o':
        //printf("Welded %d\n", glmWeld(model, weld_distance));
        glmVertexNormals(model, smoothing_angle);
        lists();
        break;
        
    case 'O':
        weld_distance += 0.01;
        printf("Weld distance: %.2f\n", weld_distance);
        glmWeld(model, weld_distance);
        glmFacetNormals(model);
        glmVertexNormals(model, smoothing_angle);
        lists();
        break;
        
    case '-':
        smoothing_angle -= 1.0;
        printf("Smoothing angle: %.1f\n", smoothing_angle);
        glmVertexNormals(model, smoothing_angle);
        lists();
        break;
        
    case '+':
        smoothing_angle += 1.0;
        printf("Smoothing angle: %.1f\n", smoothing_angle);
        glmVertexNormals(model, smoothing_angle);
        lists();
        break;
        
    case 'W':
        glmScale(model, 1.0/scale);
        glmWriteOBJ(model, "out.obj", GLM_SMOOTH | GLM_MATERIAL);
        break;
        
    case 'R':
        {
            GLuint i;
            GLfloat swap;
            for (i = 1; i <= model->numvertices; i++) {
                swap = model->vertices[3 * i + 1];
                model->vertices[3 * i + 1] = model->vertices[3 * i + 2];
                model->vertices[3 * i + 2] = -swap;
            }
            glmFacetNormals(model);
            lists();
            break;
        }
		
	case 'y':
		rstrz = !rstrz;
		break;
        
    case 27:
        exit(0);
        break;
    }
    
    glutPostRedisplay();
}

void
menu(int item)
{
    int i = 0;
    DIR* dirp;
    char* name;
    struct dirent* direntp;
    
    if (item > 0) {
        keyboard((unsigned char)item, 0, 0);
    } else {
        dirp = opendir(DATA_DIR);
        while ((direntp = readdir(dirp)) != NULL) {
            if (strstr(direntp->d_name, ".obj")) {
                i++;
                if (i == -item)
                    break;
            }
        }
        if (!direntp)
            return;
        name = (char*)malloc(strlen(direntp->d_name) + strlen(DATA_DIR) + 1);
        strcpy(name, DATA_DIR);
        strcat(name, direntp->d_name);
        model = glmReadOBJ(name);
        scale = glmUnitize(model);
        glmFacetNormals(model);
        glmVertexNormals(model, smoothing_angle);
        
        if (model->nummaterials > 0)
            material_mode = 2;
        else
            material_mode = 0;
        
        lists();
        free(name);
        
        glutPostRedisplay();
    }
}

static GLint      mouse_state;
static GLint      mouse_button;

void
mouse(int button, int state, int x, int y)
{
    GLdouble model[4*4];
    GLdouble proj[4*4];
    GLint view[4];
    
    /* fix for two-button mice -- left mouse + shift = middle mouse */
    if (button == GLUT_LEFT_BUTTON && glutGetModifiers() & GLUT_ACTIVE_SHIFT)
        button = GLUT_MIDDLE_BUTTON;
    
    gltbMouse(button, state, x, y);
    
    mouse_state = state;
    mouse_button = button;
    
    if (state == GLUT_DOWN && button == GLUT_MIDDLE_BUTTON) {
        glGetDoublev(GL_MODELVIEW_MATRIX, model);
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
        glGetIntegerv(GL_VIEWPORT, view);
        gluProject((GLdouble)x, (GLdouble)y, 0.0,
            model, proj, view,
            &pan_x, &pan_y, &pan_z);					//********
        gluUnProject((GLdouble)x, (GLdouble)y, pan_z,
            model, proj, view,
            &pan_x, &pan_y, &pan_z);
        pan_y = -pan_y;
    }
    
    glutPostRedisplay();
}

void
motion(int x, int y)
{
    GLdouble model[4*4];
    GLdouble proj[4*4];
    GLint view[4];
    
    gltbMotion(x, y);
    
    if (mouse_state == GLUT_DOWN && mouse_button == GLUT_MIDDLE_BUTTON) {
        glGetDoublev(GL_MODELVIEW_MATRIX, model);
        glGetDoublev(GL_PROJECTION_MATRIX, proj);
        glGetIntegerv(GL_VIEWPORT, view);
        gluProject((GLdouble)x, (GLdouble)y, 0.0,
            model, proj, view,
            &pan_x, &pan_y, &pan_z);
        gluUnProject((GLdouble)x, (GLdouble)y, pan_z,
            model, proj, view,
            &pan_x, &pan_y, &pan_z);
        pan_y = -pan_y;
    }
    
    glutPostRedisplay();
}

int
main(int argc, char** argv)
{
    int buffering = GLUT_DOUBLE;
    struct dirent* direntp;
    DIR* dirp;
    int models;
    
    glutInitWindowSize(512, 512);			//initialize window
    glutInit(&argc, argv);					//initialize GLUT
    
    while (--argc) {
        if (strcmp(argv[argc], "-sb") == 0)
            buffering = GLUT_SINGLE;
        else
            model_file = argv[argc];
    }
    
    if (!model_file) {
        model_file = "data/cube.obj";			//load .obj char* model_file = NULL; name of the obect file
    }
    
    glutInitDisplayMode(GLUT_RGB | GLUT_DEPTH | buffering);		//initialize display mode
    glutCreateWindow("Smooth");
    
    glutReshapeFunc(reshape);					//reshape window
    glutDisplayFunc(display);					//display
    glutKeyboardFunc(keyboard);					//keyboard(unsigned char key, int x, int y) keyboard func
    glutMouseFunc(mouse);						//mouse(int button, int state, int x, int y) mouse func
    glutMotionFunc(motion);						//motion(int x, int y)  motion func
    
    models = glutCreateMenu(menu);				//menu(int item) menu func
    dirp = opendir(DATA_DIR);					//DATA_DIR "data/"
    if (!dirp) {
        fprintf(stderr, "%s: can't open data directory.\n", argv[0]);
    } else {
        while ((direntp = readdir(dirp)) != NULL) {
            if (strstr(direntp->d_name, ".obj")) {
                entries++;
                glutAddMenuEntry(direntp->d_name, -entries);
            }
        }
        closedir(dirp);
    }
    
    glutCreateMenu(menu);
    glutAddMenuEntry("Smooth", 0);
    glutAddMenuEntry("", 0);
    glutAddSubMenu("Models", models);						
    glutAddMenuEntry("", 0);
    glutAddMenuEntry("[w]   Toggle wireframe/filled", 'w');
    glutAddMenuEntry("[c]   Toggle culling on/off", 'c');
    glutAddMenuEntry("[n]   Toggle face/smooth normals", 'n');
    glutAddMenuEntry("[b]   Toggle bounding box on/off", 'b');
    glutAddMenuEntry("[p]   Toggle frame rate on/off", 'p');
    glutAddMenuEntry("[t]   Toggle model statistics", 't');
    glutAddMenuEntry("[m]   Toggle color/material/none mode", 'm');
    glutAddMenuEntry("[r]   Reverse polygon winding", 'r');
    glutAddMenuEntry("[s]   Scale model smaller", 's');
    glutAddMenuEntry("[S]   Scale model larger", 'S');
    glutAddMenuEntry("[o]   Weld redundant vertices", 'o');
    glutAddMenuEntry("[+]   Increase smoothing angle", '+');
    glutAddMenuEntry("[-]   Decrease smoothing angle", '-');
    glutAddMenuEntry("[W]   Write model to file (out.obj)", 'W');
	glutAddMenuEntry("[y]   Use rasterization algorithm", 'y');
    glutAddMenuEntry("", 0);
    glutAddMenuEntry("[Esc] Quit", 27);
    glutAttachMenu(GLUT_RIGHT_BUTTON);			//Attach right mouse click with menu
    	
    init();										//initialize
    
    glutMainLoop();								//glutMainLoop enters the GLUT event processing loop.
    return 0;
}
