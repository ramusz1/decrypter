#include <iostream>
#include <vector>
#include <list>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <ctype.h>
#include <string>

using namespace std;

const char _interpunction[] = {'.', ',', '?','!', ' ', '\n'};
int _interpunctionSize = 6;

string readBytes(){
	string line;
	cin.ignore(256,':');
	//empty line 
	getline(cin,line);
	//actual line
	getline(cin,line);
	return line;
}

// converts strings of bytes like "00101011" to ASCII code 
vector<char> getCryptogram(string& bytes){
	int byteSize = 8;
	vector<char> out;
	out.reserve(( bytes.size() + 1) /( byteSize + 1));
	for(int i=0; i < bytes.size(); i+= byteSize + 1){		//+1 to skip the space
		char a = 0;
		for(int j = 0; j < byteSize; j++){
			a = a << 1;
			a += (int) (bytes[i + j] - '0');			//converts '1' to 1  and '0' to 0
		}
		out.push_back(a);
	}
	return out;
}

//performs xor on two cryptograms
vector<char> xorCryptograms(const vector<char>& a, const vector<char>& b){
	int size = min(a.size(),b.size());
	vector<char> out(size);
	for(int i = 0; i < size; i++){
		out[i] = a[i] ^ b[i];
	}
	return out;
}

// Rates how well the character fits our reguirements
int getScore(char c){
	if ( ( 'a' <= c && c <= 'z'  ) || ('A' <= c && c <= 'Z') )
		return 0;

	int i = _interpunctionSize;
	while(i--  && _interpunction[i] != c){}
	if( i >= 0 )
		return 1;

	if('0' <= c && c <= '9')
		return 2;

	return 4;
}

// returns a word that has a letter on the k-th position in the w vector
string getWord(vector<char>& w, int k){
	char space = ' ';
	if(w[k] == space )
		return " ";
	auto start = find(w.rbegin() + (w.size() - k - 1), w.rend(), space ).base();
	auto end = find(w.begin() + k, w.end(), space );
	string output;
	for(auto& it = start; it != end; it++){
		output+=*it;
	}
	return output;
}

// removes punctuation signs from the begining and from the end of a word
string removePunctuation(string& str){
	int i = 0;
	while(ispunct(str[i])){
		i++;
	}
	int j = str.size() - 1;
	while(ispunct(str[j])){
		j--;
	}
	return str.substr(i,j-i+1);
}

void toLowerCase(string& str){
	transform(str.begin(),str.end(),str.begin(), ::tolower);
}

void dump(vector<vector<char> >& cryptograms){

	for(int i = 0; i < cryptograms.size(); i++){
		cout<<"\t### start : "<<i<<" ###"<<endl;
		for(auto& c : cryptograms[i])
			cout<<c;
		cout<<endl;
	}	
}

int main(int argc, char* argv[]){
	// handle flag
	bool runDictionaryEnhancement = false;
	if(argc > 1){
		string arg(argv[1]);
		if(arg == "-d"){
			cout<<"Dictionary Enhancement enabled\n";
			runDictionaryEnhancement = true;
		}
	}

	vector< vector< char > > cryptograms;

	// read raw data
	string bytes = readBytes();
	while(bytes != ""){
		cryptograms.push_back(  getCryptogram(bytes) );
		bytes = readBytes();
	}

	vector<vector<vector<char> > > mix ( cryptograms.size() );

	// calculate the llength of the longest message, used later
	int maxSize = 0;

	for(int i = 0; i < cryptograms.size(); i++){

		maxSize = max(maxSize,(int)cryptograms[i].size() ); 
		mix[i].resize(cryptograms.size()); 	// waste
		for(int j = i + 1; j < cryptograms.size(); j ++)
			mix[i][j] = xorCryptograms(cryptograms[i],cryptograms[j]);
	}

	vector<char> chars = { ' ' , 'a','u','e','o','A','U','E','O', 'i','I','.','z','Z'};
	for(int k = 0; k < maxSize; k++ ){

		pair<int,char> bestPair {0,'X'};
		int bestScore = cryptograms.size() * 10;

		// Calculates score of each possible char, position(i) pair	
		for(int i = 0; i < cryptograms.size(); i++){

			if( k >= cryptograms[i].size() )
				continue;

			for (auto& special : chars){

				int score = 0;
				for (int j = 0; j < cryptograms.size() ; j++ ){

					if ( i == j || k >= cryptograms[j].size())
						continue;
					if ( i > j )
						score += getScore(mix[j][i][k] ^ special);
					else 
						score += getScore(mix[i][j][k] ^ special);					
				}

				if(score < bestScore){
					bestPair.first = i;
					bestPair.second = special;
					bestScore = score;
					if(bestScore == 0)
						break;
				}
			}
			if(bestScore == 0){
				break;
			}
		}

		// Applies best possible fit, on this position(k)
		cryptograms[ bestPair.first ][k] = bestPair.second; 
		for(int i = 0; i < cryptograms.size(); i ++){
			if( k >= cryptograms[i].size()  || bestPair.first == i)
				continue;
			if( bestPair.first < i )
				cryptograms[i][k] = mix[ bestPair.first ][i][k] ^ bestPair.second;
			else 
				cryptograms[i][k] = mix[i][ bestPair.first ][k] ^ bestPair.second;
		}
	}

	if( !runDictionaryEnhancement ){
		dump(cryptograms);
		return 0;
	}

	// Dictionary Enhancement 
	// ENHANCE 

	//read the dict 
	ifstream in("words.txt");
	vector<vector<string>> dict;
	// 40 - approximate longest word length
	dict.resize(40);
	string word;
	getline(in,word);
	while(word.size() != 0){
		dict[word.size()].push_back(word);
		word ="";
		getline(in,word);
	}

	// iterate through cryptograms
	int currCryptIndex = 0;
	for( auto& currCrypt : cryptograms){

		int start = 0;
		//search for words in current cryptogram
		while (start < currCrypt.size()){

			while(start < currCrypt.size() && ( ispunct(currCrypt[start]) || currCrypt[start] == ' ') ){
				start++;
			}

			int end = start;
			string word;

			while ( end < currCrypt.size() && !ispunct(currCrypt[end]) && currCrypt[end] != ' '){
				word += currCrypt[end];
				end++;
			}

			//cout<<"word: " << word << endl;
			string copy = word;

			toLowerCase(word);

			if (word.size() == 0 || binary_search(dict[word.size()].begin(),dict[word.size()].end(),word) ){
				start = end;
				start++;
				//cout<<"\tlegit or empty"<<endl;
				continue;
			}

			int maxErrors = 2;	
			if( word.size() < 4)
				maxErrors = 1;

			//gather possible corrections
			vector<vector<pair<char,int>>> corrections;
			for (auto& e : dict[word.size()]){

				if(e.size() != word.size())
					continue;

				vector<pair<char,int>> adjustment;
				int err = 0;
				int i = 0;
				while(i < word.size() && err <= maxErrors){
					if(word[i] != e[i]){
						pair<char,int> p(e[i],i);
						adjustment.push_back(p);
						err++;
					}
					i++;
				}
				if( err <= maxErrors){
					corrections.push_back(adjustment);
				}
			}
			if(corrections.size() == 0){
				//cout<<"\tnothing comes close\n";
				start = end;
				start++;
				continue;
			}
			// check possible corrections
			for(auto& c : corrections){
				auto cryptogramsCopy = cryptograms;

				// correct the word
				for(auto& p : c)
					cryptogramsCopy[currCryptIndex][p.second+start] = p.first;

				float bad = 0.0;
				float good = 0.0;
				float minRatio = 0.7;
				for(int i = 0; i < cryptograms.size(); i++){

					if( currCryptIndex == i)
							continue;

					// apply the changes
					for(auto& p : c){
						int absPos = p.second + start;
						char substitute = p.first;

						if( absPos >= cryptograms[i].size() )
							continue;

						if( currCryptIndex < i )
							cryptogramsCopy[i][absPos] = mix[ currCryptIndex ][i][absPos] ^ substitute;
						else 
							cryptogramsCopy[i][absPos] = mix[i][ currCryptIndex ][absPos] ^ substitute;
					}

					// check 
					for(auto&p : c){
						int absPos = p.second + start;

						if( absPos >= cryptograms[i].size() )
							continue;

						string corrected = getWord(cryptogramsCopy[i],absPos);
						corrected = removePunctuation(corrected);
						toLowerCase(corrected);
						if( corrected != " " && !binary_search(dict[corrected.size()].begin(),dict[corrected.size()].end(),corrected) ){
							bad++;
						} else {
							good++;
						} 

					}
				}
				if( good / (good + bad ) >= minRatio){
					cryptograms = cryptogramsCopy;
					//cout<<"\tREPAIRED ########################\n";
					//cout<<"word: " << word << endl;
					//cout<<"\t currCryptIndex: " << currCryptIndex << endl;
					break;
				}
			}
			start = end;
			start++;
		}
		currCryptIndex++;
	}

	dump(cryptograms);
	
	return 0;
}