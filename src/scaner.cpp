#ifndef SCANER
#define SCANER
#include<iostream>
#include<fstream>
#include<string>
#include<map>
using namespace std;

struct LineNo
{
    int row=0;
    int col=0;
};

struct Token
{
    enum Enum{Empty,Integer,Demical,String,Char,ID,Sign,Undefined,Comment,Eof}kind=Empty;
    string text="";
    LineNo lineNo;
};

char c;
int row=0,col=0;
bool reserve = false;
bool abandon = true;    //设为true,使ret.text不记录未初始化的c

inline bool isEnter()       {return c=='\n';}
inline bool isSpace()       {return c==' ';}
inline bool isLetter()      {return (c>='a'&&c<='z')||(c>='A'&&c<='Z');}
inline bool isReal()        {return c>='0'&&c<='9';}
inline bool isSingleSign()  {return c=='{'||c=='}'||c=='('||c==')'||c=='['||c==']'||c==';'||c==',';}

Token getToken(istream &file)
{
    enum {start,inCommentSign,inCommentBlock,inCommentLine,exitCommentBlock,
            inCompare,inID,inInt,inDec,inSmaller,inGreater,inNot,inAdd,inSub,
            inAnd,inOr,inString,inChar,exitChar,inMul,done}state = start;
    Token ret;
    while(true)
    {
        if(reserve)
            reserve=false;
        else
        {
            if(abandon)
                abandon=false;
            else 
            {
                ret.text+=c;
            }
            col++;

            if(!file.get(c)&&ret.kind==Token::Empty)
            {
                bool abandon = true;
                char c='\000';
                return Token{Token::Eof,""};
            }
        }

        //done
        if(ret.kind!=Token::Empty)
        {
            reserve=true;
            if(ret.kind==Token::Comment)return getToken(file);//是注释就丢掉
            ret.lineNo.row=row;
            return ret;
        }

        switch(state)
        {
            case start: 
                if(isEnter())           {state=start;abandon=true;row++;col=0;}
                else if(isSpace())      {state=start;abandon=true;}
                else if(isReal())       state=inInt;
                else if(isLetter())     state=inID;
                else if(c=='*')         state=inMul;
                else if(c=='/')         state=inCommentSign;
                else if(c=='=')         state=inCompare;
                else if(c=='>')         state=inGreater;
                else if(c=='<')         state=inSmaller;
                else if(c=='!')         state=inNot;
                else if(c=='+')         state=inAdd;
                else if(c=='-')         state=inSub;
                else if(c=='&')         state=inAnd;
                else if(c=='|')         state=inOr;
                else if(c=='"')         {state=inString;abandon=true;}
                else if(c=='\'')        state=inChar;
                else if(isSingleSign()) ret.kind=Token::Sign;
                else                    ret.kind=Token::Undefined;
                ret.lineNo.col=col;
                break;
            case inCommentSign:
                if(c=='*')              state=inCommentBlock;
                else if(c=='/')         state=inCommentLine;
                else if(c=='=')         state=inInt;
                else                    {ret.kind=Token::Sign;reserve=true;}
                break;
            case inCommentBlock:
                if(c=='*')              state=exitCommentBlock;
                //else                    state=inCommentBlock;
                break;
            case inCommentLine:
                if(isEnter())           {ret.kind=Token::Comment;reserve=true;}
                //else                    state=inCommentLine;
                break;
            case exitCommentBlock:
                if(c=='/')              ret.kind=Token::Comment;
                else if(c=='*')         state=exitCommentBlock;
                else                    state=inCommentBlock;
                break;
            case inCompare:
                if(c=='='||c=='>'||c=='<')ret.kind=Token::Sign;
                else                    {ret.kind=Token::Sign;reserve=true;}
                break;
            case inID:
                if(isReal()||isLetter())state=inID;
                else                    {ret.kind=Token::ID;reserve=true;}
                break;
            case inInt:
                if(isReal())            state=inInt;
                else if(c=='.')         state=inDec;
                else                    {ret.kind=Token::Integer;reserve=true;}
                break;
            case inDec:
                if(isReal())            state=inDec;
                else                    {ret.kind=Token::Demical;reserve=true;}
                break;
            case inSmaller:
                if(c=='=')              ret.kind=Token::Sign;
                else                    {ret.kind=Token::Sign;reserve=true;}
                break;
            case inGreater:
                if(c=='=')              ret.kind=Token::Sign;
                else                    {ret.kind=Token::Sign;reserve=true;}
                break;
            case inNot:
                if(c=='=')              ret.kind=Token::Sign;
                else                    {ret.kind=Token::Sign;reserve=true;}
                break;
            case inAdd:
                if(c=='=')              ret.kind=Token::Sign;
                else if(c=='+')         ret.kind=Token::Sign;
                else                    {ret.kind=Token::Sign;reserve=true;}
                break;
            case inOr:
                if(c=='|')              ret.kind=Token::Sign;
                else                    {ret.kind=Token::Undefined;reserve=true;}
                break;
            case inString:
                if(c=='"')              {ret.kind=Token::String;abandon=true;}
                else                    state=inString;
                break;
            case inChar:
                if(isEnter())           {ret.kind=Token::Undefined;reserve=true;} 
                else                    state=exitChar;
                break;
            case exitChar:
                if(isEnter())           {ret.kind=Token::Undefined;;reserve=true;}
                else if(c=='\'')        {ret.kind=Token::Char;}
            case inMul: 
                if(c=='=')              {ret.kind=Token::Sign;}
                else                    {ret.kind=Token::Sign;reserve=true;}           
                break;
            case inAnd:
                if(c=='&')              ret.kind=Token::Sign;
                else                    {ret.kind=Token::Undefined;reserve=true;}
                break;
            case inSub:
                if(c=='-')              ret.kind=Token::Sign;
                else                    {ret.kind=Token::Sign;reserve=true;}
                break;
        }
    }
}

#endif