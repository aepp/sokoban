#include <windows.h>
#include <iostream>
#include <stdlib.h>  
#include <gl\gl.h>
#include <gl\glu.h>
#include <gl\glut.h>
#include <stdio.h>
#include <iomanip>
#include <vector>
#include <string.h>
#include "field.c"
#include "lib\SOIL.h"

using namespace std;

char* texture_ground[] = {"images/tex_ground_0.jpg",
	  				      "images/tex_ground_1.jpg",
	  				      "images/tex_ground_2.jpg",};
char* texture_walls[]  = {"images/tex_walls_0.jpg",
	  				      "images/tex_walls_1.jpg",
	  				      "images/tex_walls_2.jpg"};
char* texture_dest[]   = {"images/tex_dest_0.jpg",
	  				      "images/tex_dest_1.jpg",
	  				      "images/tex_dest_2.jpg"};
char* texture_cubes[]  = {"images/tex_cubes_0.jpg",
	  				      "images/tex_cubes_1.jpg",
	  				      "images/tex_cubes_2.jpg"};
char* texture_player[] = {"images/tex_player_0.jpg",
	  				      "images/tex_player_1.jpg",
	  				      "images/tex_player_2.jpg"};
int skinNum = 0;
int skinCount = sizeof(texture_ground)/sizeof(char*);

int levelCount = sizeof(initLevel)/sizeof(char)/801; //Levelanzahl aus initLevel
int levelNum = 0; // Erstes Level
int fieldSize; // Groesse des Spielfeldes
int player,initPlayer; // Aktuelle Spielerposition; Startposition d. Spielers
int cubesCount; // Anzahl d. Kisten
int wallsCount; // Anzahl d. Waende
int destCount; // Anzahl d. Zielfelder
int groundCount; // Anzahl d. Spielfelder
int success; // Anzahl d. Kisten im Ziel
int mooves=0; // Anzahl d. gemachten Schritte
int ground[400],walls[400],dest[20],cubes[20],initCubes[20]; // Positionsarrays
bool next = false,noGround = false,showHelp=false,showLegend=false; //Hilfsflags
float colorR[20], colorG[20], colorB[20]; // Kistenfarbe: im Ziel/aufm Feld
int windowWidth = 1300;                // window size
int windowHeight = 680;

/*Beleuchtung und Kameraposition*/
float ambientLight[] = { 0.2f, 0.2f, 0.2f, 1.0f};
float diffuseLight[] = { 0.7f, 0.7f, 0.7f, 1.0f};
float noLight[]      = { 0.0f, 0.0f, 0.0f, 1.0f};
float lightPos[]     = { 100.0f, 300.0f, 100.0f, 1.0f};
float cameraPos[]    = { 20.0f, 600.0f, 300.0f, 0.0f};

/*GLUT Schriften*/
LPVOID glutFonts[7] = { 
    GLUT_BITMAP_9_BY_15, 
    GLUT_BITMAP_8_BY_13, 
    GLUT_BITMAP_TIMES_ROMAN_10, 
    GLUT_BITMAP_TIMES_ROMAN_24, 
    GLUT_BITMAP_HELVETICA_10, 
    GLUT_BITMAP_HELVETICA_12, 
    GLUT_BITMAP_HELVETICA_18 
}; 

GLuint tex;	//Storage for Textures

/*
  Hilfsfktionen d. Textausgabe
  
  Enter 2D damit Texte statisch aufm Bildschirm sind
  und sich mit der Kamera nicht bewegen.
*/
void glEnter2D(void) {
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    gluOrtho2D(0, windowWidth, 0, windowHeight);
 
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();
 
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_LIGHTING);
}
 
void glLeave2D(void) {
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();
 
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
}

/*Fkt. zur Textausgabe*/
void renderBitmapString(float x, float y, void *font, char *string){  
    char *c;
    glRasterPos2f(x,y);
    for (c=string; *c != '\0'; c++){
      glutBitmapCharacter(font, *c);
    }
}

/*Initiert alle Spielobjekte*/
void calcCount(){  
    int i=levelNum*800+levelNum; 
    int x = 0;
    cubesCount=0;
    wallsCount=0;
    destCount=0;
    groundCount=0;  
    player = 0;
    initPlayer = 0; 
    success = 0; 
    mooves = 0;
    char level[400];
    while (initLevel[i]!='N'){
        if(initLevel[i] == '\0'){i=0;levelNum=0;} 
        if(initLevel[i] != '\t'){ 
            level[x] = initLevel[i];
            x++;
        }     
        i++;
    }
    i=0;
    while (i<400){    
        if(level[i] == '0'){
            ground[groundCount]=i;
            groundCount++;
        }   
        else if(level[i] == '1'){
            walls[wallsCount]=i;
            wallsCount++;
        }        
        else if(level[i] == '2'){
            dest[destCount]=i;
            destCount++;
        } 
        else if(level[i] == '3'){
            ground[groundCount]=i;
            cubes[cubesCount]=i;
            initCubes[cubesCount]=i;
            groundCount++;
            cubesCount++;
        }  
        else if(level[i] == '4'){
            ground[groundCount]=i;
            groundCount++;
            player = i;
            initPlayer = i;
        }                    
        i++; 
    } 
}

/*Spielfeld/objekte zeichnen*/
void DrawModels(){ 
/*------------------------PRUEFE OB FERTIG------------------------------------*/
/*
    Prueft fuer jedes Wuerfel, ob es Ziel erreicht hat und wenn ja, dann 
    wird eine neue Farbe zugewiesen, sonst bleibt die Standardfarbe
*/      
    success=0;
    for(int i = 0; i<cubesCount; i++){
          for(int j = 0; j<destCount; j++){
              if (dest[j] == cubes[i]){
                 colorR[i] = 1;  // R-Komponente wenn fertig
                 colorG[i] = 0;  // G-Komponente wenn fertig
                 colorB[i] = 0;  // B-Komponente wenn fertig
                 success++;           
              }               
          }
    }   
    for(int i = 0; i<cubesCount; i++){
          for(int j = 0; j<groundCount; j++){
              if (ground[j] == cubes[i]){
                 colorR[i] = 1;  // R-Komponente normal
				 colorG[i] = 1;  // G-Komponente normal
                 colorB[i] = 1;  // B-Komponente normal
              }               
          }
    }       
/*--------------------------------FELD----------------------------------------*/ 
    glPushMatrix();
    tex = SOIL_load_OGL_texture(texture_ground[skinNum],0,1,16|2);
    for(int i = 0; i<groundCount; i++){                     
        glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D,tex);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                                          GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,
                                          GL_LINEAR);
	
			glColor3f(1, 1, 1);
						
			glNormal3f(0.0f, 1.0f, 0.0f);			// Top
			glTexCoord2f(0,0); glVertex3f(field[ground[i]][0]-15,-15,field[ground[i]][1]+15);	 // Top Left
			glTexCoord2f(1,0); glVertex3f(field[ground[i]][0]+15,-15,field[ground[i]][1]+15);	 // Top Right
			glTexCoord2f(1,1); glVertex3f(field[ground[i]][0]+15,-15,field[ground[i]][1]-15);	 // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[ground[i]][0]-15,-15,field[ground[i]][1]-15);	 // Bottom Left
		glEnd();
		glDisable(GL_TEXTURE_2D); 					
    }  
/*-------------------------------WAENDE---------------------------------------*/    
	tex = SOIL_load_OGL_texture(texture_walls[skinNum],0,1,16);

    for(int i = 0; i<wallsCount; i++){
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D,tex);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
			glColor3f(1,1,1);

			glNormal3f(0.0f, 0.0f, 1.0f);			// Back
			glTexCoord2f(0,0); glVertex3f(field[walls[i]][0]-15,15,field[walls[i]][1]+15);     // Top Left
			glTexCoord2f(1,0); glVertex3f(field[walls[i]][0]+15,15,field[walls[i]][1]+15);     // Top Right
			glTexCoord2f(1,1); glVertex3f(field[walls[i]][0]+15,-15,field[walls[i]][1]+15);    // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[walls[i]][0]-15,-15,field[walls[i]][1]+15);    // Bottom Left			
			
			glNormal3f(0.0f, 0.0f, 1.0f);			// Front			
			glTexCoord2f(0,0); glVertex3f(field[walls[i]][0]-15,15,field[walls[i]][1]-15);	   // Top Left
			glTexCoord2f(1,0); glVertex3f(field[walls[i]][0]+15,15,field[walls[i]][1]-15);	   // Top Right
			glTexCoord2f(1,1); glVertex3f(field[walls[i]][0]+15,-15,field[walls[i]][1]-15);	   // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[walls[i]][0]-15,-15,field[walls[i]][1]-15);	   // Bottom Left

			glNormal3f(-1.0f, 0.0f, 0.0f);			// Left
			glTexCoord2f(0,0); glVertex3f(field[walls[i]][0]-15,15,field[walls[i]][1]+15);     // Top Left
			glTexCoord2f(1,0); glVertex3f(field[walls[i]][0]-15,15,field[walls[i]][1]-15);     // Top Right
			glTexCoord2f(1,1); glVertex3f(field[walls[i]][0]-15,-15,field[walls[i]][1]-15);    // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[walls[i]][0]-15,-15,field[walls[i]][1]+15); 	// Bottom Left
			
			glNormal3f(1.0f, 0.0f, 0.0f);			// Right
			glTexCoord2f(0,0); glVertex3f(field[walls[i]][0]+15,15,field[walls[i]][1]+15);     // Top Left
			glTexCoord2f(1,0); glVertex3f(field[walls[i]][0]+15,15,field[walls[i]][1]-15);     // Top Right
			glTexCoord2f(1,1); glVertex3f(field[walls[i]][0]+15,-15,field[walls[i]][1]-15);    // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[walls[i]][0]+15,-15,field[walls[i]][1]+15);    // Bottom Left			
						
			glNormal3f(0.0f, 1.0f, 0.0f);			// Top
			glTexCoord2f(0,0); glVertex3f(field[walls[i]][0]-15,15,field[walls[i]][1]+15);	   // Top Left
			glTexCoord2f(1,0); glVertex3f(field[walls[i]][0]+15,15,field[walls[i]][1]+15);	   // Top Right
			glTexCoord2f(1,1); glVertex3f(field[walls[i]][0]+15,15,field[walls[i]][1]-15);	   // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[walls[i]][0]-15,15,field[walls[i]][1]-15);	   // Bottom Left
		glEnd();
		glDisable(GL_TEXTURE_2D); 					
    }
/*-------------------------------SPIELER--------------------------------------*/  
    tex = SOIL_load_OGL_texture(texture_player[skinNum],0,1,16|2);	
	glEnable(GL_TEXTURE_2D);
	glBegin(GL_QUADS);
		glBindTexture(GL_TEXTURE_2D,tex);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
		glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glColor3f(1, 1, 1);
		
		glNormal3f(0.0f, 0.0f, 1.0f);			// Back
		glTexCoord2f(0,0); glVertex3f(field[player][0]-15,15,field[player][1]+15);     // Top Left
		glTexCoord2f(1,0); glVertex3f(field[player][0]+15,15,field[player][1]+15);     // Top Right
		glTexCoord2f(1,1); glVertex3f(field[player][0]+15,-15,field[player][1]+15);    // Bottom Right
		glTexCoord2f(0,1); glVertex3f(field[player][0]-15,-15,field[player][1]+15);    // Bottom Left
		
		glNormal3f(0.0f, 0.0f, 1.0f);			// Front
		glTexCoord2f(0,0); glVertex3f(field[player][0]-15,15,field[player][1]-15);     // Top Left
		glTexCoord2f(1,0); glVertex3f(field[player][0]+15,15,field[player][1]-15);     // Top Right
		glTexCoord2f(1,1); glVertex3f(field[player][0]+15,-15,field[player][1]-15);    // Bottom Right
		glTexCoord2f(0,1); glVertex3f(field[player][0]-15,-15,field[player][1]-15);    // Bottom Left	

		glNormal3f(-1.0f, 0.0f, 0.0f);			// Left
		glTexCoord2f(0,0); glVertex3f(field[player][0]-15,15,field[player][1]+15);     // Top Left
		glTexCoord2f(1,0); glVertex3f(field[player][0]-15,15,field[player][1]-15);     // Top Right
		glTexCoord2f(1,1); glVertex3f(field[player][0]-15,-15,field[player][1]-15);    // Bottom Right
		glTexCoord2f(0,1); glVertex3f(field[player][0]-15,-15,field[player][1]+15);    // Bottom Left
		
		glNormal3f(1.0f, 0.0f, 0.0f);			// Right
		glTexCoord2f(0,0); glVertex3f(field[player][0]+15,15,field[player][1]+15);     // Top Left
		glTexCoord2f(1,0); glVertex3f(field[player][0]+15,15,field[player][1]-15);     // Top Right
		glTexCoord2f(1,1); glVertex3f(field[player][0]+15,-15,field[player][1]-15);    // Bottom Right
		glTexCoord2f(0,1); glVertex3f(field[player][0]+15,-15,field[player][1]+15);    // Bottom Left
		
		glNormal3f(0.0f, 1.0f, 0.0f);			// Top
		glTexCoord2f(0,0); glVertex3f(field[player][0]-15,15,field[player][1]+15);	   // Top Left
		glTexCoord2f(1,0); glVertex3f(field[player][0]+15,15,field[player][1]+15);	   // Top Right
		glTexCoord2f(1,1); glVertex3f(field[player][0]+15,15,field[player][1]-15);	   // Bottom Right
		glTexCoord2f(0,1); glVertex3f(field[player][0]-15,15,field[player][1]-15); 	   // Bottom Left
	glEnd();
	glDisable(GL_TEXTURE_2D); 	
/*----------------------------WUERFEL ZU VERSCHIEBEN--------------------------*/ 
    tex = SOIL_load_OGL_texture(texture_cubes[skinNum],0,1, 16|2);	
    glColor3f(1.0f, 1.0f, 1.0f); 
    for(int i = 0; i<cubesCount; i++){
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D,tex);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
			glColor3f(colorR[i], colorG[i], colorB[i]);	// Farbe des Wuerfels i
			
			glNormal3f(0.0f, 0.0f, 1.0f);			// Back
			glTexCoord2f(0,0); glVertex3f(field[cubes[i]][0]-15,15,field[cubes[i]][1]+15);     // Top Left
			glTexCoord2f(1,0); glVertex3f(field[cubes[i]][0]+15,15,field[cubes[i]][1]+15);     // Top Right
			glTexCoord2f(1,1); glVertex3f(field[cubes[i]][0]+15,-15,field[cubes[i]][1]+15);    // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[cubes[i]][0]-15,-15,field[cubes[i]][1]+15);    // Bottom Left			
			
			glNormal3f(0.0f, 0.0f, 1.0f);			// Front			
			glTexCoord2f(0,0); glVertex3f(field[cubes[i]][0]-15,15,field[cubes[i]][1]-15);	   // Top Left
			glTexCoord2f(1,0); glVertex3f(field[cubes[i]][0]+15,15,field[cubes[i]][1]-15);	   // Top Right
			glTexCoord2f(1,1); glVertex3f(field[cubes[i]][0]+15,-15,field[cubes[i]][1]-15);	   // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[cubes[i]][0]-15,-15,field[cubes[i]][1]-15);	   // Bottom Left

			glNormal3f(-1.0f, 0.0f, 0.0f);			// Left
			glTexCoord2f(0,0); glVertex3f(field[cubes[i]][0]-15,15,field[cubes[i]][1]+15);     // Top Left
			glTexCoord2f(1,0); glVertex3f(field[cubes[i]][0]-15,15,field[cubes[i]][1]-15);     // Top Right
			glTexCoord2f(1,1); glVertex3f(field[cubes[i]][0]-15,-15,field[cubes[i]][1]-15);    // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[cubes[i]][0]-15,-15,field[cubes[i]][1]+15);    // Bottom Left
			
			glNormal3f(1.0f, 0.0f, 0.0f);			// Right
			glTexCoord2f(0,0); glVertex3f(field[cubes[i]][0]+15,15,field[cubes[i]][1]+15);     // Top Left
			glTexCoord2f(1,0); glVertex3f(field[cubes[i]][0]+15,15,field[cubes[i]][1]-15);     // Top Right
			glTexCoord2f(1,1); glVertex3f(field[cubes[i]][0]+15,-15,field[cubes[i]][1]-15);    // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[cubes[i]][0]+15,-15,field[cubes[i]][1]+15);    // Bottom Left			
						
			glNormal3f(0.0f, 1.0f, 0.0f);			// Top
			glTexCoord2f(0,0); glVertex3f(field[cubes[i]][0]-15,15,field[cubes[i]][1]+15);	   // Top Left
			glTexCoord2f(1,0); glVertex3f(field[cubes[i]][0]+15,15,field[cubes[i]][1]+15);	   // Top Right
			glTexCoord2f(1,1); glVertex3f(field[cubes[i]][0]+15,15,field[cubes[i]][1]-15);	   // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[cubes[i]][0]-15,15,field[cubes[i]][1]-15);	   // Bottom Left
		glEnd();
		glDisable(GL_TEXTURE_2D); 		 
    }    
/*---------------------------------ZIEL---------------------------------------*/
    tex = SOIL_load_OGL_texture(texture_dest[skinNum],0,1,16|2|1);
    for(int i = 0; i<destCount; i++){
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D,tex);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
			                               GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf( GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	
            glColor3f(1.0f, 1.0f, 1.0f); 

			glNormal3f(0.0f, 1.0f, 0.0f);			// Top
			glTexCoord2f(0,0); glVertex3f(field[dest[i]][0]-15,-15,field[dest[i]][1]+15);	 // Top Left
			glTexCoord2f(1,0); glVertex3f(field[dest[i]][0]+15,-15,field[dest[i]][1]+15);	 // Top Right
			glTexCoord2f(1,1); glVertex3f(field[dest[i]][0]+15,-15,field[dest[i]][1]-15);	 // Bottom Right
			glTexCoord2f(0,1); glVertex3f(field[dest[i]][0]-15,-15,field[dest[i]][1]-15);	 // Bottom Left
		glEnd();
		glDisable(GL_TEXTURE_2D); 				   
    } 
/*---------------------------------PANEL--------------------------------------*/    
    glEnter2D();
    glColor3f(1, 1, 1);
	glRectf(0.0f, 0.0f, windowWidth, 30.0f);
    glLeave2D();     
/*---------------------------------TEXT---------------------------------------*/    
    char buffer[256];
    
    glEnter2D();
    if(showHelp){    
		glColor3f(1,1,1);		
	    sprintf(buffer,"Help:");     
		renderBitmapString(windowWidth-190,windowHeight-60,glutFonts[6],buffer);			 
	    glColor3f(1,1,0);           
	    sprintf(buffer,"'Arrow keys' to move");     
		renderBitmapString(windowWidth-250,windowHeight-80,glutFonts[5],buffer);			 
	    sprintf(buffer,"'W','A','S','D' to rotate camera");     
		renderBitmapString(windowWidth-250,windowHeight-100,glutFonts[5],buffer);			 
	    sprintf(buffer,"'Spacebar' to restart level");     
		renderBitmapString(windowWidth-250, windowHeight-120,glutFonts[5],buffer);			 
	    sprintf(buffer,"'Tab' to skip level");     
		renderBitmapString(windowWidth-250,windowHeight-140,glutFonts[5],buffer);			 
	    sprintf(buffer,"'Backspace' to go level back");     
		renderBitmapString(windowWidth-250,windowHeight-160,glutFonts[5],buffer);			 
	    sprintf(buffer,"'C' to change skin");     
		renderBitmapString(windowWidth-250,windowHeight-180,glutFonts[5],buffer);			 
	    sprintf(buffer,"'Right mouse button' context menu");     
		renderBitmapString(windowWidth-250,windowHeight-200,glutFonts[5],buffer);			 
	    sprintf(buffer,"'ESC' to quit");     
		renderBitmapString(windowWidth-250,windowHeight-220,glutFonts[5],buffer);			 
    }       
	else{
		glColor3f(1,1,0); 
	    sprintf(buffer,"Press 'H' for Help");     
		renderBitmapString(windowWidth-200,windowHeight-60,glutFonts[6],buffer);						 
    }   
    glLeave2D();   
/*---------------------------------LEGEND-------------------------------------*/    
    if(showLegend){    
		glColor3f(1,1,1);
		glEnter2D();
		    sprintf(buffer,"Legend");     
			renderBitmapString(200,windowHeight-60,glutFonts[6],buffer);						 
	    glLeave2D();                            
	    glPushMatrix();
	    glLoadIdentity();
	    gluOrtho2D(0, windowWidth, 0, windowHeight);
		
		//Legend: Spieler
		tex = SOIL_load_OGL_texture(texture_player[skinNum],0,1,16|2);	
		glEnable(GL_TEXTURE_2D);
		glDisable(GL_LIGHTING);
		glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D,tex);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,
                                          GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	
            glColor3f(1, 1, 1); 

			glTexCoord2f(0,0); glVertex3f(142,windowHeight-30,2);    
			glTexCoord2f(1,0); glVertex3f(192,windowHeight-30,2);   
			glTexCoord2f(1,1); glVertex3f(192,windowHeight-80,2);   
			glTexCoord2f(0,1); glVertex3f(142,windowHeight-80,2);    
		glEnd();
		glDisable(GL_TEXTURE_2D); 
		
		//Legend: Wuerfel
		tex = SOIL_load_OGL_texture(texture_cubes[skinNum],0,1,16|2);	
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D,tex);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			                                GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
            glColor3f(1, 1, 1); 			

			glTexCoord2f(0,0); glVertex3f(142,windowHeight-100,2);    
			glTexCoord2f(1,0); glVertex3f(192,windowHeight-100,2);   
			glTexCoord2f(1,1); glVertex3f(192,windowHeight-150,2);    
			glTexCoord2f(0,1); glVertex3f(142,windowHeight-150,2);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		//Legend: Ziel
		tex = SOIL_load_OGL_texture(texture_dest[skinNum],0,1, 16|2);	
		glEnable(GL_TEXTURE_2D);
		glBegin(GL_QUADS);
			glBindTexture(GL_TEXTURE_2D,tex);
			glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, 
			                                GL_LINEAR_MIPMAP_NEAREST);
			glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
            glColor3f(1.0f, 1.0f, 1.0f); 			

			glTexCoord2f(0,0); glVertex3f(142,windowHeight-170,2);    
			glTexCoord2f(1,0); glVertex3f(192,windowHeight-170,2);   
			glTexCoord2f(1,1); glVertex3f(192,windowHeight-220,2);    
			glTexCoord2f(0,1); glVertex3f(142,windowHeight-220,2);
		glEnd();
		glDisable(GL_TEXTURE_2D);

		glPushMatrix();     
		glColor3f(1,1,0);
		glEnter2D();
		    sprintf(buffer,"This is you");     
			renderBitmapString(50,windowHeight-105,glutFonts[5],buffer);
		    sprintf(buffer,"you have to push them");     
			renderBitmapString(50,windowHeight-165,glutFonts[5],buffer);
		    sprintf(buffer,"to these squares");     
			renderBitmapString(50,windowHeight-225,glutFonts[5],buffer);								
	    glLeave2D();                        
    }       
	else{
		glColor3f(1,1,0); 
		glEnter2D();	
		    sprintf(buffer,"Press 'L' to see legend");     
			renderBitmapString(60,windowHeight-60,glutFonts[6],buffer); 		
	    glLeave2D();
    }	      
    glColor3f(0,0,0);
    glEnter2D();
	    sprintf(buffer,"Sokoban 3D");     
		renderBitmapString(windowWidth-120,18,glutFonts[2],buffer); 
	    sprintf(buffer,"by A.Epp & W.Sawtschenko");     
		renderBitmapString(windowWidth-120,6,glutFonts[2],buffer); 	
	    sprintf(buffer,"Moves: %d",mooves);      
	    renderBitmapString(600,7,glutFonts[6],buffer); 
	    sprintf(buffer,"Ready: %d of %d", success,cubesCount);      
	    renderBitmapString(300,7,glutFonts[6],buffer);      
	    sprintf(buffer,"Level: %d of %d", levelNum+1,levelCount);
	    renderBitmapString(20,7,glutFonts[6],buffer);
    glLeave2D();     
/*----------------------------------------------------------------------------*/    
    glColor3f(1,1,0);

    if (success==cubesCount){
        glEnter2D();
		    sprintf(buffer,"YOU WIN!");
	        renderBitmapString(windowWidth/2-50, windowHeight-100,glutFonts[3],buffer);
		    sprintf(buffer,"Press 'ENTER' to continue");
	        renderBitmapString(windowWidth/2-100, windowHeight-130,glutFonts[6],buffer);  	        
        glLeave2D();     
        success=0;
        next=true;
    }   
}

void RenderScene(){
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluPerspective(60.0f, 1.0f, 1.0f, 4000.0f);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    gluLookAt(cameraPos[0], cameraPos[1], cameraPos[2], 
              0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    glViewport(0, 0, windowWidth, windowHeight);
    
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glLightfv(GL_LIGHT0, GL_AMBIENT, ambientLight);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuseLight);

    DrawModels();
    
    glutSwapBuffers();
}

void SetupRC(){  
    calcCount();
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GLUT_MULTISAMPLE);
    glEnable(GL_POLYGON_SMOOTH);
    glEnable(GL_POINT_SMOOTH);
    glClearColor(0.2f, 0.6f, 1.0f, 0.0f );
    glShadeModel(GL_SMOOTH);
    glEnable(GL_LIGHTING);
    glEnable(GL_COLOR_MATERIAL);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHT0);
}

void ProcessMenu(int value){
    switch(value){
        case 1:
            cameraPos[0] = 20.0f;
            cameraPos[1] = 600.0f;
            break;
        case 2:
            player = initPlayer;
            for(int i=0;i<10;i++){
                 cubes[i] = initCubes[i];
            }
            next = false;   
            break;
        case 3:
            exit(0);
            break;
        default:
            break;
    }
    glutPostRedisplay();
}

void KeyPressFunc(unsigned char key, int x, int y){
    switch (key){
    case 'a':
        cameraPos[0] += 5.0f;
        if(cameraPos[0] >= 500) cameraPos[0] = 500;
        break;
    case 'd':
        cameraPos[0] -= 5.0f;
        if(cameraPos[0] <= -500) cameraPos[0] = -500;
        break;
    case 'w':
        cameraPos[1] += 5.0f;
        if(cameraPos[1] >= 850) cameraPos[1] = 850;
        break;
    case 's':
        cameraPos[1] -= 5.0f;
        if(cameraPos[1] <= 100) cameraPos[1] = 100;
        break;
    case 'h':
        if(showHelp==true) showHelp=false;
        else if(showHelp==false) showHelp=true;
        break; 
    case 'l':
        if(showLegend==true) showLegend=false;
        else if(showLegend==false) showLegend=true;
        break;  
    case 'c':
        if(skinNum==skinCount-1) skinNum = 0;
        else skinNum++;
        break;				       
    case 8: /*BACKSPACE*/
        if(levelNum==0){levelNum=0;}
        else {levelNum--;cameraPos[0] = 20.0f;cameraPos[1] = 600.0f;}
        calcCount();
        break;         
    case 9: /*TABULATOR*/
        if(levelNum==levelCount-1){levelNum=levelCount-1;}
        else{levelNum++;cameraPos[0] = 20.0f;cameraPos[1] = 600.0f;calcCount();}
        break;      
    case 13: /*ENTER*/
        if(next){
            cameraPos[0] = 20.0f;
            cameraPos[1] = 600.0f;  				 
            next=false;
			levelNum++;
		    if(levelNum==levelCount)levelNum=0;
			calcCount();
        }
        break;   
    case 32: /*SPACE*/
        cameraPos[0] = 20.0f;
        cameraPos[1] = 600.0f;    
        player = initPlayer;
        for(int i=0;i<cubesCount;i++){
             cubes[i] = initCubes[i];
             colorR[i] = 0;
             colorG[i] = 0;
             colorB[i] = 0;
        }
        next = false;
        break;        
    case 27 : /* ESC */
        exit(0);
    }
    glutPostRedisplay();
}

void SpecialKeys(int key, int x, int y){
    int currentCube, stopCube, stopWall;
    bool mooved = false,noGround = true;
    stopCube=0; stopWall=0;
    
    if(key == GLUT_KEY_UP){  
		player -= 20;   // Spieler bewegen
        mooved = true; 
        for(int i = 0; i<cubesCount; i++){
            if(player == cubes[i]){
                cubes[i]-= 20; 
                currentCube = i;
                for(int j = 0; j<cubesCount; j++){ // Kollisionen mit Kisten
                      if (cubes[j] == cubes[currentCube]) stopCube++;                 
                }
                for(int j = 0; j<wallsCount; j++){ // Kollisionen mit Waenden
                      if (walls[j] == cubes[currentCube]) stopWall++;                 
                }
                if (stopCube>1 || stopWall>0) {
                    player += 20;
                    cubes[currentCube] += 20;
                    mooved = false;
                }      
				for(int j = 0; j<groundCount; j++){
                      if (ground[j] == cubes[currentCube]) noGround=false;    
                }
                for(int j = 0; j<destCount; j++){
                      if (dest[j] == cubes[currentCube]) noGround=false;    
                }
                if (noGround) {
                    cubes[currentCube] += 40;
                    mooved = false;
                }             
            }
        }
        for(int i = 0; i<wallsCount; i++){
            if(player == walls[i]){
   			    player += 20;
  				mooved = false;
		    } 
        }         
    }
    
    stopCube=0;
    if(key == GLUT_KEY_DOWN){
        player += 20;
        mooved = true;
        for(int i = 0; i<cubesCount; i++){
            if(player == cubes[i]){
                cubes[i]+= 20; 
                currentCube = i;
                for(int j = 0; j<cubesCount; j++){
                      if (cubes[j] == cubes[currentCube]) stopCube++;                 
                }
                for(int j = 0; j<wallsCount; j++){
                      if (walls[j] == cubes[currentCube]) stopWall++;                 
                }
                if (stopCube>1 || stopWall>0) {
                    player -= 20;
                    cubes[currentCube] -= 20;
                    mooved = false;
                }                 
            }
        }
        for(int i = 0; i<wallsCount; i++){
            if(player == walls[i]){
                player -= 20;
				mooved = false;
			} 
        }         
    }    
    
    stopCube=0;
    if(key == GLUT_KEY_LEFT){
        player -= 1;
        mooved = true;
        for(int i = 0; i<cubesCount; i++){
            if(player == cubes[i]){
                cubes[i]-= 1;
                currentCube = i; 
                for(int j = 0; j<cubesCount; j++){
                      if (cubes[j] == cubes[currentCube]) stopCube++;                 
                }
                for(int j = 0; j<wallsCount; j++){
                      if (walls[j] == cubes[currentCube]) stopWall++;                 
                }
                if (stopCube>1 || stopWall>0) {
                    player += 1;
                    cubes[currentCube] += 1;
                    mooved = false;
                }                      
            }
        }
        for(int i = 0; i<wallsCount; i++){
            if(player == walls[i]){
  			    player += 1; 
			    mooved = false;
		    } 
        } 
    }
    
    stopCube=0;
    if(key == GLUT_KEY_RIGHT){
        player += 1;
        mooved = true;
        for(int i = 0; i<cubesCount; i++){
            if(player == cubes[i]){
                cubes[i] += 1;
                currentCube = i;
                for(int j = 0; j<cubesCount; j++){
                      if (cubes[j] == cubes[currentCube]) stopCube++; 
                }
                for(int j = 0; j<wallsCount; j++){
                      if (walls[j] == cubes[currentCube]) stopWall++;                 
                }
                if (stopCube>1 || stopWall>0) {
                    player -= 1;
                    cubes[currentCube] -= 1;
                    mooved = false;
                }
            }   
        }   
        for(int i = 0; i<wallsCount; i++){
            if(player == walls[i]){
			    player -= 1; 
				mooved = false;
		    }
        }
	}
    if(mooved) mooves++;
    glutPostRedisplay();   
}

void ChangeSize(int w, int h){
    windowWidth = w;
    windowHeight = h;
}

int main(int argc, char* argv[]){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH | GLUT_MULTISAMPLE);
    glutInitWindowSize(windowWidth, windowHeight);
    glutInitWindowPosition(20, 40);
    glutCreateWindow("Sokoban 3D");
    glutReshapeFunc(ChangeSize);
    glutKeyboardFunc(KeyPressFunc);
    glutSpecialFunc(SpecialKeys);
    glutDisplayFunc(RenderScene);
    glutCreateMenu(ProcessMenu);
    glutAddMenuEntry("Reset camera", 1);
    glutAddMenuEntry("Restart level", 2);
    glutAddMenuEntry("Quit", 3);
    glutAttachMenu(GLUT_RIGHT_BUTTON);
    SetupRC();
    glutMainLoop();
    return 0;
}
