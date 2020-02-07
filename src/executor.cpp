#ifndef EXECUTOR
#define EXECUTOR
#include"parser.cpp"
#include<map>
#include<string>
#include<iostream>
#include<list>
using namespace std;
/*
    我知道原本可以靠良好的设计规避这个文件里的重复无用又长的代码
    但是我懒得再改parser了
    放飞自我硬编码算了

    一堆因为parser没考虑周全多出来的if else switch
    我要吐了
*/
namespace Executor
{
    struct Var
    {
        union Value
        {
            int integer;
            float decmical;
            bool boolean;
        }value;
        void* array = nullptr;
        string text;
        AttrKind kind;
    };
    int inLoop = 0;
    bool exitLoop = false;
    bool jumpLoop = false;
    typedef map<string,Var*> VarClosure;
    list<VarClosure> closures;
    void setVar(string name,Var* var)
    {
        closures.front().insert(make_pair(name,var));
    }
    Var* getVar(string name)
    {
        for(auto iter=closures.begin();iter!=closures.end();iter++)
        {
            auto ret = iter->find(name);
            if(ret!=iter->end())
            {
                return ret->second;
            }
        }
        reportError(Error::UndefinedID);
    }
    void constructClosure()
    {
        closures.push_front(VarClosure());
    }
    void destructClosure()
    {
        for(auto pair:closures.front())
        {
            delete pair.second;
        }
        closures.pop_front();
    }
  
    #define GETDIGIT(node) (node->attrKind==AttrKind::Integer ? node->attr.integer : node->attr.decmical)
    bool checkDigit(treeNode* node)
    {
        return (node->attrKind==AttrKind::Integer||node->attrKind==AttrKind::Decmical);
    }
    treeNode* exp(treeNode* node)
    {
        treeNode* ret = nullptr;
        if(node->kind.expKind==ExpKind::constExp)return new treeNode(*node);
        if(node->kind.expKind==ExpKind::idExp)
        {
            auto retVar = getVar(node->text);
            switch(retVar->kind)
            {
                case AttrKind::Text:return new treeNode(retVar->text);
                case AttrKind::Integer:return new treeNode(retVar->value.integer);
                case AttrKind::Decmical:return new treeNode(retVar->value.decmical);
                case AttrKind::Boolean:return new treeNode(retVar->value.boolean);
            }
        }
        if(node->child[0]->kind.expKind==ExpKind::opExp)
        {
            ret = exp(node->child[1]);
            ret->attr.boolean=!ret->attr.boolean;
            return ret;
        }
        else 
        {
            auto left = exp(node->child[0]);
            string op = node->child[1]->text;
            auto right = exp(node->child[2]);
            if(op=="+")
            {
                if(left->attrKind==AttrKind::Text&&right->attrKind==AttrKind::Text)
                    ret = new treeNode(left->text + right->text);
                else if(checkDigit(left)&&checkDigit(right))
                    ret = new treeNode(GETDIGIT(left)+GETDIGIT(right));//string
                else
                    reportError(Error::errorType);
            } 
            else if(op=="-") ret = new treeNode(GETDIGIT(left)-GETDIGIT(right));
            else if(op=="*") ret = new treeNode(GETDIGIT(left)*GETDIGIT(right));
            else if(op=="/")
            {
                if(GETDIGIT(right)==0)
                    reportError(Error::divZero);
                else
                    ret = new treeNode(GETDIGIT(left)/GETDIGIT(right));
            }
            else if(op=="&&"||op=="||")
            {
                if(right->attrKind!=AttrKind::Boolean&&left->attrKind!=AttrKind::Boolean) 
                    reportError(Error::errorType);
                else if(op=="&&")ret = new treeNode(left->attr.boolean && right->attr.boolean);
                else if(op=="||")ret = new treeNode(left->attr.boolean || right->attr.boolean);
            }
            else
            {
                if(!(checkDigit(left)&&checkDigit(right)))reportError(Error::errorType);
                else if(op==">")ret = new treeNode(GETDIGIT(left)>GETDIGIT(right));
                else if(op=="<")ret = new treeNode(GETDIGIT(left)<GETDIGIT(right));
                else if(op=="==")ret = new treeNode(GETDIGIT(left)==GETDIGIT(right));
                else if(op=="!=")ret = new treeNode(GETDIGIT(left)!=GETDIGIT(right));
                else if(op=="<="||op=="=<")ret = new treeNode(GETDIGIT(left)<=GETDIGIT(right));
                else if(op==">="||op=="=>")ret = new treeNode(GETDIGIT(left)>=GETDIGIT(right));
            }
            delete left;
            delete right;
            return ret;
        }
    }

    void print(treeNode* node)
    {
        auto tmp = exp(node->child[0]);
        switch(tmp->attrKind)
        {
            case AttrKind::Text:
                cout<<tmp->text;
                break;
            case AttrKind::Boolean:
                cout<<(tmp->attr.boolean?"true":"false");
                break;
            case AttrKind::Integer:
                cout<<tmp->attr.integer;
                break;
            case AttrKind::Decmical:
                cout<<tmp->attr.decmical;
                break;
        }
        cout<<endl;
        delete tmp;
    }
    void stmtSequence(treeNode* tree);
    void ifStmt(treeNode* node)
    {
        auto condition = exp(node->child[0]);
        auto thenPart = node->child[1];
        auto elsePart = node->child[2];
        if(condition->attr.boolean==true)
        {
           if(thenPart!=nullptr)stmtSequence(thenPart);
        }
        else if(elsePart!=nullptr)
        {
            stmtSequence(elsePart);
        }
        delete condition;
    }
    void whileStmt(treeNode* node)
    {
        auto stmtClosure = node->child[1];
        auto condition = exp(node->child[0]);
        //varclosure=============================================================
        inLoop++;
        constructClosure();
        while(condition->attr.boolean)
        {
            if(stmtClosure!=nullptr)stmtSequence(stmtClosure);
            if(jumpLoop)jumpLoop=false;
            if(exitLoop)
            {
                exitLoop=false;
                break;
            }
            delete condition;
            condition = exp(node->child[0]);
        }
        destructClosure();
        inLoop--;
        delete condition;
    }
    void doStmt(treeNode* node)
    {
        auto stmtClosure = node->child[0];
        treeNode* condition = nullptr;
        inLoop++;
        constructClosure();
        do
        {
            if(stmtClosure!=nullptr)stmtSequence(stmtClosure);
            if(jumpLoop)jumpLoop=false;
            if(exitLoop)
            {
                exitLoop=false;
                break;
            }
            if(condition!=nullptr)delete condition;
            condition = exp(node->child[1]);
        }while(condition->attr.boolean);
        destructClosure();
        inLoop--;
        delete condition;
    }
    void forStmt(treeNode* node)
    {
        //auto declStmt = 
    }
    void _break()
    {
        if(inLoop==0)reportError(Error::notAllowedBreak);
        exitLoop=true;
    }
    void _continue()
    {
        if(inLoop==0)reportError(Error::notAllowedContinue);
        jumpLoop=true;
    }

    treeNode* idExp(treeNode* node)
    {
        auto left = node->child[0];
        auto name = node->child[1]->text;
        auto right = node->child[2];
        auto var = getVar(name);
        if(left!=nullptr)
        {
            if(var->kind==AttrKind::Integer)
            {
                var->value.integer++;
            }
            else if(var->kind==AttrKind::Decmical)
            {
                var->value.decmical++;
            }
            else reportError(Error::errorType);
            return new treeNode(ExpKind::idExp,name);
        }
        if(right!=nullptr)
        {
            auto tmp = new treeNode(ExpKind::idExp,name);
            if(var->kind==AttrKind::Integer)
            {
                var->value.integer++;
            }
            else if(var->kind==AttrKind::Decmical)
            {
                var->value.decmical++;
            }
            else reportError(Error::errorType);
            return tmp;
        }

        if(var->kind==AttrKind::Integer)       return new treeNode(var->value.integer);
        else if(var->kind==AttrKind::Decmical) return new treeNode(var->value.decmical);
        else if(var->kind==AttrKind::Boolean)  return new treeNode(var->value.boolean);
        else if(var->kind==AttrKind::Text)     return new treeNode(var->text);
    }

    void declareSequence(treeNode* node,string flag)
    {
        auto id = node->child[0];
        auto val = node->child[1];
        if(node->kind.expKind==ExpKind::idExp)
        {
            auto defalutVal = new Var();
            if(flag=="int")         {defalutVal->value.integer=0;defalutVal->kind=AttrKind::Integer;}
            else if(flag=="float")  {defalutVal->value.decmical=0;defalutVal->kind=AttrKind::Decmical;}
            else if(flag=="bool")   {defalutVal->value.boolean=false;defalutVal->kind=AttrKind::Boolean;}
            else if(flag=="string") {defalutVal->text="";defalutVal->kind=AttrKind::Text;}
            setVar(node->text,defalutVal);
        }
        else if(id->child[0]!=nullptr)//是数组！麻烦死了
        {
            // auto p1 = id->child[0];
            // auto p2 = val;
            
        }
        else
        {
            auto var = new Var();
            if(val->sibling!=nullptr)reportError(Error::errorType);
            val = exp(val);
            if(flag=="int")
            {
                if(val->attrKind!=AttrKind::Integer)reportError(Error::errorType);
                var->value.integer=val->attr.integer;
                var->kind=AttrKind::Integer;
            }
            else if(flag=="float")
            {
                if(val->attrKind!=AttrKind::Decmical)reportError(Error::errorType);
                var->value.decmical=val->attr.decmical;
                var->kind=AttrKind::Decmical;
            }
            else if(flag=="bool")
            {
                if(val->attrKind!=AttrKind::Boolean)reportError(Error::errorType);
                var->value.boolean=val->attr.boolean;
                var->kind=AttrKind::Boolean;
            }
            else if(flag=="string")
            {
                if(val->attrKind!=AttrKind::Text)reportError(Error::errorType);
                var->text=val->text;
                var->kind=AttrKind::Text;
            }
            setVar(id->text,var);
            delete val;
        }
        if(node->sibling!=nullptr)declareSequence(node->sibling,flag);
    }
    void declareStmt(treeNode* node)
    {
        declareSequence(node->child[1], node->child[0]->text);
    }

    void assignStmt(treeNode* node)
    {
        treeNode* left = idExp(node->child[0]);
        if(node->child[1]!=nullptr)
        {
            treeNode* right = exp(node->child[2]);
            string op = node->child[1]->text;
            if(node->child[0]->child[3]!=nullptr)reportError(Error::notAllowedSelfOp);
            auto var = getVar(left->text);
            if(var->kind!=right->attrKind)reportError(Error::errorType);
            if(var->kind==AttrKind::Boolean||right->attrKind==AttrKind::Boolean)reportError(Error::errorType);
            
            if(op=="=") 
            {
                switch(var->kind)
                {
                    case AttrKind::Integer:var->value.integer=right->attr.integer;break;
                    case AttrKind::Decmical:var->value.decmical=right->attr.decmical;break;
                    case AttrKind::Text:var->text=right->text;break;
                }
            }   
            else if(op=="+=") 
            {
                switch(var->kind)
                {
                    case AttrKind::Integer:var->value.integer+=right->attr.integer;break;
                    case AttrKind::Decmical:var->value.decmical+=right->attr.decmical;break;
                    case AttrKind::Text:var->text=var->text+right->text;break;
                }
            }
            else if(op=="-=")
            {
                switch(var->kind)
                {
                    case AttrKind::Integer:var->value.integer-=right->attr.integer;break;
                    case AttrKind::Decmical:var->value.decmical-=right->attr.decmical;break;
                    case AttrKind::Text:reportError(Error::notAllowOpInString);break;
                }
            }
            else if(op=="*=")
            {
                switch(var->kind)
                {
                    case AttrKind::Integer:var->value.integer*=right->attr.integer;break;
                    case AttrKind::Decmical:var->value.decmical*=right->attr.decmical;break;
                    case AttrKind::Text:reportError(Error::notAllowOpInString);break;
                }
            }
            else if(op=="/=")
            {
                switch(var->kind)
                {
                    case AttrKind::Integer:var->value.integer/=right->attr.integer;break;
                    case AttrKind::Decmical:var->value.decmical/=right->attr.decmical;break;
                    case AttrKind::Text:reportError(Error::notAllowOpInString);break;
                }
            }
        }
    }
    void read(treeNode* node)
    {
        auto id = node->child[0];
        if(id->kind.expKind!=ExpKind::idExp)reportError(Error::lackIdentifiter);
        auto var = getVar(id->text);
        if(var->kind==AttrKind::Text)
            getline(cin,var->text);
        else if(var->kind==AttrKind::Integer)
            cin>>var->value.integer;
        else if(var->kind==AttrKind::Boolean)
            cin>>var->value.boolean;
        else if(var->kind==AttrKind::Decmical)
            cin>>var->value.decmical;
       // cout<<2333333333;
    }
    void statement(treeNode* node)
    {
        switch(node->kind.stmtKind)
        {
            case StmtKind::ifStmt:ifStmt(node);break;
            case StmtKind::printStmt:print(node);break;
            case StmtKind::whileRpt:whileStmt(node);break;
            case StmtKind::doRpt:doStmt(node);break;
            case StmtKind::forRpt:forStmt(node);break;
            case StmtKind::continueStmt:_continue();break;
            case StmtKind::assignStmt:assignStmt(node);break;
            case StmtKind::declareStmt:declareStmt(node);break;
            case StmtKind::readStmt:read(node);break;
            case StmtKind::breakStmt:_break();break;
        }
    }
    void stmtSequence(treeNode* node)
    {
        statement(node);
        if(jumpLoop||exitLoop)return;
        if(node->sibling!=nullptr)stmtSequence(node->sibling);
    }
    void execute(treeNode* tree)
    {
        closures.push_front(VarClosure());
        stmtSequence(tree);
    }
}
#endif