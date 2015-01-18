#ifndef UTIL_UTIL_H
#define UTIL_UTIL_H

#include<string>
#include<map>
#include<iostream>
#include<vector>
#include<sstream>
#include "interface.h"

#ifndef M_PI
#define M_PI 3.1415
#endif

Pwm_output pwm_convert(double);

int write_file(std::string const& filename,std::string const& contents);
int read_file(std::string const& filename,std::string &out);//I don't like out parameters.

template<typename A,typename B>
std::ostream& operator<<(std::ostream& o,std::pair<A,B> const& p){
	return o<<"("<<p.first<<","<<p.second<<")";
}

template<typename K,typename V>
std::ostream& operator<<(std::ostream& o,std::map<K,V> const& m){
	o<<"{";
	for(typename std::map<K,V>::const_iterator at=m.begin();at!=m.end();++at){
		o<<*at;
	}
	return o<<"}";
}

template<typename T>
std::vector<T>& operator|=(std::vector<T>& v,T t){
	v.push_back(t);
	return v;
}

template<typename T>
std::ostream& operator<<(std::ostream& o,std::vector<T> const& v){
	o<<"[";
	for(typename std::vector<T>::const_iterator at=v.begin();at!=v.end();++at){
		o<<' '<<*at;
	}
	return o<<" ]";
}

double sum(std::vector<double>);
double mean(std::vector<double>);
double stddev(std::vector<double>);

template<typename T>
std::string as_string(T t){
	std::stringstream ss;
	ss<<t;
	return ss.str();
}

double clip(double);

//this is not efficient.
template<typename T>
Maybe<T> parse_enum(std::vector<T> options,std::string value){
	for(unsigned i=0;i<options.size();i++){
		T opt=options[i];
		if(as_string(opt)==value) return Maybe<T>(opt);
	}
	return Maybe<T>();
}

std::vector<std::string> split(std::string,char);
std::vector<std::string> split(std::string);
double atof(std::string const&);
std::string inside_parens(std::string const&);

#endif
