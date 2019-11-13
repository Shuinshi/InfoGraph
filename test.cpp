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

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace glm;

void readData(){
  ifstream file ( "../data/rankpts.csv" ); // declare file stream: http://www.cplusplus.com/reference/iostream/ifstream/
	string value;
	getline ( file, value, '\n' );
	while ( file.good() ) {

	     getline ( file, value, ',' ); // read a string until next comma: http://www.cplusplus.com/reference/string/getline/
	     string villedep = string( value, 0, value.length() );
	     getline ( file, value, ',' );
	     string ranko = string( value, 0, value.length() );
	     getline ( file, value, ',' );
	     string ptso = string( value, 0, value.length() );
	     getline ( file, value, ',' );
	     string villej1 = string( value, 0, value.length() );
	     getline ( file, value, ',' );
	     string scorej1 = string( value, 0, value.length() );
	     //float lon = stof(slon, NULL);
	     getline ( file, value, ',' );
	     string scorej2 = string( value, 0, value.length() );
	     //float lat = stof(slat, NULL);
	     getline ( file, value, ',' );
	     string villej2 = string( value, 0, value.length() );
	     /*getline ( file, value, '\n' );
	     string ssur = string( value, 0, value.length() );
*/
	     //cout << lon << " ... " << lat << endl; // display value
	}
}
void loadData(string fn, vector<int> &pts, vector<int> &ranks)
    	{
    	ifstream file ( "../data/rankpts.csv" ); // declare file stream: http://www.cplusplus.com/reference/iostream/ifstream/
    	//ifstream file (fn);
    	/*int points [20][38];
    	int rk [20][38];
    	int nbequipe = 20;
    	string nomTeam[nbequipe];
    	
    	for(int i = 0; i < nbequipe; i++)
    {
        string value;
        /*getline ( file, value, '\n' );
        int nbteam = nbteam;
        */
       /* string Team;
        getline(file, Team, ',');
        nomTeam[i] = Team;
        string tabres [38][6]; //38j, 6 entrees par jour
        int j = 0;
        while(file.good())
        {
             getline ( file, value, ',' );
             string ranko = string( value, 0, value.length() );
             tabres[i][0]=ranko;
             getline ( file, value, ',' );
             string ptso = string( value, 0, value.length() );
             tabres[i][1]=ptso;
             getline ( file, value, ',' );
             string villej1 = string( value, 0, value.length() );
             tabres[i][2]=villej1;
             getline ( file, value, ',' );
             string scorej1 = string( value, 0, value.length() );
             //float lon = stof(slon, NULL);
             tabres[i][3]=scorej1;
             getline ( file, value, ',' );
             string scorej2 = string( value, 0, value.length() );
             //float lat = stof(slat, NULL);
             tabres[i][4]=scorej2;
             getline( file, value, ',' );
             string villej2 = string( value, 0, value.length() );
             tabres[i][5]=villej2;
        }

    }
    */
    pts.clear;
    ranks.clear
    while ( file.good() ) 
    {
    ifstream file (fn);
    getline ( file, value, ',' );
    string ranko = string( value, 0, value.length() );
    ranks.push_back(stio(ranko));
    getline ( file, value, ',' );
    string ptso = string( value, 0, value.length() );
    pts.push_back(stoi(ptso));
    getline ( file, value, ',' );
    string villej1 = string( value, 0, value.length() );
    //tabres[i][2]=villej1;
    getline ( file, value, ',' );
    string scorej1 = string( value, 0, value.length() );
    //float lon = stof(slon, NULL);
    //tabres[i][3]=scorej1;
    getline ( file, value, ',' );
    string scorej2 = string( value, 0, value.length() );
    //float lat = stof(slat, NULL);
    //tabres[i][4]=scorej2;
    getline( file, value, ',' );
    string villej2 = string( value, 0, value.length() );
    //tabres[i][5]=villej2;
    }
    
}

