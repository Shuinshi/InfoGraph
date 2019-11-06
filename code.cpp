// sous mac 
// g++ -I/usr/local/include/ -lglfw -lGLEW -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs main6.cpp -framework OpenGL -omain6
// ./main6

// sous linux 	
// g++ -I/usr/local/include/ -I/public/ig/glm/ -c main6.cpp  -omain6.o
// g++ -I/usr/local main6.o -lglfw  -lGLEW  -lGL -lopencv_core -lopencv_imgproc -lopencv_highgui  -lopencv_imgcodecs -omain6
// ./main6

// Inclut les en-têtes standards
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#ifdef __APPLE__
#include <OpenGL/gl.h>
#else
#include <GL/gl.h>
#endif

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
using namespace cv;

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;


// Le nombre de donnees
const int N = 12;

// Avec 3 parties dans le VBO ca commence a etre complique. On va stocker la taille
// des 3 parties dans des variables pour plus facilement calculer les decallages
long vertexSize, colorSize, texCoordSize;

// Les donnees sous forme de pointeur. Il faudra donc faire un malloc
// pour allouer de l'espace avant de mettre des donnees dedans
GLfloat *g_vertex_buffer_data   = NULL;
GLfloat *g_vertex_color_data    = NULL;
GLfloat *g_vertex_texcoord_data = NULL;

// l'angle de rotation de la camera autour du modele
float angleRot = 0.0f;

// LA fenetre
GLFWwindow* window; 

// La texture d'alphabet
GLuint texId;

// la vitesse initiale
float speed = 0.0; 

// La taille de notre fenetre
int winWidth  = 800;
int winHeight = 400;

// La taille de notre texture
int texWidth  = -1;
int texHeight = -1;

// This will identify our vertex buffer (VBO)
GLuint vertexbuffer;

// Identifiant de notre VAO
GLuint vertexArrayID;

// identifiant de notre programme de shaders
GLuint programID;


// stocke les variables uniformes qui seront communes a tous les vertex dessines
GLint uniform_proj, uniform_view, uniform_model, uniform_texture;


// Charge une texture et retourne l'identifiant openGL
GLuint LoadTexture(string fileName){
  GLuint tId = -1;
  // On utilise OpenCV pour charger l'image
  Mat image = imread(fileName, CV_LOAD_IMAGE_UNCHANGED);

  // On va utiliser des TEXTURE_RECTANGLE au lieu de classiques TEXTURE_2D
  // car avec ca les coordonnees de texture s'exprime en pixels  et non en coordoonnes homogenes (0.0...1.0)
  // En effet la texture est composee de lettres et symbole que nous voudrons extraire... or la position et
  // taille de ces symboles dans la texture sont connuees en "pixels". Ca sera donc plus facile 

  //comme d'hab on fait generer un numero unique(ID) par OpenGL
  glGenTextures(1, &tId);

  texWidth  = image.cols;
  texHeight = image.rows;

  glBindTexture(GL_TEXTURE_RECTANGLE, tId);
    // on envoie les pixels a la carte graphique
	  glTexImage2D(GL_TEXTURE_RECTANGLE, 
	  	           0,           // mipmap level => Attention pas de mipmap dans les textures rectangle
	  	           GL_RGBA,     // internal color format
	  	           image.cols, 
	  	           image.rows, 
	  	           0,           // border width in pixels
	  	           GL_BGRA,     // input file format. Arg le png code les canaux dans l'autre sens
	  	           GL_UNSIGNED_BYTE, // image data type
	  	           image.ptr());
	  // On peut donner des indication a opengl sur comment la texture sera utilisee 
	  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  // INTERDIT sur les textures rectangle!
	//glGenerateMipmap(GL_TEXTURE_2D);
  glBindTexture(GL_TEXTURE_RECTANGLE, 0);

  return tId;
}


// Charge un programme de shaders, le compile et recupere dedans des pointeurs vers
// les variables homogenes que nous voudront mettre a jour plus tard, a chaque dessin 
GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	string VertexShaderCode;
	ifstream VertexShaderStream(vertex_file_path, ios::in);
	if(VertexShaderStream.is_open()){
		string Line = "";
		while(getline(VertexShaderStream, Line))
			VertexShaderCode += "\n" + Line;
		VertexShaderStream.close();
	}else{
		printf("Impossible to open %s. Are you in the right directory ? Don't forget to read the FAQ !\n", vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	string FragmentShaderCode;
	ifstream FragmentShaderStream(fragment_file_path, ios::in);
	if(FragmentShaderStream.is_open()){
		string Line = "";
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
	if ( InfoLogLength > 0 ){
		vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		printf("%s\n", &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	printf("Compiling shader : %s\n", fragment_file_path);
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		printf("%s\n", &FragmentShaderErrorMessage[0]);
	}


	// Link the program
	printf("Linking program\n");
	GLuint progID = glCreateProgram();
	glAttachShader(progID, VertexShaderID);
	glAttachShader(progID, FragmentShaderID);
	glLinkProgram(progID);


	// Check the program
	glGetProgramiv(progID, GL_LINK_STATUS, &Result);
	glGetProgramiv(progID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	if ( InfoLogLength > 0 ){
		vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(progID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		printf("%s\n", &ProgramErrorMessage[0]);
	}

	
	glDetachShader(progID, VertexShaderID);
	glDetachShader(progID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return progID;
}


void generateData(float texw, float texh){
  vertexSize   = 9*5*N*sizeof(GLfloat);
  colorSize    = 9*5*N*sizeof(GLfloat);
  texCoordSize = 3*2*5*N*sizeof(GLfloat);

  if (g_vertex_buffer_data != NULL)
	  free(g_vertex_buffer_data);

  if (g_vertex_color_data!= NULL)
  	free(g_vertex_color_data);
  
  if (g_vertex_texcoord_data!= NULL)
	  free(g_vertex_texcoord_data);

	g_vertex_buffer_data   = (GLfloat*)malloc(vertexSize);
	g_vertex_color_data    = (GLfloat*)malloc(colorSize);
	g_vertex_texcoord_data = (GLfloat*)malloc(texCoordSize);

  for (int i=0; i<N; i++){
    float r = (500+rand()%500)/1000.0; 
    g_vertex_buffer_data[ 0+i*9*5] = 0.0;
		g_vertex_buffer_data[ 1+i*9*5] = 0.0;
		g_vertex_buffer_data[ 2+i*9*5] = 0.0;

		g_vertex_buffer_data[ 3+i*9*5] = 0.0+r*cos((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[ 4+i*9*5] = 0.0+r*sin((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[ 5+i*9*5] = r/5.0;

		g_vertex_buffer_data[ 6+i*9*5] = 0.0+r*cos((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[ 7+i*9*5] = 0.0+r*sin((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[ 8+i*9*5] = r/5.0;


    g_vertex_buffer_data[ 9+i*9*5] = 0.0;
		g_vertex_buffer_data[10+i*9*5] = 0.0;
		g_vertex_buffer_data[11+i*9*5] = 0.0;

		g_vertex_buffer_data[12+i*9*5] = 0.0+r*cos((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[13+i*9*5] = 0.0+r*sin((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[14+i*9*5] = r/5.0;

		g_vertex_buffer_data[15+i*9*5] = 0.0+r*cos((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[16+i*9*5] = 0.0+r*sin((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[17+i*9*5] = 0.0;


    g_vertex_buffer_data[18+i*9*5] = 0.0;
		g_vertex_buffer_data[19+i*9*5] = 0.0;
		g_vertex_buffer_data[20+i*9*5] = 0.0;

		g_vertex_buffer_data[21+i*9*5] = 0.0+r*cos((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[22+i*9*5] = 0.0+r*sin((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[23+i*9*5] = r/5.0;

		g_vertex_buffer_data[24+i*9*5] = 0.0+r*cos((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[25+i*9*5] = 0.0+r*sin((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[26+i*9*5] = 0.0;		


		g_vertex_buffer_data[27+i*9*5] = 0.0+r*cos((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[28+i*9*5] = 0.0+r*sin((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[29+i*9*5] = r/5.0;

		g_vertex_buffer_data[30+i*9*5] = 0.0+r*cos((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[31+i*9*5] = 0.0+r*sin((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[32+i*9*5] = r/5.0;

		g_vertex_buffer_data[33+i*9*5] = 0.0+r*cos((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[34+i*9*5] = 0.0+r*sin((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[35+i*9*5] = 0.0;		

		g_vertex_buffer_data[36+i*9*5] = 0.0+r*cos((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[37+i*9*5] = 0.0+r*sin((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[38+i*9*5] = r/5.0;

		g_vertex_buffer_data[39+i*9*5] = 0.0+r*cos((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[40+i*9*5] = 0.0+r*sin((2*i+0)*M_PI/(N));
		g_vertex_buffer_data[41+i*9*5] = 0.0;

		g_vertex_buffer_data[42+i*9*5] = 0.0+r*cos((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[43+i*9*5] = 0.0+r*sin((2*i+1.9)*M_PI/(N));
		g_vertex_buffer_data[44+i*9*5] = 0.0;		


		g_vertex_color_data[ 0+i*9*5] = 0.0;
		g_vertex_color_data[ 1+i*9*5] = 0.0;
		g_vertex_color_data[ 2+i*9*5] = 1.0;

		g_vertex_color_data[ 3+i*9*5] = 0.5;
		g_vertex_color_data[ 4+i*9*5] = 0.0;
		g_vertex_color_data[ 5+i*9*5] = 0.0;

		g_vertex_color_data[ 6+i*9*5] = 1.0;
		g_vertex_color_data[ 7+i*9*5] = 0.0;
		g_vertex_color_data[ 8+i*9*5] = 0.0;

		g_vertex_color_data[ 9+i*9*5] = 0.5;
		g_vertex_color_data[10+i*9*5] = 0.0;
		g_vertex_color_data[11+i*9*5] = 1.0;

		g_vertex_color_data[12+i*9*5] = 0.5;
		g_vertex_color_data[13+i*9*5] = 0.0;
		g_vertex_color_data[14+i*9*5] = 0.0;

		g_vertex_color_data[15+i*9*5] = 0.5;
		g_vertex_color_data[16+i*9*5] = 0.0;
		g_vertex_color_data[17+i*9*5] = 0.0;		

		g_vertex_color_data[18+i*9*5] = 1.0;
		g_vertex_color_data[19+i*9*5] = 0.0;
		g_vertex_color_data[20+i*9*5] = 1.0;

		g_vertex_color_data[21+i*9*5] = 1.0;
		g_vertex_color_data[22+i*9*5] = 0.0;
		g_vertex_color_data[23+i*9*5] = 0.0;

		g_vertex_color_data[24+i*9*5] = 1.0;
		g_vertex_color_data[25+i*9*5] = 0.0;
		g_vertex_color_data[26+i*9*5] = 0.0;		

		g_vertex_color_data[27+i*9*5] = 1.0;
		g_vertex_color_data[28+i*9*5] = 0.0;
		g_vertex_color_data[29+i*9*5] = 0.0;		

		g_vertex_color_data[30+i*9*5] = 0.5;
		g_vertex_color_data[31+i*9*5] = 0.0;
		g_vertex_color_data[32+i*9*5] = 0.0;

		g_vertex_color_data[33+i*9*5] = 1.0;
		g_vertex_color_data[34+i*9*5] = 0.0;
		g_vertex_color_data[35+i*9*5] = 0.0;	

		g_vertex_color_data[36+i*9*5] = 0.5;
		g_vertex_color_data[37+i*9*5] = 0.0;
		g_vertex_color_data[38+i*9*5] = 0.0;		

		g_vertex_color_data[39+i*9*5] = 0.5;
		g_vertex_color_data[40+i*9*5] = 0.0;
		g_vertex_color_data[41+i*9*5] = 0.0;

		g_vertex_color_data[42+i*9*5] = 1.0;
		g_vertex_color_data[43+i*9*5] = 0.0;
		g_vertex_color_data[44+i*9*5] = 0.0;	


		// on doit maintenant generer des coordonnees de texture pour chaque vertex. 
		// Meme ceux qu'on ne veux pas texturer ! La prochaine fois on fera 2 programmes "shader"
		// avec 2 VBO, 2 VAO, etc. 
		g_vertex_texcoord_data[ 0+i*6*5] = 0.0;
		g_vertex_texcoord_data[ 1+i*6*5] = 0.0;

		g_vertex_texcoord_data[ 2+i*6*5] = 0.0;
		g_vertex_texcoord_data[ 3+i*6*5] = 0.0;

		g_vertex_texcoord_data[ 4+i*6*5] = 0.0;
		g_vertex_texcoord_data[ 5+i*6*5] = 0.0;

		g_vertex_texcoord_data[ 6+i*6*5] = 0.0;
		g_vertex_texcoord_data[ 7+i*6*5] = 0.0;

		g_vertex_texcoord_data[ 8+i*6*5] = 0.0;
		g_vertex_texcoord_data[ 9+i*6*5] = 0.0;

		g_vertex_texcoord_data[10+i*6*5] = 0.0;
		g_vertex_texcoord_data[11+i*6*5] = 0.0;

		g_vertex_texcoord_data[12+i*6*5] = 0.0;
		g_vertex_texcoord_data[13+i*6*5] = 0.0;

		g_vertex_texcoord_data[14+i*6*5] = 0.0;
		g_vertex_texcoord_data[15+i*6*5] = 0.0;

		g_vertex_texcoord_data[16+i*6*5] = 0.0;
		g_vertex_texcoord_data[17+i*6*5] = 0.0;

		g_vertex_texcoord_data[18+i*6*5] = 0.0;
		g_vertex_texcoord_data[19+i*6*5] = 0.0;

		g_vertex_texcoord_data[20+i*6*5] = texw;
		g_vertex_texcoord_data[21+i*6*5] = 0.0;

		g_vertex_texcoord_data[22+i*6*5] = 0.0;
		g_vertex_texcoord_data[23+i*6*5] = texh;

		g_vertex_texcoord_data[24+i*6*5] = texw;
		g_vertex_texcoord_data[25+i*6*5] = 0.0;

		g_vertex_texcoord_data[26+i*6*5] = texw;
		g_vertex_texcoord_data[27+i*6*5] = texh;

		g_vertex_texcoord_data[28+i*6*5] = 0.0;
		g_vertex_texcoord_data[29+i*6*5] = texh;
  }
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // on teste si la touche E est pressee et si c'est le cas on re-genere des donnees
    if (key== GLFW_KEY_E && action == GLFW_PRESS){
	    
	    generateData(texWidth, texHeight);
    	// ici on n'envoie que les sommets car on souhaite garder les memes couleurs ... et le nombre
	    // n'a pas change !
    	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    	glBufferSubData(GL_ARRAY_BUFFER, 0,                            vertexSize, g_vertex_buffer_data);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
    } else if (key== GLFW_KEY_LEFT && action == GLFW_PRESS ){
	    speed = speed - 1.0/256.0;
    } else if (key== GLFW_KEY_RIGHT && action == GLFW_PRESS){
	    speed = speed + 1.0/256.0;
    } else if ((key== GLFW_KEY_UP || key== GLFW_KEY_DOWN) && action == GLFW_PRESS){
	    speed = 0.0;
    } 

}


void initOpenGL(){
  // Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS);
	glDepthRange(-1, 1);

	// creation du glVertexAttribPointer
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);


	// Generate 1 buffer, put the resulting identifier in vertexbuffer
	glGenBuffers(1, &vertexbuffer);

	// The following commands will talk about our 'vertexbuffer' buffer
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	  // Only allocate memory. Do not send yet our vertices to OpenGL.
	  glBufferData(GL_ARRAY_BUFFER, vertexSize+colorSize+texCoordSize, 0, GL_STATIC_DRAW);

    // send vertices in the first part of the buffer
	  glBufferSubData(GL_ARRAY_BUFFER, 0,                            vertexSize, g_vertex_buffer_data);

	  // send colors in the second part of the buffer
	  glBufferSubData(GL_ARRAY_BUFFER, vertexSize, colorSize, g_vertex_color_data);

	  // send tex coords in the third part of the buffer
	  glBufferSubData(GL_ARRAY_BUFFER, vertexSize+colorSize, texCoordSize, g_vertex_texcoord_data);
	
		// ici les commandes stockees "une fois pour toute" dans le VAO 
		// avant on faisait ca a chaque dessin
		glVertexAttribPointer(
		   0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
		   3,                  // size
		   GL_FLOAT,           // type
		   GL_FALSE,           // normalized?
		   0,                  // stride
		   (void*)0            // array buffer offset
		);
		glEnableVertexAttribArray(0);

    glVertexAttribPointer( // same thing for the colors
    	1, 
    	3, 
    	GL_FLOAT, 
    	GL_FALSE, 
    	0, 
    	(void*)vertexSize);
		glEnableVertexAttribArray(1);

	  glVertexAttribPointer(
	  	2, 
	  	2, 
	  	GL_FLOAT, 
	  	GL_FALSE, 
	  	0, 
	  	(void*)(vertexSize+colorSize));
		glEnableVertexAttribArray(2);

	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// on desactive le VAO a la fin de l'initialisation
	glBindVertexArray (0);
}


GLFWwindow *initMainwindow(){
  // Nous allons apprendre a lire une texture de "symboles" generee a partir d'un outil comme :
	// https://evanw.github.io/font-texture-generator/

	glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // On veut OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Pour rendre MacOS heureux ; ne devrait pas être nécessaire
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // On ne veut pas l'ancien OpenGL
  glfwWindowHint(GLFW_DEPTH_BITS, 24);

	// Ouvre une fenêtre et crée son contexte OpenGl
	GLFWwindow *win = glfwCreateWindow( winWidth, winHeight, "Main 06", NULL, NULL);
	if( win == NULL ){
	    fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are maybe not 3.3 compatible. \n" );
	    glfwTerminate();
	}

  // 
	glfwMakeContextCurrent(win); 
	
	// Assure que l'on peut capturer la touche d'échappement 
	glfwSetInputMode(win, GLFW_STICKY_KEYS, GL_TRUE);

	// active une callback = une fonction appellee automatiquement quand un evenement arrive 
	glfwSetKeyCallback(win, key_callback);

	return win;
}


void draw(){
	angleRot = (angleRot+M_PI*speed);

	// clear before every draw 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Use our shader
	glUseProgram(programID); 

	//  matrice de projection proportionelle a la taille de la fenetre
	mat4 projectionMatrix = ortho( -1.0f, 1.0f, -1.0f*winHeight/winWidth, 1.0f*winHeight/winWidth, -3.f, 3.f );
	mat4 viewMatrix       = lookAt(
					                      vec3(1.5*cos(angleRot), 1.5*sin(angleRot), -0.35), // where is the camara
					                      vec3(0, 0, 0.1), //where it looks
					                      vec3(0, 0, 1.0) // head is up
					                    );
	mat4 modelMatrix      = mat4(1.0);

  glUniformMatrix4fv(uniform_proj,  1, GL_FALSE, value_ptr(projectionMatrix));
  glUniformMatrix4fv(uniform_view,  1, GL_FALSE, value_ptr(viewMatrix));
  glUniformMatrix4fv(uniform_model, 1, GL_FALSE, value_ptr(modelMatrix));

  // La texture aussi est donnee en variable uniforme. On lui donne le No 0
	glUniform1i(uniform_texture, 0);

  // on re-active le VAO avant d'envoyer les buffers
  glBindVertexArray(vertexArrayID);

	// On active la texture 0
	glActiveTexture(GL_TEXTURE0);

	// Verrouillage de la texture
  glBindTexture(GL_TEXTURE_RECTANGLE, texId);

	// Draw the triangle(s) !
	glDrawArrays(GL_TRIANGLES, 0, vertexSize/(3*sizeof(float))); // Starting from vertex 0; 6 vertices total -> 2 triangles
	
	// Déverrouillage de la texture
	glBindTexture(GL_TEXTURE_RECTANGLE, 0);

	// on desactive le VAO a la fin du dessin
	glBindVertexArray (0);

	// on desactive les shaders
	glUseProgram(0);

  // Swap buffers
  glfwSwapBuffers(window);
  glfwPollEvents();
}


int main(){
  // Initialise GLFW
	if( !glfwInit() ) {
	    fprintf( stderr, "Failed to initialize GLFW\n" );
	    return -1;
	}

  window = initMainwindow();
	
  // Initialise GLEW
	glewExperimental=true; // Nécessaire dans le profil de base
	if (glewInit() != GLEW_OK) {
	    fprintf(stderr, "Failed to initialize GLEW\n");
	    return -1;
	}
	
  texId = LoadTexture("font.png");
  
	generateData(texWidth, texHeight);

	initOpenGL();

	programID = LoadShaders( "SimpleVertexShader6.vertexshader", "SimpleFragmentShader6.fragmentshader" );
	uniform_proj    = glGetUniformLocation(progID, "projectionMatrix");
	uniform_view    = glGetUniformLocation(progID, "viewMatrix");
	uniform_model   = glGetUniformLocation(progID, "modelMatrix");
	uniform_texture = glGetUniformLocation(progID, "loctexture");

  double before = glfwGetTime();
  int cpt = 0;
	do{
		draw();
		cpt++;

    // On a fait 100 dessins, il est temps de voir combien de temps cela a pris
    // et quel est le taux de raffraichissement 
	  if (cpt==100) {
		  double after = glfwGetTime();
			cout << 100.0/(after-before) << "Hz"<<endl;
		  before = after;
		  cpt=0;
		}
	} // Vérifie si on a appuyé sur la touche échap (ESC) ou si la fenêtre a été fermée
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );
}
