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
void loadData(string fn, vector<int> &pts, vector<int> &ranks)
{
    	ifstream file (fn); // declare file stream: Http://Www.Cplusplus.Com/Reference/Iostream/Ifstream/http://www.cplusplus.com/reference/iostream/ifstream/
    	int points [20][38];
    	int rk [20][38];
    	int nbequipe = 20;
    	vector<string> nomTeam;

    	for(int i = 0; i < nbequipe; i++)
    {
        string line;
        string value;
        cout << "1st" << endl;
        getline ( file, value, '\n' );
        cout << "2nd" << endl;
        //nomTeam[i]= string(value,0,value.length());
        string TeamBuff;
        getline(file, TeamBuff, ',');
        cout << "3rd" << endl;
        nomTeam.push_back(TeamBuff);
        cout << nomTeam[i] << endl;
        string tabres [38][6]; //38j, 6 entrees par jour
        int j = 0;
        while(file.good())
        {
             getline ( file, value, ',' );
                cout << value <<endl;
             ranks.push_back(stoi(DeathToSpace((string( value, 0, value.length() )))));
             //tabres[i][0]=ranko;
                       cout << "4rd" << endl;
             getline ( file, value, ',' );
             pts.push_back(stoi(DeathToSpace(string( value, 0, value.length() ))));
             //tabres[i][1]=ptso;
        cout << "5rd" << endl;
             getline ( file, value, ',' );
             string villej1 = string( value, 0, value.length() );
             tabres[i][2]=villej1;
        cout << "6rd" << endl;
             getline ( file, value, ',' );
             string scorej1 = string( value, 0, value.length() );
             //float lon = stof(slon, NULL);
             tabres[i][3]=scorej1;
        cout << "7rd" << endl;
             getline ( file, value, ',' );
             string scorej2 = string( value, 0, value.length() );
             //float lat = stof(slat, NULL);
             tabres[i][4]=scorej2;
        cout << "8rd" << endl;
             getline( file, value, ',' );
             string villej2 = string( value, 0, value.length() );
             tabres[i][5]=villej2;
        cout << "9rd" << endl;
        }

    }
}

int main(void)
{
	int j = 0;
	vector<int> pts(20);
	vector<int> ranks(20);
	loadData("data/rankspts.csv",pts,ranks);
	cout << pts.size()<<endl;
	cout << ranks.size()<<endl;
	while(j < 20)
	{
		cout << pts[j] << endl;
		cout << ranks[j] << endl;
		j++;
	}
	return (0);
}
