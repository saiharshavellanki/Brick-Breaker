#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
#include <ctime>
#include <list>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
//#include<mpg123.h>
using namespace std;

struct VAO {
    GLuint VertexArrayID;
    GLuint VertexBuffer;
    GLuint ColorBuffer;

    GLenum PrimitiveMode;
    GLenum FillMode;
    int NumVertices;
};
typedef struct VAO VAO;

struct GLMatrices {
	glm::mat4 projection;
	glm::mat4 model;
	glm::mat4 view;
	GLuint MatrixID;
} Matrices;

GLuint programID;

/* Function to load Shaders - Use it as it is */
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path) {

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open())
	{
		std::string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::string Line = "";
		while(getline(FragmentShaderStream, Line))
			FragmentShaderCode += "\n" + Line;
		FragmentShaderStream.close();
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;

	// Compile Vertex Shader
	printf("Compiling shader : %s\n", vertex_file_path);
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> VertexShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &VertexShaderErrorMessage[0]);

	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> FragmentShaderErrorMessage(InfoLogLength);
	glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
	fprintf(stdout, "%s\n", &FragmentShaderErrorMessage[0]);

	// Link the program
	fprintf(stdout, "Linking program\n");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	std::vector<char> ProgramErrorMessage( max(InfoLogLength, int(1)) );
	glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
	fprintf(stdout, "%s\n", &ProgramErrorMessage[0]);

	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}

static void error_callback(int error, const char* description)
{
    fprintf(stderr, "Error: %s\n", description);
}

void quit(GLFWwindow *window)
{
    glfwDestroyWindow(window);
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}


/* Generate VAO, VBOs and return VAO handle */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat* color_buffer_data, GLenum fill_mode=GL_FILL)
{
    struct VAO* vao = new struct VAO;
    vao->PrimitiveMode = primitive_mode;
    vao->NumVertices = numVertices;
    vao->FillMode = fill_mode;

    // Create Vertex Array Object
    // Should be done after CreateWindow and before any other GL calls
    glGenVertexArrays(1, &(vao->VertexArrayID)); // VAO
    glGenBuffers (1, &(vao->VertexBuffer)); // VBO - vertices
    glGenBuffers (1, &(vao->ColorBuffer));  // VBO - colors

    glBindVertexArray (vao->VertexArrayID); // Bind the VAO
    glBindBuffer (GL_ARRAY_BUFFER, vao->VertexBuffer); // Bind the VBO vertices
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), vertex_buffer_data, GL_STATIC_DRAW); // Copy the vertices into VBO
    glVertexAttribPointer(
                          0,                  // attribute 0. Vertices
                          3,                  // size (x,y,z)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    glBindBuffer (GL_ARRAY_BUFFER, vao->ColorBuffer); // Bind the VBO colors
    glBufferData (GL_ARRAY_BUFFER, 3*numVertices*sizeof(GLfloat), color_buffer_data, GL_STATIC_DRAW);  // Copy the vertex colors
    glVertexAttribPointer(
                          1,                  // attribute 1. Color
                          3,                  // size (r,g,b)
                          GL_FLOAT,           // type
                          GL_FALSE,           // normalized?
                          0,                  // stride
                          (void*)0            // array buffer offset
                          );

    return vao;
}

/* Generate VAO, VBOs and return VAO handle - Common Color for all vertices */
struct VAO* create3DObject (GLenum primitive_mode, int numVertices, const GLfloat* vertex_buffer_data, const GLfloat red, const GLfloat green, const GLfloat blue, GLenum fill_mode=GL_FILL)
{
    GLfloat* color_buffer_data = new GLfloat [3*numVertices];
    for (int i=0; i<numVertices; i++) {
        color_buffer_data [3*i] = red;
        color_buffer_data [3*i + 1] = green;
        color_buffer_data [3*i + 2] = blue;
    }

    return create3DObject(primitive_mode, numVertices, vertex_buffer_data, color_buffer_data, fill_mode);
}

/* Render the VBOs handled by VAO */
void draw3DObject (struct VAO* vao)
{
    // Change the Fill Mode for this object
    glPolygonMode (GL_FRONT_AND_BACK, vao->FillMode);

    // Bind the VAO to use
    glBindVertexArray (vao->VertexArrayID);

    // Enable Vertex Attribute 0 - 3d Vertices
    glEnableVertexAttribArray(0);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->VertexBuffer);

    // Enable Vertex Attribute 1 - Color
    glEnableVertexAttribArray(1);
    // Bind the VBO to use
    glBindBuffer(GL_ARRAY_BUFFER, vao->ColorBuffer);

    // Draw the geometry !
    glDrawArrays(vao->PrimitiveMode, 0, vao->NumVertices); // Starting from vertex 0; 3 vertices total -> 1 triangle
}

/**************************
 * Customizable functions *
 **************************/

float triangle_rot_dir = 1,pan=0,zoom=1,pany=0,blockdist=0.02;
float rectangle_rot_dir = 1;
bool triangle_rot_status = true;
bool rectangle_rot_status = true;
float binpos[3]={0},brickpos[10005]={0},laserpos[3]={0},brick_width,brick_height,leftbrick[100005][5],rightbrick[100005][5],bullets[10005][5];
double last_update_time,current_time,fall_down_time,shoot_time,show_time,bullet_update_time,black_create_time;
int ctrl=0,alt=0,leftleft=0,leftright=0,rightright=0,rightleft=0,laserup=0,laserdown=0,laserrotup=0,laserrotdown=0,panup=0,pandown=0,score=0;
int leftstart=0,rightstart=0,panleft=0,panright=0,zoomin=0,zoomout=0,dele[100005],cou=0,leftend=-1,rightend=-1,fire=0,bulletstart=0,bulletend=-1;
/* Executed when a regular key is pressed/released/held-down */
/* Prefered for Keyboard events */
int leftclick=0,rightclick=0,redbin=0,greenbin=0,canon=0,increasespeed=0,decreasespeed=0,collidedmirror[100005];
int gameover=0,leftvisit[1000]={0},rightvisit[1000]={0},onlaser=0,pause=0,leftlives=3,rightlives=3;
double mouse_x,mouse_y;
void keyboard (GLFWwindow* window, int key, int scancode, int action, int mods)
{
     // Function is called first on GLFW_PRESS.

    if (action == GLFW_RELEASE) {
        switch (key) {
            case GLFW_KEY_C:
                rectangle_rot_status = !rectangle_rot_status;
                break;
            case GLFW_KEY_P:
                triangle_rot_status = !triangle_rot_status;
                break;
            case GLFW_KEY_X:
                // do something ..
                break;
            default:
                break;
        }
    }
    else if (action == GLFW_PRESS) {
        switch (key) {
            case GLFW_KEY_ESCAPE:
                quit(window);
                break;
            default:
                break;
        }
    }
    if(action==GLFW_PRESS)
    {
      if(key==GLFW_KEY_P)
      pause=1-pause;
      //if(key==GLFW_KEY_R)
      //pause=0;
      if(key==GLFW_KEY_N)
      increasespeed=1;
      if(key==GLFW_KEY_M)
      decreasespeed=1;
       if(key==GLFW_KEY_LEFT)
       panleft=1;
       if(key==GLFW_KEY_RIGHT)
       panright=1;
       if(key==GLFW_KEY_H)
       panup=1;
       if(key==GLFW_KEY_L)
       pandown=1;
      if(key==GLFW_KEY_SPACE)
      {
        fire=1;
      }
       if(key==GLFW_KEY_UP)
       {
         zoomin=1;
       }
       if(key==GLFW_KEY_DOWN)
       {
         zoomout=1;
       }
      if(key==GLFW_KEY_S)
      laserup=1;
      if(key==GLFW_KEY_F)
      {
        laserdown=1;
      }
      if(key==GLFW_KEY_A)
      {
        laserrotup=1;
      }
      if(key==GLFW_KEY_D)
      {
        laserrotdown=1;
      }
      if(key==GLFW_KEY_RIGHT_CONTROL || key==GLFW_KEY_LEFT_CONTROL)
      {
        ctrl=1;
      }
        if(ctrl==1 && key==GLFW_KEY_LEFT)
        {
          leftleft=1;
        }
        if(ctrl==1 && key==GLFW_KEY_RIGHT)
        {
          leftright=1;
        }

        if(key==GLFW_KEY_RIGHT_ALT || key==GLFW_KEY_LEFT_ALT)
        {
          alt=1;
        }
          if(alt==1 && key==GLFW_KEY_LEFT)
          {
            rightleft=1;
          }
          if(alt==1 && key==GLFW_KEY_RIGHT)
          {
            rightright=1;
          }
        }
        else if(action==GLFW_RELEASE)
        {
        //   if(key==GLFW_KEY_P)
        //   pause=1;
        //   if(key==GLFW_KEY_R)
        //   pause=0;
          if(key==GLFW_KEY_N)
          increasespeed=0;
          if(key==GLFW_KEY_M)
          decreasespeed=0;
          if(key==GLFW_KEY_H)
          panup=0;
          if(key==GLFW_KEY_L)
          pandown=0;
          if(key==GLFW_KEY_LEFT)
           panleft=0;
          if(key==GLFW_KEY_RIGHT)
           panright=0;
           if(key==GLFW_KEY_SPACE)
           {
             fire=0;
           }
           if(key==GLFW_KEY_UP)
             zoomin=0;
           if(key==GLFW_KEY_DOWN)
           {
             zoomout=0;
           }
          if(key==GLFW_KEY_RIGHT_CONTROL || key==GLFW_KEY_LEFT_CONTROL)
          {
            ctrl=0;
            leftright=0;
            leftleft=0;
          }
          if(key==GLFW_KEY_RIGHT_ALT || key==GLFW_KEY_LEFT_ALT)
          {
            alt=0;
            rightright=0;
            rightleft=0;
          }
          if(key==GLFW_KEY_LEFT)
          {
            leftleft=0;
            rightleft=0;
          }
          if(key==GLFW_KEY_RIGHT)
          {
            leftright=0;
            rightright=0;
          }
          if(key==GLFW_KEY_S)
          {
            laserup=0;
          }
          if(key==GLFW_KEY_F)
          {
            laserdown=0;
          }
          if(key==GLFW_KEY_A)
          {
            laserrotup=0;
          }
          if(key==GLFW_KEY_D)
          {
            laserrotdown=0;
          }
        }
}

/* Executed for character input (like in text boxes) */
void keyboardChar (GLFWwindow* window, unsigned int key)
{
	switch (key) {
		case 'Q':
		case 'q':
            quit(window);
            break;
		default:
			break;
	}
}

/* Executed when a mouse button is pressed/released */
void mouseButton (GLFWwindow* window, int button, int action, int mods)
{
   if(action==GLFW_PRESS)
   {
     if(button==GLFW_MOUSE_BUTTON_LEFT)
     {
        leftclick=1;
        glfwGetCursorPos(window, &mouse_x, &mouse_y);
        mouse_x=(mouse_x-500)/125;
        mouse_y=(500-mouse_y)/125;
        float s,c;
        s=sin(laserpos[2]*5*M_PI/180.0f);
        c=cos(laserpos[2]*5*M_PI/180.0f);
        //cout<<mouse_x<<" "<<mouse_y<<endl;
        if(mouse_x>=-4 && mouse_x<=-3.25 && mouse_y>=0.5+laserpos[1] && mouse_y<=laserpos[1]+1)
        {
          onlaser=1;
        }
        else if(mouse_y>=-0.125*c+laserpos[1]+0.75 && mouse_y<=0.625*s+0.125*c+laserpos[1]+0.75 && mouse_x>=-0.125*s-3.375 && mouse_x<=0.625*c+0.125*s-3.375)
        {
          onlaser=1;
        }
        else if(mouse_x>=-1.75+binpos[1] && mouse_x<=1+binpos[1]-1.75 && mouse_y<=-2.5 && mouse_y>=-4)
        {
          redbin=1;
        }
        else if(mouse_x>=1.5+binpos[2] && mouse_x<=1+1.5+binpos[2] && mouse_y<=2.5 && mouse_y>=-4)
        {
          greenbin=1;
        }
        else
        {
          if(mouse_x>=-3.375)
          {
          float init_x,init_y;
          init_x=-3.375;
          init_y=laserpos[1]+0.75;
      //    cout<<mouse_y<<" "<<init_y<<endl;
          float angle=atan((mouse_y-init_y)/(mouse_x-init_x));
          angle=(angle*180.0f)/M_PI;
      //    cout<<angle<<endl;
          if(angle>=-80 && angle<=80)
          {
          laserpos[2]=angle/5;
          bulletend=(bulletend+1)%100;
          bullets[bulletend][0]=-3.375+0.625*cos(angle*M_PI/180.0f);
          bullets[bulletend][1]=laserpos[1]+0.75+0.625*sin(angle*M_PI/180.0f);
          bullets[bulletend][2]=0.4;
          bullets[bulletend][3]=0.05;
          bullets[bulletend][4]=angle;
         }
       }
     }
     }
     if(button==GLFW_MOUSE_BUTTON_RIGHT)
     {
       glfwGetCursorPos(window, &mouse_x, &mouse_y);
       rightclick=1;
       mouse_x=(mouse_x-500)/125;
       mouse_y=(500-mouse_y)/125;
     }
   }
   if(action==GLFW_RELEASE)
   {
     if(button==GLFW_MOUSE_BUTTON_LEFT)
     {
        leftclick=0;
        redbin=0;
        greenbin=0;
        onlaser=0;
     }
     if(button==GLFW_MOUSE_BUTTON_RIGHT)
     {
       rightclick=0;
     }
   }
}

void mouseZoom(GLFWwindow* window,double xoffset,double yoffset)
{
  if(yoffset==1)
  {
      if(zoom>0.5)
      zoom-=0.01;
  }
  if(yoffset==-1)
  {
      if(zoom<1)
      zoom+=0.01;
  }
}
/* Executed when window is resized to 'width' and 'height' */
/* Modify the bounds of the screen here in glm::ortho or Field of View in glm::Perspective */
void reshapeWindow (GLFWwindow* window, int width, int height)
{
    int fbwidth=width, fbheight=height;
    /* With Retina display on Mac OS X, GLFW's FramebufferSize
     is different from WindowSize */
    glfwGetFramebufferSize(window, &fbwidth, &fbheight);

	GLfloat fov = 90.0f;

	// sets the viewport of openGL renderer
	glViewport (0, 0, (GLsizei) fbwidth, (GLsizei) fbheight);

	// set the projection matrix as perspective
	/* glMatrixMode (GL_PROJECTION);
	   glLoadIdentity ();
	   gluPerspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1, 500.0); */
	// Store the projection matrix in a variable for future use
    // Perspective projection for 3D views
    // Matrices.projection = glm::perspective (fov, (GLfloat) fbwidth / (GLfloat) fbheight, 0.1f, 500.0f);

    // Ortho projection for 2D views
    Matrices.projection = glm::ortho(-4.0f, 4.0f, -4.0f, 4.0f, 0.1f, 500.0f);
}

VAO *triangle, *bin[3],*circle[4],*brick[10005],*laser[3],*semicircle,*mirror[5],*leftside[10005],*rightside[10005],*bullet[10005],*temp,*segment[100005],*lives[7];

void createcircle(int p,float r,float R,float G,float B,float x,float y)
{
  GLfloat vertex_buffer_data[9*360],color_buffer_data[9*360];
  int i;
  for(i=0;i<360;i++)
  {
    vertex_buffer_data[9*i]=x+r*cos(i*M_PI/180.0);
    vertex_buffer_data[9*i+1]=y+r*sin(i*M_PI/180.0);
    vertex_buffer_data[9*i+2]=0;
    vertex_buffer_data[9*i+3]=x+r*cos((i+1)*M_PI/180.0);
    vertex_buffer_data[9*i+4]=y+r*sin((i+1)*M_PI/180.0);
    vertex_buffer_data[9*i+5]=0;
    vertex_buffer_data[9*i+6]=x;
    vertex_buffer_data[9*i+7]=y;
    vertex_buffer_data[9*i+8]=0;
  }
  for(i=0;i<9*360;i+=3)
  {
    color_buffer_data[i]=R;
    color_buffer_data[i+1]=G;
    color_buffer_data[i+2]=B;
  }
 circle[p] = create3DObject(GL_TRIANGLES, 1080, vertex_buffer_data, color_buffer_data, GL_FILL);
}


void createsemicircle(float r,float R,float G,float B)
{
  GLfloat vertex_buffer_data[9*360],color_buffer_data[9*360];
  int i;
  for(i=0;i<360;i++)
  {
    vertex_buffer_data[9*i]=r*cos(i*M_PI/180.0);
    vertex_buffer_data[9*i+1]=r*sin(i*M_PI/180.0);
    vertex_buffer_data[9*i+2]=0;
    vertex_buffer_data[9*i+3]=r*cos((i+1)*M_PI/180.0);
    vertex_buffer_data[9*i+4]=r*sin((i+1)*M_PI/180.0);
    vertex_buffer_data[9*i+5]=0;
    vertex_buffer_data[9*i+6]=0;
    vertex_buffer_data[9*i+7]=0;
    vertex_buffer_data[9*i+8]=0;
  }
  for(i=0;i<9*360;i+=3)
  {
    color_buffer_data[i]=R;
    color_buffer_data[i+1]=G;
    color_buffer_data[i+2]=B;
  }
 semicircle = create3DObject(GL_TRIANGLES, 1080, vertex_buffer_data, color_buffer_data, GL_FILL);
}

// Creates the rectangle object used in this sample code
void createRectangle(float x,float y,float length,float width,float R,float G,float B,int flag,int i)
{
  // GL3 accepts only Triangles. Quads are not supported
  GLfloat vertex_buffer_data [] = {
    x,y-width,0, // vertex 1
    x,y,0, // vertex 2
    x+length,y,0, // vertex 3

    x+length,y,0, // vertex 3
    x+length,y-width,0, // vertex 4
    x,y-width,0  // vertex 1
  };

    GLfloat color_buffer_data [] = {
      R,G,B, // color 1
      R,G,B, // color 2
      R,G,B, // color 3

      R,G,B, // color 3
      R,G,B, // color 4
      R,G,B  // color 1
    };

  // create3DObject creates and returns a handle to a VAO that can be used later
  if(flag==1)
  bin[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else if(flag==2)
  brick[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else if(flag==3)
  laser[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else  if(flag==4)
  mirror[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else if(flag==5)
  leftside[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else if(flag==6)
  rightside[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else if(flag==7)
  bullet[i]= create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else if(flag==9)
  segment[i]=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
  else
  temp=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}
void createlives(float x,float y,int i)
{
  float length=0.2,width=0.2,R=1,G=0.2,B=0.6;
  // GL3 accepts only Triangles. Quads are not supported
  GLfloat vertex_buffer_data [] = {
    x,y-width,0, // vertex 1
    x,y,0, // vertex 2
    x+length,y,0, // vertex 3

    x+length,y,0, // vertex 3
    x+length,y-width,0, // vertex 4
    x,y-width,0  // vertex 1
  };

    GLfloat color_buffer_data [] = {
      R,G,B, // color 1
      R,G,B, // color 2
      R,G,B, // color 3

      R,G,B, // color 3
      R,G,B, // color 4
      R,G,B  // color 1
    };
lives[i]=create3DObject(GL_TRIANGLES, 6, vertex_buffer_data, color_buffer_data, GL_FILL);
}

float camera_rotation_angle = 90;
float rectangle_rotation = 0;
float triangle_rotation = 0;

/* Render the scene with openGL */
/* Edit this function according to your assignment */
int checkhit(float a,float b,float x1,float x2,float y1,float y2)
{
  if(a>=x1 && a<=x2 && b>=y1 && b<=y2)
  return 1;
  else
  return 0;
}
void increaseblockdist()
{
  blockdist=0.02+(score/25)*0.002;
}
void collisionwithbrick()
{
  int i,p,h,r,u;
  int j;
  float c1,c2,c4,c3,bx,by,l,w,angle,x,y,len,wid,leng,wids;
  for(i=bulletstart;i!=(bulletend+1)%100;i=(i+1)%100)
  {
    for(j=leftstart;j!=(leftend+1)%100;j=(j+1)%100)
    {
      bx=bullets[i][0];
      by=bullets[i][1];
      l=bullets[i][2];
      w=bullets[i][3];
      angle=(bullets[i][4]*M_PI)/180.0f;
      x=leftbrick[j][1];
      y=leftbrick[j][2];
      len=leftbrick[j][3];
      wid=leftbrick[j][4];
      // c1=(2*bx+w*sin(angle)+l*cos(angle))/2;
      // c2=(2*by-w*cos(angle)+l*sin(angle))/2;
      c1=bx+(l*cos(angle))/2;
      c2=by+(l*sin(angle))/2;
      c3=x+len/2;
      c4=y-wid/2;
      if(fabs(c1-c3)<=(l+len)/2 && fabs(c2-c4)<=(w+wid)/2)
      {
        system("mpg123 -vC sounds/4.mp3 &");
        bullets[i][0]=10;
        bullets[i][1]=10;
       leftbrick[j][1]=100;
       leftbrick[j][2]=100;
       if(leftbrick[j][0]==0)
       score+=2;
       else
       score-=1;
       increaseblockdist();
      }
    }
  }
  for(i=bulletstart;i!=(bulletend+1)%100;i=(i+1)%100)
  {
    for(j=rightstart;j!=(rightend+1)%100;j=(j+1)%100)
    {
      bx=bullets[i][0];
      by=bullets[i][1];
      l=bullets[i][2];
      w=bullets[i][3];
      angle=bullets[i][4]*M_PI/180.0f;
      x=rightbrick[j][1];
      y=rightbrick[j][2];
      len=rightbrick[j][3];
      wid=rightbrick[j][4];
      c1=bx+(l*cos(angle))/2;
      c2=by+(l*sin(angle))/2;
      // c1=(2*bx+w*sin(angle)+l*cos(angle))/2;
      // c2=(2*by-w*cos(angle)+l*sin(angle))/2;
      c3=x+len/2;
      c4=y-wid/2;
      if(fabs(c1-c3)<=(l+len)/2 && fabs(c2-c4)<=(w+wid)/2)
      {
        system("mpg123 -vC sounds/4.mp3 &");
        bullets[i][0]=10;
        bullets[i][1]=10;
       rightbrick[j][1]=100;
       rightbrick[j][2]=100;
       if(rightbrick[j][0]==0)
       score+=2;
       else
       score-=1;
       increaseblockdist();
      }
    }
  }
}
int checkpoint(float x,float y,float a,float b,float c)
{
  float ans;
  ans=a*x+b*y+c;
  if(ans>=0)
  return 1;
  else
  return -1;
}
int checkinside(float x,float y,float a,float b)
{
  if(y>=a && y<=b)
  return 1;
  else
  return -1;
}
void collisionwithleftbrick(float x,float y,float dy,float uy,int number)
{
  float a,b,c,bx,by,l,w,angle,a1,b1,c1,intersection_x,intersection_y,slope,final_angle;
  int p,h,u,r,i;
  a=1;
  b=0;
  c=-1*x;
  for(i=bulletstart;i!=(bulletend+1)%100;i=(i+1)%100)
  {
    bx=bullets[i][0];
    by=bullets[i][1];
    l=bullets[i][2];
    w=bullets[i][3];
    angle=(bullets[i][4]*M_PI)/180.0f;
    p=checkpoint(bx,by,a,b,c);
    h=checkpoint(bx+l*cos(angle),by+l*sin(angle),a,b,c);
    u=checkpoint(bx+w*sin(angle),by-w*cos(angle),a,b,c);
    r=checkpoint(bx+w*sin(angle)+l*cos(angle),by-w*cos(angle)+l*sin(angle),a,b,c);
    if((p==1 && h==1 && u==1 && r==1) || (p==-1 && h==-1 && u==-1 && r==-1))
    {
      //cout<<"no\n";
    }
    else
    {
      angle=angle*180.0f/M_PI;
      if(angle!=90)
        {
        a1=-1.0*tan(angle*M_PI/180);
        b1=1;
        c1=tan(angle*M_PI/180)*bx-by;
        }
        else
        {
          a1=1;
          b1=0;
          c1=-1*bx;
        }
        intersection_x=(b*c1-b1*c)/(b1*a-b*a1);
        intersection_y=(a*c1-a1*c)/(b*a1-b1*a);
//        cout<<intersection_x<<" "<<intersection_y<<" "<<dy<<" "<<uy<<endl;
        if(intersection_y>=dy && intersection_y<=uy)
        {
        leftbrick[number][1]=10;
        leftbrick[number][2]=10;
        bullets[i][0]=10;
        bullets[i][1]=10;
         }
       if(angle!=90)
       {
       a1=-1.0*tan(angle*M_PI/180);
       b1=1;
       c1=tan(angle*M_PI/180)*(bx+w*sin(angle))-(by-w*cos(angle));
       }
       else
       {
         a1=1;
         b1=0;
         c1=-1*(bx+w*sin(angle));
       }
       intersection_x=(b*c1-b1*c)/(b1*a-b*a1);
       intersection_y=(a*c1-a1*c)/(b*a1-b1*a);
       //cout<<intersection_x<<" "<<intersection_y<<" "<<dy<<" "<<uy<<endl;
       if(intersection_y>=dy && intersection_y<=uy)
       {
       leftbrick[number][1]=10;
       leftbrick[number][2]=10;
       //collidedmirror[i]=number;
       bullets[i][0]=100;
       bullets[i][1]=100;
       //bullets[i][4]=final_angle;
      }
      }
    }
}
void collisionwithmirror(float mirror_angle,float x,float y,float dy,float uy,int number)
{
  float a,b,c,bx,by,l,w,angle,a1,b1,c1,intersection_x,intersection_y,slope,final_angle;
  int p,h,u,r,i;
  if(mirror_angle!=90)
  {
  slope=tan((mirror_angle*M_PI)/180.0f);
  a=-1.0*slope;
  b=1;
  c=slope*x-y;
  }
 else
 {
  a=1;
  b=0;
  c=-1*x;
 }
  for(i=bulletstart;i!=(bulletend+1)%100;i=(i+1)%100)
  {
    bx=bullets[i][0];
    by=bullets[i][1];
    l=bullets[i][2];
    w=bullets[i][3];
    angle=(bullets[i][4]*M_PI)/180.0f;
    p=checkpoint(bx,by,a,b,c);
    h=checkpoint(bx+l*cos(angle),by+l*sin(angle),a,b,c);
    u=checkpoint(bx+w*sin(angle),by-w*cos(angle),a,b,c);
    r=checkpoint(bx+w*sin(angle)+l*cos(angle),by-w*cos(angle)+l*sin(angle),a,b,c);
    if((p==1 && h==1 && u==1 && r==1) || (p==-1 && h==-1 && u==-1 && r==-1))
    {
    }
    else
    {
      p=checkinside(bx,by,dy,uy);
      h=checkinside(bx+l*cos(angle),by+l*sin(angle),dy,uy);
      u=checkinside(bx+w*sin(angle),by-w*cos(angle),dy,uy);
      r=checkinside(bx+w*sin(angle)+l*cos(angle),by-w*cos(angle)+l*sin(angle),dy,uy);
      angle=angle*180.0f/M_PI;
      if(p==1 || h==1 || u==1 || r==1)
      {
        //cout<<"yes\n";
        if(angle!=90)
        {
        a1=-1.0*tan(angle*M_PI/180);
        b1=1;
        c1=tan(angle*M_PI/180)*bx-by;
        }
        else
        {
          a1=1;
          b1=0;
          c1=-1*bx;
        }
        intersection_x=(b*c1-b1*c)/(b1*a-b*a1);
        intersection_y=(a*c1-a1*c)/(b*a1-b1*a);
        final_angle=2*mirror_angle-bullets[i][4];
        //cout<<intersection_x<<" "<<intersection_y<<" "<<final_angle<<" "<<fl<<endl;
        if(intersection_y>=dy && intersection_y<=uy && collidedmirror[i]!=number)
        {
        collidedmirror[i]=number;
        bullets[i][0]=intersection_x;
        bullets[i][1]=intersection_y;
        bullets[i][4]=final_angle;
        system("mpg123 -vC sounds/2.mp3 &");
       }
      }
    }
  }
}
int checkinredbin(float x,float y)
{
  if(x>=-1.75+binpos[1] && x<=1+binpos[1]-1.75 && y<=-2.5 && y>=-4)
  return 1;
  else
  return -1;
}
int checkingreenbin(float x,float y)
{
  if(x>=1.5+binpos[2] && x<=1+1.5+binpos[2] && y<=-2.5 && y>=-4)
  {
//    cout<<"s\n"<<endl;
  return 1;
 }
  else
  return -1;
}

void draw (double xpos,double ypos)
{
  if(pause==1)
  return;
  if(zoomin==1)
  {
    if(zoom>0.5)
    zoom-=0.01;
  }
  if(zoomout==1)
  {
    if(zoom<1)
    zoom+=0.01;
  }
  if(panleft==1)
    pan-=0.1;
  if(panright==1)
    pan+=0.1;
  if(panup==1)
    pany+=0.1;
  if(pandown==1)
    pany-=0.1;
  if(rightclick==1)
  {
  pan+=xpos-mouse_x;
  mouse_x= xpos;
  pany+=ypos-mouse_y;
  mouse_y = ypos;
  }
  if(-4*zoom+pan<-4)
    pan+=(-4+4*zoom-pan);
  if(4*zoom+pan>4)
    pan-=4*zoom+pan-4;
  if(-4*zoom+pany<-4)
  pany+=-4+4*zoom-pany;
  if(4*zoom+pany>4)
    pany-=4*zoom+pany-4;
  Matrices.projection = glm::ortho(-4.0f*zoom+pan, 4.0f*zoom+pan, -4.0f*zoom+pany, 4.0f*zoom+pany, 0.1f, 500.0f);

  // clear the color and depth in the frame buffer
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // use the loaded shader program
  // Don't change unless you know what you are doing
  glUseProgram (programID);

  // Eye - Location of camera. Don't change unless you are sure!!
  glm::vec3 eye ( 5*cos(camera_rotation_angle*M_PI/180.0f), 0, 5*sin(camera_rotation_angle*M_PI/180.0f) );
  // Target - Where is the camera looking at.  Don't change unless you are sure!!
  glm::vec3 target (0, 0, 0);
  // Up - Up vector defines tilt of camera.  Don't change unless you are sure!!
  glm::vec3 up (0, 1, 0);

  // Compute Camera matrix (view)
  // Matrices.view = glm::lookAt( eye, target, up ); // Rotating Camera for 3D
  //  Don't change unless you are sure!!
  Matrices.view = glm::lookAt(glm::vec3(0,0,3), glm::vec3(0,0,0), glm::vec3(0,1,0)); // Fixed camera for 2D (ortho) in XY plane

  // Compute ViewProject matrix as view/camera might not be changed for this frame (basic scenario)
  //  Don't change unless you are sure!!
  glm::mat4 VP = Matrices.projection * Matrices.view;

  // Send our transformation to the currently bound shader, in the "MVP" uniform
  // For each model you render, since the MVP will be different (at least the M part)
  //  Don't change unless you are sure!!
  glm::mat4 MVP;	// MVP = Projection * View * Model

  // Pop matrix to undo transformations till last push matrix instead of recomputing model matrix
  // glPopMatrix ();
  // if(leftclick==1 && xpos>=-1.75+binpos[1] && xpos<=1+binpos[1]-1.75 && ypos<=-2.5 && ypos>=-4 && greenbin==0)
  // {
  //    redbin=1;
  //  }
 if(redbin==1)
 {
   binpos[1]+=xpos-mouse_x;
   mouse_x=xpos;
   mouse_y=ypos;
  if(-1.75+binpos[1]<-2.928)
  {
    //redbin=0;
    binpos[1]=-2.928+1.75;
  }
    if(-0.75+binpos[1]>-0.712)
   {
    // redbin=0;
    binpos[1]=-0.712+0.75;
  }
 }
  if(leftleft==1)
  {
  binpos[1]-=0.02;
  if(-1.75+binpos[1]<-2.928)
  binpos[1]+=0.02;
  }
  if(leftright==1)
  {
    binpos[1]+=0.02;
    if(-0.75+binpos[1]>-0.712)
    binpos[1]-=0.02;
  }
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatebin= glm::translate (glm::vec3(-1+binpos[1]-0.75, -4, 0));        // glTranslatef
//  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatebin);// * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(redbin==1)
  createRectangle (0,1.5,1,1.5,1,0,0,1,1);
  else
  createRectangle (0,1.5,1,1.5,1,0.3,0.3,1,1);
  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bin[1]);
// if(leftclick==1 && xpos>=1.5+binpos[2] && xpos<=1+1.5+binpos[2] && ypos<=2.5 && ypos>=-4 && redbin==0)
// {
//   greenbin=1;
// }
if(greenbin==1)
{
  binpos[2]+=xpos-mouse_x;
  mouse_x=xpos;
  mouse_y=ypos;
  if(1.5+binpos[2]<-0.264)
  {
  binpos[2]=-0.264-1.5;
//  greenbin=0;
 }
 if(2.5+binpos[2]>2.712)
 {
   binpos[2]=2.712-2.5;
  // greenbin=0;
 }
}
if(rightleft==1){
  binpos[2]-=0.02;
  if(1.5+binpos[2]<-0.264)
  binpos[2]+=0.02;
}
if(rightright==1)
{
  binpos[2]+=0.02;
  //if(2.5+binpos[2]>3.2)
  if(2.5+binpos[2]>2.712)
  binpos[2]-=0.02;
}
  Matrices.model = glm::mat4(1.0f);
  translatebin = glm::translate (glm::vec3(1.5+binpos[2], -4, 0));        // glTranslatef
  //  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatebin);// * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(greenbin==1)
  createRectangle (0,1.5,1,1.5,0,0.6,0,1,2);
  else
  createRectangle (0,1.5,1,1.5,0.1,1,0.1,1,2);
  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(bin[2]);


if(onlaser==1)
{
  //cout<<"yes\n";
    laserpos[1]+=ypos-mouse_y;
  //  cout<<laserpos[1]<<endl;
    mouse_x=xpos;
    mouse_y=ypos;
    if(laserpos[1]>3)
    laserpos[1]=3;
    if(0.5+laserpos[1]<-2.5)
    laserpos[1]=-3;
}
if(laserup==1)
{
  laserpos[1]+=0.02;
  if(laserpos[1]>3)
  laserpos[1]-=0.02;
}
if(laserdown==1)
{
  laserpos[1]-=0.02;
  if(0.5+laserpos[1]<-2.5)
  laserpos[1]+=0.02;
}
  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatelaser = glm::translate (glm::vec3(0,0+laserpos[1], 0));        // glTranslatef
  //  glm::mat4 rotateRectangle = glm::rotate((float)(rectangle_rotation*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatelaser);// * rotateRectangle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(onlaser==1)
  {
  createRectangle (-4,1,0.75,0.5,0,0,1,3,1);
  }
  else
  {
    createRectangle (-4,1,0.75,0.5,0.4,0.4,1,3,1);
  }
  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(laser[1]);

if(laserrotup==1)
{
  laserpos[2]+=0.1;
  if(laserpos[2]>18)
  laserpos[2]-=0.1;
}
if(laserrotdown==1)
{
  laserpos[2]-=0.1;
  if(laserpos[2]<-18)
  laserpos[2]+=0.1;
}
  Matrices.model = glm::mat4(1.0f);
//  glm::mat4 translatelaser2= glm::translate (glm::vec3(-3.25,0.75, 0));
  translatelaser = glm::translate (glm::vec3(-3.375,laserpos[1]+0.75, 0));        // glTranslatef
  glm::mat4 rotatelaser = glm::rotate((float)(laserpos[2]*5*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatelaser * rotatelaser);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
  if(onlaser==1)
  createRectangle(0,0.125,0.5,0.25,0,0,1,3,2);
  else
  createRectangle(0,0.125,0.5,0.25,0.4,0.4,1,3,2);
  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(laser[2]);


  Matrices.model = glm::mat4(1.0f);

  glm::mat4 translatecircle1= glm::translate (glm::vec3(0.5,0,0));        // glTranslatef
  translatelaser= glm::translate (glm::vec3(-3.375,laserpos[1]+0.75,0));        // glTranslatef
  rotatelaser = glm::rotate((float)(laserpos[2]*5*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatelaser * rotatelaser* translatecircle1);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
if(onlaser==1)
createcircle(3,0.125,0,0,1,0,0);
else
createcircle(3,0.125,0.4,0.4,1,0,0);
      // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(circle[3]);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatecircle= glm::translate (glm::vec3(-0.5+binpos[1]-0.75,-2.5, 0));        // glTranslatef
   glm::mat4 rotatecircle = glm::rotate((float)(-60*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatecircle * rotatecircle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);


  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(circle[1]);

  Matrices.model = glm::mat4(1.0f);

  translatecircle= glm::translate (glm::vec3(2+binpos[2],-2.5, 0));        // glTranslatef
  rotatecircle = glm::rotate((float)(-60*M_PI/180.0f), glm::vec3(1,0,0)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatecircle * rotatecircle);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(circle[2]);


  Matrices.model = glm::mat4(1.0f);
  glm::mat4 translatemirror= glm::translate (glm::vec3(-0.75,0, 0));        // glTranslatef
 glm::mat4 rotatemirror = glm::rotate((float)(60*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
  Matrices.model *= (translatemirror * rotatemirror);
  MVP = VP * Matrices.model;
  glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

  // draw3DObject draws the VAO given to it using current MVP matrix
  draw3DObject(mirror[1]);


    Matrices.model = glm::mat4(1.0f);
    translatemirror= glm::translate (glm::vec3(2.75,-1.5, 0));        // glTranslatef
    rotatemirror = glm::rotate((float)(60*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translatemirror * rotatemirror);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(mirror[2]);

    Matrices.model = glm::mat4(1.0f);
    translatemirror= glm::translate (glm::vec3(-0.25,3,0));         // glTranslatef
    rotatemirror = glm::rotate((float)(120*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translatemirror * rotatemirror);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(mirror[3]);

    Matrices.model = glm::mat4(1.0f);
    translatemirror= glm::translate (glm::vec3(3,2, 0));        // glTranslatef
    rotatemirror = glm::rotate((float)(120*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
    Matrices.model *= (translatemirror * rotatemirror);
    MVP = VP * Matrices.model;
    glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

    // draw3DObject draws the VAO given to it using current MVP matrix
    draw3DObject(mirror[4]);
 int i;
 int j;
 float leng,wids,c1,c2,c4,c3;
 if(current_time - bullet_update_time >=0.01)
 {
   bullet_update_time = current_time;
   for(i=bulletstart;i!=(bulletend+1)%100;i=(i+1)%100)
   {
     bullets[i][0]=bullets[i][0]+0.025*cos(bullets[i][4]*M_PI/180.0f);
     bullets[i][1]=bullets[i][1]+0.025*sin(bullets[i][4]*M_PI/180.0f);
   }

  i=bulletstart;
   while((bullets[i][0]>4 || bullets[i][0]<-4 || bullets[i][1]>4 || bullets[i][1]<-4) && i<=bulletend)
   {
     i=(i+1)%100;
     bulletstart=(bulletstart+1)%100;
   }
  }
if(increasespeed==1)
blockdist+=0.002;
if(decreasespeed==1)
{
blockdist-=0.002;
if(blockdist<0.02)
blockdist=0.02;
}
 if(current_time-show_time>=0.01)
 {
  show_time=current_time;
  for(i=leftstart;i!=(leftend+1)%100;i=(i+1)%100)
  {
    leftbrick[i][2]-=(blockdist);
    if(leftbrick[i][2]<=-2.2)
    leftbrick[i][2]-=0.4;
  }

  i=leftstart;
  while(int(leftbrick[i][2])<-7)
  {
    leftstart=(leftstart+1)%100;
    i=(i+1)%100;
  }
  for(i=rightstart;i!=(rightend+1)%100;i=(i+1)%100)
  {
    rightbrick[i][2]-=blockdist;
    if(rightbrick[i][2]<=-2.2)
    rightbrick[i][2]-=0.4;
  }
  //collisionwithbrick();
  i=rightstart;
  while(int(rightbrick[i][2])<-7)
  {
    rightstart=(rightstart+1)%100;
    i=(i+1)%100;
  }
}
// for(i=leftstart;i!=(leftend+1)%100;i=(i+1)%100)
// {
// collisionwithleftbrick(leftbrick[i][1],leftbrick[i][2],leftbrick[i][2]-leftbrick[i][4],leftbrick[i][2],i);
// collisionwithleftbrick(leftbrick[i][1]+leftbrick[i][3],leftbrick[i][2],leftbrick[i][2]-leftbrick[i][4],leftbrick[i][2],i);
// }
 collisionwithbrick();
 collisionwithmirror(60,-0.75,0,0,0.664,1);
 collisionwithmirror(60,2.75,-1.5,-1.5,-0.85,2);
 collisionwithmirror(120,-0.25,3,3,3.649,3);
 collisionwithmirror(120,3,2,2,2.649,4);

int q,w;
 for(i=leftstart;i!=(leftend+1)%100;i=(i+1)%100)
 {
     q=checkinredbin(leftbrick[i][1],leftbrick[i][2]-leftbrick[i][4]);
      w=checkinredbin(leftbrick[i][1]+leftbrick[i][3],leftbrick[i][2]-leftbrick[i][4]);
      if(q==1 && w==1)
      {

        if(leftbrick[i][0]==0)
        {
        system("mpg123 -vC sounds/5.mp3 &");
        leftlives--;
    //    cout<<lives<<endl;
        if(leftlives==0)
        gameover=1;
        }
      else
      {
      system("mpg123 -vC sounds/3.mp3 &");
      if(leftvisit[i]==0)
      {
      score+=2;
      increaseblockdist();
      leftvisit[i]=1;
      }
      }
      leftbrick[i][2]-=2;
     }
        Matrices.model = glm::mat4(1.0f);
       translatemirror= glm::translate (glm::vec3(0,0, 0));        // glTranslate00f0
      // rotatemirror = glm::rotate((float)(120*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
       Matrices.model *= (translatemirror);
       MVP = VP * Matrices.model;
       glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
       if(leftbrick[i][0]==0)
       createRectangle(leftbrick[i][1],leftbrick[i][2],leftbrick[i][3],leftbrick[i][4],0,0,0,5,i);
       if(leftbrick[i][0]==1)
       createRectangle(leftbrick[i][1],leftbrick[i][2],leftbrick[i][3],leftbrick[i][4],1,0,0,5,i);
      // draw3DObject draws the VAO given to it using current MVP matrix

       draw3DObject(leftside[i]);
 }
 for(i=rightstart;i!=(rightend+1)%100;i=(i+1)%100)
 {
  q=checkingreenbin(rightbrick[i][1],rightbrick[i][2]-rightbrick[i][4]);
   w=checkingreenbin(rightbrick[i][1]+rightbrick[i][3],rightbrick[i][2]-rightbrick[i][4]);
   if(q==1 && w==1)
   {

  if(rightbrick[i][0]==0)
  {
    system("mpg123 -vC sounds/5.mp3 &");
    rightlives--;
    //cout<<lives<<endl;
    if(rightlives==0)
    gameover=1;
  //gameover=1;
   }
  else
  {
    system("mpg123 -vC sounds/3.mp3 &");
    if(rightvisit[i]==0)
     {
       //cout<<"s\n";
      rightvisit[i]=1;
      score+=2;
      increaseblockdist();
     }
  }
  rightbrick[i][2]-=2;
   }
       Matrices.model = glm::mat4(1.0f);
       translatemirror= glm::translate (glm::vec3(0,0,0));        // glTranslate00f0
      // rotatemirror = glm::rotate((float)(120*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
       Matrices.model *= (translatemirror);
       MVP = VP * Matrices.model;
       glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

       if(rightbrick[i][0]==0)
       createRectangle(rightbrick[i][1],rightbrick[i][2],rightbrick[i][3],rightbrick[i][4],0,0,0,6,i);
       if(rightbrick[i][0]==2)
       createRectangle(rightbrick[i][1],rightbrick[i][2],rightbrick[i][3],rightbrick[i][4],0,1,0,6,i);
       // draw3DObject draws the VAO given to it using current MVP matrix

       draw3DObject(rightside[i]);
 }

 for(i=bulletstart;i!=(bulletend+1)%100;i=(i+1)%100)
 {
//   cout<<bullets[i][4]<<endl;
   Matrices.model = glm::mat4(1.0f);
   translatemirror= glm::translate (glm::vec3(bullets[i][0],bullets[i][1],0));        // glTranslate00f0
   rotatemirror = glm::rotate((float)(bullets[i][4]*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
   Matrices.model *= (translatemirror*rotatemirror);
   MVP = VP * Matrices.model;
   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);

   createRectangle(0,0,0.4,0.05,1,1,0,7,i);
   // draw3DObject draws the VAO given to it using current MVP matrix
   draw3DObject(bullet[i]);
 }
 for(i=0;i<leftlives;i++)
 {
   Matrices.model = glm::mat4(1.0f);
   glm::mat4 translatelive = glm::translate (glm::vec3(0,-i*0.3, 0));        // glTranslatef
   glm::mat4 rotatelive = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
   Matrices.model *= (translatelive * rotatelive);
   MVP = VP * Matrices.model;
   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
   createlives(-3.7,-2.6,i);
   draw3DObject(lives[i]);
 }

 for(i=0;i<rightlives;i++)
 {
   Matrices.model = glm::mat4(1.0f);
   glm::mat4 translatelive = glm::translate (glm::vec3(0,-i*0.3, 0));        // glTranslatef
   glm::mat4 rotatelive = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
   Matrices.model *= (translatelive * rotatelive);
   MVP = VP * Matrices.model;
   glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
   createlives(3.3,-2.464,i+3);
   draw3DObject(lives[i+3]);
 }

 createRectangle(2.5,3.872,0.2,0.02,0,0,0,9,1);
 createRectangle(2.5,3.871,0.01,0.2,0,0,0,9,2);
 createRectangle(2.5,3.671,0.2,0.015,0,0,0,9,3);
 createRectangle(2.7,3.656,0.01,0.2,0,0,0,9,4);
 createRectangle(2.5,3.456,0.2,0.015,0,0,0,9,5);

 createRectangle(2.725,3.872,0.2,0.02,0,0,0,9,6);
 createRectangle(2.725,3.871,0.02,0.45,0,0,0,9,7);
 createRectangle(2.725,3.436,0.2,0.015,0,0,0,9,8);

 createRectangle(2.967,3.872,0.2,0.02,0,0,0,9,9);
 createRectangle(2.967,3.871,0.02,0.45,0,0,0,9,10);
 createRectangle(2.967,3.436,0.2,0.015,0,0,0,9,11);
 createRectangle(3.167,3.872,0.015,0.45,0,0,0,9,12);

 createRectangle(3.22,3.872,0.015,0.45,0,0,0,9,13);
 createRectangle(3.235,3.872,0.2,0.02,0,0,0,9,14);
 createRectangle(3.435,3.871,0.015,0.23,0,0,0,9,15);
 createRectangle(3.22,3.642,0.23,0.015,0,0,0,9,16);
 createRectangle(0,0,0.32,0.0152,0,0,0,9,17);

 createRectangle(3.5,3.872,0.015,0.46,0,0,0,9,18);
 createRectangle(3.515,3.872,0.2,0.015,0,0,0,9,19);
 createRectangle(3.515,3.642,0.2,0.015,0,0,0,9,20);
 createRectangle(3.515,3.414,0.2,0.015,0,0,0,9,21);
float ha,a,b;
for(i=1;i<=21;i++)
{
  if(i==17)
  {
  ha=315;
  a=3.22;
  b=3.642;
}
  else
  {
  ha=0;a=0;b=0;
}
 Matrices.model = glm::mat4(1.0f);
 translatemirror= glm::translate (glm::vec3(a,b,0));        // glTranslate00f0
 rotatemirror = glm::rotate((float)(ha*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 Matrices.model *= (translatemirror*rotatemirror);
 MVP = VP * Matrices.model;
 glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 draw3DObject(segment[i]);
 }
 //cout<<score<<endl;
 if(score<0)
 score=0;
 int lop,dig=-1,score1,sx=0,sy=0;
 lop=score;
// cout<<score<<endl;
 while(lop>0)
 {
   lop/=10;
   dig++;
 }
 score1=score;
   for(i=22;i<=28;i++)
   {
     createRectangle(0,0,0.22,0.01,0,0,0,9,i);
   }
   if(dig==-1)
 			dig=0;createRectangle(0,0.125,0.5,0.25,0,0,1,3,2);

 		for(int a=dig;a>=0;a--){
 			int p=score1%10;
 			if(p==0||p==4||p==5||p==6||p==8||p==9){
 				Matrices.model = glm::mat4(1.0f);

 				glm::mat4 translatesegment1 = glm::translate (glm::vec3(2.69+a*0.4, 2.4+0.5, 0));        // glTranslatef
 				glm::mat4 rotatesegment1 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 				Matrices.model *= (translatesegment1 * rotatesegment1);
 				MVP = VP * Matrices.model;
 				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 				draw3DObject(segment[22]);
 			}
 			if(p==0||p==2||p==6||p==8){
 				Matrices.model = glm::mat4(1.0f);

 				glm::mat4 translatesegment2 = glm::translate (glm::vec3(2.69+a*0.4, 2.21+0.5, 0));        // glTranslatef
 				glm::mat4 rotatesegment2 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 				Matrices.model *= (translatesegment2 * rotatesegment2);
 				MVP = VP * Matrices.model;
 				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 				draw3DObject(segment[23]);
 			}
 			if(p==0||p==2||p==3||p==5||p==6||p==8){
 				Matrices.model = glm::mat4(1.0f);

 				glm::mat4 translatesegment3 = glm::translate (glm::vec3(2.69+a*0.4, 2.2+0.5, 0));        // glTranslatef
 				glm::mat4 rotatesegment3 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 				Matrices.model *= (translatesegment3 * rotatesegment3);
 				MVP = VP * Matrices.model;
 				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 				draw3DObject(segment[24]);
 			}
 			if(p==0||p==1||p==3||p==4||p==5||p==6||p==7||p==8||p==9){
 				Matrices.model = glm::mat4(1.0f);

 				glm::mat4 translatesegment4 = glm::translate (glm::vec3(2.93+a*0.4, 2.21+0.5, 0));        // glTranslatef
 				glm::mat4 rotatesegment4 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 				Matrices.model *= (translatesegment4 * rotatesegment4);
 				MVP = VP * Matrices.model;
 				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 				draw3DObject(segment[25]);
 			}
 			if(p==0||p==1||p==2||p==3||p==4||p==7||p==8||p==9){
 				Matrices.model = glm::mat4(1.0f);

 				glm::mat4 translatesegment5 = glm::translate (glm::vec3(2.93+a*0.4, 2.4+0.5, 0));        // glTranslatef
 				glm::mat4 rotatesegment5 = glm::rotate((float)(90*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 				Matrices.model *= (translatesegment5 * rotatesegment5);
 				MVP = VP * Matrices.model;
 				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 				draw3DObject(segment[26]);
 			}
 			if(p==0||p==2||p==3||p==5||p==6||p==7||p==8||p==9){
 				Matrices.model = glm::mat4(1.0f);

 				glm::mat4 translatesegment6 = glm::translate (glm::vec3(2.69+a*0.4, 2.55+0.56, 0));        // glTranslatef
 				glm::mat4 rotatesegment6 = glm::rotate((float)(	0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 				Matrices.model *= (translatesegment6 * rotatesegment6);
 				MVP = VP * Matrices.model;
 				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 				draw3DObject(segment[27]);
 			}
 			if(p==8 ||p==2||p==5||p==3||p==4||p==6||p==9){
 				Matrices.model = glm::mat4(1.0f);

 				glm::mat4 translatesegment7 = glm::translate (glm::vec3(2.69+a*0.4, 2.37+0.5, 0));        // glTranslatef
 				glm::mat4 rotatesegment7 = glm::rotate((float)(0*M_PI/180.0f), glm::vec3(0,0,1)); // rotate about vector (-1,1,1)
 				Matrices.model *= (translatesegment7 * rotatesegment7);
 				MVP = VP * Matrices.model;
 				glUniformMatrix4fv(Matrices.MatrixID, 1, GL_FALSE, &MVP[0][0]);
 				draw3DObject(segment[28]);
 			}
 			score1=score1/10;
 		}
  float increments = 1;
  //camera_rotation_angle++; // Simulating camera rotation
  //triangle_rotation = triangle_rotation + increments*triangle_rot_dir*triangle_rot_status;
  //rectangle_rotation = rectangle_rotation + increments*rectangle_rot_dir*rectangle_ruwill sned ot_status;
}

/* Initialise glfw window, I/O callbacks and the renderer to use */
/* Nothing to Edit here */
GLFWwindow* initGLFW (int width, int height)
{
    GLFWwindow* window; // window desciptor/handle

    glfwSetErrorCallback(error_callback);
    if (!glfwInit()) {
//        exit(EXIT_FAILURE);
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "Laser Game", NULL, NULL);

    if (!window) {
        glfwTerminate();
//        exit(EXIT_FAILURE);
    }

    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc) glfwGetProcAddress);
    glfwSwapInterval( 1 );

    /* --- register callbacks with GLFW --- */

    /* Register function to handle window resizes */
    /* With Retina display on Mac OS X GLFW's FramebufferSize
     is different from WindowSize */
     /* Initialize the OpenGL rendering properties */
    glfwSetFramebufferSizeCallback(window, reshapeWindow);
    glfwSetWindowSizeCallback(window, reshapeWindow);

    /* Register function to handle window close */
    glfwSetWindowCloseCallback(window, quit);

    /* Register function to handle keyboard input */
    glfwSetKeyCallback(window, keyboard);      // general keyboard input
    glfwSetCharCallback(window, keyboardChar);  // simpler specific character handling

    /* Register function to handle mouse click */
    glfwSetMouseButtonCallback(window, mouseButton);  // mouse button clicks
    glfwSetScrollCallback(window,mouseZoom);
    return window;
}

/* Add all the models to be created here */
void initGL (GLFWwindow* window, int width, int height)
{
    /* Objects should be created before any other gl function and shaders */
	// Create the models
  // Generate the VAO, VBOs, vertices data & copy into the array buffer
  //createRectangle (0,1.5,1,1.5,1,0,0,1,1);
  //createRectangle (0,1.5,1,1.5,0,1,0,1,2);
  //createRectangle (-4,1,0.75,0.5,0,0,1,3,1);
  //createRectangle(0,0.125,0.5,0.25,0,0,1,3,2);
  createRectangle(0,0,0.75,0.125,0.66,0.66,0.66,4,1);
  createRectangle(0,0,0.75,0.125,0.66,0.66,0.66,4,2);
  createRectangle(0,0,0.75,0.1125,0.66,0.66,0.66,4,3);
  createRectangle(0,0,0.75,0.1125,0.66,0.66,0.66,4,4);
  createcircle(1,0.5,1,0.4,0.4,0,0);
  createcircle(2,0.5,0.3,1,0.3,0,0);
  //createcircle(3,0.125,0,0,1,0,0);

	// Create and compile our GLSL program from the shaders
	programID = LoadShaders( "Sample_GL.vert", "Sample_GL.frag" );
	// Get a handle for our "MVP" uniform
	Matrices.MatrixID = glGetUniformLocation(programID, "MVP");


	reshapeWindow (window, width, height);

    // Background color of the scene
	glClearColor (1.0f, 1.0f, 1.0f, 0.0f); // R, G, B, A
	glClearDepth (1.0f);

	glEnable (GL_DEPTH_TEST);
	glDepthFunc (GL_LEQUAL);

    cout << "VENDOR: " << glGetString(GL_VENDOR) << endl;
    cout << "RENDERER: " << glGetString(GL_RENDERER) << endl;
    cout << "VERSION: " << glGetString(GL_VERSION) << endl;
    cout << "GLSL: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << endl;
}

int main (int argc, char** argv)
{
	int width = 1000;
	int height = 1000;
  double x,y;
     GLFWwindow* window = initGLFW(width, height);

	    initGL (window, width, height);
      last_update_time=glfwGetTime();
      fall_down_time=last_update_time;
      shoot_time=last_update_time;
      show_time=last_update_time;
      bullet_update_time=last_update_time;
    /* Draw in loop */
    while (!glfwWindowShouldClose(window)) {
//return 0;
        // OpenGL Draw commands
        //score+=100;
        if(gameover==1)
        {
        cout<<"Your final score is "<<score<<endl;
        return 0;
        }
        glfwGetCursorPos(window,&x, &y);
        x=(x-500)/125;
        y=(500-y)/125;
        draw(x,y);
          // Swap Frame Buffer in double buffering
        glfwSwapBuffers(window);

        // Poll for Keyboard and mouse events
        glfwPollEvents();

      // Control based on time (Time based transformation like 5 degrees rotation every 0.5s)
      if(pause==0)
        current_time = glfwGetTime(); // Time in seconds
        if(fire==1 && current_time-shoot_time>=0.5 && pause==0)
        {
          shoot_time=current_time;
          bulletend=(bulletend+1)%100;
          bullets[bulletend][0]=-3.375+0.625*cos(laserpos[2]*5*M_PI/180.0f);
          bullets[bulletend][1]=laserpos[1]+0.75+0.625*sin(laserpos[2]*5*M_PI/180.0f);
          bullets[bulletend][2]=0.4;
          bullets[bulletend][3]=0.05;
          bullets[bulletend][4]=laserpos[2]*5;
          collidedmirror[bulletend]=0;
          system("mpg123 -vC sounds/1.mp3 &");
          //createRectangle(0,0,0.4,0.05,1,1,0,7,bulletend);
        }
        if (current_time - last_update_time >=1 && pause==0) { // atleast 0.5s elapsed since last frame
            // do something every 0.5 seconds ..
            last_update_time = current_time;
            int i;
            srand(time(NULL));
            int l,h;
            float pos;
            l=rand()%2;
            if(l==0)
            {
              h=rand()%2;
              srand(time(NULL));
              pos=-2.392+1.224*((rand()%100)*1.0)/100;
              leftend=(leftend+1)%100;
              if(h==0)
              {
                if(current_time - black_create_time >=2)
                {
                black_create_time = current_time;
                leftbrick[leftend][0]=0;
                }
                else
                leftbrick[leftend][0]=1;
                //br.color=0; //black
              }
              else
              {
                leftbrick[leftend][0]=1;
                //br.color=1; //red
              }
              leftbrick[leftend][1]=pos;  //xpos
              leftbrick[leftend][2]=4;  //ypos
              leftbrick[leftend][3]=0.2;  //length
              leftbrick[leftend][4]=0.6;  //width
              leftvisit[leftend]=0;
              // if(leftbrick[leftend][0]==0)
              // {
              // createRectangle(leftbrick[leftend][1],leftbrick[leftend][2],leftbrick[leftend][3],leftbrick[leftend][4],0,0,0,5,leftend);
              // }
              // else
              // {
              // createRectangle(leftbrick[leftend][1],leftbrick[leftend][2],leftbrick[leftend][3],leftbrick[leftend][4],1,0,0,5,leftend);
              // }

            }
            else{
              h=rand()%2;
              srand(time(NULL));
              pos=0.488+1.744*((rand()%100)*1.0)/100;
              rightend=(rightend+1)%100;
              if(h==0)
              {
                if(current_time - black_create_time>=2)
                {
                black_create_time=current_time;
                rightbrick[rightend][0]=0;
               }
               else
               rightbrick[rightend][0]=2;
          //      br.color=0; //black
              }
              else
              {
                rightbrick[rightend][0]=2;
                //br.color=1; //green
              }
              rightbrick[rightend][1]=pos;  //xpos
              rightbrick[rightend][2]=4;  //ypos
              rightbrick[rightend][3]=0.2;  //length
              rightbrick[rightend][4]=0.6;  //width
              rightvisit[rightend]=0;
              // if(rightbrick[rightend][0]==0)
              // createRectangle(rightbrick[rightend][1],rightbrick[rightend][2],rightbrick[rightend][3],rightbrick[rightend][4],0,0,0,6,rightend);
              // else
              // createRectangle(rightbrick[rightend][1],rightbrick[rightend][2],rightbrick[rightend][3],rightbrick[rightend][4],0,1,0,6,rightend);

            }

        }
    }
    glfwTerminate();
//    exit(EXIT_SUCCESS);
}
