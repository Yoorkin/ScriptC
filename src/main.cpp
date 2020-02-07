#include<stdio.h>
#include<string.h>
#include<iostream>
#include"scaner.cpp"
#include"parser.cpp"
#include"executor.cpp"
using namespace std;
int main()
{
    ifstream file("test.txt");
    treeNode* tree=parse(file);
    Executor::execute(tree);
    file.close();
}