/*
 * Project 4 - TRING: a version of a carum pool game
 * Authors: Ali Faiz <afrazor@cc.gatech.edu> 
 *          Charles Brian Quinn <me@seebq.com>
 * Version: 0.8
 * Date:    Thur 21 Nov 2002
 * 
 * code segments and help provided by tutorials on:
 *  http://nehe.gamedev.net/
 *
 */

#include "defs.h"


/*
 * OpenGL variables
 */
/* ambient light in direction (10, 10, 10) */
GLfloat light1_x = 10.0;
GLfloat light1_y = 10.0;
GLfloat light1_z = 10.0;

/* eye point coordinates for view */
GLfloat eye_x = 0.0001;
GLfloat eye_y = 0;
GLfloat eye_z = 20;
GLfloat set_eye_x = 0.0001;
GLfloat set_eye_y = 0;
GLfloat set_eye_z = 20;

/* the shadow matrix */
GLfloat M[16];

/* Variables allowing for the rotation of the scene using the mouse */
float sphi=0, stheta=0, sdepth = 1;
int downX, downY, leftButton = 0, rightButton = 0;

GLUquadricObj *quadObj;

/* for displaying text */
int font=(int)GLUT_BITMAP_8_BY_13;
int bitmapHeight=13;
char s[30];

/* Mouse positions variables */
GLfloat startX, startY, endX, endY;
int motionX = 0;
int motionY = 0;
int xsize = 0, ysize = 0, aspect;

/* these are used to represent the 3 windows we display */
int window_main;
int window_3dview;
int window_score;

/* 
 * Game Play variables
 */
/* Radius dimensions of the balls and the board */
float ball_r = 1;
float board_r = 10;

int player = 1;                /* player 1 or 2 */
int turn = RED;                /* which ball to hit */
int p1score = 0, p2score = 0;  /* scores */
int no_motion = 1;             /* if all balls stopped or not */
int change_turn = 0;           /* if the player and ball turns can change */
int collide[] = {0,0,0};       /* which balls have been collided */
int cue_ball = -1;             /* which ball is the cue ball */

/* Arrays for ball velocity, position, and mass */
TVector ball_vel[3];                 /* holds velocity of balls */
TVector ball_pos[3];                 /* position of balls */
TVector old_pos[3];             /* old position of balls */
float mass[] = {0.5, 0.5, 0.5}; /* mass of the balls */
int ball_count;                  /* sets the number of balls */
double TimeStep = .6;           /* timestep of simulation */

/* variables for determining velocities */
float max_pullback = .2;
float angle = 0;

float friction = 0.015;
int english=NONE;
float english_x, english_y=0;

TVector veloc(0.0,0.0,0); 

/* Cylinder structure */
struct Cylinder {                          
  TVector _Position;
  TVector _Axis;
  double _Radius;
};

Cylinder cyl1;          /* the outer cylinder for board */


/*********************************************************/
/***************** INITIALIZATION ************************/
/*********************************************************/

/* openGL init function required */
void init(void)
{    
  glClearColor (0.0, 0.0, 0.0, 0.0);
  glShadeModel(GL_FLAT);
  glEnable(GL_DEPTH_TEST);
  glDepthFunc(GL_LEQUAL);

  /* lighting stuff */
  glShadeModel (GL_SMOOTH);

  glEnable(GL_LIGHTING);  /* enable lighting */
  glEnable(GL_LIGHT0);    /* enable light0 */
  glEnable(GL_NORMALIZE);
}

/* initialize variables */
void InitVars() {
  int i;
  
  /* create outer cylinder */
  cyl1._Position = TVector(0,0,0);
  cyl1._Axis = TVector(0,0,1);
  cyl1._Radius = board_r - ball_r;

  /* Set initial positions and velocities of balls */
  ball_count = 3;
  ball_vel[0] = veloc;
  ball_pos[0] = TVector(-2,-1,ball_r);
  ball_vel[1] = veloc;
  ball_pos[1] = TVector(1,2,ball_r);
  ball_vel[2] = veloc;
  ball_pos[2] = TVector(2,-2,ball_r);
}

/*********************************************************/
/********************* DRAWING ***************************/
/*********************************************************/

void reshape(int w, int h)
{
  /* Necessary to move scene with the mouse */
  xsize = w;
  ysize = h;
  aspect = xsize/ysize;

  glViewport(0, 0, xsize, ysize);
  glMatrixMode(GL_PROJECTION);
  glLoadIdentity();

  glFrustum (-1.0, 1.0, -1.0, 1.0, 1.5, 500.0); 

  glMatrixMode (GL_MODELVIEW);
  glLoadIdentity();

  /* assume a square window of size WINDOW_SIZE */
  gluLookAt (WINX/2.0,WINX/2.0,3.0*(WINX)/4.0,  WINX/2.0,WINX/2.0,0.0,
	     0.0,1.0,0.0);
}

/* Draws the green circular carum table with border and bumpers */
void drawBoard()
{
  GLfloat mat_border[] = {0.6, 0.6, 0.6, 0.0};

  glPushMatrix();  
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_black);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_border);

    /* draws the gray upraised border */
    glTranslatef(0,0,0.9);
    gluDisk(quadObj, board_r, board_r + (board_r * .05), 70, 70);

    glTranslatef(0,0,-1);
    gluCylinder( quadObj, board_r, board_r, 1, 50, 50);

    /* draws the green disk/board */
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_green);
    gluDisk(quadObj, 0, board_r, 50, 50);

  glPopMatrix();
}

/* Generic ball drawing function that also displays the balls' shadows */
void drawPoolBalls() {

  GLfloat mat_color[] = {0.0, 0.0, 0.0, 0.0};
  int i;

  for (i = 0; i < ball_count; i++) {
    switch(i) {
    case RED: 
      mat_color = mat_red;
      break;
    case YELLOW: 
      mat_color = mat_yellow;
      break;
    case BLUE: 
      mat_color = mat_blue;
      break;
    default:
      break;
    }

    glPushMatrix();  
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_color);
    glTranslatef(ball_pos[i].X(), ball_pos[i].Y(), ball_pos[i].Z());
    gluSphere(quadObj, ball_r, 20, 20); 
    glPopMatrix();
    
    glPushMatrix();
    glMultMatrixf(M);
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_black);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_black);
    glTranslatef(ball_pos[i].X(), ball_pos[i].Y(), ball_pos[i].Z());
    gluSphere(quadObj, ball_r, 20, 20);
    glPopMatrix();

    /* specular highlights have color 'mat_specular'*/
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
    
    /* shininess ('N') = mat_shininess*/
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);
  }
}

/* display cue stick arrow */
void display_arrow()
{
  float scale_factor = 0;
  
  glPushMatrix();
    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, mat_cue);
    glTranslatef(startX,startY,ball_r+1);
    glRotatef(angle+90, 0, 0, 1);
    glRotatef(90, 1, 0, 0);
    scale_factor = sqrt( (startX - endX) * (startX - endX) + 
			 (startY - endY) * (startY - endY) );
    glScalef(1,1,scale_factor/3); 
    gluCylinder(quadObj, 0.1, 0.2, 3, 50, 50);
  glPopMatrix();
}

/* openGL display function */
void display()
{
  GLfloat light_position[] = {0.0,0.0,0.0,0.0};
  light_position[0] = light1_x;
  light_position[1] = light1_y;
  light_position[2] = light1_z;

  /* the shadow matrix */
  M[0]= 1; M[4]= 0; M[8] =(-1*(light1_x)/light1_z); M[12]= 0;
  M[1]= 0; M[5]= 1; M[9] =(-1*(light1_y)/light1_z); M[13]= 0;
  M[2]= 0; M[6]= 0; M[10]= 0;                       M[14]= 0;
  M[3]= 0; M[7]= 0; M[11]= 0;                       M[15]= 1;

  /* specular highlights have color 'mat_specular'*/
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);
  /* shininess ('N') = mat_shininess*/
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  /* replaces the matrix in the top of the stack with the identity matrix */
  glLoadIdentity(); 

  /* If the window is the main one, we want the 2D view, otherwise we want
     the position alterable 3D view */
  if(glutGetWindow() == window_main)
    gluLookAt(set_eye_x,set_eye_y,set_eye_z, 0,0,0, 0,0,1); 
  else{
    gluLookAt(18,4,10, 0,0,0, 0,0,1); 
    
    glRotatef(sphi, 0, 0, 1);
    glRotatef(stheta, 1, 0, 0);
  }
    
  /* move light0 to 'light_position' */
  glLightfv(GL_LIGHT0, GL_POSITION, light_position); 

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  /* initialize the quadric object */
  quadObj = gluNewQuadric();

  /* Draws the TRING Board */
  drawBoard();

  /* Displays the cue arrow */
  if(no_motion)
    display_arrow();

  /* specular highlights have color 'mat_specular'*/
  glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, mat_specular);

  /* shininess ('N') = mat_shininess*/
  glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, mat_shininess);

  /* Display the balls */
  drawPoolBalls();

  glutSwapBuffers();    
  glFlush();
}

/* Functions for setting up score window */
void renderBitmapString(float x, float y, void *font,char *string)
{  
  char *c;
  glRasterPos2f(x, y);
  for (c=string; *c != '\0'; c++) {
    glutBitmapCharacter(font, *c);
  }
}

/* Draws text on the score window */
void draw_text(GLint x, GLint y, char* s, GLfloat r, GLfloat g, GLfloat b)
{
  int lines;
  char* p;
  
  glMatrixMode(GL_PROJECTION);
  glPushMatrix();
  glLoadIdentity();
  glOrtho(0.0, glutGet(GLUT_WINDOW_WIDTH), 
	  0.0, glutGet(GLUT_WINDOW_HEIGHT), -1.0, 1.0);
  glMatrixMode(GL_MODELVIEW);
  glPushMatrix();
  glLoadIdentity();
  glColor3f(r,g,b);
  glRasterPos2i(x, y);
  for(p = s, lines = 0; *p; p++) {
    if (*p == '\n') {
      lines++;
      glRasterPos2i(x, y-(lines*18));
    }
    glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *p);
  }
  glPopMatrix();
  glMatrixMode(GL_PROJECTION);
  glPopMatrix();
  glMatrixMode(GL_MODELVIEW);
}

/* this display function is called by the score window */
void display_score() 
{
  char player1[20];
  char player2[20];
  char ball[20];
  char turnstr[20];
  char *color = " ";
  
  switch(turn){
  case YELLOW:
    color = "Yellow";
    break;
  case BLUE:
    color = "Blue";
    break;
  case RED:
    color = "Red";
  default:
    break;
  }

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  /* if player 1 or 2 win, exit the game */
  if(p1score == 21 || p2score == 21){
    draw_text(5, 50, "GAME OVER!", 1.0, 0.0, 0.0); 
    if(p1score == 21)
      draw_text(5, 30, "PLAYER 1 WINS", 1.0, 0.0, 0.0);    
    else
      draw_text(5, 30, "PLAYER 2 WINS", 1.0, 0.0, 0.0);    
  }

  else{
    sprintf(ball, "Ball: %s", color);
    sprintf(turnstr, "Turn: Player %d", player);
    sprintf(player1, "Player 1: %d", p1score);
    sprintf(player2, "Player 2: %d", p2score);
    
    draw_text(5, 70, turnstr, 1.0, 0.0, 0.0);
    draw_text(5, 50, ball, 1.0, 0.0, 0.0); 
    draw_text(5, 30, player1, 1.0, 0.0, 0.0);
    draw_text(5, 10, player2, 1.0, 0.0, 0.0);
  }

  glutSwapBuffers();
}

/*********************************************************/
/********************** INPUT ****************************/
/*********************************************************/

/* Mouse handler function */
void mouse(int button, int state, int x, int y)
{
  int i;
  float pullback_x, pullback_y = 0;

  if (no_motion) {
    switch (button) {
    case GLUT_LEFT_BUTTON:
      if (state == GLUT_DOWN) {
	/* Scaling the screen map to the board */
	startX = (GLfloat) (WINY - y - 400) * -0.033333333; 
	startY = (GLfloat) (x - 400) * 0.0333333333; 

	/* The for loop determines if cue ball's start postion is on a ball */
	for (i = 0; i < ball_count; i++) {
	  if((startX < (ball_pos[i].X()+ball_r))&& 
	     (startX>(ball_pos[i].X()-ball_r))
	     && (startY<(ball_pos[i].Y()+ball_r))&&
	     (startY>(ball_pos[i].Y()-ball_r)))
	    {
	      //if the cue is on a ball, determine if it's the ball's 
	      //turn to be hit
	      switch(turn){
	      case RED:
		if(i == RED)
		  cue_ball = i;
		break;
	      case YELLOW:
		if(i == YELLOW)
		  cue_ball = i;
		break;
	      case BLUE:
		if(i == BLUE)
		  cue_ball = i;
		break;
	      default:
		break;
	      }
	      if(cue_ball != -1){
		startX = ball_pos[i].X();
		startY = ball_pos[i].Y();
	      }
	    }
	}
      }
      else {
	/* Scaling the screen map to the board */
	endX = (GLfloat) (WINY - y - 400) * -0.033333;
	endY = (GLfloat) (x - 400) * 0.033333; 

	pullback_x = startX-endX;
	pullback_y = startY-endY;

	pullback_x *= max_pullback;
	pullback_y *= max_pullback;

	TVector new_veloc(pullback_x, pullback_y, 0.0f);

	/* if a ball has been selected validly, use the pullback to determine
	   the new velocity of the ball */
	if (cue_ball != -1) {

	  ball_vel[cue_ball] = new_veloc;
	  no_motion = 0;
	  change_turn = 1;
	}
	cue_ball = -1;
      }
      break;
    case GLUT_MIDDLE_BUTTON:
      if (state == GLUT_DOWN) {
      }
    default:
      break;
    }
  }
}

/* openGL mouse motion function */
void motion(int x, int y)
{
  endX = (GLfloat) (WINY - y - 400) * -0.033333;
  endY = (GLfloat) (x - 400) * 0.033333; 
  angle = (180 / 3.141592) * atan2(endY-startY, endX-startX);
  glutPostRedisplay();
} 

/* Mouse handler function for the 3d-view window*/
void mouse_3d(int button, int state, int x, int y) 
{
  downX = x; 
  downY = y;
  leftButton = ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN));
  rightButton = ((button == GLUT_RIGHT_BUTTON) && (state == GLUT_DOWN));

  glutPostRedisplay();
}

/* For mouse button pressing handler for 3d-view window */
void motion_3d(int x, int y) 
{
  if (leftButton) 
    { 
      /* rotate */
      sphi+=(float)(x-downX)/4.0;
      stheta+=(float)(downY-y)/4.0;
    }
  if(rightButton)
    {
      /* scale */
      sdepth = (float) (downY-y)/500.0;
      if(sdepth<0.5)
	sdepth = 0.55;
    }
  
  downX = x;   downY = y;
  glutPostRedisplay();
}

/* openGL keyboard function */
void keyboard (unsigned char key, int x, int y)
{
  /* Keyboard keys that move the view and light position */
   switch (key) {
      case 27:
         exit(0);
         break;
      default:
         break;
   }
}

/*********************************************************/
/********************* PHYSICS ***************************/
/*********************************************************/

/* Most of the physics code was gathered from http://nehe.gamedev.net/ */


/* find if any of the current balls intersect with each other in the 
 * current timestep.  returns the index of the 2 intersecting balls, 
 * the point and time of intersection */
int find_collision(TVector& point, double& timePoint, double time2, 
	     int& ball1, int& ball2) {

  TVector relativeV;
  TRay rays;
  double Mytime = 0.0, Add = time2/150.0, timedummy = 10000, timedummy2 = -1;
  TVector posi;
  
  /* Test all balls against each other in 150 small steps */
  for (int i = 0; i < ball_count - 1; i++){
    for (int j = i+1; j < ball_count; j++){

      // Find Distance
      relativeV = ball_vel[i]-ball_vel[j];
      rays = TRay(old_pos[i],TVector::unit(relativeV));
      Mytime = 0.0;
      
      // If distance detween centers greater than 2*radius an 
      // intersection occurred loop to find the exact intersection point
      if ( (rays.dist(old_pos[j])) > ball_r * 2 ) 
	continue;  
      
      while (Mytime < time2) {
	Mytime += Add;
	posi = old_pos[i] + relativeV*Mytime;
	if (posi.dist(old_pos[j]) <= ball_r * 2 ) {
	  point = posi;
	  if (timedummy > (Mytime - Add)) timedummy = Mytime-Add;
	  ball1 = i;
	  ball2 = j;
	  break;
	}
      }
    }
  }
  
  if (timedummy!=10000) { 
    timePoint=timedummy;
    return 1;
  }
  
  return 0;
}

/* fast intersection function between ray/cylinder */
int intersect_cylinder(const Cylinder& cylinder, const TVector& position, 
			     const TVector& direction, double& lamda, 
			     TVector& pNormal, TVector& newposition) {

  TVector RC;
  double d;
  double t,s;
  TVector n,O;
  double ln;
  double in,out;

  TVector::subtract(position,cylinder._Position,RC);
  TVector::cross(direction,cylinder._Axis,n);
  
  ln = n.mag();
  if ( (ln<ZERO)&&(ln>-ZERO) ) 
    return 0;
  
  n.unit();
  
  d = fabs( RC.dot(n) );

  if (d <= cylinder._Radius) {
    TVector::cross(RC,cylinder._Axis,O);
    t = - O.dot(n)/ln;
    TVector::cross(n,cylinder._Axis,O);
    O.unit();
    s = fabs( sqrt(cylinder._Radius * cylinder._Radius - d*d) / 
	      direction.dot(O) );
    
    in = t-s;
    out = t+s;
    if (in<-ZERO) {
      if (out<-ZERO) return 0;
      else lamda = out;
    }
    else
      if (out<-ZERO) {
	lamda = in;
      }
      else
	if (in<out) lamda = in;
	else lamda=out;
    
    newposition = position+direction*lamda;
    TVector HB = newposition-cylinder._Position;
    pNormal = HB - cylinder._Axis*(HB.dot(cylinder._Axis));
    pNormal.unit();
    
    return 1;
  }
  
  return 0;
}

/* Gives a point dependant on ball collisions */
void give_point(int ball1, int ball2)
{

  /* In order to determine if points should be awarded, we have to 
   * check all possible combination of balls hitting to result in a 
   * double hit
   */
  switch(turn){
  case RED:
    collide[RED] = 1;
    if(ball1 == RED){
      if(ball2 == YELLOW)
	collide[YELLOW] = 1;
      if(ball2 == BLUE)
	collide[BLUE] = 1;
    }
    if(ball2 == RED){
      if(ball1 == YELLOW)
	collide[YELLOW] = 1;
      if(ball1 == BLUE)
	collide[BLUE] = 1;
    }
    break;
  case YELLOW:
    collide[YELLOW] = 1;
    if(ball1 == YELLOW){
      if(ball2 == RED)
	collide[RED] = 1;
      if(ball2 == BLUE)
	collide[BLUE] = 1;
    }
    if(ball2 == YELLOW){
      if(ball1 == RED)
	collide[RED] = 1;
      if(ball1 == BLUE)
	collide[BLUE] = 1;
    }
    break;
  case BLUE:
    collide[BLUE] = 1;
    if(ball1 == BLUE){
      if(ball2 == YELLOW)
	collide[YELLOW] = 1;
      if(ball2 == RED)
	collide[RED] = 1;
    }
    if(ball2 == BLUE){
      if(ball1 == YELLOW)
	collide[YELLOW] = 1;
      if(ball1 == RED)
	collide[RED] = 1;
    }
    break;
  }

}

/* Changes turns for balls and players */
void determine_turn()
{
  /* Change the player's turn when the balls stop moving */
  if(no_motion && change_turn){ 
    
    /* switch players */
    if(player==1){
      if(collide[RED] && collide[YELLOW] && collide[BLUE])
	p1score++;
      else
	player=2;
    }
    else{
      /* if all the balls were hit, add a point */
      if(collide[RED] && collide[YELLOW] && collide[BLUE])
	p2score++;
      else
	player=1;
    }

    /* reset the ball collisions */
    collide[RED] = 0; collide[YELLOW] = 0; collide[BLUE] = 0;

    /* switch the balls */
    switch(turn){
    case RED:
      turn = YELLOW;
      break;
    case YELLOW:
      turn = BLUE;
      break;
    case BLUE:
      turn = RED;
    default:
      break;
    }

    glutSetWindow(window_score);
    glutPostRedisplay();

    glutSetWindow(window_main);
    change_turn = 0;
  }
}

/* 
 *The timestep and collision situations are used to find points of
 * contact and new velocities after balls hit each other
 */
void compute_velocities()
{
  double rt,rt2,rt4,lamda=10000;
  TVector norm,uveloc;
  TVector normal,point;
  double RestTime,BallTime;
  TVector col_pos;
  int ball=0,ball1,ball2;
  TVector Nc;
  int j;
  
  RestTime = TimeStep;
  lamda = 1000;

  /* Compute velocity for next timestep using Euler equations */
  for (j = 0; j < ball_count; j++) {
    ball_vel[j]-= (ball_vel[j] * friction);
  }

  /* while timestep not over */
  while (RestTime>ZERO) {
    lamda = 10000;   /* initialize to very large value */
    
    /* For all the balls find closest intersection between 
     * balls and planes/cylinders */
    for (int i = 0; i < ball_count; i++) {
      /* compute new position and distance */
      old_pos[i] = ball_pos[i];
      TVector::unit(ball_vel[i],uveloc);
      ball_pos[i] = ball_pos[i]+ball_vel[i]*RestTime;
      rt2 = old_pos[i].dist(ball_pos[i]);

      /* Now test intersection with the outer cylinder */      
      if (intersect_cylinder(cyl1,old_pos[i],uveloc,rt,norm,Nc)) {

	rt4 = rt*RestTime / rt2;

	if (rt4 <= lamda) { 
	  if (rt4 <= RestTime+ZERO)
	    if (! ((rt <= ZERO)&&(uveloc.dot(norm) < ZERO)) ) {
	      normal=norm;
	      point=Nc;
	      lamda=rt4;
	      ball=i;
	    }
	}	
      }
    }
    
    /* After all balls were tested with planes/cylinders test for collision 
     * between them and replace if collision time smaller */
    if (find_collision(col_pos,BallTime,RestTime,ball1,ball2)) {
      if ( (lamda == 10000) || (lamda > BallTime) ) {
	RestTime = RestTime-BallTime;
	
	TVector pb1,pb2,xaxis,U1x,U1y,U2x,U2y,V1x,V1y,V2x,V2y;
	double a,b;
	
	pb1=old_pos[ball1]+ball_vel[ball1]*BallTime; // Find position of ball1
	pb2=old_pos[ball2]+ball_vel[ball2]*BallTime; // Find position of ball2
	xaxis=(pb2-pb1).unit();
	
	a=xaxis.dot(ball_vel[ball1]);

	/* U1 and U2 are the velocity vectors of the two spheres at the 
	 * time of impact */
	U1x=xaxis*a;
	U1y=ball_vel[ball1]-U1x;
	
	xaxis=(pb1-pb2).unit();


	b=xaxis.dot(ball_vel[ball2]);
	U2x=xaxis*b;
	U2y=ball_vel[ball2]-U2x;


	/* V1,V2 are the new velocities after the impact */
	V1x=( (U1x*mass[ball1]+U2x*mass[ball2] - 
	     (U1x-U2x))*mass[ball2] ) * 
	  (1 / (mass[ball1] + mass[ball2]));
	V2x=( (U1x*mass[ball1]+U2x*mass[ball2] - 
	     (U2x-U1x))*mass[ball1] ) * 
	  (1 / (mass[ball1] + mass[ball2]));
	V1y=U1y;
	V2y=U2y;
	
	for (j=0;j<ball_count;j++)
	  ball_pos[j]=old_pos[j]+ball_vel[j]*BallTime;
	
	ball_vel[ball1]=V1x+V1y;
	ball_vel[ball2]=V2x+V2y;
	/* continue; */
	
	give_point(ball1, ball2);
      }
    }
    
    /* End of tests */
    /* If test occured move simulation for the correct timestep */
    /* and compute response for the colliding ball */
    if (lamda!=10000) {		 
      RestTime-=lamda;
      
      for (j=0;j<ball_count;j++)
	ball_pos[j]=old_pos[j]+ball_vel[j]*lamda;
      
      rt2=ball_vel[ball].mag();
      ball_vel[ball].unit();
      ball_vel[ball]=TVector::unit( (normal*(2*normal.dot(-ball_vel[ball]))) + 
				      ball_vel[ball] );
      ball_vel[ball]=ball_vel[ball]*rt2;
    }
    else {
      RestTime=0;    
    }
  }
}


/* Idle function that takes care of movement, collisions, turn, and score
 * updating
 */
void MyIdleFunc() 
{

  if ( (ball_vel[0].mag() <= 0.005) && (ball_vel[1].mag() <= 0.005) && 
       (ball_vel[2].mag() <= 0.005)) {
    no_motion = 1;
  }
  else{
    no_motion = 0;
  }

  /* changes the turns if necessary */
  determine_turn();

  /* updates the velocities */
  compute_velocities(); 

  glutSetWindow(window_3dview);
  glutPostRedisplay();
  glutSetWindow(window_main);
  glutPostRedisplay();
}


void RunIdleFunc(void) {   glutIdleFunc(MyIdleFunc); }
void PauseIdleFunc(void) {   glutIdleFunc(NULL); }

/*
 * main() - receives a command line argument and initializes openGL stuff
 */
int main(int argc, char** argv)
{

  InitVars();

  /* Setup the main 2D view window */
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
  glutInitWindowSize(WINX, WINY);
  glutInitWindowPosition(50, 50);
  window_main = glutCreateWindow(TITLE);
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);

  /* Setup the 3D view window */
  glutInitWindowSize(300, 300);
  glutInitWindowPosition(WINX + 100, 50);
  window_3dview = glutCreateWindow(TITLE_3D);
  init();
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutMouseFunc(mouse_3d);
  glutMotionFunc(motion_3d);

  /* Setup the score window */
  glutInitWindowSize(200, 100);
  glutInitWindowPosition(WINX + 100, 400);
  window_score = glutCreateWindow(TITLE_SCORE);
  init();
  glutDisplayFunc(display_score);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);
  glutIdleFunc(MyIdleFunc); 

  glutMainLoop();
  return 0; 
}
