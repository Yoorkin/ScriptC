#include<stdio.h>
#include<string.h>
#include<iostream>
#include"scaner.cpp"
#include"parser.cpp"
#include"executor.cpp"
using namespace std;
int main()
{
    cout<<"Please input the script path:"<<endl;
    string path;
    cin>>path;
    ifstream file(path);
    treeNode* tree=parse(file);
    Executor::execute(tree);
    file.close();
}