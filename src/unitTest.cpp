#ifndef UNITTEST
#define UNITTEST
#include<fstream>
#include<iostream>
#include<queue>
#include<list>
#include<stack>
#include"scaner.cpp"
#include"parser.cpp"
#include"executor.cpp"

using namespace std;

void testGetToken(ifstream &file)
{
    string tokenTypeName[]={"Empty","Integer","Demical","String","Char","ID","Undefined","Comment","Eof"};
    while(true)
    {
        Token token=getToken(file);
        if(token.kind==Token::Eof)
            break;
        else
        {
            cout<<"Token:";
            cout.width(20);
            cout.fill(' ');
            cout.setf(ios::left);
            cout<<tokenTypeName[token.kind]<<"Name:'"<<token.text<<"'"<<endl;
        }
        
    }
}

class TREE:public PrintableTree
{
    treeNode* node=nullptr;
public:
    TREE(treeNode* node):node(node){};
    int getChildSize()override
    {
        return 3;
    }
    int getfieldCount()override
    {
        return 2;
    }
    std::string getfield(int index)override
    {
        string NOTEK[]={"stmt","exp"};
        string STMTK[]={"stmt","ifStmt","doRpt","whileRpt","forRpt","asgnStmt","declStmt"};
        string EXPK[]={"opExp","constExp","idExp","exp"};
        switch(index)
        {
            case 0:return (node->nodeKind==NodeKind::statement ? STMTK[(int)node->kind.stmtKind] : EXPK[node->kind.expKind]);
            case 1:return node->text;
        }
    }
    PrintableTree* getChild(int index)override
    {
        return node->child[index]==nullptr ? nullptr : new TREE(node->child[index]);
    }
};
//w:13*3+1 h:6
void outputTree(treeNode* tree,ostream &stream,string leftSpace="",bool isLast = false,int index=0,bool isTop = true,bool isSibling=false)
{
    string STMTK[]={"statement","ifStmt","doRpt","whileRpt","forRpt","asgnStmt","declStmt","continueStmt","readStmt","printStmt","breakStmt"};
    string EXPK[]={"opExp","constExp","idExp","exp"};
    if(tree!=nullptr)
    {
        
        if(!isTop)
        {
            if(isSibling)
            {
                stream<<leftSpace;
                leftSpace+=isLast?"   ":"↓  ";
            }
            else
            {
                stream<<leftSpace<<(isLast?"└──":"├──");//<<index<<" ";
                leftSpace+=isLast?"   ":"│  ";
            }
        }
        else
        {
            leftSpace+="  ";
        }
        stream.fill(' ');
        stream.width(9);
        stream.setf(ios::left);
        stream<<(tree->nodeKind==NodeKind::statement ? STMTK[(int)tree->kind.stmtKind] : EXPK[tree->kind.expKind]);
        stream<<tree->text<<endl;

        if(tree->child[0]!=nullptr||tree->child[1]!=nullptr||tree->child[2]!=nullptr)
        {
            outputTree(tree->child[0],stream,leftSpace,false,0,false);
            outputTree(tree->child[1],stream,leftSpace,false,1,false);
            outputTree(tree->child[2],stream,leftSpace,true,2,false);
        }
        else if(isLast)stream<<leftSpace<<endl;
        if(!isSibling)
        {
            auto p=tree;
            while(p->sibling!=nullptr)
            {
                stream<<leftSpace<<"↓  "<<endl;
                //stream<<leftSpace<<endl;
                outputTree(p->sibling,stream,leftSpace,p->sibling->sibling==nullptr,0,false,true);
                p=p->sibling;
            }

        }
        
    }
    else
    {
        stream<<leftSpace<<(isLast?"└┄┄":"├┄┄")<<endl;;
    }
    
}

// void testExp()
// {
//     file.open("test.txt");
//     treeNode* tree=exp();
//     outputTree(tree,cout);
//     file.close();
// }
// void testDeclareStmt()
// {
//     file.open("test.txt");
//     ofstream f("output.txt");
//     if(nextToken.kind==Token::Empty)nextToken = getToken(file);
//     treeNode* tree=declareStmt();
//     outputTree(tree,cout);
//     f.close();
//     file.close();
// }
void testParser()
{
    ifstream file("test.txt");
    treeNode* tree=parse(file);
    outputTree(tree,cout);
    file.close();
}
void testExecutor()
{
    ifstream file("test.txt");
    treeNode* tree=parse(file);
    //outputTree(tree,cout);
    //cout<<endl;
    Executor::execute(tree);
    file.close();
}
int main()
{
    // //输出到控制台
    // outputTree(cout);
    // //输出到文件
    // outputTree(outfile);
    // outfile.close();
    //testouputTree();
    // //testGetToken(f);
    // f.close();
    //testParser();
    testExecutor();
    //system("pause");
    // ofstream outfile("output.txt");
    

}
#endif