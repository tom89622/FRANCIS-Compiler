#include<iostream>
#include<string.h>
#include<vector>
#include<ctype.h>
#include<stdlib.h>
#include<stack>
#include<fstream>
#include<sstream>
#include<limits>
#include<queue>

#define FORWARD -87 
using namespace std;

struct Identifier {
	string identify ;
	int scope ;
	int type ;
	int pointer ;
};

struct Middle {
	vector< pair<int,int> > posPair ;
	string ins ;
};

class Table{
	public:
		Table() {
			table0.clear() ;
			table1.push_back("");
			table2.push_back("");
			table3.clear() ;
			table4.clear() ;
			table5.resize(100) ;
			table6.clear();
			table7.clear() ;
		} // Table()
		
		void Clear() {
			table0.clear() ;
			table3.clear();
			table4.clear();
			table5.clear();
			table6.clear();
			table7.clear();
			table5.resize(100) ;
		} // Clear()
		
		bool InputTable() ;
		vector<string> table0 ; // tmp
		vector<string> table1 ; // Delimiters 
		vector<string> table2 ; // Reserved Word Table
		vector<int> table3 ; // Int Table
		vector<float> table4 ;	// Real Number Table	
		vector<Identifier> table5 ; // Identifier Table
		vector< Middle > table6 ;
		vector<int> table7 ; // Information Table
};

void TestTable( Table tables ) ;
void Output();
bool InputInt( int &i );
bool ReadLine( string fileName, Table &tables );
bool CutToken( string line, Table &tables, string filename, int &scope );
vector<pair<int,int> > Distribution( vector<string> tokenLine, Table &tables ); 
// 將token放入對應table  NOTE: 1.table5 只填入identify !!(打算syntax時放入其他項) 2.ERROR !!?? 
// syntax 依照message.txt做文法分析, 其token&是哪個table的東西
bool SyntaxAnalysis( vector<string> tokenLine, vector<pair<int,int> > tokenClassify, Table &tables, int &scope, int &pointer  ) ;
bool ProcessFormula( vector<string> &oneMidTab, vector< vector<string> > &midTab, stack<string> &idStack, stack<string> &operatorStack, Table &tables ) ;
vector< pair<int,int> > ReturnAssignmentPos(  vector<string> &oneMidTab, Table &tables ) ;
int TransOp2Compare( string str ) ;
int CheckType( string type ) ;
bool CheckArray( vector<string> tokenLine, vector<pair<int,int> > tokenClassify, vector<string> id , int &type ) ;
bool CheckVariable( vector<string> tokenLine, vector<pair<int,int> > tokenClassify , vector<string> &id, int &type ) ;
bool CheckDimension( vector<string> tokenLine, vector<pair<int,int> > tokenClassify , vector<int> &dimensionSize ) ;
bool ForwardReference( Table &tables ) ;
string ToUpper( string input ) ;
int AsciiSum( string input ) ;
bool isFloat( string myString ) ;
bool Collision( vector<Identifier> &table, int &key, string content ) ;
void WriteAll( Table tables, string filename ) ;
bool firstT = false, inMain = true ;
queue <string> forwardIDStream ;
vector<string> error ;

int main(){
	int cmd ;
	string filename;
	Table tables ;
	tables.InputTable();
	Output() ;
	while ( !InputInt( cmd ) || cmd < 0 || cmd > 1 ) { // 接收0~2的指令 
		cout << "Wrong Command !!!\n" << endl;
		Output();
	} // while
	
	while( cmd != 0 ) {
		tables.Clear();
		cout << "Input FileName : " ;
		cin >> filename;
		cout << "\n" ;
		ReadLine( filename+".txt", tables );
		Output();
		while ( !InputInt( cmd ) || cmd < 0 || cmd > 1 ) { // 接收0~2的指令 
			cout << "Wrong Command !!!\n" << endl;
			Output();
		} // while
	} // while 
} // main()

bool ReadLine( string filename, Table &tables ){
	int scope ;
	fstream file;
	file.open( filename.c_str() , ios::in );
	firstT = false ;
	if (!file) {
		cout << "檔案無法開啟\n";
		return false ;
	} // if
	else { 
		while( !file.eof() ) {
			char line[500];
			file.getline( line, sizeof(line) );
			CutToken( line, tables, filename, scope );
			cout << "\n\n-------next line-------\n\n" ;
		} // while 
		
		ForwardReference( tables ) ;
		WriteAll( tables, filename ) ; 
		TestTable( tables ) ;
		return true ;
	} // else
	
	file.close() ;
}  //ReadLine()

void TestTable( Table tables ) {
	cout << "table0	: \n" ; 
	for( int i = 0; i < tables.table0.size(); i++ )
		cout << tables.table0.at(i) << "  " ;
	cout << "\n" ;
	cout << "table3	: \n" ; 
	for( int i = 0; i < tables.table3.size(); i++ )
		cout << tables.table3.at(i) << "  " ;
	cout << "\n" ;
	cout << "table4	: \n" ;
	for( int i = 0; i < tables.table4.size(); i++ )
		cout << tables.table4.at(i) << "  " ;
	cout << "\n" ;
	cout << "table5	: \n" ;
	for( int i = 0; i < tables.table5.size(); i++ ) {
		if( !tables.table5.at(i).identify.empty() ) cout << i << "-" << tables.table5.at(i).identify << " " << tables.table5.at(i).scope << " " 
			<< tables.table5.at(i).type  << " " << tables.table5.at(i).pointer << endl ;
	} // for
	cout << "\n" ;
	cout << "table6	: \n" ;
	for( int i = 0; i < tables.table6.size(); i++ ) {
		cout << i+1 << "	(" ;
		for( int out = 0; out < 4; out++ ) {
			if( out >= tables.table6[i].posPair.size() )
				cout << "( , ), " ;
			else
				cout << "(" << tables.table6[i].posPair[out].first << "," << tables.table6[i].posPair[out].second << "), " ;
		} // for
		cout << ")	" << tables.table6[i].ins << "\n" ;
	} // for
	cout << "\n" ;
	cout << "table7	: \n" ;
	for( int i = 0; i < tables.table7.size(); i++ ) {
		cout << tables.table7.at(i) << "  " ;
	} // for
	cout << "\n" ;
	
} // TestTable

bool CutToken( string line, Table &tables, string filename, int &scope ){ // DONE 
	int cur = 0, next = 0;
	int pointer = 0 ;
	vector<string> tokenLine ;
	vector<pair<int,int> > tokenClassify ; 
	while(1) {
		//int test = 0 ;
		bool twoQuote = false ;
		next = line.find_first_of(" ;	()=+-*/^:,", cur) ; // 遇到space,tab,delimiter切 			
		if( next != cur ) {
			string tmp = line.substr(cur, next-cur) ;
			if( tmp.size() > 0 ) {
				tokenLine.push_back(tmp) ;
			} // if
		} // if
		
		if( line[next] != ' ' && line[next] != '\n' && line[next] != '\t' && line[next] != '\0' ) {
			string delimiter ;				
			delimiter.push_back(line[next]) ;
			tokenLine.push_back(delimiter) ;
		} // if
		
		if( next == string::npos ) 
			break ;
		cur = next + 1 ;
	} // while
	
	cout << "tokens:  " ;
	for( int t = 0; t < tokenLine.size(); t++ ) {
		cout << tokenLine[t] << "|" ;
	} // for
	cout << line << '\n' ;
	tokenClassify = Distribution( tokenLine, tables ) ;
	
	cout << "\noutput : " ;
	for( int i = 0; i < tokenClassify.size(); i++ ) {
		cout << "  " << tokenLine[i] << "(" << tokenClassify.at(i).first << ", " << tokenClassify.at(i).second << ")" ; // 要把兩個vector打包再一起嗎?? 還是傳兩個過去syntax? 
		if( i % 10 == 9 ) cout << "\n" ;
	} // for
	
	cout << "\n" ;
	
	SyntaxAnalysis( tokenLine, tokenClassify, tables, scope, pointer ) ; // return要啥再說~~
	 
} // CutToken()

vector<pair<int,int> > Distribution( vector<string> tokenLine, Table &tables ){ // distribute token to tables 到底要return啥比較好?Identifier要怎麼存好? 
	vector<pair<int,int> > output ;
	int pos = 0 ;
	for( int tokenNum = 0; tokenNum < tokenLine.size(); tokenNum++ ) {
		bool processed = false ;
		for( int tableNum = 1; !processed && tableNum < tables.table1.size(); tableNum++ ) { // find in table1
			if( ! stricmp( tokenLine.at(tokenNum).c_str(), tables.table1.at(tableNum).c_str() ) ) {	
				output.push_back(make_pair(1,tableNum)); // put in the vector for final output
				cout << "(" << 1 << "," << tableNum << ") ";
				processed = true ;
				break;
			} // if
		} // for
		
		for( int tableNum = 1; !processed && tableNum < tables.table2.size(); tableNum++ ) { // find in table2
			if( ! stricmp( tokenLine.at(tokenNum).c_str(), tables.table2.at(tableNum).c_str() ) ) {	
				output.push_back(make_pair(2,tableNum)); // put in the vector for final output
				cout << "(" << 2 << "," << tableNum << ") ";
				processed = true ;
				break;
			} // if
		} // for
			
		if( !processed && isdigit( tokenLine[tokenNum].at(0) ) ) { // if is digit put in table3 or table4
			int tmp = 0 ;
			processed = true ;
			for( int i = 0; i < tokenLine[tokenNum].size(); i++ ) {
				if( tokenLine[tokenNum].at(i) == '.' ) tmp++ ;
			} // for
			
			istringstream istr(tokenLine[tokenNum]) ;
			int des = 0 ;
			if( tmp == 1 ) { // float
				float input ;
				bool duplicate = false ;
				istr >> input ;
				for( int i = 0; i < tables.table4.size(); i++ ) { // 尋找是否已存在 
					if( tables.table4.at(i) == input ) {
						des = i ;
						duplicate = true ;
						break ;
					} // if
				} // for
				
				if( !duplicate ) { // 若存在 
					tables.table4.push_back(input) ; // push in table4
					output.push_back(make_pair(4,tables.table4.size()));
					cout << "(" << 4 << "," << tables.table4.size() << ") "; // start from 1 
				} // if
				else {
					output.push_back(make_pair(4,des));
					cout << "(" << 4 << "," << des << ") ";
				} // else
			} // if
			else if( tmp == 0 ) { // int
				int input ;
				bool duplicate = false ;
				istr >> input ;
				//vector<int>::iterator duplicate = find( tables.table3.begin(), tables.table3.end(), input ) ;
				for( int i = 0; i < tables.table3.size(); i++ ) { // 尋找是否已存在 
					if( tables.table3.at(i) == input ) {
						des = i ;
						duplicate = true ;
						break ;
					} // if
				} // for
				
				if( !duplicate ) { // 若存在 
					tables.table3.push_back(input) ; // push in table3
					output.push_back(make_pair(3,tables.table3.size()));
					cout << "(" << 3 << "," << tables.table3.size() << ") ";  // start from 1 
				} // if
				else {
					output.push_back(make_pair(3,des));
					cout << "(" << 3 << "," << des << ") ";
				} // else
			} // else if
			else cout << "error\n" ; // How should I do ERRORRRRRRRRR 
		} // if
		
		if( !processed && strcmp( tokenLine.at(tokenNum).c_str(), " " ) ) { // symbol // 好像要等syntax再放!!
			output.push_back(make_pair(5, 0) ); // put in the vector for final output	
		} // if
		 
		if( tokenNum % 10 == 9 ) cout << "\n" ;
	} // for
	
	return output;
} // Distribution()

bool SyntaxAnalysis( vector<string> tokenLine, vector<pair<int,int> > tokenClassify, Table &tables, int &scope, int &pointer ) {
	// first PROGRAM 
	vector< pair<int,int> > pairStream ;
	Middle oneMid ;
	if( tokenLine[0][0] == 'L' && tokenLine[0][1] != 'A' && isdigit(tokenLine[0][1]) ) {
		bool isLabel = false ;
		for( int find = 0; find < tables.table5.size(); find++ ) 
			if( !strcmp( tables.table5[find].identify.c_str(), tokenLine[0].c_str() ) ) {
				tables.table5[find].pointer = tables.table6.size() +1 ;
				isLabel = true ;
			} // if
		for( int i = 0; isLabel && i < tables.table6.size(); i++ ) {
			if( tables.table6[i].posPair[3].second == FORWARD ) // 無正確比較是否是對應L 
				tables.table6[i].posPair[3].second = tables.table6.size() +1 ;
		} // for
		if( isLabel ) {
			tokenLine.erase(tokenLine.begin()) ;
			tokenClassify.erase( tokenClassify.begin()) ;
		} // if
		
		cout << tokenLine[0] << tokenClassify[0].first << endl ;
	} // if
	
	if( !strcmp( tokenLine[0].c_str(), "ENP" ) && !strcmp( tokenLine[1].c_str(), ";" ) && inMain ) {
		cout << "(" << 2 << "," << 6 << "), " ;
		pairStream.push_back( make_pair(2,6) ) ;
		oneMid.posPair = pairStream ;
		oneMid.ins = tokenLine[0] ;
		tables.table6.push_back(oneMid) ;
		pairStream.clear() ;
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "ENS" ) && !strcmp( tokenLine[1].c_str(), ";" ) && !inMain ) {
		cout << "(" << 2 << "," << 7 << "), " ;
		pairStream.push_back( make_pair(2,7) ) ;
		oneMid.posPair = pairStream ;
		oneMid.ins = tokenLine[0] ;
		tables.table6.push_back(oneMid) ;
		pairStream.clear() ;
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "PROGRAM" ) ) {
		cout << "\n----Hey found PROGRAM----\n----Processing----\n" ;
		if( tokenLine.size() == 3 && tokenClassify[1].first == 5 && !strcmp( tokenLine[2].c_str(), ";" ) ) {
			// put id into table5(IDtable)
			int index = AsciiSum( tokenLine[1] )%100 ;  
			if( !Collision( tables.table5, index, tokenLine[1] ) ) { //table is full
				cout << "error message : symbol table is full. (Line:281)\n" ;
			} // if 
			else{
				tables.table5[index].pointer = ++pointer ;
				scope = index ;
				cout << "Program sucessed!\n" ;
			} // else
			
			return true ; // 姑且先回傳是否正確 
		} // if
		else {
			error.push_back( "program error Line1." ) ;
			cout << "program wrong.(Line:291)\n" ;
			if( tokenLine.size() > 1 )
				cout << "find out : " << tokenClassify[1].first << "," << tokenLine[2].c_str() << endl;
		} // else
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "VARIABLE" ) ) {
		cout << "\n----Hey found VARIABLE----\n----Processing----\n" ;
		if( !strcmp( tokenLine[tokenLine.size()-1].c_str(), ";" ) ) {
			vector<string> id, vars ;
			int type ;
			vector<pair<int,int> > tmpTokenClassify ;
			for( int cur = 1; cur < tokenLine.size()-1; cur++ ) {
				vars.push_back( tokenLine[cur] ) ; // 將<type>:<id>{,<id>}
				tmpTokenClassify.push_back( tokenClassify[cur] ) ;
			} // for
			
			if( CheckVariable( vars, tmpTokenClassify, id, type ) ) { // 都沒問題再放入 
				for( int cur = 0; cur < id.size(); cur++ ) {
					int index = AsciiSum( id[cur] )%100 ;  
					if( !Collision( tables.table5, index, id[cur] ) ) { //table is full
						cout << "error message : symbol table is full. (Line:313)\n" ;
					} // if 
					else{
						tables.table5[index].type = type ;
						tables.table5[index].scope = scope ;
						cout << "Variable sucessed!\n" ;
						cout << "(" << 5 << "," << index << "), " ;
						pairStream.push_back( make_pair(5,index) ) ;
						oneMid.posPair = pairStream ;
						oneMid.ins = id[cur] ;
						tables.table6.push_back(oneMid) ;
						pairStream.clear() ;
					} // else	
				} // for
			} // if
			else error.push_back( "variable error.") ;
		} // if
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "DIMENSION" ) ) {
		cout << "\n----Hey found DIMENSION----\n----Processing----\n" ;
		int type = CheckType( tokenLine[1] ) ; // array type
		vector<string> id ;
		vector< vector<int> > arrayInformation ;
		bool startCut = false, syntaxError = false ;
		if( type != -1 && !strcmp( tokenLine[2].c_str(), ":" ) ) {
			int t = 0 ;
			vector<string> oneDimension ;
			for( int cur = 0; cur < tokenLine.size()-1 || strcmp( tokenLine[cur].c_str(), ";" ); cur++ ) {
				if( !strcmp( tokenLine[cur].c_str(), "(") ) {
					startCut = true ;					
					if( tokenClassify[cur-1].first == 5 ) {
						id.push_back( tokenLine[cur-1] ) ;
					} // if
					else cout << "Didn't has id.(Line:338)\n" ;
				} // if 
				
				if( startCut ) {
					oneDimension.push_back( tokenLine[cur] ) ;
				} // if
				
				if( !strcmp( tokenLine[cur].c_str(), ")") ) {
					vector<int> oneArrayInformation ;
					startCut = false ;
					if ( !CheckDimension( oneDimension, tokenClassify, oneArrayInformation ) ) {
						cout << "Dimension wrong.(Line:349)\n" ;
						syntaxError = true ;
						// 做處理 ! 
					} // if
					
					arrayInformation.push_back( oneArrayInformation ) ;
					oneDimension.clear() ; // 換下一括號 
				} // if
			} // for
			
			// 放進table7 !! 
			if( !syntaxError && arrayInformation.size() == id.size() ) {
				for( int cur = 0; cur < id.size(); cur++ ) {
					int pointer = tables.table7.size() ;
					tables.table7.push_back( type ) ;
					tables.table7.push_back( arrayInformation[cur].size() ) ;
					tables.table7.insert( tables.table7.end(), arrayInformation[cur].begin(), arrayInformation[cur].end() ) ;
					int index = AsciiSum( id[cur] )%100 ;  //拉到外面，等確定完syntax再放入!!! 
					if( !Collision( tables.table5, index, id[cur] ) ) { //table is full
						cout << "error message : symbol table is full. (Line:368)\n" ;
					} // if 
					else{
						tables.table5[index].type = 1 ;
						tables.table5[index].scope = scope ;
						tables.table5[index].pointer = pointer ;
						cout << "Array sucessed!\n" ;
						cout << "(" << 5 << "," << index << "), " ;
						pairStream.push_back( make_pair(5,index) ) ;
						oneMid.posPair = pairStream ;
						oneMid.ins = id[cur] ;
						tables.table6.push_back(oneMid) ;
						pairStream.clear() ;
						
					} // else
				} // for
			} // if
			else error.push_back( " size wrong" ) ;
		} // if
		else cout << "dimension wrong.(Line:379)\n" ;
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "LABEL" ) ) {
		cout << "\n----Hey found LABEL----\n----Processing----\n" ;
		vector<string> id ;
		bool syntaxError = false ;
		for( int run = 1; strcmp(tokenLine[run].c_str(), ";") || run < tokenLine.size()-1; run++ ) {
			if( run%2 == 1 && tokenClassify[run].first == 5 ) {
				id.push_back( tokenLine[run] ) ;
			} // if
			else if ( run%2 == 0 && !strcmp(tokenLine[run].c_str(), ",") ) ; // pass ','
			else {
				cout << "\n----->" << tokenLine[run] << "This isn't a label.(Line:391)\n" ;
				syntaxError = true ;
				break ;
			} // else
		} // for
			
		if( syntaxError ) cout << "Label syntaxError.(Line:399)\n" ;
		else {
			for( int cur = 0; cur < id.size(); cur++ ) {
				int index = AsciiSum( id[cur] )%100 ;  //拉到外面，等確定完syntax再放入!!! 
				if( !Collision( tables.table5, index, id[cur] ) ) { //table is full
					cout << "error message : symbol table is full. (Line:401)\n" ;
				} // if 
				else{
					tables.table5[index].type = 5 ;
					tables.table5[index].scope = scope ;
																						// pointer need forward reference.!!! 尚未處理 
					cout << "Label sucessed!\n" ;
					cout << "(" << 5 << "," << index << "), " ;
					pairStream.push_back( make_pair(5,index) ) ;
					oneMid.posPair = pairStream ;
					oneMid.ins = id[cur] ;
					tables.table6.push_back(oneMid) ;
					pairStream.clear() ;
				} // else
			} // for
		} // else
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "GTO" ) ) {
		if( tokenClassify[1].first == 5 && !strcmp( tokenLine[2].c_str(), ";" ) ) {
			// syntax ok 沒問題 
			// 未考慮forward reference 
			cout << "(" << 2 << "," << 11 << "), " ;
			pairStream.push_back( make_pair(2,11) ) ;
			pairStream.push_back( make_pair(-1,-1) ) ;
			pairStream.push_back( make_pair(-1,-1) ) ;
			for( int find = 0; find < tables.table5.size(); find++ ) 
				if( !strcmp( tables.table5[find].identify.c_str(), tokenLine[1].c_str() ) ) {
					if( tables.table5[find].pointer == 0 ) pairStream.push_back( make_pair(6,FORWARD) ) ;
					else pairStream.push_back( make_pair(6,tables.table5[find].pointer) ) ;
				} // if
			
			oneMid.posPair = pairStream ;
			oneMid.ins = "GTO " + tokenLine[1] ;
			tables.table6.push_back(oneMid) ;
			pairStream.clear() ;
		} // if
		else {
			cout << "GTO wrong.(Line:421)\n" ;
		} // else
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "SUBROUTINE" ) ) { // 可能有variable or array 有點複雜@@  記得scope要改
	 	
		if( tokenClassify[1].first == 5 && !strcmp( tokenLine[tokenLine.size()-1].c_str(), ";" ) ) {
			bool startCut = false, syntaxOK = true ;
			string id = tokenLine[1] ;
			vector<pair<int,int> > tmpTokenClassify ;
			vector<string> innerId ;
			vector< vector<string> > innerIdStream ;
			vector<int> innerTypeStream ;
			vector<string> oneDefinition ;
			for( int cur = 2; cur < tokenLine.size()-1; cur++ ) { // 在切每一份<parameter group>{,<parameter group>}
				if( !strcmp( tokenLine[cur].c_str(), "(") && CheckType( tokenLine[cur+1] ) != -1 ) {
					startCut = true ;
					oneDefinition.push_back( tokenLine[cur+1] ) ;
					tmpTokenClassify.push_back( tokenClassify[cur+1] ) ;
					cur++ ;
				} // if
				else if( !strcmp( tokenLine[cur].c_str(), "," ) && CheckType( tokenLine[cur+1] ) != -1 ) { // start next definition
					int checkTmpType ; // 要加進innerTypeStream 
					cout << endl ;
					for( int test = 0; test < oneDefinition.size(); test++ ) cout << oneDefinition[test] << " " ;
					cout << endl ;
					// send to check func.
					if( CheckVariable( oneDefinition, tmpTokenClassify, innerId, checkTmpType ) ) {
						innerIdStream.push_back( innerId ) ; // 確認文法沒問題就放入vector代稍後放入table 
						innerTypeStream.push_back( checkTmpType ) ;
						cout << "variable Check.\n" ;
					} // if
					else if( CheckArray( oneDefinition, tmpTokenClassify, innerId, checkTmpType ) ) {
						innerIdStream.push_back( innerId ) ; // 確認文法沒問題就放入vector代稍後放入table 
						innerTypeStream.push_back( checkTmpType ) ;
						cout << "array Check.\n" ;
					} // else if
					else {
						syntaxOK = false ;
						cout << "Neither <identifier> nor <array>.\n" ;	
					} // else
					
					oneDefinition.clear() ;
					oneDefinition.push_back( tokenLine[cur+1] ) ;
					tmpTokenClassify.clear() ;
					tmpTokenClassify.push_back( tokenClassify[cur+1] ) ;
					cur++ ;
				} // if
				else if( !strcmp( tokenLine[cur].c_str(), ")" ) && !strcmp( tokenLine[cur+1].c_str(), ";" ) ) { // is finish
					int checkTmpType ; // 要加進innerTypeStream
					cout << "\n Finish : " ;
					for( int test = 0; test < oneDefinition.size(); test++ ) cout << oneDefinition[test] << " " ;
					cout << endl ;
					// send to check func.
					if( CheckVariable( oneDefinition, tmpTokenClassify, innerId, checkTmpType ) ) {
						cout << innerId.size() << "~~~~~~\n" ; 
						for( int i = 0; i < oneDefinition.size(); i++ ) cout << oneDefinition[i] << ", " ;
						innerIdStream.push_back( innerId ) ; // 確認文法沒問題就放入vector代稍後放入table 
						innerTypeStream.push_back( checkTmpType ) ;
						cout << "variable Check.\n" ;
					} // if
					else if( CheckArray( oneDefinition, tmpTokenClassify, innerId, checkTmpType ) ) {
						innerIdStream.push_back( innerId ) ; // 確認文法沒問題就放入vector代稍後放入table 
						innerTypeStream.push_back( checkTmpType ) ;
						cout << "array Check.\n" ;
					} // else if
					else {
						syntaxOK = false ;
						cout << "Neither <identifier> nor <array>.\n" ;	
					} // else
				} // else if
				else {
					oneDefinition.push_back( tokenLine[cur] ) ;
					tmpTokenClassify.push_back( tokenClassify[cur] ) ;
				} // else
			} // for
			
			if( syntaxOK ) {
				cout << "What happen."  ;
				int index = AsciiSum(id) ;
				cout << id << " " << index << endl ;
				if( !Collision( tables.table5, index, id ) ) {
					cout << "table5 is full for subid.\n" ;
				} // if
				else {
					scope = index ;
					// 要放pointer (for 中間碼table) 
					tables.table5[index].pointer = tables.table7.size()+1 ;
				} // else
				
				for( int cur = 0; cur < innerIdStream.size(); cur++ ) {
					for( int innerCur = 0; innerCur < innerIdStream[cur].size(); innerCur++ ) {
						cout << innerIdStream[cur].at(innerCur) << " " ;
						int index = AsciiSum(innerIdStream[cur].at(innerCur)) ;
						if( !Collision( tables.table5, index, innerIdStream[cur].at(innerCur) ) ) {
							cout << "table5 is full for innerid.\n" ;
						} // if
						else {
							tables.table5[index].type = innerTypeStream[cur] ;
							tables.table5[index].scope = scope ;
							pairStream.push_back( make_pair(5,index) ) ;
							oneMid.posPair = pairStream ;
							oneMid.ins = innerIdStream[cur].at(innerCur) ;
							tables.table6.push_back(oneMid) ;
							pairStream.clear() ;
							cout << "(" << 5 << "," << index << "), " ;
						} // else
					} // for
				} // for
			} // if
			
			inMain = false ;
		} // if
		else {
			cout << "Sub wrong.\n" ;
		} // else
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "CALL" ) ) {
		bool findSubId = false ;
		int tmpSubIDPosition ;
		vector<int> informationTabTmp ;
		for( int cur = 0; cur < tables.table5.size(); cur++ ) {
			if( !strcmp( tables.table5[cur].identify.c_str(), tokenLine[1].c_str( ) ) && tables.table5[cur].scope == 0 ) { // 有此subroutineID 
				findSubId = true ;
				tmpSubIDPosition = cur ;
				break ;
			} // if
		} // for
		
		if( !strcmp( tokenLine[2].c_str(), "(" ) && !strcmp( tokenLine[tokenLine.size()-2].c_str(), ")" ) && !strcmp( tokenLine[tokenLine.size()-1].c_str(), ";" ) ) {
			for( int cur = 3; cur < tokenLine.size()-2; cur++ ) {
				if( !strcmp( tokenLine[cur].c_str(), ",") ) ;
				else if( tokenClassify[cur].first == 5 ) {
					bool error = true ;
					for( int find = 0; find < tables.table5.size(); find++ ) {
						if( !strcmp( tables.table5[find].identify.c_str(), tokenLine[cur].c_str() ) ) {
							informationTabTmp.push_back( 5 ) ;
							informationTabTmp.push_back( find ) ;
							error = false ;
						} // if
					} // for
					
					if( error ) cout << "Call's inner " << tokenLine[cur] << " table5.\n" ;
				} // if
				else if( tokenClassify[cur].first == 3 ) {
					bool error = true ;
					int theValue ;
					std::istringstream ss(tokenLine[cur]) ;
					ss >> theValue;	
					for( int find = 0; find < tables.table3.size(); find++ ) {
						if( tables.table3[find] == theValue ) {
							informationTabTmp.push_back( 3 ) ;
							informationTabTmp.push_back( find ) ;
							error = false ;
						} // if
					} // for
					
					if( error ) cout << "Call's inner " << tokenLine[cur] << " table3.\n" ;
				} // else if
				else if( tokenClassify[cur].first == 4 ) {
					bool error = true ;
					float theValue ;
					std::istringstream ss(tokenLine[cur]) ;
					ss >> theValue;	
					for( int find = 0; find < tables.table4.size(); find++ ) {
						if( tables.table4[find] == theValue ) {
							informationTabTmp.push_back( 4 ) ;
							informationTabTmp.push_back( find ) ;
							error = false ;
						} // if
					} // for
					
					if( error ) cout << "Call's inner " << tokenLine[cur] << " table4.\n" ;
				} // else if
				else {
					cout << tokenLine[cur] << " error call type.\n" ;
				} // else
			} // for
			
			cout << "Perfect CALL.\n" ;
			int tmpInformationTabStart = tables.table7.size() ;
			tables.table7.push_back( informationTabTmp.size()/2 ) ;
			for( int cur = 0; cur < informationTabTmp.size(); cur++ ) {
				tables.table7.push_back( informationTabTmp[cur] ) ;
			} // for
			
			
			cout << "start pos : " << tmpInformationTabStart << endl ;
			pairStream.push_back( make_pair(2,3) ) ;
			if( findSubId ) pairStream.push_back( make_pair(5,tmpSubIDPosition) ) ;
			else {
				pairStream.push_back( make_pair(5,FORWARD) ) ;
				forwardIDStream.push( tokenLine[1] ) ;
			} // else
			pairStream.push_back( make_pair(-1,-1) ) ;
			pairStream.push_back( make_pair(7,tmpInformationTabStart+1) ) ;
			oneMid.posPair = pairStream ;
			// combine ins
			string tmpIns ;
			tmpIns = tokenLine[0] + " " ;
			for( int end = 1; end < tokenLine.size()-1; end++ ) tmpIns += tokenLine[end] ;
			oneMid.ins = tmpIns ;
			tables.table6.push_back(oneMid) ;
			pairStream.clear() ;
			cout << "Information Table :" ; // 看起來是對的!!! 
			for( int cur = 0; cur < tables.table7.size(); cur++ ) {
				cout << tables.table7[cur] << " " ;
			} // for
			
			system("pause") ;
		} // if
	} // if
	
	else if( tokenClassify[0].first == 5  && !strcmp( tokenLine[tokenLine.size()-1].c_str(), ";" ) ) { // Reverse Polish Notation  !!!!!!!!
		cout << "is assignment,\n " ;
		stack<string> idStack ;
		stack<string> operatorStack ; 
		vector<string> oneMidTab ;
		vector< vector<string> > midTab ;
		bool parenthesesInStack = false, good = true, right = false ; 
		for( int cur = 0; cur < tokenLine.size()-1 ; cur++ ) {
			cout << tokenLine[cur] << "," << cur << "\n" ;
			if( tokenClassify[cur].first == 5 || tokenClassify[cur].first == 3 || tokenClassify[cur].first == 4 ) {
				if( !strcmp( tokenLine[cur].c_str(), "," ) ) operatorStack.push(tokenLine[cur]) ;
				else	idStack.push( tokenLine[cur] ) ;
			} // if
			else if( tokenClassify[cur].first == 1 && parenthesesInStack && !strcmp( tokenLine[cur].c_str(), ")" )) { // 將'('前的全部處理!! 
				// vector<string> formulaInParentheses ;
				// operatorStack.push(tokenLine[cur]) ;
				//做到operatorStack.top()是(
				bool loopIn = false ;
				/*cout << "遇到)馬上做\n" ;
				cout << "idStack : " ;
				stack<string> a = idStack ;
				while (!a.empty()) {
			      std::cout<<a.top()<<" ";
			      a.pop();
			 	} // while()
			 	cout << "  operatorStack: " ;
			 	stack<string> b = operatorStack ;
				while (!b.empty()) {
			      std::cout<<b.top()<<" ";
			      b.pop();
			 	} // while()
			 	cout << endl ;*/
				while( strcmp( operatorStack.top().c_str(), "(" ) ) {
					ProcessFormula( oneMidTab, midTab, idStack, operatorStack, tables ) ;
					loopIn = true ;
				} // while
					
				cout << "遇到)馬上做結束了\n" ;
				/*a = idStack ;
				while (!a.empty()) {
			      std::cout<<a.top()<<" ";
			      a.pop();
			 	} // while()
			 	cout << "  operatorStack: " ;
			 	b = operatorStack ;
				while (!b.empty()) {
			      std::cout<<b.top()<<" ";
			      b.pop();
			 	} // while()
			 	cout << endl ;*/
				for( int test = 0; test < tokenLine.size(); test++ ) {
					if( right && !strcmp( tokenLine[test].c_str(), "(") && test > 1 && tokenClassify[test-1].first == 5 ) { 
						// 若是在等號右邊就將陣列整合Tx 
						ProcessFormula( oneMidTab, midTab, idStack, operatorStack, tables ) ;
						loopIn = false ; // ?? 為了不要多吃掉一個等號  
						break ;
					} // if
				} // for
				
				if( loopIn ){
					operatorStack.pop() ; // ??
				} // if
			} // else if
			else if( tokenClassify[cur].first == 1 && ( operatorStack.empty() || TransOp2Compare( operatorStack.top() ) <= TransOp2Compare( tokenLine[cur] ) || !strcmp( tokenLine[cur].c_str(), "(" ) )  ) { 
				// 若stack中的operater<string中，放入 
				operatorStack.push(tokenLine[cur]) ;
				parenthesesInStack = true ; 
			} // else if
			else if ( tokenClassify[cur].first == 1 && TransOp2Compare( operatorStack.top() ) > TransOp2Compare( tokenLine[cur] ) ) {
				// 做一個中間碼，cur停留
				if( !strcmp( tokenLine[cur].c_str(), "=" ) ) {
					operatorStack.push(tokenLine[cur]) ;
					right = true ;
					cur++ ;
				} // if
				else { 
					cout << "stack得比較大\n" ;
					ProcessFormula( oneMidTab, midTab, idStack, operatorStack, tables ) ;
				} // else 
				cur-- ; 
			} // else if
			else good = false ;
		} // for
		
		while( operatorStack.size() > 0 && idStack.size() > 0 ) {
			if( operatorStack.size() == idStack.size() && !strcmp( operatorStack.top().c_str(), "(" ) ) operatorStack.pop() ;
			cout << "idStack : " ;
			stack<string> a = idStack ;
			while (!a.empty()) {
		      std::cout<<a.top()<<" ";
		      a.pop();
		 	} // while()
		 	cout << "\n operatorStack: " ;
		 	stack<string> b = operatorStack ;
			while (!b.empty()) {
		      std::cout<<b.top()<<" ";
		      b.pop();
		 	} // while()
		 	cout << endl ;
		 	cout << "正常做\n" ;
			ProcessFormula( oneMidTab, midTab, idStack, operatorStack, tables ) ;
			cout << "opsize : " << operatorStack.size() << ", idsize" << idStack.size() << endl ;
			cout << "==============================\n" ;
		} // while()
		
		cout << "sucess.\n" ;
	} // if
	
	else if( !strcmp( tokenLine[0].c_str(), "IF" ) && tokenClassify[0].first == 2 ) {
		cout << "is IF,\n " ;
		vector< string > conditionTmp, trueTmp, falseTmp, oneMidTab ;
		vector< pair<int,int> > classifyTmp ;
		int findThen = 1 ;
		for( findThen; findThen < tokenLine.size() ; findThen++ ) {
			cout << tokenLine[findThen] << " " ;
			if( !strcmp( tokenLine[findThen].c_str(), "THEN") ) break ;
			else if( tokenClassify[findThen].first == 5 && findThen == 1 ) {
				conditionTmp.push_back(tokenLine[findThen]) ;
				
			} // if
			else if( tokenClassify[findThen].first == 5 && findThen > 1 ) {
				oneMidTab.push_back( tokenLine[findThen] ) ;
				char  tmpName [80] ;
				itoa( tables.table0.size()+1, tmpName, 10 ) ;
				string tmpStore = "T" ; 
				tmpStore += tmpName ;
				oneMidTab.push_back(tmpStore) ;
				conditionTmp.push_back(tmpStore) ;
				tables.table0.push_back(tmpStore) ;
				oneMid.posPair = ReturnAssignmentPos( oneMidTab, tables ) ;
				string tmpIns = oneMidTab[3] +  " = " + oneMidTab[1] + " " + oneMidTab[0] + " " + oneMidTab[2] ;
				oneMid.ins = tmpIns ;
				tables.table6.push_back(oneMid) ;
				cout << "----------oneConditionMid : " ;
				for( int i = 0; i < oneMidTab.size(); i ++ ) cout << oneMidTab[i] << " " ;
				cout << "." << endl ;
				oneMidTab.clear() ;
			} // else if
			else if( tokenClassify[findThen].first == 2 ) { //無仔細判斷 
				oneMidTab.push_back( tokenLine[findThen] ) ;
				oneMidTab.push_back( conditionTmp[conditionTmp.size()-1] ) ;
				conditionTmp.erase(conditionTmp.begin() ) ;
			} // else if
			else cout << tokenLine[findThen] << endl ;
		} // for
		
		cout << "if\n" ;
		oneMid.posPair.clear() ;
		oneMid.ins.clear() ;
		oneMid.posPair.push_back( make_pair(2,12) ) ;
		for( int find = 0; find < tables.table0.size(); find++ ) 
			if( !strcmp( tables.table0[find].c_str(), conditionTmp[conditionTmp.size()-1].c_str() ) ) {
				oneMid.posPair.push_back( make_pair(0,find) ) ;
				break ;
			} // if
		
		conditionTmp.erase(conditionTmp.begin() ) ;
		oneMid.posPair.push_back( make_pair(6,tables.table6.size()+2) ) ;
		oneMid.posPair.push_back( make_pair(6,FORWARD) ) ;
		for( int t = 0; t < tokenLine.size(); t++ ) oneMid.ins = oneMid.ins + " " + tokenLine[t] ;
		tables.table6.push_back(oneMid) ;
		
		
		for( findThen+=1; findThen < tokenLine.size() ; findThen++ ) {
			if( !strcmp( tokenLine[findThen].c_str(), "ELSE") ) break ;
			else { //無仔細判斷 
				cout << tokenLine[findThen] << " " ;
				trueTmp.push_back( tokenLine[findThen] ) ;
				classifyTmp.push_back(tokenClassify[findThen]) ;
			} // else if
		} // for
		
		trueTmp.push_back( ";" ) ;
		SyntaxAnalysis( trueTmp, tokenClassify, tables, scope, pointer ) ;
		classifyTmp.clear() ;
		oneMid.posPair.clear() ; // then 最後的gto 
		oneMid.ins.clear() ;
		oneMid.posPair.push_back( make_pair(2,11) ) ;
		oneMid.posPair.push_back( make_pair(-1,-1) ) ;
		oneMid.posPair.push_back( make_pair(-1,-1) ) ;
		oneMid.posPair.push_back( make_pair(6,FORWARD) ) ;
		oneMid.ins = "GTO " ;  
		tables.table6.push_back(oneMid) ;
		for( int findIf = tables.table6.size()-1; findIf > 0; findIf-- ) {
			if( tables.table6[findIf].posPair[0].first == 2 && tables.table6[findIf].posPair[0].second == 12 && tables.table6[findIf].posPair[3].second == FORWARD ) {
				tables.table6[findIf].posPair[3].second = tables.table6.size()+1 ;
				break ;
			} // if
		} // for
		
		
		for( findThen+=1; findThen < tokenLine.size() ; findThen++ ) {
			if( !strcmp( tokenLine[findThen].c_str(), ";") ) break ;
			else { //無仔細判斷 
				cout << tokenLine[findThen] << " " ;
				falseTmp.push_back( tokenLine[findThen] ) ;
				classifyTmp.push_back(tokenClassify[findThen]) ;
			} // else if
		} // for
		
		falseTmp.push_back( ";" ) ;
		SyntaxAnalysis( falseTmp, classifyTmp, tables, scope, pointer ) ;
		
		for( int findIf = tables.table6.size()-1; findIf > 0; findIf-- ) {
			if( tables.table6[findIf].posPair[0].first == 2 && tables.table6[findIf].posPair[0].second == 11 && tables.table6[findIf].posPair[3].second == FORWARD )
				tables.table6[findIf].posPair[3].second = tables.table6.size()+1 ;
		} // for
		
	} // if
	else {
		cout << endl ;
		for( int i = 0; i < tokenLine.size() ; i++ )	
			cout << tokenLine[i] << " " ; 
		error.push_back("syntax wrong.") ;
		cout << "ERROR!!\n" ;
	} // else
} // SyntaxAnalysis()

bool ProcessFormula( vector<string> &oneMidTab, vector< vector<string> > &midTab, stack<string> &idStack, stack<string> &operatorStack, Table &tables ) {
		Middle oneMid ;
		if( !strcmp( operatorStack.top().c_str(), "," ) ) { // 只能處理二維 
			vector< string > tmpRow ;
			vector< string > comma ;
			while( strcmp( operatorStack.top().c_str(), "(" ) ) {
				cout << operatorStack.top() << " " ;
				comma.push_back( operatorStack.top() ) ;
				operatorStack.pop() ;
				cout << idStack.top() << " " ;
				tmpRow.push_back( idStack.top() ) ;
				idStack.pop() ;
			} // while()
			
			cout << idStack.top() << " " ;
			tmpRow.push_back( idStack.top() ) ;
			idStack.pop() ;
			for( int find = 0; find < tables.table5.size() ; find++ ) {
				if( !strcmp( tables.table5[find].identify.c_str(), idStack.top().c_str() ) ) {
					vector<int> arrayInformation ;
					int putin = 0 ;
					int here = tables.table5[find].pointer, arraySize = tables.table7[ here+1 ] ;
					arrayInformation.push_back( tables.table7[ here ] ) ;
					arrayInformation.push_back( arraySize ) ;
					for( int i = 1; i <= arraySize ; i++ ) {
						arrayInformation.push_back( tables.table7[ here+1+i ] ) ;
					} // for
					
					for( int i = 0; i < arrayInformation.size() ; i++ ) {
						cout << arrayInformation[i] << " " ;
					} // for
					
					char  tmpName [80] ; // Tx = J - 1
					itoa( tables.table0.size()+1, tmpName, 10 ) ;
					string tmpStore = "T" ; 
					tmpStore += tmpName ;
					tables.table0.push_back( tmpStore ) ;
					oneMidTab.push_back("-") ;
					oneMidTab.push_back( tmpRow[putin++] ) ;
					oneMidTab.push_back("1") ;
					oneMidTab.push_back(tmpStore) ;
					oneMid.posPair = ReturnAssignmentPos( oneMidTab, tables ) ;
					string tmpIns = tmpStore + " = " + tmpRow[putin-1] + "-1" ;
					oneMid.ins = tmpIns ;
					tables.table6.push_back(oneMid) ;
					cout << "----------oneMid : " ;
					for( int i = 0; i < oneMidTab.size(); i ++ ) cout << oneMidTab[i] << "," ;
					cout << "." << endl ;
					oneMidTab.clear() ;
					
					itoa( tables.table0.size()+1, tmpName, 10 ) ;
					string tmpStore1 = "T" ; 
					tmpStore1 += tmpName ;
					oneMidTab.push_back("*") ;
					oneMidTab.push_back(tmpStore) ;
					tables.table0.push_back( tmpStore ) ;
					char tmpInt[88] ;
					itoa( arrayInformation.at(arrayInformation.size()-2), tmpInt, 10 ) ;
					oneMidTab.push_back( tmpInt ) ;
					oneMidTab.push_back(tmpStore1) ;
					oneMid.posPair = ReturnAssignmentPos( oneMidTab, tables ) ;
					tmpIns = tmpStore1 + " = " + tmpStore + "*" + tmpInt ;
					oneMid.ins = tmpIns ;
					tables.table6.push_back(oneMid) ;
					cout << "----------oneMid : " ;
					for( int i = 0; i < oneMidTab.size(); i ++ ) cout << oneMidTab[i] << "," ;
					cout << "." << endl ;
					oneMidTab.clear() ;
					
					itoa( tables.table0.size()+1, tmpName, 10 ) ;
					string tmpStore2 = "T" ; 
					tmpStore2 += tmpName ;
					tables.table0.push_back( tmpStore ) ;
					oneMidTab.push_back("+") ;
					oneMidTab.push_back(tmpStore1) ;
					oneMidTab.push_back(tmpRow[tmpRow.size()-1] ) ;
					oneMidTab.push_back(tmpStore2) ;
					idStack.push(tmpStore2) ;
					oneMid.posPair = ReturnAssignmentPos( oneMidTab, tables ) ;
					tmpIns = tmpStore2 + " = " + tmpRow[tmpRow.size()-1] + "+" + tmpStore1 ;
					oneMid.ins = tmpIns ;
					tables.table6.push_back(oneMid) ;
					cout << "----------oneMid : " ;
					for( int i = 0; i < oneMidTab.size(); i ++ ) cout << oneMidTab[i] << "," ;
					cout << "." << endl ;
					oneMidTab.clear() ;
					
					break ;	
				} // if
			} // for
		} // if
		else if( !strcmp( operatorStack.top().c_str(), "=" ) ) {
			oneMidTab.push_back(operatorStack.top()) ;
			operatorStack.pop() ;
			if( midTab.size() > 0 ) {
				oneMidTab.push_back(midTab[midTab.size()-1][midTab[midTab.size()-1].size()-1]) ;
				idStack.pop() ;
			} // if
			else {
				oneMidTab.push_back(idStack.top()) ;
				idStack.pop() ;
			} // else
			
			if( operatorStack.size() > 1 ) {
				string tmp = idStack.top() ;
				idStack.pop() ;
				oneMidTab.push_back(idStack.top()) ;
				idStack.pop() ;
				oneMidTab.push_back(tmp) ;
				cout << "processing\n" ;
			} // if
			else {
				oneMidTab.push_back(" ") ;
				oneMidTab.push_back(idStack.top()) ;
				idStack.pop() ;
				if( idStack.size() == 1 && operatorStack.size() == 1 && !strcmp( operatorStack.top().c_str(), "(" ) ) {
					oneMidTab.at(2) = idStack.top() ;
					idStack.pop() ;
					operatorStack.pop() ;
				} // if
			} // else
			
			string tmpIns ;
			cout << "----------oneMid : " ;
			for( int i = 0; i < oneMidTab.size(); i ++ ) cout << oneMidTab[i] << "," ;
			cout << "." << endl ;
			oneMid.posPair = ReturnAssignmentPos( oneMidTab, tables ) ;
			if( strcmp( oneMidTab[2].c_str(), " " ) && strcmp( oneMidTab[3].c_str(), "" ) ) tmpIns = oneMidTab[2] + "(" + oneMidTab[3] + ")" ;
			else if( !strcmp( oneMidTab[2].c_str(), " " ) && strcmp( oneMidTab[3].c_str(), "" ) ) tmpIns = oneMidTab[3] ;
			else cout << "Line 933 error \n" ;
			tmpIns = tmpIns + oneMidTab[0] + oneMidTab[1] ;
			oneMid.ins = tmpIns ;
			tables.table6.push_back(oneMid) ;
			midTab.push_back(oneMidTab) ;
			oneMidTab.clear() ;
		} // if
		else {
			if( !strcmp( operatorStack.top().c_str(), "(" ) || !strcmp( operatorStack.top().c_str(), ")" ) ) {
				oneMidTab.push_back("=") ;
			} // if
			else oneMidTab.push_back(operatorStack.top()) ;
			
			operatorStack.pop() ;
			string tmp = idStack.top() ;
			idStack.pop() ;
			oneMidTab.push_back(idStack.top()) ;
			idStack.pop() ;
			oneMidTab.push_back(tmp) ;
			char  tmpName [80] ;
			itoa( tables.table0.size()+1, tmpName, 10 ) ;
			string tmpStore = "T" ; 
			tmpStore += tmpName ;
			oneMidTab.push_back(tmpStore) ;
			tables.table0.push_back(tmpStore) ;
			idStack.push(tmpStore) ;
			cout << oneMidTab.size() << endl ; 
			oneMid.posPair = ReturnAssignmentPos( oneMidTab, tables ) ;
			string tmpIns ;
			if( strcmp( oneMidTab[0].c_str(), "=" ) ) tmpIns = oneMidTab[3] +  " = " + oneMidTab[1] + oneMidTab[0] + oneMidTab[2] ;
			else tmpIns = oneMidTab[3] +  " = " + oneMidTab[1] + "(" + oneMidTab[2] + ")" ;
			oneMid.ins = tmpIns ;
			tables.table6.push_back(oneMid) ;
			cout << "----------oneMid : " ;
			for( int i = 0; i < oneMidTab.size(); i ++ ) cout << oneMidTab[i] << " " ;
			cout << "." << endl ;
			midTab.push_back(oneMidTab) ;
			oneMidTab.clear() ;
		} // else
		
	//} // while()
	
	return true ;
} // ProcessFormula()


vector< pair<int,int> > ReturnAssignmentPos( vector<string> &oneMidTab, Table &tables ) {
	vector< pair<int,int> > returnV ;
	bool first = false ;
	for( int find = 0; find < tables.table1.size(); find++ ) {
		if( !strcmp( tables.table1[find].c_str(), oneMidTab[0].c_str() ) ) {
			returnV.push_back( make_pair(1,find) ) ;
			first = true ;
			break ;
		} // if
	} // for
	
	for( int find = 0; !first && find < tables.table2.size(); find++ ) {
		if( !strcmp( tables.table2[find].c_str(), oneMidTab[0].c_str() ) ) {
			returnV.push_back( make_pair(2,find) ) ;
			break ;
		} // if
	} // for
	
	for( int findEach = 1; findEach < oneMidTab.size(); findEach++ ) {
		if( oneMidTab[findEach][0] == ' ' ) returnV.push_back( make_pair(-1,-1) ) ;
		else if( isdigit(oneMidTab[findEach][0]) ) {
			bool isFloat = false ;
			for( int find = 0; find < oneMidTab[findEach].size(); find++ ) if( oneMidTab[findEach][find] == '.' ) isFloat = true ;
			
			for( int find = 0; !isFloat && find < tables.table3.size(); find++ ) {
				if( tables.table3[find] == atoi(oneMidTab[findEach].c_str()) ) {
					returnV.push_back( make_pair(3, AsciiSum(oneMidTab[findEach]) % 100 ) );
					break ;
				} // if
			} // for
			
			istringstream istr(oneMidTab[findEach]) ;
			float com ;
			istr >> com ;
			for( int find = 0; isFloat && find < tables.table4.size(); find++ ) {
				if( tables.table4[find] == com ) {
					returnV.push_back( make_pair(4,AsciiSum(oneMidTab[findEach]) % 100 )) ;
					break ;
				} // if
			} // for
		} // if
		else {
			bool in = false ;
			for( int find = 0; find < tables.table0.size(); find++ ) {
				if( !strcmp( tables.table0[find].c_str(), oneMidTab[findEach].c_str() ) ) {
					returnV.push_back( make_pair(0,find+1) ) ;
					in =true ;
					break ;
				} // if
			} // for
			
			for( int find = 0; !in && find < tables.table5.size(); find++ ) {
				if( !strcmp( tables.table5[find].identify.c_str(), oneMidTab[findEach].c_str() ) ) {
					returnV.push_back( make_pair(5,find) ) ;
					break ;
				} // if
			} // for
		} // else
	} // for
	
	cout << "length:" << returnV.size() << endl;
	return returnV ;
} // ReturnAssignmentPos()

int TransOp2Compare( string str ) {
	if( !strcmp( str.c_str(), "=" ) ) return 1 ;
	else if( !strcmp( str.c_str(), "(" ) ) return 2 ;
	else if( !strcmp( str.c_str(), ")" ) ) return 3 ;
	else if( !strcmp( str.c_str(), "+" ) ) return 4 ;
	else if( !strcmp( str.c_str(), "-" ) ) return 5 ;
	else if( !strcmp( str.c_str(), "*" ) ) return 6 ;
	else if( !strcmp( str.c_str(), "/" ) ) return 7 ;
	else if( !strcmp( str.c_str(), "^" ) ) return 8 ;
	else return 0 ;
	
} // CompareOperater()

bool CheckArray( vector<string> tokenLine, vector<pair<int,int> > tokenClassify, vector<string> id , int &type ) {
	cout << endl ;
	for( int i = 0; i < tokenLine.size(); i++ ) cout << tokenLine[i] << " " ; // 將兩括號成功切下 
	cout << endl ;
	bool nextComma = false ;
	type = CheckType( tokenLine[0] ) ;
	if( type != -1 && !strcmp( tokenLine[1].c_str(), ":" ) ) {
		for( int cur = 2; cur < tokenLine.size(); cur++ ) {
			if( !nextComma && cur+2 < tokenLine.size() && tokenClassify[cur].first == 5 && !strcmp( tokenLine[cur+1].c_str(), "(" ) && !strcmp( tokenLine[cur+2].c_str(), ")" ) ) {
				id.push_back( tokenLine[cur] ) ;
				cur+=2 ;
				nextComma = true ;
			} // if
			else if ( nextComma && !strcmp(tokenLine[cur].c_str(), ",") ) nextComma = false ; // pass ','
			else {
				cout << "cur:" << cur << "\n" << tokenLine[cur] << tokenLine[cur+1] << tokenLine[cur+2] << "This isn't an array.(Line:526)\n" ;
				return false ;
			} // else
		} // for
	} // if
	else {
		cout << tokenLine[1] ;
		cout << "Array wrong.\n" ;
		return false ;
	} // else
	
	return true ;
	
} // CheckArray()

bool CheckVariable( vector<string> tokenLine, vector<pair<int,int> > tokenClassify , vector<string> &id, int &type ) { // id{,id}
	type = CheckType( tokenLine[0] ) ;
	if( type != -1 && !strcmp( tokenLine[1].c_str(), ":" ) ) {
		for( int cur = 2; cur < tokenLine.size(); cur++ ) {
			if( cur%2 == 0 && tokenClassify[cur].first == 5 ) {
				id.push_back( tokenLine[cur] ) ;
			} // if
			else if ( cur%2 == 1 && !strcmp(tokenLine[cur].c_str(), ",") ) ; // pass ','
			else {
				cout << "cur:" << cur << "\n-----> ' " << tokenLine[cur] << " ' This isn't a variable.(Line:550)\n" ;
				return false ;
			} // else
		} // for
	} // if
	else {
		cout << tokenLine[1] ;
		cout << "variable wrong.\n" ;
		return false ;
	} // else
	
	return true ;
} // CheckVariable()

bool CheckDimension( vector<string> tokenLine, vector<pair<int,int> > tokenClassify , vector<int> &dimensionSize ) { // ( num {,num} ) 
	cout << endl ;
	for( int i = 0; i < tokenLine.size(); i++ ) cout << tokenLine[i] << "  " ; // 將兩括號成功切下 
	cout << endl ;
	int checkParentheses = 0 ;
	bool onlyNum = false, nextShouldBeParenthese = true ; // 防止在奇數出現identify & 偶數該為括號時為逗點 
	for( int cur = 0; cur < tokenLine.size(); cur++ ) {
		cout << tokenLine[cur] ;
		if( onlyNum && cur%2 == 1 && tokenLine[cur].find_first_of( ".", 0 ) == string::npos? 1:0 ) {
			istringstream istr(tokenLine[cur]) ;
			int tmp ;
			istr >> tmp ;
			dimensionSize.push_back( tmp ) ;
		} // else if
		else if( nextShouldBeParenthese && cur%2 == 0 && !strcmp( tokenLine[cur].c_str(), "(") ) {
			checkParentheses++ ;
			nextShouldBeParenthese = false ;
			onlyNum = true ;
			dimensionSize.clear() ;
		} // else if
		else if( cur%2 == 0 && !strcmp( tokenLine[cur].c_str(), ")") ) {
			checkParentheses-- ;
			onlyNum = false ;
		} // else if
		else if( !nextShouldBeParenthese && cur%2 == 0 && !strcmp( tokenLine[cur].c_str(), ",") ) ; // pass ','
		else {
			cout << "\n----->" << tokenLine[cur] << "This is the place that was wrong in dimension.\n" ;
			return false ;
		} // else	
	} // for
	
	if( checkParentheses != 0 ) { // 檢查括號是否對稱 
		cout << "checkParentheses is wrong.\n" ;
		return false ;
	} // if*/
	
	return true ;
} // CheckDimension()

bool ForwardReference( Table &tables ) {
	for( int i = 0; i < tables.table6.size(); i++ ) {
		for( int out = 0; out < 4; out++ ) {
			if( tables.table6[i].posPair[out].second == FORWARD ) {
				for( int find = 0; find < tables.table5.size(); find++ ) {
					if( !strcmp( tables.table5[find].identify.c_str(), forwardIDStream.front().c_str() ) ) {
						tables.table6[i].posPair[out].second = find ;
						cout << " change it .\n" ;
						break ;
					} // if
				} // for
				
				forwardIDStream.pop() ;
			} // if
		} // for
	} // for
} // ForwardReference()

int CheckType( string type ) {
	if( !strcmp( type.c_str(), "ARRAY") ) return 1 ;
	else if( !strcmp( type.c_str(), "BOOLEAN") ) return 2 ;
	else if( !strcmp( type.c_str(), "CHARACTER") ) return 3 ;
	else if( !strcmp( type.c_str(), "INTEGER") ) return 4 ;
	else if( !strcmp( type.c_str(), "LABEL") ) return 5 ;
	else if( !strcmp( type.c_str(), "REAL") ) return 6 ;
	else return -1 ;
} // CheckType()

void WriteAll( Table tables, string filename ) {
	fstream txtFile ;  // input file 
	int num = 0 ; 
	if( !firstT) {
		txtFile.open((filename +  + "_output.txt").c_str(), ios::out) ;  // open a txt file 
		firstT = true ;
	} // if
	else
		txtFile.open((filename +  + "_output.txt").c_str(), ios::app) ;
	if( txtFile.is_open()) { 
		for( int i = 0; i < tables.table6.size(); i++ ) {
			txtFile << i+1 << " (" ;
			for( int out = 0; out < 4; out++ ) {
				if( out >= tables.table6[i].posPair.size() || ( tables.table6[i].posPair[out].first == -1 && tables.table6[i].posPair[out].second == -1 ) )
					txtFile << ", " ;
				else
					txtFile << "(" << tables.table6[i].posPair[out].first << "," << tables.table6[i].posPair[out].second << ") , " ;
			} // for
			txtFile << ") " << tables.table6[i].ins << "\n" ;
		} // for
	} // end if 
	
	
	int r = 0 ;
	while( error.size() > r ) {
		txtFile << r+1 << "  " << error[r++] <<endl ;
	} // while
	txtFile.close() ;  // close input file
} // WriteLineByLine

bool Collision( vector<Identifier> &table, int &key, string content ) { // hashtable full!!?
	int isFull = 1 ;
	key %= 100 ;
	while( !table[key].identify.empty() ) { // && strcmp( table[key].identify.c_str(), content.c_str()) != 0 不能重複拿調 
		key = ( key+1 ) % 100 ;
		isFull++ ;
		if( isFull == 100 ) {
			return false ;
		} // if
	} // while
	
	table[key].identify = content ;
	return true;
} // Collision

string ToUpper( string input ){
	ostringstream oss;
	for( int t = 0; t < input.size(); t++ ) {
		input[t] = toupper(input[t]) ;
		oss << input.at(t);
	} // for
			
	return oss.str() ;
} // toUpper

bool isFloat( string myString ) {
    std::istringstream iss(myString);
    float f;
    iss >> noskipws >> f; // noskipws considers leading whitespace invalid
    // Check the entire string was consumed and if either failbit or badbit is set
    return iss.eof() && !iss.fail(); 
} // isFloat

int AsciiSum( string input ){
	int sum = 0;
	for( int t = 0; t < input.size(); t++ ) {
		sum += input[t] ;
	} // for
	
	return sum ;
} // ASiiSum()

void Output() {
	cout << "**************************" << endl;
	cout << "* 0. Quit                *" << endl;
	cout << "* 1. FrancisCompiler     *" << endl;
	cout << "**************************" << endl;
	cout << "Input a command(0, 1): ";
} // OutPut()

bool InputInt( int &i ) { // (防呆)
  if(!(cin >> i)) {
    cin.clear();  // 清除cin的錯誤狀態 
    cin.ignore(numeric_limits<streamsize>::max(),'\n');  // 把從現在起，以後整行的input都丟棄掉 
    return false;
  } // if
  
  return true;
} // InputInt()

bool Table::InputTable(){
	fstream fileTable;
	string filename = "Table1.table";
	fileTable.open( filename.c_str(), ios::in );
	char buf[511];
	while( !fileTable.eof() ) {
		fileTable >> buf;
		table1.push_back(buf);
	} // while
	
	fileTable.close();
	filename = "Table2.table";
	fileTable.open( filename.c_str(), ios::in );
	while( !fileTable.eof() ) {
		fileTable >> buf;
		table2.push_back(buf);
	} // while
	
	fileTable.close();
} // InputTalbe
