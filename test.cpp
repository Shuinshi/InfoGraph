
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

void readData(){
    int nbteam = nbteam;
    String Team;
    getline(
    