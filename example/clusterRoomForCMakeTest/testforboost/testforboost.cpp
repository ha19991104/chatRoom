#include <iostream>
#include <boost/timer.hpp>
#include <boost/bind.hpp>
#include <string>
using namespace std;

class Hello
{
public:
	void say(string name) 
	{ cout << name << " say: hello world!" << endl; }
};
int main()
{
    /*
    g++ -o testforboost testforboost.cpp -std=c++11 -g
    */
	boost::timer t;
	cout << "max timespan:"<<t.elapsed_max()/3600<<"h"<<endl;
 
	cout << "min tmiespan:"<<t.elapsed_min()<<"s"<<endl;
 
	cout<<"now time elapsed:"<<t.elapsed()<<"s"<<endl;
	Hello h;
	auto func = boost::bind(&Hello::say, &h, "zhang san");
	func();
	return 0;
}
