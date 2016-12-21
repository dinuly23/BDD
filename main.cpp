#include <iostream>      
#include <cstdlib>     
#include <getopt.h>
#include <string>
#include <fstream>
#include <regex>
#include <algorithm>
#include <bdd.h>
#include <vector>

using namespace std;

void empty_gbc_handler(int pre, bddGbcStat *stat) {}

string string_for_dec_to_bin="";
string dec_to_bin(int var){
    if( var >1) dec_to_bin(var/2); 
    string_for_dec_to_bin += to_string(var %2);
    return string_for_dec_to_bin;
}

string key(int var,int size){
    string_for_dec_to_bin ="";
    string res=dec_to_bin(var);
    if(res.size() < size){
        int n = size-res.size();
        for(int i=0; i < n; i++){
            res="0" + res;
        }
    }
    return res;
}


int num_of_param(int var){
    int res=1;
    int pow=0;
    do{
			pow++;
            res*=2;
    }while(res<var);
    return pow;
}
	
	

int
main(int argc, char **argv)
{
	int c; 
    int option_index = 0;
	static struct option long_options[] = {
        {"help",     no_argument, 0,  'h' },
		{"g",  required_argument, 0,  'g' },
        {"c",  required_argument, 0,  'c' },
        {0, 0, 0, 0 }
    };
    string roads, commands;
	
	try{
		c = getopt_long(argc, argv, "hg:c:",
                long_options, &option_index);
		if (c == -1){
			throw "input error";
		}
		switch (c) {
        case 'h':
            cout<< long_options[0].name << "\thelp me" << endl;
            cout<< long_options[1].name << "\troads" << endl;
            cout<< long_options[2].name << "\tcommands" << endl;
            exit(EXIT_SUCCESS);
            break;
        case 'g':
			if(optarg){
				roads = string(optarg);
				//cout << roads << endl ;
            }
            else throw "error need argument";
			if((c = getopt_long(argc, argv, "c:",
                 long_options, &option_index)) == -1){
				throw "error input";
				break;
			}	
		case 'c':
			if(optarg){
				commands = string(optarg);
				//cout << commands << endl ;
            }
            else throw "error need argument";
            break;
        default:
            break;
        }
        
        ifstream fin(roads.c_str());
        int num_vertex, num_var;
        fin >> num_vertex;
        num_var = num_of_param(num_vertex);
        //cout << num_var;
        
        // случайные числа на то, какие должны быть общий размер массива, хранящего все вершины bdd, и размера кэша вершин
		const size_t node_num = 20000000;
		const size_t cache_size = 2000000;
		bdd_init(node_num, cache_size);
		// lдля сборщика
		bdd_gbc_hook(empty_gbc_handler);
		// явно говорим, какие хотим переменные, тут - x0 ... x4
		bdd_setvarnum(2*num_var);
        //заведем массивы для BDD меток дуг и вершин 
        int num_edge, num_edge_labels, num_vertex_labels;
        fin >> num_edge >> num_vertex_labels >> num_edge_labels;
        bdd edge[num_edge_labels+1], vertex[num_vertex_labels+1];
        
        vector<string> var(num_vertex);
        for(int i=0; i< num_vertex ; i++){
			var[i]= key(i, num_var);
			//cout<< var[i] << " ";
		}
        //cout<< endl;
        
        bdd param[2*num_var];
        for(int i=0; i< 2*num_var; i++){
			param[i]= bdd_ithvar(i);
		}
		
		int vertex_label;
		int visited[num_vertex_labels];
		for(int i=0; i< num_vertex_labels ;i++)
			visited[i]=0;
		bdd conjunction; //конъюнкт 
		for(int i=0; i< num_vertex; i++){ //цикл по всем вершинам
			fin>> vertex_label; 
			if(var[i][0] == '0')
				conjunction= bdd_not(param[0]);
			else conjunction= param[0];
			for(int j=1 ; j<num_var ; j++){
				if(var[i][j]== '0')
					conjunction= bdd_apply(conjunction,bdd_not(param[j]), bddop_and);
				else conjunction= bdd_apply(conjunction,param[j], bddop_and); 
			}
			if(visited[vertex_label] == 0)
				vertex[vertex_label]= conjunction;
			else vertex[vertex_label]= bdd_apply(vertex[vertex_label],conjunction, bddop_or);
			visited[vertex_label]++;
		}
        
        int to, from, label;
        int visited_edge[num_edge_labels];
        for(int i=0; i< num_edge_labels ;i++)
			visited_edge[i]=0;
        for(int i=0; i< num_edge ; i++){
			fin>> to >> label >> from;
			if(var[to][0] == '0')
				conjunction= bdd_not(param[0]);
			else conjunction= param[0];
			for(int j=1; j < num_var; j++){
				if(var[to][j]== '0')
					conjunction= bdd_apply(conjunction, bdd_not(param[j]), bddop_and);
				else conjunction= bdd_apply(conjunction, param[j], bddop_and);
			}
			for(int j=0; j< num_var; j++){
				if(var[from][j]=='0')
					conjunction= bdd_apply(conjunction, bdd_not(param[j+num_var]), bddop_and);
				else conjunction= bdd_apply(conjunction, param[j+ num_var], bddop_and);
			}
			if(visited_edge[label] == 0)
				edge[label]= conjunction;
			else edge[label]= bdd_apply(edge[label],conjunction, bddop_or);
			visited_edge[label]++;
		}
        
        bdd result= !param[0];
        for(int i=1; i< num_var; i++)
			result=bdd_apply(result, bdd_not(param[i]), bddop_and);
        bdd temp;
        
        int first=1;
        
        fin.close();
        fin.open(commands);
        string s;
        while(getline(fin,s)) {
			const std::regex r("move -?\\d+(?:,-?\\d+)*", std::regex::ECMAScript);
			const std::regex r_check("check -?\\d+(?:,-?\\d+)*", std::regex::ECMAScript);
			const std::regex r_back("back -?\\d+(?:,-?\\d+)*", std::regex::ECMAScript);
			const std::regex replace_move(":");
			const std::regex replace_not("not ");
			const std::regex numbers("-?\\d+", std::regex::ECMAScript);
			//~ string s = "move 1,2:2,1 ";
			if (regex_match(s,regex("move.*")))
				s= regex_replace(s, replace_move, " move ");
			else
				s= regex_replace(s, replace_move, " back ");
			s= regex_replace(s, replace_not, "-");
			//s= regex_replace(s, std::regex (",") , " ");
			cout << s << endl;
			
			std::sregex_iterator iter(s.begin(), s.end(), r);
			std::sregex_iterator end;
			while(iter != end)
			{
				cout <<  string((*iter)[0]) << std::endl;
				string str=string((*iter)[0]);
				std::sregex_iterator it(str.begin(), str.end(), numbers);
				while(it != end){
					cout<< (*it)[0] << endl;
					int num= stoi(string((*it)[0]));
					if (num<0) {
						for (int i=0;i<num_edge_labels;i++) {
							if (visited_edge[i] && i!=-num) {
								if(first){
									temp=edge[i];
									first=0;
								}
								else temp= bdd_apply(temp,edge[i], bddop_or); 
							}
						}
					}
					else {
						if(first){
							temp=edge[num];
							first=0;
						}
						else temp= bdd_apply(temp,edge[num], bddop_or); 
					}
					it++;
				}
				//~ bdd_printtable(temp);
				result = bdd_apply(result,temp, bddop_and);
				//~ bdd_printtable(result);			
				for(int i=0; i< num_var; i++)
					result=bdd_exist(result,param[i]);
				bddPair *p;
				p= bdd_newpair();
				const int size = num_var;
				int from[size], to[size];
				for(int i=num_var; i< 2*num_var; i++) {
					from[i-num_var]=i;
					to[i-num_var]=i-num_var;
				}
				bdd_setpairs(p, from, to, size);
				result=bdd_replace(result,p);
				bdd_freepair(p);
				//~ bdd_printtable(result);			 
				
				++iter;
				first=1;
			}
			
			iter=sregex_iterator(s.begin(), s.end(), r_back);
			while(iter != end)
			{
				cout <<  string((*iter)[0]) << std::endl;
				string str=string((*iter)[0]);
				std::sregex_iterator it(str.begin(), str.end(), numbers);
				while(it != end){
					cout<< (*it)[0] << endl;
					int num= stoi(string((*it)[0]));
					if (num<0) {
						for (int i=0;i<num_edge_labels;i++) {
							if (visited_edge[i] && i!=-num) {
								if(first){
									temp=edge[i];
									first=0;
								}
								else temp= bdd_apply(temp,edge[i], bddop_or); 
							}
						}
					}
					else {
						if(first){
							temp=edge[num];
							first=0;
						}
						else temp= bdd_apply(temp,edge[num], bddop_or); 
					}
					it++;
				}
				//~ bdd_printtable(temp);
				bddPair *p;
				p= bdd_newpair();
				const int size = num_var;
				int from[size], to[size];
				for(int i=0; i< num_var; i++) {
					from[i]=i;
					to[i]=i+num_var;
				}
				bdd_setpairs(p, from, to, size);
				result=bdd_replace(result,p);
				bdd_freepair(p);
				result = bdd_apply(result,temp, bddop_and);
				//~ bdd_printtable(result);			
				for(int i=num_var; i< 2*num_var; i++)
					result=bdd_exist(result,param[i]);
				//~ bdd_printtable(result);			 
				
				++iter;
				first=1;
			}
			
			iter=sregex_iterator(s.begin(), s.end(), r_check);
			if (iter!=end) {
				cout <<  string((*iter)[0]) << std::endl;
				string str=string((*iter)[0]);
				std::sregex_iterator it(str.begin(), str.end(), numbers);
				while(it != end){
					cout<< (*it)[0] << endl;
					int num= stoi(string((*it)[0]));
					if (num<0) {
						for (int i=0;i<num_vertex_labels;i++) {
							if (visited[i] && i!=-num) {
								if(first){
									temp=vertex[i];
									first=0;
								}
								else temp= bdd_apply(temp,vertex[i], bddop_or); 
							}
						}
					}
					else {
						if(first){
							temp=vertex[num];
							first=0;
						}
						else temp= bdd_apply(temp,vertex[num], bddop_or); 
					}
					it++;
				}
				result=bdd_apply(result,temp,bddop_and);
				first=1;
			}
		}
		if (bdd_pathcount(result)>0)
			cout << "YES" << endl;
		else
			cout << "NO" << endl;
		
     }
    catch(char const * e){
        cout<< e << endl;
   }
    return 0;
}
