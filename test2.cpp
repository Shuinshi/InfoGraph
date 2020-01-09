// sous linux   
// >g++ -I/usr/local/include/ -I/public/ig/glm/ -c test2.cpp  -otest2.o
// >g++ -I/usr/local test2.o -lglfw  -lGLEW  -lGL  -otest2
// >./test2

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

string DeathToSpace(string a)
{
    int i = 0;
    while(a[i])
    {
        if (a[i] == 32)
        {
            a[i] = a[i+1];
            if(a[i+1])
                a[i+1] = 32;
        }
        i++;
    }
    return a;
}
void loadData(string fn, vector<int> &pts, vector<int> &ranks, vector<string> &nomTeam)
{
    	ifstream file (fn); // declare file stream: Http://Www.Cplusplus.Com/Reference/Iostream/Ifstream/http://www.cplusplus.com/reference/iostream/ifstream/
    	
    	int nbequipe = 20;
        int nbj = 41;
        string value;
    	for(int i = 0; i < nbequipe*nbj ; i++)
        {
             if (i % 41 == 0)
            {
             getline ( file, value, ',' );
             nomTeam.push_back(value);
            }
             getline( file, value, ',' );
             ranks.push_back(stoi(DeathToSpace((string( value, 0, value.length() )))));
             getline( file, value, ',' );
             pts.push_back(stoi(DeathToSpace(string( value, 0, value.length() ))));
             getline( file, value, ',' );
             getline( file, value, ',' );
             getline( file, value, ',' );
             getline( file, value, ',' );
            if (i % 41 == 40)
             getline(file, value, '\n');     
        }
        int j = 0;
        while(j < nomTeam.size())
	{
		cout << nomTeam[j] << endl;
		j++;
	}
}

int main(void)
{
	int j = 0;
	vector<int> pts;
	vector<int> ranks;
	vector<string> nomTeam;
	loadData("data/rankspts.csv",pts,ranks, nomTeam);
	cout << pts.size()<<endl;
	cout << ranks.size()<<endl;
	cout << nomTeam.size() << endl;
	cout << "Points" << "                         " << "Ranks" << endl;
	while(j < nomTeam.size())
	{
	    cout << nomTeam[j] << endl;
	    for(int i = 0; i < 41; i++){
		    cout << pts[i+41*j] << "                         " << ranks[i+41*j] << endl;
		    
		    }
	    j++;	    
	}
	return (0);
}
