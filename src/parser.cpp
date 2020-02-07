#ifndef PARSER
#define PARSER
#include<string>
#include"scaner.cpp"
using namespace std;
enum class NodeKind:int {statement,expression};
enum class StmtKind:int {Unknown,ifStmt,doRpt,whileRpt,forRpt,assignStmt,declareStmt,continueStmt,readStmt,printStmt,breakStmt};
enum ExpKind  {opExp,constExp,idExp,Unknown};
enum AttrKind {Integer,Decmical,Boolean,Text};
class treeNode
{
public:
    treeNode()=default;
    treeNode(NodeKind kind)
    {
        nodeKind=kind;
        if(kind==NodeKind::expression)
            this->kind.expKind=ExpKind::Unknown;
        else
            this->kind.stmtKind=StmtKind::Unknown;
    }
    treeNode(StmtKind stmt,string text)
    {
        nodeKind=NodeKind::statement;
        kind.stmtKind=stmt;
        this->text=text;
    }
    treeNode(ExpKind exp,string text)
    {
        kind.expKind=exp;
        nodeKind=NodeKind::expression;
        this->text=text;
    }
    explicit treeNode(int integer)
    {
        attr.integer=integer;
        attrKind=AttrKind::Integer;
        nodeKind=NodeKind::expression;
        kind.expKind=ExpKind::constExp;
        text=to_string(integer);
    }
    explicit treeNode(float decmical)
    {
        attr.decmical=decmical;
        attrKind=AttrKind::Decmical;
        nodeKind=NodeKind::expression;
        kind.expKind=ExpKind::constExp;
        text=to_string(decmical);
    }
    explicit treeNode(bool boolean)
    {
        attr.boolean=boolean;
        attrKind=AttrKind::Boolean;
        nodeKind=NodeKind::expression;
        kind.expKind=ExpKind::constExp;
        text=boolean?"true":"false";
    }
    treeNode(string text)
    {
        this->text=text;
        attrKind=AttrKind::Text;
        nodeKind=NodeKind::expression;
        kind.expKind=ExpKind::constExp;
    }
    LineNo lineNo;
    
    NodeKind nodeKind=NodeKind::statement;
    union Kind
    {
        Kind(){}
        StmtKind stmtKind=StmtKind::Unknown;
        ExpKind  expKind;
    }kind;

    treeNode* child[3]={nullptr,nullptr,nullptr};
    treeNode* sibling = nullptr;

    string text;
    AttrKind attrKind=AttrKind::Text;
    union Attr
    {
        Attr(){}
        ~Attr(){}
        int integer;
        float decmical;
        bool boolean;
    }attr;
};

Token nextToken,preToken;
istream *file;

enum class Error{lackRightBucket,notAllowedContinue,notAllowOpInString,notAllowedSelfOp,notAllowedBreak,unknownKey,errorBool,divZero,errorType,UndefinedID,illegalKey,lackLeftBucket,lackSem,lackWhile,lackExp,unknownKind,lackIdentifiter,Empty,lackConst};
map<Error,string> errorTable={
    {Error::lackRightBucket,"缺少右括号'}'、')'、']'或者分号';'"},
    {Error::lackExp,"缺少表达式"},
    {Error::unknownKind,"未知的数据类型"},
    {Error::unknownKey,"未知的关键字"},
    {Error::lackIdentifiter,"缺少标识符、表达式或']'"},
    {Error::lackConst,"缺少或未知的常量"},
    {Error::lackWhile,"此处应有关键字while"},
    {Error::divZero,"除数为零"},
    {Error::illegalKey,"未定义的关键字或缺少赋值表达式"},
    {Error::lackSem,"缺少分号"},
    {Error::lackLeftBucket,"缺少左括号'{'、'('或'['"},
    {Error::UndefinedID,"未定义的变量"},
    {Error::errorType,"数据类型错误或不匹配"},
    {Error::errorBool,"布尔类型不可运算"},
    {Error::notAllowedContinue,"不在循环里continue个p"},
    {Error::notAllowedBreak,"不在循环和switch里break个p"},
    {Error::notAllowedSelfOp,"赋值语句中左值不可使用后缀形式的自增运算"},
    {Error::notAllowOpInString,"我想你该好好思考  这个操作对字符串到底是什么意思"}
};

void reportError(Error error)
{
    cout<<endl<<"行"<<nextToken.lineNo.row+1<<" 列"<<nextToken.lineNo.col<<" 出现错误:"<<errorTable[error]<<endl<<endl;
    string line;
    file->clear();
    file->seekg(0,ios::beg);
    for(int i=nextToken.lineNo.row;i>0;i--)
        getline(*file,line);
    cout<<line<<endl;
    getline(*file,line);
    cout<<line<<endl;
    cout.fill(' ');
    cout.width(nextToken.lineNo.col-1);
    cout<<"~~~~~~^"<<endl;
    system("pause");
    exit(0);
}

bool match(string expected,Error errorIfNotMatch=Error::Empty)
{
    // if(nextToken.kind==Token::Empty)nextToken = getToken(file);
    if(nextToken.kind==Token::Undefined)
        reportError(Error::unknownKey);
    if(nextToken.text==expected)
    {
        preToken = nextToken;
        nextToken = getToken(*file);
        return true;
    }
    else 
    {
        if(errorIfNotMatch!=Error::Empty)reportError(errorIfNotMatch);
        return false;
    }
}

bool match(Token::Enum expected,Error errorIfNotMatch=Error::Empty)
{
    if(nextToken.kind==Token::Undefined)reportError(Error::unknownKey);
    if(nextToken.kind==expected)
    {
        preToken = nextToken;
        nextToken = getToken(*file);
        return true;
    }
    else 
    {
        if(errorIfNotMatch!=Error::Empty)reportError(errorIfNotMatch);
        return false;
    }
}


treeNode* identifider();
treeNode* exp();
treeNode* ifStmt();
treeNode* statementClosure();
treeNode* stmtSequence();
treeNode* statement();

treeNode* ID()
{
    match(Token::ID,Error::lackIdentifiter);
    return new treeNode(ExpKind::idExp,preToken.text);
}

treeNode* constVal()
{
    if(match(Token::Integer))
        return new treeNode(atoi(preToken.text.c_str()));
    else if(match(Token::Demical))
        return new treeNode((float)atof(preToken.text.c_str()));
    else if(match("true")||match("false"))
        return new treeNode(preToken.text=="true"?true:false);
    else if(match(Token::String))
        return new treeNode(preToken.text);
    else
        reportError(Error::lackConst);
    return nullptr;
}

treeNode* identifiderExp()
{
    treeNode* ret=new treeNode(NodeKind::expression);
    if(match("++")||match("--"))
    {
        ret->child[0]=new treeNode(ExpKind::opExp,preToken.text);
    }
    ret->child[1]=identifider();
    if(match("++")||match("--"))
    {
        ret->child[2]=new treeNode(ExpKind::opExp,preToken.text);
    }

    if(ret->child[0]==nullptr&&ret->child[2]==nullptr)
    {
        auto tmp = ret->child[1];
        delete ret;
        return tmp;
    }
    else
        return ret;
}

treeNode* factor()
{
    if(match("("))
    {
        treeNode* ret=exp();
        match(")",Error::lackRightBucket);
        return ret;
    }
    else if(nextToken.text=="false"||nextToken.text=="true"||nextToken.kind==Token::Integer||nextToken.kind==Token::String||nextToken.kind==Token::Char||nextToken.kind==Token::Demical)
    {
        return constVal();
    }
    else
    {
        return identifiderExp();
    }
}

treeNode* term()
{
    auto pfactor=factor();
    if(match("*")||match("/"))
    {
        treeNode* ret = new treeNode(NodeKind::expression);
        ret->child[0]=pfactor;
        ret->child[1]=new treeNode(ExpKind::opExp,preToken.text);
        ret->child[2]=term();
        return ret;
    }
    else
    {
        return pfactor;
    }
}

treeNode* simpleExp()
{
    auto pterm=term();
    if(match("+")||match("-"))
    {
        treeNode* ret=new treeNode(NodeKind::expression);
        ret->child[0]=pterm;
        ret->child[1]=new treeNode(ExpKind::opExp,preToken.text);
        ret->child[2]=simpleExp();
        return ret;
    }
    else
    {
        return pterm;
    }
}

treeNode* comparisonExp()
{
    auto psimpleExp=simpleExp();
    if(match(">")||match("<")||match("==")||match(">=")||match("<=")||match("=>")||match("=<")||match("!="))
    {
        treeNode* ret=new treeNode(NodeKind::expression);
        ret->child[0]=psimpleExp;
        ret->child[1]=new treeNode(ExpKind::opExp,preToken.text);
        ret->child[2]=comparisonExp();
        return ret;
    }
    else
    {
        return psimpleExp;
    }
}

treeNode* logicExp()
{
    auto pcomExp=comparisonExp();
    if(match("&&")||match("||"))
    {
        treeNode* ret=new treeNode(NodeKind::expression);
        ret->child[0]=pcomExp;
        ret->child[1]=new treeNode(ExpKind::opExp,preToken.text);
        ret->child[2]=logicExp();
        return ret;
    }
    else
    {
        return pcomExp;
    }
}

treeNode* exp()
{
    if(match("!"))
    {
        treeNode* ret=new treeNode(NodeKind::expression);
        ret->child[0]=new treeNode(ExpKind::opExp,"!");
        ret->child[1]=exp();
        return ret;
    }    
    else
        return logicExp();
    
}

treeNode* identifider()
{
    treeNode* id = ID();
    if(match("["))
    {
        if(nextToken.text!="]")
            id->child[0]=exp();
        else
            id->child[0]=new treeNode(-1);//数组大小根据后接的表达式推测
        match("]",Error::lackRightBucket);
        auto p=id->child[0];
        while(match("["))
        {
            p->sibling=(nextToken.text=="]"?new treeNode(-1):exp());
            p=p->sibling;
            match("]",Error::lackRightBucket);
        }
    }
    return id;
}

treeNode* valType()
{
    if(!(match("int")||match("float")||match("bool")||match("byte")||match("string")||match("char")))
        reportError(Error::unknownKind);
    return new treeNode(StmtKind::declareStmt,preToken.text);
}

treeNode* arrayClosure()
{
    if(match("{"))
    {
        auto ret = new treeNode(StmtKind::declareStmt,"sequence");
        ret->child[0]=arrayClosure();
        auto p=ret->child[0];
        while(match(","))
        {
            p->sibling=arrayClosure();
            p=p->sibling;
        }
        match("}",Error::lackRightBucket);
        return ret;
    }
    else
    {
        return exp();
    }
}
treeNode* declare()
{
    auto iden = identifider();
    if(match("="))
    {
        auto ret = new treeNode(StmtKind::declareStmt,"sequence");
        ret->child[0]=iden;
        ret->child[1]=arrayClosure();
        return ret;
    }
    else
        return iden;
}
treeNode* declareSequence()
{
    auto ret = declare();
    auto p=ret;
    while(match(","))
    {
        p->sibling=declare();
        p=p->sibling;
    }
    return ret;
}
treeNode* declareStmt()
{
    auto ret = new treeNode(StmtKind::declareStmt,"");
    ret->child[0]=valType();
    ret->child[1]=declareSequence();
    return ret;
}
treeNode* assignStmt()
{
    auto ret = new treeNode(StmtKind::assignStmt,"");
    ret->child[0]=identifiderExp();
    if(match("=")||match("+=")||match("-=")||match("*=")||match("/="))
    {
        ret->child[1]=new treeNode(ExpKind::opExp,preToken.text);
        ret->child[2]=exp();
    }
    return ret;
}
treeNode* doStmt()
{
    auto ret = new treeNode(StmtKind::doRpt,"");
    match("do");
    ret->child[0]=statementClosure();
    match("while",Error::lackWhile);
    match("(");
    ret->child[1]=exp();
    match(")",Error::lackRightBucket);
    return ret;
}
treeNode* whileStmt()
{
    auto ret = new treeNode(StmtKind::whileRpt,"");
    match("while");
    match("(");
    ret->child[0]=exp();
    match(")",Error::lackRightBucket);
    ret->child[1]=statementClosure();
    return ret;
}
treeNode* forStmt()
{
    auto ret = new treeNode(StmtKind::forRpt,"");
    auto forExp = new treeNode(StmtKind::forRpt,"forExp");
    ret->child[0]=forExp;
    match("for");
    match("(");
    forExp->child[0]=statement();
    match(";",Error::lackSem);
    forExp->child[1]=(nextToken.text==";"?nullptr:exp());
    match(";",Error::lackSem);
    forExp->child[2]=statement();
    match(")",Error::lackRightBucket);
    ret->child[1]=statementClosure();
    return ret;
}
treeNode* ifStmt()
{
    auto ret = new treeNode(StmtKind::ifStmt,"");
    match("if");
    match("(");
    ret->child[0]=exp();
    match(")",Error::lackRightBucket);
    ret->child[1]=statementClosure();
    if(match("else"))
        ret->child[2]=statementClosure();
    return ret;
}
treeNode* statementClosure()
{
    if(match("{"))
    {
        treeNode* ret = nullptr;
        if(nextToken.text!="}")ret = stmtSequence();
        match("}",Error::lackRightBucket);
        return ret;
    }
    else
    {
        auto ret = statement();
        switch(ret->kind.stmtKind)
        {
            case StmtKind::assignStmt:
            case StmtKind::breakStmt:
            case StmtKind::continueStmt:
            case StmtKind::doRpt:
            case StmtKind::printStmt:
            case StmtKind::readStmt:
            case StmtKind::declareStmt:
                match(";",Error::lackSem);
                break;
        }
        return ret;
    }
}
treeNode* readStmt()
{
    auto ret = new treeNode(StmtKind::readStmt,"");
    match("read");
    ret->child[0]=identifider();
    return ret;
}
treeNode* printStmt()
{
    auto ret = new treeNode(StmtKind::printStmt,"");
    match("print");
    ret->child[0]=exp();
    return ret;
}
treeNode* statement()
{
    auto flag = nextToken.text;
    treeNode* ret=nullptr;
    if(flag=="if")
        return ifStmt();
    else if(flag=="read")
        return readStmt();
    else if(flag=="print")
        return printStmt();
    else if(match("continue"))
        return new treeNode(StmtKind::continueStmt,"");
    else if(match("break"))
        return new treeNode(StmtKind::breakStmt,"");
    else if(flag=="do")
        ret = doStmt();
    else if(flag=="while")
        return whileStmt();
    else if(flag=="for")
        return forStmt();
    else if(flag==";")
        return nullptr;
    else if(flag=="int"||flag=="float"||flag=="char"||flag=="string"||flag=="bool"||flag=="byte")
        ret = declareStmt();
    else if(nextToken.kind==Token::ID)
        ret = assignStmt();
    else 
        return nullptr;

    
    //match(";",Error::lackSem);
    return ret;
}
treeNode* stmtSequence()
{
    treeNode* ret=nullptr;
    treeNode* p=nullptr;
    treeNode* pre=nullptr;
    while(nextToken.kind!=Token::Eof&&nextToken.text!="}")
    {
        p=statement();
        if(p!=nullptr)
        {
            if(pre!=nullptr)pre->sibling=p;else ret=p;
            pre=p;
            switch(pre->kind.stmtKind)
            {
                case StmtKind::assignStmt:
                case StmtKind::breakStmt:
                case StmtKind::continueStmt:
                case StmtKind::doRpt:
                case StmtKind::printStmt:
                case StmtKind::readStmt:
                case StmtKind::declareStmt:
                    match(";",Error::lackSem);
                    break;
            }
        }
    }
    //match(";",Error::lackSem);
    return ret;
}
treeNode* parse(istream &stream)
{
    file=&stream;
    nextToken = getToken(*file);
    return stmtSequence();
}


#endif