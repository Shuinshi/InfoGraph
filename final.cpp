// sous mac 
// g++ -I/usr/local/include/ -lglfw -lGLEW -lopencv_core -lopencv_imgproc -lopencv_highgui -lopencv_imgcodecs main6.cpp -framework OpenGL -omain6
// ./main6

// sous linux 	
// g++ -I/usr/local/include/ -I/public/ig/glm/ -c final.cpp  -ofinal.o
// g++ -I/usr/local final.o -lglfw  -lGLEW  -lGL -lopencv_core -lopencv_imgproc -lopencv_highgui  -lopencv_imgcodecs -ofinal
// ./final

// Inclut les en-têtes standards
#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <math.h>
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
const int N = 41;


float rad = 1.0;
float posx = 0.;
float posy = 0.;

// Avec 3 parties dans le VBO ca commence a etre complique. On va stocker la taille
// des 3 parties dans des variables pour plus facilement calculer les decallages
long vertexSize, colorSize, texCoordSize;

// Les donnees sous forme de pointeur. Il faudra donc faire un malloc
// pour allouer de l'espace avant de mettre des donnees dedans
vector<GLfloat *>g_vertex_buffer_data(20,NULL);
vector<GLfloat *>g_vertex_color_data(20,NULL);
vector<GLfloat *>g_vertex_texcoord_data(20,NULL);

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
vector<GLuint> vertexArrayID(20);
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


void loadData(string fn, vector<vector<int>> &pts, vector<vector<int>> &ranks, vector<string>&nomTeam){
  ifstream file (fn);
  for(int i = 0; i<20; i++){
    string value;
    getline(file, value, ',');
    string nomTeamA = string(value, 0, value.length());
    nomTeam.push_back(nomTeamA);
    for(int j = 0; j<41; j++){
      getline(file, value, ',');
      string rang = string(value, 0, value.length());
      ranks[i][j] = stoi(rang);
      //cout << i<<","<<j<< " => " << ranks[i][j]<<endl;
      getline(file, value, ',');
      string point = string(value, 0, value.length());
      pts[i][j] = stoi(point);
      getline(file, value, ',');
      getline(file, value, ',');
      getline(file, value, ',');
      getline(file, value, ',');
    }
  }
}


void affiche(vector<vector<int>> pts, vector<vector<int>> ranks, vector<string>nomTeam){
  for(int i = 0; i<20; i++){
    cout<<nomTeam[i]<<" : "<<endl;
    for(int j = 0; j<41; j++){
      cout<<"Points : "<<pts[i][j]<<" Rang : "<<ranks[i][j]<<endl;
    }
    cout<<endl;
  }
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


void generateData(float texw, float texh, vector<vector<int>> pts, vector<vector<int>> ranks){
  vertexSize   = 6*18*(N-1)*sizeof(GLfloat);
  colorSize    = 6*18*(N-1)*sizeof(GLfloat);
  texCoordSize = 6*18*(N-1)*sizeof(GLfloat);
  
  for(int i = 0; i<20; i++){
    if (g_vertex_buffer_data[i] != NULL)
      free(g_vertex_buffer_data[i]);
    
    if (g_vertex_color_data[i] != NULL)
      free(g_vertex_color_data[i]);
    
    if (g_vertex_texcoord_data[i] != NULL)
      free(g_vertex_texcoord_data[i]);
  }
  for(int i = 0;i<20; i++){
    g_vertex_buffer_data[i]    = (GLfloat*)malloc(vertexSize);
    g_vertex_color_data[i]     = (GLfloat*)malloc(colorSize);
    g_vertex_texcoord_data[i]  = (GLfloat*)malloc(texCoordSize);
    
  }
  
  float r = 0.020;
  
  for(int e = 0; e<20; e++){  //QUE POUR UNE EQUIPE
    
    float c1 = 0;
    float c2 = 0;
    float c3 = 0;
    
    for (int i=0; i<N-1; i++){
    
        float y   = ((((19-ranks[e][i])/19.)+(pts[e][i]/98.))/2.)*2-1;
        float yn  = ((((19-ranks[e][i+1])/19.)+(pts[e][i+1]/98.))/2.)*2-1;
        
        float x = ((float) i/ (float) N)*2-1;
        float xn = ((float) (i+1)/ (float) N)*2-1;
        
        float WL = 0;
        
        for(int t = 0; t<6; t++){
           
          float theta = t*(M_PI/6);
          if(y>yn){
            WL = -0.02;
          } else {
            WL = 0.02;
          }
      
          g_vertex_buffer_data[e][0+(t*18)+i*18*6] = x;
          g_vertex_buffer_data[e][1+(t*18)+i*18*6] = y+r*cos(theta);
          g_vertex_buffer_data[e][2+(t*18)+i*18*6] = r*sin(theta) + WL;
          
          g_vertex_buffer_data[e][3+(t*18)+i*18*6] = x;
          g_vertex_buffer_data[e][4+(t*18)+i*18*6] = y+r*cos(theta + M_PI/6);
          g_vertex_buffer_data[e][5+(t*18)+i*18*6] = r*sin(theta + M_PI/6) + WL;
          
          g_vertex_buffer_data[e][6+(t*18)+i*18*6] = xn;
          g_vertex_buffer_data[e][7+(t*18)+i*18*6] = yn+r*cos(theta);
          g_vertex_buffer_data[e][8+(t*18)+i*18*6] = r*sin(theta) + WL;
          
          g_vertex_buffer_data[e][9+(t*18)+i*18*6] = x;
          g_vertex_buffer_data[e][10+(t*18)+i*18*6] = y+r*cos(theta + M_PI/6);
          g_vertex_buffer_data[e][11+(t*18)+i*18*6] = r*sin(theta + M_PI/6) + WL;
          
          g_vertex_buffer_data[e][12+(t*18)+i*18*6] = xn;
          g_vertex_buffer_data[e][13+(t*18)+i*18*6] = yn+r*cos(theta + M_PI/6);
          g_vertex_buffer_data[e][14+(t*18)+i*18*6] = r*sin(theta + M_PI/6) + WL;
          
          g_vertex_buffer_data[e][15+(t*18)+i*18*6] = xn;
          g_vertex_buffer_data[e][16+(t*18)+i*18*6] = yn+r*cos(theta);
          g_vertex_buffer_data[e][17+(t*18)+i*18*6] = r*sin(theta) + WL;
          
          
          
          //Dans le sujet, les equipes sont rangées dans l'ordre de leur classement final
          if(e==0){
            c1 = 0.;
            c2 = 0.;
            c3 = 1.0;
          }else if(e<4) {
            c1 = 0.4;
            c2 = 0.4;
            c3 = 1.0;
          }else if(e<7) {
            c1 = 1.0;
            c2 = 1.0;
            c3 = 0.;
          }else if(e<11){
            c1 = 0.7;
            c2 = 0.7;
            c3 = 0.7;
          }else if(e<16){
            c1 = 0.3;
            c2 = 0.3;
            c3 = 0.3;
          }else {
            c1 = 1.0;
            c2 = 0.;
            c3 = 0.;
          }
          float ombrage = t/12.0 + 0.25;
          g_vertex_color_data[e][0+(t*18)+i*18*6] = c1*ombrage;
          g_vertex_color_data[e][1+(t*18)+i*18*6] = c2*ombrage;
          g_vertex_color_data[e][2+(t*18)+i*18*6] = c3*ombrage;
         
          g_vertex_color_data[e][3+(t*18)+i*18*6] = c1*ombrage;
          g_vertex_color_data[e][4+(t*18)+i*18*6] = c2*ombrage;
          g_vertex_color_data[e][5+(t*18)+i*18*6] = c3*ombrage;
          
          g_vertex_color_data[e][6+(t*18)+i*18*6] = c1*ombrage;
          g_vertex_color_data[e][7+(t*18)+i*18*6] = c2*ombrage;
          g_vertex_color_data[e][8+(t*18)+i*18*6] = c3*ombrage;
          
          g_vertex_color_data[e][9+(t*18)+i*18*6] = c1*ombrage;
          g_vertex_color_data[e][10+(t*18)+i*18*6] = c2*ombrage;
          g_vertex_color_data[e][11+(t*18)+i*18*6] = c3*ombrage;
          
          g_vertex_color_data[e][12+(t*18)+i*18*6] = c1*ombrage;
          g_vertex_color_data[e][13+(t*18)+i*18*6] = c2*ombrage;
          g_vertex_color_data[e][14+(t*18)+i*18*6] = c3*ombrage;
          
          g_vertex_color_data[e][15+(t*18)+i*18*6] = c1*ombrage;
          g_vertex_color_data[e][16+(t*18)+i*18*6] = c2*ombrage;
          g_vertex_color_data[e][17+(t*18)+i*18*6] = c3*ombrage;
      }
    }
  }
  
}


void initOpenGL(){
  // Enable depth test
  glEnable(GL_DEPTH_TEST);
  // Accept fragment if it closer to the camera than the former one
  glDepthFunc(GL_LESS);
  glDepthRange(-1, 1);
  
  for(int i = 0; i<20; i++){
    // creation du glVertexAttribPointer
    glGenVertexArrays(1, &vertexArrayID[i]);
    glBindVertexArray(vertexArrayID[i]);
    
    
    // Generate 1 buffer, put the resulting identifier in vertexbuffer
    glGenBuffers(1, &vertexbuffer);
    
    // The following commands will talk about our 'vertexbuffer' buffer
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    // Only allocate memory. Do not send yet our vertices to OpenGL.
    glBufferData(GL_ARRAY_BUFFER, vertexSize+colorSize+texCoordSize, 0, GL_STATIC_DRAW);
    
    // send vertices in the first part of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, 0, vertexSize, g_vertex_buffer_data[i]);
    
    // send colors in the second part of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize, colorSize, g_vertex_color_data[i]);
    
    // send tex coords in the third part of the buffer
    glBufferSubData(GL_ARRAY_BUFFER, vertexSize+colorSize, texCoordSize, g_vertex_texcoord_data[i]);
    
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
  
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{       
    if (glfwGetKey(window, GLFW_KEY_UP ) == GLFW_PRESS)
      rad = rad-0.1;
    if (glfwGetKey(window, GLFW_KEY_DOWN ) == GLFW_PRESS)
      rad = rad+0.1;
    if (glfwGetKey(window, GLFW_KEY_RIGHT ) == GLFW_PRESS)
      angleRot = (angleRot+M_PI/1000);
    if (glfwGetKey(window, GLFW_KEY_LEFT ) == GLFW_PRESS)
      angleRot = (angleRot-M_PI/1000);
    if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS)
      posx = posx - 0.1;
    if (glfwGetKey(window, GLFW_KEY_D ) == GLFW_PRESS)
      posx = posx + 0.1;
    if (glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS)
      posy = posy - 0.1;
    if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS)
      posy = posy + 0.1;

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
  GLFWwindow *win = glfwCreateWindow( winWidth, winHeight, "Projet IGSD", NULL, NULL);
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
  angleRot = (angleRot+M_PI/1000);
  
  // clear before every draw 
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  
  // Use our shader
  glUseProgram(programID); 
  
  //  matrice de projection proportionelle a la taille de la fenetre
  mat4 projectionMatrix = ortho( -rad*1.0f, rad  *1.0f, -rad*1.0f, rad*1.0f, -10.0f, 10.0f );
  mat4 viewMatrix       = lookAt(
    vec3(0. + posx, 0. + posy, 0.5), // where is the camera
    vec3(0. + posx, 0. + posy, 0), //where it looks
    vec3(0, 1.0, 0) // head is up
  );
  mat4 modelMatrix      = mat4(1.0);
  
  glUniformMatrix4fv(uniform_proj,  1, GL_FALSE, value_ptr(projectionMatrix));
  glUniformMatrix4fv(uniform_view,  1, GL_FALSE, value_ptr(viewMatrix));
  
 for(int i = 0; i<20; i++){

    glUniformMatrix4fv(uniform_model, 1, GL_FALSE, value_ptr(modelMatrix));
    
    // La texture aussi est donnee en variable uniforme. On lui donne le No 0
    glUniform1i(uniform_texture, 0);
    
    // on re-active le VAO avant d'envoyer les buffers
    glBindVertexArray(vertexArrayID[i]);
    
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
  } 
  // on desactive les shaders
  glUseProgram(0);
  
  // Swap buffers
  glfwSwapBuffers(window);
  glfwPollEvents();
}


int main(){
  // Initialise GLFW
  
  string fn = "rankspts.csv";
  vector<vector<int>> pts;
  vector<vector<int>> ranks;
  vector<string> nomTeam;
  for(int i = 0; i<20; i++){
    vector<int> pts1;
    vector<int> rank1;
    for(int j = 0; j<41; j++){
      pts1.push_back(0);
      rank1.push_back(0);
    }
    pts.push_back(pts1);
    ranks.push_back(rank1);
  }
  
  loadData(fn, pts, ranks, nomTeam);
  
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
  
  generateData(texWidth, texHeight, pts, ranks);
  
  initOpenGL();
  
  programID = LoadShaders( "SimpleVertexShader6.vertexshader", "SimpleFragmentShader6.fragmentshader" );
  uniform_proj    = glGetUniformLocation(programID, "projectionMatrix");
  uniform_view    = glGetUniformLocation(programID, "viewMatrix");
  uniform_model   = glGetUniformLocation(programID, "modelMatrix");
  uniform_texture = glGetUniformLocation(programID, "loctexture");
  
  double before = glfwGetTime();
  int cpt = 0;
  do{
    draw();
    cpt++;
    
    // On a fait 100 dessins, il est temps de voir combien de temps cela a pris
    // et quel est le taux de raffraichissement 
    if (cpt==100) {
      double after = glfwGetTime();
      //cout << 100.0/(after-before) << "Hz"<<endl;
      before = after;
      cpt=0;
    }
    
  } // Vérifie si on a appuyé sur la touche échap (ESC) ou si la fenêtre a été fermée
  
  
  while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS && glfwWindowShouldClose(window) == 0 );
}
