// PL 10727129
# include <iostream>
# include <cstdio>
# include <cstdlib>
# include <string>
# include <stdio.h>
# include <stdlib.h>
# include <vector>
# include <iomanip>
# include <cctype>
# include <cstring>
# include <map>
# include <sstream>
# include <exception>

using namespace std;

# define CmdNum 38

static int uTestNum = 0;
int gLine = 1;
int gColumn = 0;
bool gExit = false;
bool gSExpMode = false;
bool gPrintErrorSExp = false;

enum TokenType {
  LEFT_PAREN, RIGHT_PAREN, INT, STRING, DOT, FLOAT, NIL, T, QUOTE, SYMBOL, NONE,             
  SHARP, OPERATOR
};
enum ExceptionType {
  NO_MORE_INPUT, NO_CLOSING_QUOTE, NO_LEFT_PAREN, NO_RIGHT_PAREN,
  APPLY_NON_FUNC, WRONG_ARG_NUM, UNBOUND, WRONG_ARG_TYPE, ONLYINTERELCMD, NO_RETURN_VAL,
  CLEAN_TABLE, ERROR_FORMAT, EXIT, DIVED_ZERO
};

string gInternelCommand[ CmdNum ] = {
    "cons", "list", "quote", "Define", "car", "cdr", "atom?", "pair?", "list?", "null?", "integer?",
    "real?", "number?", "string?", "boolean?", "symbol?", "+", "-", "*", "/", "not", "and", "or",
    ">", ">=", "<", "<=", "=", "string-append", "string>?", "string<?", "string=?",
    "eqv?", "equal?", "begin", "if", "cond", "clean-environment"};

class Token;
typedef Token* TokenPtr ;
class Token {

public :

  TokenType mType;
  string mContent;
  bool mQuoted;

  TokenPtr mLeft;
  TokenPtr mRight;
  TokenPtr mParent;

  Token() {
    mType = NONE;
    mQuoted = false;
    mContent = "";
    mLeft = NULL;
    mRight = NULL;
    mParent = NULL;
  } // Token()

  Token( TokenType type, string content ) {
    mType = type;
    mContent = content;
    mLeft = NULL;
    mRight = NULL;
    mParent = NULL;
  } // Token()

};

vector<TokenPtr> gUserDefineTable;

class Exception {

public :

  string mErrorString = "";
  TokenPtr mErrorPtr = NULL;

  bool mPrint = false;

  Exception() {

    mErrorString = "Nope";
    mErrorPtr = NULL;
    bool mPrint = false;

  } // Exception()

  Exception( ExceptionType type ) {
    if ( type == NO_MORE_INPUT ) {

      mErrorString = "ERROR (no more input) : END-OF-FILE encountered" ;

    } // if

    if ( type == NO_CLOSING_QUOTE ) {

      stringstream temp;
      temp << "ERROR (no closing quote) : END-OF-LINE encountered at Line "
            << gLine 
            << " Column "
            << gColumn ;
      mErrorString = temp.str();

    } // if

    if ( type == CLEAN_TABLE ) {

      mErrorString = "environment cleaned\n";

    } // if

    if ( type == EXIT ) {
      ;
    } // if

    if ( type == DIVED_ZERO ) {

      mErrorString = "ERROR (division by zero) : /\n";

    } // if

  } // Exception()

  Exception( TokenPtr thisToken, ExceptionType type ) {

    if ( type == NO_LEFT_PAREN ) {
      stringstream temp;
      temp << "ERROR (unexpected token) : atom or '(' expected when token at Line "
            << gLine
            << " Column "
            << gColumn - thisToken->mContent.length() + 1
            << " is >>"
            << thisToken->mContent
            << "<<";
      mErrorString = temp.str();

    } // if

    if ( type == NO_RIGHT_PAREN ) {
      stringstream temp;
      temp << "ERROR (unexpected token) : ')' expected when token at Line "
            << gLine
            << " Column "
            << gColumn - thisToken->mContent.length() + 1
            << " is >>"
            << thisToken->mContent
            << "<<";
      mErrorString = temp.str();

    } // if

    if ( type == WRONG_ARG_NUM ) {

      stringstream temp;

      temp << "ERROR (incorrect number of arguments) : "
           << thisToken->mLeft->mContent;

      mErrorString = temp.str();

    } // if

    if ( type == UNBOUND ) {

      stringstream temp;
      temp << "ERROR (unbound symbol) : ";

      mErrorString = temp.str();
      mErrorPtr = thisToken;

      mPrint = true;
      
    } // if

    if ( type == ONLYINTERELCMD ) {

      stringstream temp;
      temp << "#<procedure "
           << thisToken->mContent
           << ">\n";

      mErrorString = temp.str();

    } // if

    if ( type == NO_RETURN_VAL ) {

      stringstream temp;
      temp << "ERROR ( no return value) : ";

      mErrorString = temp.str();
      mErrorPtr = thisToken;

      mPrint = true;
      
    } // if

    if ( type == APPLY_NON_FUNC ) {

      stringstream temp;
      temp << "ERROR (attempt to apply non-function) : ";
      mErrorString = temp.str();
      mErrorPtr = thisToken;

      mPrint = true;

    } // if

  } // Exception()

  Exception( TokenPtr thisToken, string NowCmd, ExceptionType type, bool IsInternalCmd ) {

    if ( type == WRONG_ARG_TYPE ) {

      stringstream temp;
      temp << "ERROR ( "
            << NowCmd
            << " with incorrect argument type ) : ";
      if ( IsInternalCmd ) 
        temp << "# <procedure ";

      temp << thisToken->mContent << ">\n";

      mErrorString = temp.str();

    } // if
  } // Exception()

  Exception( TokenPtr ErrorPtr, string NowCmd, ExceptionType type ) {

    if ( type == WRONG_ARG_TYPE ) {

      stringstream temp;
      temp << "ERROR ("
           << NowCmd
           << " with incorrect argument type) : ";

      mErrorString = temp.str();
      mErrorPtr = ErrorPtr;

      mPrint = true;

    } // if

    if ( type == ERROR_FORMAT ) {

      stringstream temp;
      temp << "ERROR ("
           << NowCmd
           << " format) : ";

      mErrorString = temp.str();
      mErrorPtr = ErrorPtr;

      mPrint = true;

    } // if  

  } // Exception()

};

class Scanner;
class Scanner {

public:
  bool mOutputed ;
  bool mInReading[10];
  bool mNeedRP;
  char mCh;

  Scanner() {
    for ( int i = 0 ; i < 10 ; i ++ ) {
      mInReading[i] = false;
    } // for

    mCh = '\0';
    mOutputed = true;
    mNeedRP = false;
    gLine = 1;
    gColumn = 0;

  } // Scanner()

  void Initial() {
    for ( int i = 0 ; i < 10 ; i ++ ) {
      mInReading[i] = false;
    } // for

    mCh = '\0';
    mNeedRP = false;
  } // Initial()

  char Peekone() {
    char ch = getchar();
    cin.putback( ch );
    return ch;
  } // Peekone()

  char GetChar() {
    mCh = getchar();

    if ( mCh == EOF ) {
      return EOF;
    } // if

    gColumn++;
    return mCh;
  } // GetChar()

  void PutBack( char ch ) {
    cin.putback( ch );
    gColumn--;
  } // PutBack()

};
Scanner gScanner;

TokenPtr ReadToken() {
  bool stop = false;
  char nowChar = '\0';

  string buffer;
  Token * thisToken = new Token();
  
  while ( !stop ) {
    nowChar = gScanner.GetChar();

    // cout << nowChar << " - Line : " << gLine;

    if ( thisToken->mType == NONE ) {
      if ( nowChar == '(' || nowChar == ')' ) {
        if ( nowChar == '(' ) {
          thisToken->mType = LEFT_PAREN;
        } // if
        else { 
          thisToken->mType = RIGHT_PAREN;

          if ( gSExpMode ) {
            if ( !gScanner.mInReading[ DOT ] && !gScanner.mInReading[ QUOTE ] ) {
              ;
            } // if
            else {
              thisToken->mContent = ")";
              throw new Exception( thisToken, NO_LEFT_PAREN ); // ' ) : ERROR
            } // else
          } // if
          else { // 如果不是gSExpMode 就不能單獨存在
            thisToken->mContent = ")";
            throw new Exception( thisToken, NO_LEFT_PAREN ); // ' ) : ERROR
          } // else
        } // else

        stop = true;
      } // if
      else if ( nowChar == 't' ) {
        thisToken->mType = T;
      } // else if
      else if ( nowChar == '.' ) {  // ' . 的exception還沒處理
        thisToken->mType = DOT;
      } // else if
      else if ( nowChar == '\'' ) {
        thisToken->mType = QUOTE;
        thisToken->mContent = "quote";
        stop = true;
      } // else if
      else if ( nowChar == '+' || nowChar == '-' ) {
        thisToken->mType = OPERATOR;
      } // else if
      else if ( nowChar == '#' ) {
        thisToken->mType = SHARP;
      } // else if
      else if ( nowChar == '\"' ) {
        thisToken->mType = STRING;
      } // else if
      else if ( isdigit( nowChar ) ) {
        thisToken->mType = INT;
      } // else if
      else if ( nowChar == '\n' ) {
        // cout << gLine << "enter!" << endl;
        if ( gScanner.mOutputed ) { // 避免上一個SExp與其同一行的\n算做是下一個SExp的第一行
          gLine = 1 ;
          gScanner.mOutputed = false;
        } // if
        else
          gLine++;
        gColumn = 0;
      } // else if
      else if ( nowChar == ' ' ) {
        ; // Do nothing
      } // else if
      else if ( nowChar == ';' ) {
        bool run = true;
        do {
          char ch = gScanner.GetChar();
          if ( ch == '\n' ) {
            run = false;
          } // if

          if ( ch == EOF ) {
            stop = true;
            gExit = true;

            throw new Exception( NO_MORE_INPUT );
          } // if
        } while ( run ) ;

        cin.putback( '\n' );
      } // else if
      else if ( nowChar == EOF ) {
        stop = true;
        gExit = true;
        throw new Exception( NO_MORE_INPUT );
      } // else if 
      else if ( nowChar == '\r' ) {
        cout << "gecha";
      } // else if
      else {
        thisToken->mType = SYMBOL;
      } // else

      if ( !isspace( nowChar ) && nowChar != ';' )  // 存每個char的content
        thisToken->mContent += nowChar;
    
    } // if ( thisToken->mType == NONE )
    else if ( thisToken->mType == SHARP ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {
        gScanner.PutBack( nowChar );
        thisToken->mType = SYMBOL;
        stop = true;
      } // if
      else {
        if ( nowChar == 't' ) {
          thisToken->mType = T;
        } // if
        else if ( nowChar == 'f' ) {
          thisToken->mType = NIL;
        } // else if
        else
          thisToken->mType = SYMBOL;
        thisToken->mContent += nowChar;
      } // else 
    } // else if ( thisToken->mType == SHARP )
    else if ( thisToken->mType == OPERATOR ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {
        gScanner.PutBack( nowChar );
        thisToken->mType = SYMBOL;
        stop = true;
      } // if
      else {
        if ( nowChar == '.' ) {
          thisToken->mType = FLOAT;
        } // if
        else if ( isdigit( nowChar ) ) {
          thisToken->mType = INT;
        } // else if
        else {
          thisToken->mType = SYMBOL;
        } // else

        thisToken->mContent += nowChar;
      } // else 
    } // else if ( thisToken->mType == OPERATOR )
    else if ( thisToken->mType == NIL ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {
        gScanner.PutBack( nowChar );
        thisToken->mContent = "nil";
        stop = true;
      } // if
      else {
        thisToken->mType = SYMBOL;
        thisToken->mContent += nowChar;
      } // else
    } // else if ( thisToken->mType == NIL )
    else if ( thisToken->mType == DOT ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {

        thisToken->mType = DOT;
        gScanner.PutBack( nowChar );
        stop = true;

        if ( gSExpMode ) {
          if ( !gScanner.mInReading [ LEFT_PAREN ] && !gScanner.mInReading[ DOT ] && 
               !gScanner.mInReading[ QUOTE ] ) {
            ;
          } // if
          else {
            throw new Exception( thisToken, NO_LEFT_PAREN );
          } // else
        } // if
        else {
          throw new Exception( thisToken, NO_LEFT_PAREN );
        } // else
      } // if
      else {
        if ( isdigit( nowChar ) ) {
          thisToken->mType = FLOAT;
        } // if
        else
          thisToken->mType = SYMBOL;
        thisToken->mContent += nowChar;
      } // else
    } // else if ( thisToken->mType == DOT )
    else if ( thisToken->mType == INT ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {
             
        gScanner.PutBack( nowChar );
        stringstream temp;
        temp << atoi( thisToken->mContent.c_str() );
        thisToken->mContent = temp.str();
        stop = true;
      } // if
      else {
        if ( nowChar == '.' ) {
          thisToken->mType = FLOAT;
        } // if
        else if ( isdigit( nowChar ) )
          ;
        else
          thisToken->mType = SYMBOL;
        thisToken->mContent += nowChar;
      } // else
    } // else if ( thisToken->mType == INT )
    else if ( thisToken->mType == FLOAT ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {
             
        gScanner.PutBack( nowChar );
        bool isFLOAT = false;
        for ( int i = 0 ; i < thisToken->mContent.length() ; i ++ ) {   // +. or -. 都是SYMBOL
          if ( isdigit( thisToken->mContent[i] ) )
            isFLOAT = true;
        } // for

        if ( isFLOAT ) {
          stringstream temp;
          temp << fixed << setprecision( 3 ) << atof( thisToken->mContent.c_str() );
          thisToken->mContent = temp.str();
        } // if
        else
          thisToken->mType = SYMBOL;
        stop = true;
      } // if
      else {
        if ( isdigit( nowChar ) ) {
          ;
        } // if
        else
          thisToken->mType = SYMBOL;
        thisToken->mContent += nowChar;
      } // else
    } // else if ( thisToken->mType == FLOAT )
    else if ( thisToken->mType == T ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {
             
        gScanner.PutBack( nowChar );
        thisToken->mContent = "#t";
        stop = true;
      } // if
      else {
        thisToken->mType = SYMBOL;
        thisToken->mContent += nowChar;
      } // else
    } // else if ( thisToken->mType == T )
    else if ( thisToken->mType == STRING ) {

      bool run;
      if ( nowChar != '\"' )
        run = true;
      else
        run = false;

      char ch = '\0';
      while ( run ) {

        if ( nowChar == EOF ) {
          stop = true;
          gExit = true;
          throw new Exception( NO_MORE_INPUT );
        } // if

        if ( nowChar == '\n' ) {
          throw new Exception( NO_CLOSING_QUOTE );
        } // if

        if ( nowChar == '\\' ) {
          if ( gScanner.Peekone() == 'n' || gScanner.Peekone() == 't' 
               || gScanner.Peekone() == '\"' || gScanner.Peekone() == '\\' )
            ch = gScanner.GetChar();
          if ( ch == 'n' )
            nowChar = '\n';
          else if ( ch == 't' )
            nowChar = '\t';
          else if ( ch == '\"' )
            nowChar = '\"';
          else if ( ch == '\\' )
            ;
        } // if

        thisToken->mContent += nowChar;
        nowChar = gScanner.GetChar();
        if ( nowChar == '\"' ) {
          run = false;
        } // if
      } // while

      thisToken->mContent += nowChar;
      stop = true;

    } // else if ( thisToken->mType == STRING )
    else if ( thisToken->mType == SYMBOL ) {
      if ( nowChar == '(' || nowChar == ')' || nowChar == '\'' 
           || nowChar == '\"' || nowChar == ';' || isspace( nowChar ) ) {
             
        gScanner.PutBack( nowChar );
        if ( thisToken->mContent == "nil" ) // //////// important!!
          thisToken->mType = NIL;
        else
          ;
        stop = true;
      } // if
      else {
        thisToken->mContent += nowChar;
      } // else
    } // else if ( thisToken->mType == SYMBOL )
    else {
      cout << "BAD! : ReadToken->else" << endl;
    } // else 
  } // while

  // cout << thisToken->mType << " - " << thisToken->mContent << endl;

  if ( nowChar != EOF ) {
    if ( gScanner.mNeedRP ) {
      if ( thisToken->mType != RIGHT_PAREN ) {
        throw new Exception( thisToken, NO_RIGHT_PAREN );
      } // if 
      else
        return thisToken;
    } // if
    else {
      return thisToken;
    } // else
  } // if
  else
    return NULL;
} // ReadToken()

void ReadSExp( TokenPtr thisToken, int level ) {
  // if quote
  gScanner.Initial();
  bool dotted = false;

  if ( thisToken->mType == QUOTE ) {

    
    thisToken->mType = LEFT_PAREN;
    thisToken->mContent = "(";
    thisToken->mLeft = new Token( QUOTE, "quote" );       //        [(]  
    thisToken->mRight = new Token( LEFT_PAREN, "(" );     //     [q]   [(]      
    thisToken->mRight->mRight = new Token( NIL, "nil" );  //              [nil]
    level++;

    gScanner.mInReading[ QUOTE ] = true;
    thisToken->mRight->mLeft = ReadToken();
    gScanner.mInReading[ QUOTE ] = false;

    if ( thisToken->mRight->mLeft->mType == LEFT_PAREN ) {
      ReadSExp( thisToken->mRight, level + 1 ) ;
    } // if
    else if ( thisToken->mRight->mType == RIGHT_PAREN ) { // error
      ;
    } // else if
    else if ( thisToken->mRight->mType == DOT ) { // error
      ;
    } // else if
    else if ( thisToken->mRight->mLeft->mType == QUOTE ) {     // ''1
      ReadSExp( thisToken->mRight->mLeft, level + 1 );
    } // else if
    else {  // SYMBOL 啥都不用做
      return;
    } // else

    
  } // if

  if ( thisToken->mLeft == NULL ) {

    gScanner.mInReading[ LEFT_PAREN ] = true;
    thisToken->mLeft = ReadToken();
    gScanner.mInReading[ LEFT_PAREN ] = false;

    if ( thisToken->mLeft->mType == LEFT_PAREN ) {
      ReadSExp( thisToken->mLeft, level + 1 );
    } // if
    else if ( thisToken->mLeft->mType == RIGHT_PAREN ) {
      thisToken->mType = NIL;
      thisToken->mContent = "nil";
      thisToken->mRight = NULL;

      return;
    } // else if
    else if ( thisToken->mLeft->mType == DOT ) {  // error
      ;
    } // else if
    else if ( thisToken->mLeft->mType == QUOTE ) {
      ReadSExp( thisToken->mLeft, level + 1 );
    }  // else if
    else {
      ;
    } // else
  } // if
  else {
    if ( thisToken->mLeft->mType == LEFT_PAREN ) {
      ReadSExp( thisToken->mLeft, level + 1 );
    } // if
  } // else
  
  if ( thisToken->mRight == NULL ) {

    thisToken->mRight = ReadToken();

    if ( thisToken->mRight->mType == LEFT_PAREN ) {
      thisToken->mRight->mLeft = new Token();
      thisToken->mRight->mLeft->mType = LEFT_PAREN;
      thisToken->mRight->mLeft->mContent = "(";
      ReadSExp( thisToken->mRight, level + 1 );
    } // if
    else if ( thisToken->mRight->mType == RIGHT_PAREN ) {
      thisToken->mRight->mType = NIL;
      thisToken->mRight->mContent = "nil";
    } // if
    else if ( thisToken->mRight->mType == DOT ) {
      
      gScanner.mInReading[ DOT ] = true;
      thisToken->mRight = ReadToken();
      gScanner.mInReading[ DOT ] = false;
      dotted = true;

      if ( thisToken->mRight->mType == LEFT_PAREN ) {
        thisToken->mRight->mContent = "(";
        ReadSExp( thisToken->mRight, level + 1 );
      } // if
      else if ( thisToken->mRight->mType == RIGHT_PAREN ) { // Error
        ;
      } // else if
      else if ( thisToken->mRight->mType == DOT ) { // Error
        ;
      } // else if
      else if ( thisToken->mRight->mType == QUOTE ) {
        ReadSExp( thisToken->mRight, level + 1 );
      } // else if
      else {
        ;
      } // else

      
    } // else if
    else if ( thisToken->mRight->mType == QUOTE ) {
      // delete thisToken;
      thisToken->mRight = new Token( LEFT_PAREN, "(" ) ;
      thisToken->mRight->mLeft = new Token( QUOTE, "quote" );
      level++;
      ReadSExp( thisToken->mRight->mLeft, level + 1 );
      ReadSExp( thisToken->mRight, level );
    } // else if
    else {
      TokenPtr temp = new Token();
      temp->mType = LEFT_PAREN;
      temp->mContent = "(";
      temp->mLeft = thisToken->mRight;
      thisToken->mRight = temp;
      ReadSExp( thisToken->mRight, level + 1 );
    } // else
  } // if

  if ( dotted ) {
    gScanner.mNeedRP = true;
    TokenPtr temp = ReadToken();
    delete temp;
    gScanner.mNeedRP = false;
  } // if

  return;

} // ReadSExp()

void PrintSExp( TokenPtr thisToken, int level, bool isRight ) {
  
  if ( isRight ) {
    for ( int i = 0 ; i < level + 1 ; i ++ )
      cout << "  ";
  } // if
  else {
    cout << "( ";
  } // else

  if ( thisToken->mLeft != NULL ) {
    if ( thisToken->mLeft->mType == LEFT_PAREN ) {
      PrintSExp( thisToken->mLeft, level + 1, false );
    } // if
    else {
      cout << thisToken->mLeft->mContent;
    } // else
  } // if

  if ( thisToken->mRight != NULL ) {
    if ( thisToken->mRight->mType == LEFT_PAREN ) {
      cout << "\n";
      PrintSExp( thisToken->mRight, level, true ) ;
    } // if
    else {
      if ( thisToken->mRight->mType == NIL ) {
        cout << "\n";
        for ( int i = 0 ; i < level ; i ++ )
          cout << "  ";
        cout << ")";
      } // if
      else {
        cout << "\n";
        for ( int i = 0 ; i < level + 1 ; i ++ )
          cout << "  ";
        cout << ".\n";
        for ( int i = 0 ; i < level + 1 ; i ++ )
          cout << "  ";
        cout << thisToken->mRight->mContent;
        cout << "\n";
        for ( int i = 0 ; i < level ; i ++ )
          cout << "  ";
        cout << ")";
      } // else
    } // else
  } // if

  return;

} // PrintSExp()

void PrintSExpOneLine( TokenPtr thisToken ) {
  cout << thisToken->mContent << " ";
  if ( thisToken->mLeft != NULL ) {
    if ( thisToken->mLeft->mType == LEFT_PAREN ) {
      PrintSExpOneLine( thisToken->mLeft );
    } // if
    else
      cout << thisToken->mLeft->mContent << " ";
  } // if

  cout << ". ";

  if ( thisToken->mRight != NULL ) {
    if ( thisToken->mRight->mType == LEFT_PAREN ) {
      PrintSExpOneLine( thisToken->mRight );
    } // if
    else 
      cout << thisToken->mRight->mContent << " ";
  } // if

  cout << ") ";
  return;
} // PrintSExpOneLine()

bool IsPureList( TokenPtr thisToken ) {
  if ( thisToken->mRight == NULL ) {
    if ( thisToken->mType != NIL ) 
      return false;
    else
      return true;
  } // if
  else {
    return IsPureList( thisToken->mRight );
  } // else
} // IsPureList()

bool IsInternalCmd( TokenPtr thisToken ) {
  for ( int i = 0 ; i < CmdNum ; i ++ ) {
    if ( thisToken->mContent == gInternelCommand[i] ) {
      return true;
    } // if
  } // for

  return false;
} // IsInternalCmd()

bool IsKnownFunction( TokenPtr thisToken ) {

  for ( int i = 0 ; i < gUserDefineTable.size() ; i ++ ) {
    if ( thisToken->mContent == gUserDefineTable[i]->mLeft->mContent ) {
      return true;
    } // if
  } // for

  return false;

} // IsKnownFunction()

TokenPtr EvalSExp( TokenPtr thisToken, int level ) ;

TokenPtr GetKnownFunction( TokenPtr thisToken ) {

  for ( int i = 0 ; i < gUserDefineTable.size() ; i ++ ) {
    if ( thisToken->mContent == gUserDefineTable[i]->mLeft->mContent ) {
      return gUserDefineTable[i]->mRight->mLeft;
    } // if
  } // for

  cout << "KnownFunction have not found!" << endl;
  return NULL;

} // GetKnownFunction()

int Arguments( TokenPtr thisToken ) {
  TokenPtr walk;
  int arg = 0;
  walk = thisToken;
  while ( walk != NULL ) {
    walk = walk->mRight;
    arg++;
  } // while

  return arg - 2;
} // Arguments()

TokenPtr DeQuote( TokenPtr thisToken ) {
  thisToken->mRight->mLeft->mQuoted = true;
  return thisToken->mRight->mLeft;
} // DeQuote()

TokenPtr CONS( TokenPtr thisToken, int level ) {
  if ( Arguments( thisToken ) == 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    thisToken->mRight->mRight->mLeft = EvalSExp( thisToken->mRight->mRight->mLeft, level + 1 ) ;

    thisToken->mRight->mRight = thisToken->mRight->mRight->mLeft;

    return thisToken->mRight;

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM ) ;
  } // else
} // CONS()

TokenPtr LIST( TokenPtr thisToken, int level ) {
  TokenPtr walk = thisToken->mRight;
  while ( walk->mRight != NULL ) {
    
    walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

    walk = walk->mRight;
  } // while

  return thisToken->mRight;
} // LIST()

TokenPtr DEFINE( TokenPtr thisToken, int level ) {
  if ( Arguments( thisToken ) == 2 ) {
    if ( !IsInternalCmd( thisToken->mRight->mLeft ) ) {

      if ( !IsInternalCmd( thisToken->mRight->mRight->mLeft ) )
        thisToken->mRight->mRight->mLeft = EvalSExp(  thisToken->mRight->mRight->mLeft, level + 1 ) ;

      TokenPtr defPtr = new Token( LEFT_PAREN, "(" );
      defPtr->mLeft = new Token( SYMBOL, thisToken->mRight->mLeft->mContent );
      defPtr->mRight = new Token( LEFT_PAREN, "(" );
      defPtr->mRight->mRight = new Token( NIL, "nil" );
      defPtr->mRight->mLeft = thisToken->mRight->mRight->mLeft;

      int index = -1;

      for ( int i = 0 ; i < gUserDefineTable.size() && index == -1 ; i ++ ) {
        if ( thisToken->mRight->mLeft->mContent == gUserDefineTable[i]->mLeft->mContent ) {
          index = i;
        } // if
      } // for

      if ( index != -1 ) 
        gUserDefineTable[index]->mRight->mLeft = defPtr->mRight->mLeft;
      else 
        gUserDefineTable.push_back( defPtr ) ;

      return new Token( SYMBOL, defPtr->mLeft->mContent + " defined" ); // 例外 印出一個SYMBOL string!!!!!
    } // if 
    else {
      cout << "ERROR (DEFINE format) : ";
      return thisToken;
    } // else

  } // if
  else {
    cout << "ERROR (DEFINE format) : ";
    return thisToken;
  } // else
} // DEFINE()

TokenPtr CAR( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 ) ;

    if ( thisToken->mRight->mLeft->mLeft != NULL ) {
      return thisToken->mRight->mLeft->mLeft;
    } // if
    else {
      if ( IsInternalCmd( thisToken->mRight->mLeft ) )
        throw new Exception( thisToken->mRight->mLeft, "CAR", WRONG_ARG_TYPE, true );
      else 
        throw new Exception( thisToken->mRight->mLeft, "CAR", WRONG_ARG_TYPE, false );
    } // else
  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else 

} // CAR()

TokenPtr CDR( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 ) ;

    if ( thisToken->mRight->mLeft->mRight != NULL ) {
      return thisToken->mRight->mLeft->mRight;
    } // if
    else {
      if ( IsInternalCmd( thisToken->mRight->mLeft ) )
        throw new Exception( thisToken->mRight->mLeft, "CDR", WRONG_ARG_TYPE, true );
      else 
        throw new Exception( thisToken->mRight->mLeft, "CDR", WRONG_ARG_TYPE, false );
    } // else
  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else 

} // CDR()

TokenPtr IS_ATOM( TokenPtr thisToken, int level ) {
  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == STRING ||
         thisToken->mRight->mLeft->mType ==  FLOAT || thisToken->mRight->mLeft->mType ==  NIL ||
         thisToken->mRight->mLeft->mType ==  T || thisToken->mRight->mLeft->mType ==  SYMBOL ) {
      
      return new Token( T, "#t" );

    } // if
    else {
      return new Token( NIL, "nil" );
    } // else
  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else
} // IS_ATOM()

TokenPtr IS_PAIR( TokenPtr thisToken, int level ) {
  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == STRING ||
         thisToken->mRight->mLeft->mType ==  FLOAT || thisToken->mRight->mLeft->mType ==  NIL ||
         thisToken->mRight->mLeft->mType ==  T || thisToken->mRight->mLeft->mType ==  SYMBOL ) {
      
      return new Token( NIL, "nil" );
    } // if
    else {
      return new Token( T, "#t" );
    } // else
  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else
} // IS_PAIR()

TokenPtr IS_LIST( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( IsPureList( thisToken->mRight->mLeft ) )
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_LIST()

TokenPtr IS_NULL( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == NIL ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_NULL()

TokenPtr IS_INTEGER( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_INTEGER()

TokenPtr IS_REAL( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_REAL()

TokenPtr IS_NUMBER( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_NUMBER()

TokenPtr IS_STRING( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == STRING ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_STRING()

TokenPtr IS_BOOLEAN( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == T || thisToken->mRight->mLeft->mType == NIL ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_BOOLEAN()

TokenPtr IS_SYMBOL( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == SYMBOL ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // IS_SYMBOL()

TokenPtr PLUS( TokenPtr thisToken, int level ) {

  bool iNTmode = true;

  if ( Arguments( thisToken ) >= 2 ) {
    float num = 0;
    TokenPtr walk = thisToken->mRight;
    
    while ( walk->mRight != NULL ) {
      
      walk->mLeft = EvalSExp( walk->mLeft, level + 1 );
      
      if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {
        if ( walk->mLeft->mType == FLOAT )
          iNTmode = false;
        num = num + atof( walk->mLeft->mContent.c_str() );
      } // if
      else {
        throw new Exception( walk->mLeft, "+", WRONG_ARG_TYPE );
      } // else

      walk = walk->mRight;
    } // while

    stringstream temp;

    if ( iNTmode ) {
      temp << num;
      return new Token( INT, temp.str() );
    } // if
    else {
      temp << fixed << setprecision( 3 ) << num;
      return new Token( FLOAT, temp.str() );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // PLUS()

TokenPtr MINUS( TokenPtr thisToken, int level ) {

  bool iNTmode = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) {

      if ( thisToken->mRight->mLeft->mType == FLOAT )
        iNTmode = false;

      float num = atof( thisToken->mRight->mLeft->mContent.c_str() );
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {
          if ( walk->mLeft->mType == FLOAT )
            iNTmode = false;
          num = num - atof( walk->mLeft->mContent.c_str() );
        } // if
        else {
          throw new Exception( walk->mLeft, "-", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      stringstream temp;

      if ( iNTmode ) {
        temp << num;
        return new Token( INT, temp.str() );
      } // if
      else {
        temp << fixed << setprecision( 3 ) << num;
        return new Token( FLOAT, temp.str() );
      } // else

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "-", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // MINUS()
 
TokenPtr MUL( TokenPtr thisToken, int level ) {

  bool iNTmode = true;

  if ( Arguments( thisToken ) >= 2 ) {
    float num = 1;
    TokenPtr walk = thisToken->mRight;

    while ( walk->mRight != NULL ) {

      walk->mLeft = EvalSExp( walk->mLeft, level + 1 );
      
      if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {
        if ( walk->mLeft->mType == FLOAT )
          iNTmode = false;
        num = num * atof( walk->mLeft->mContent.c_str() );
      } // if
      else {
        throw new Exception( walk->mLeft, "*", WRONG_ARG_TYPE );
      } // else

      walk = walk->mRight;
    } // while

    stringstream temp;

    if ( iNTmode ) {
      temp << num;
      return new Token( INT, temp.str() );
    } // if
    else {
      temp << fixed << setprecision( 3 ) << num;
      return new Token( FLOAT, temp.str() );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // MUL()

TokenPtr DIV( TokenPtr thisToken, int level ) {

  bool iNTmode = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) {

      if ( thisToken->mRight->mLeft->mType == FLOAT )
        iNTmode = false;

      int iNTnum = atof( thisToken->mRight->mLeft->mContent.c_str() );
      float num = atof( thisToken->mRight->mLeft->mContent.c_str() );
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {
          if ( walk->mLeft->mType == FLOAT )
            iNTmode = false;
          if ( atof( walk->mLeft->mContent.c_str() ) == 0 ) {
            throw new Exception( DIVED_ZERO );
          } // if

          iNTnum = iNTnum / atof( walk->mLeft->mContent.c_str() );
          num = num / atof( walk->mLeft->mContent.c_str() );
        } // if
        else {
          throw new Exception( walk->mLeft, "/", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      stringstream temp;

      if ( iNTmode ) {
        temp << iNTnum;
        return new Token( INT, temp.str() );
      } // if
      else {
        temp << fixed << setprecision( 3 ) << num;
        return new Token( FLOAT, temp.str() );
      } // else

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "/", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // DIV()

TokenPtr NOT( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 1 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == NIL ) 
      return new Token( T, "#t" );
    else
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // NOT()

TokenPtr AND( TokenPtr thisToken, int level ) {

  TokenPtr theLast = NULL;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 ) ;
    if ( thisToken->mRight->mLeft->mType == NIL ) {

      return new Token( NIL, "nil" ) ;

    } // if
    else {
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 );
        theLast = walk->mLeft;

        walk = walk->mRight;
      } // while

      return theLast;

    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // AND()

TokenPtr OR( TokenPtr thisToken, int level ) {

  TokenPtr theLast = NULL;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 ) ;
    if ( thisToken->mRight->mLeft->mType != NIL ) {

      return thisToken->mRight->mLeft;

    } // if
    else {
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 );
        theLast = walk->mLeft;

        walk = walk->mRight;
      } // while

      return theLast;

    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // OR()

TokenPtr MORE( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) {

      float num = atof( thisToken->mRight->mLeft->mContent.c_str() );
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {

          float secNum = atof( walk->mLeft->mContent.c_str() ) ;

          if ( num > secNum )
            num = secNum;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, ">", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, ">", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // MORE()

TokenPtr MOREEQ( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) {

      float num = atof( thisToken->mRight->mLeft->mContent.c_str() );
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {

          float secNum = atof( walk->mLeft->mContent.c_str() ) ;

          if ( num >= secNum )
            num = secNum;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, ">=", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, ">=", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // MOREEQ()

TokenPtr LESS( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) {

      float num = atof( thisToken->mRight->mLeft->mContent.c_str() );
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {

          float secNum = atof( walk->mLeft->mContent.c_str() ) ;

          if ( num < secNum )
            num = secNum;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, "<", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "<", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // LESS()

TokenPtr LESSEQ( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) {

      float num = atof( thisToken->mRight->mLeft->mContent.c_str() );
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {

          float secNum = atof( walk->mLeft->mContent.c_str() ) ;

          if ( num <= secNum )
            num = secNum;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, "<=", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "<=", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // LESSEQ()

TokenPtr EQ( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == INT || thisToken->mRight->mLeft->mType == FLOAT ) {

      float num = atof( thisToken->mRight->mLeft->mContent.c_str() );
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == INT || walk->mLeft->mType == FLOAT ) {

          float secNum = atof( walk->mLeft->mContent.c_str() ) ;

          if ( num == secNum )
            num = secNum;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, "=", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "=", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // EQ()

TokenPtr STRING_APPEND( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) >= 2 ) {
    float num = 0;
    TokenPtr walk = thisToken->mRight;
    string output = "";

    while ( walk->mRight != NULL ) {

      walk->mLeft = EvalSExp( walk->mLeft, level + 1 );

      if ( walk->mLeft->mType == STRING ) {

        walk->mLeft->mContent.erase( walk->mLeft->mContent.begin() );
        walk->mLeft->mContent.erase( walk->mLeft->mContent.begin() + walk->mLeft->mContent.length() - 1 );
        output = output + walk->mLeft->mContent; 

      } // if
      else {
        throw new Exception( walk->mLeft, "string-append", WRONG_ARG_TYPE );
      } // else

      walk = walk->mRight;
    } // while

    output = "\"" + output + "\"";
    return new Token( STRING, output );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // STRING_APPEND()

TokenPtr STRING_MORE( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == STRING ) {

      string myString = thisToken->mRight->mLeft->mContent;
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == STRING ) {

          if ( myString > walk->mLeft->mContent )
            myString = walk->mLeft->mContent;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, "string>?", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "string>?", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // STRING_MORE()

TokenPtr STRING_LESS( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == STRING ) {

      string myString = thisToken->mRight->mLeft->mContent;
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == STRING ) {

          if ( myString < walk->mLeft->mContent )
            myString = walk->mLeft->mContent;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, "string<?", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "string<?", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // STRING_LESS()

TokenPtr STRING_EQ( TokenPtr thisToken, int level ) {

  bool noProblem = true;

  if ( Arguments( thisToken ) >= 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( thisToken->mRight->mLeft->mType == STRING ) {

      string myString = thisToken->mRight->mLeft->mContent;
      TokenPtr walk = thisToken->mRight->mRight;

      while ( walk->mRight != NULL ) {

        walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

        if ( walk->mLeft->mType == STRING ) {

          if ( myString == walk->mLeft->mContent )
            myString = walk->mLeft->mContent;
          else
            noProblem = false;

        } // if
        else {
          throw new Exception( walk->mLeft, "string=?", WRONG_ARG_TYPE );
        } // else

        walk = walk->mRight;
      } // while

      if ( noProblem ) 
        return new Token( T, "#t" );
      else 
        return new Token( NIL, "nil" );

    } // if
    else {
      throw new Exception( thisToken->mRight->mLeft, "string=?", WRONG_ARG_TYPE );
    } // else

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // STRING_EQ()

bool TYPE_IS_ATOM( TokenPtr thisToken ) {
  if ( thisToken->mType == INT || thisToken->mType == STRING ||
       thisToken->mType == FLOAT || thisToken->mType == NIL ||
       thisToken->mType == T || thisToken->mType == SYMBOL )
    return true;
  else
    return false;
} // TYPE_IS_ATOM()

TokenPtr EQV( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );
    thisToken->mRight->mRight->mLeft = EvalSExp( thisToken->mRight->mRight->mLeft, level + 1 );
    
    if ( thisToken->mRight->mLeft == thisToken->mRight->mRight->mLeft )
      return new Token( T, "#t" );

    if ( TYPE_IS_ATOM( thisToken->mRight->mLeft ) && TYPE_IS_ATOM( thisToken->mRight->mRight->mLeft ) ) {
      if ( thisToken->mRight->mLeft->mType != STRING && thisToken->mRight->mRight->mLeft->mType != STRING ) {
        if ( thisToken->mRight->mLeft->mType == thisToken->mRight->mRight->mLeft->mType ) {
          if ( thisToken->mRight->mLeft->mContent == thisToken->mRight->mRight->mLeft->mContent ) {
            return new Token( T, "#t" );
          } // if
          else
            return new Token( NIL, "nil" );
        } // if
        else
          return new Token( NIL, "nil" );
      } // if
      else
        return new Token( NIL, "nil" );
    } // if
    else 
      return new Token( NIL, "nil" );
    
  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else

} // EQV()

bool CheckEachToken( TokenPtr left, TokenPtr right ) {
  bool leftEqual = false;
  bool rightEqual = false;

  if ( left->mLeft == NULL && right->mLeft == NULL ) {
    if ( left->mType == right->mType && 
         left->mContent == right->mContent ) {
      leftEqual = true;
    } // if
    else
      return false;
  } // if
  else if ( left->mLeft != NULL && right->mLeft != NULL ) {
    if ( CheckEachToken( left->mLeft, right->mLeft ) )
      leftEqual = true;
    else
      leftEqual = false;
  } // if
  else {
    return false;
  } // else

  if ( left->mRight == NULL && right->mRight == NULL ) {
    if ( left->mType == right->mType && 
         left->mContent == right->mContent ) {
      rightEqual = true;
    } // if
    else
      return false;
  } // if
  else if ( left->mLeft != NULL && right->mLeft != NULL ) {
    if ( CheckEachToken( left->mRight, right->mRight ) )
      rightEqual = true;
    else
      rightEqual = false;
  } // if
  else {
    return false;
  } // else

  if ( leftEqual && rightEqual )
    return true;
  else
    return false;

} // CheckEachToken()

TokenPtr EQUAL( TokenPtr thisToken, int level ) {

  if ( Arguments( thisToken ) == 2 ) {

    thisToken->mRight->mLeft = EvalSExp( thisToken->mRight->mLeft, level + 1 );
    thisToken->mRight->mRight->mLeft = EvalSExp( thisToken->mRight->mRight->mLeft, level + 1 );

    if ( CheckEachToken( thisToken->mRight->mLeft, thisToken->mRight->mRight->mLeft ) ) 
      return new Token( T, "#t" );
    else 
      return new Token( NIL, "nil" );

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else
  
} // EQUAL()

TokenPtr BEGIN( TokenPtr thisToken, int level ) {
  if ( Arguments( thisToken ) >= 1 ) {

    TokenPtr walk = thisToken->mRight;
    TokenPtr theLast = thisToken->mRight->mLeft;

    while ( walk->mRight != NULL ) {

      walk->mLeft = EvalSExp( walk->mLeft, level + 1 ) ;

      theLast = walk->mLeft;

      walk = walk->mRight;

    } // while

    return theLast;

  } // if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else
} // BEGIN()

TokenPtr IF( TokenPtr thisToken, int level ) {
  if ( Arguments( thisToken ) == 2 ) {  
    if ( thisToken->mRight->mLeft->mType != NIL )
      return EvalSExp( thisToken->mRight->mRight->mLeft, level + 1 );
    else 
      throw new Exception( thisToken, NO_RETURN_VAL );
  } // if
  else if ( Arguments( thisToken ) == 3 ) {

    TokenPtr tempPtr = EvalSExp( thisToken->mRight->mLeft, level + 1 );

    if ( tempPtr->mType != NIL ) {
      return EvalSExp( thisToken->mRight->mRight->mLeft, level + 1 );
    } // if
    else {
      return EvalSExp( thisToken->mRight->mRight->mRight->mLeft, level + 1 );
    } // else
  } // else if
  else {
    throw new Exception( thisToken, WRONG_ARG_NUM );
  } // else
} // IF()

TokenPtr COND( TokenPtr thisToken, int level ) {

  TokenPtr theLast;

  if ( Arguments( thisToken ) >= 1 ) {

    TokenPtr walk = thisToken->mRight;
    while ( walk->mRight->mType != NIL ) {
      
      if ( walk->mLeft->mLeft != NULL ) {
        
        TokenPtr temp = EvalSExp( walk->mLeft->mLeft, level + 1 );

        if ( temp->mType == T ) {
          TokenPtr walkT = walk->mLeft->mRight;
          if ( walkT->mType == NIL )  // 條件成立但沒有沒辦法印東西
            throw new Exception( thisToken, "COND", ERROR_FORMAT );

          while ( walkT->mRight->mType != NIL )
            walkT = walkT->mRight;

          return EvalSExp( walkT->mLeft, level + 1 );

        } // if
        else if ( temp->mType == NIL ) {
          walk = walk->mRight;
        } // else if
        else {

          TokenPtr walkN = walk->mLeft->mRight;
          if ( walk->mLeft->mRight->mType == NIL ) 
            throw new Exception( thisToken, "COND", ERROR_FORMAT );

          while( walkN->mRight->mType != NIL )
            walkN = walkN->mRight;

          return EvalSExp( walkN->mLeft, level + 1 );

        } // else
      } // if
      else {
        throw new Exception( thisToken, "COND", ERROR_FORMAT );
      } // else

    } // while

    if ( walk->mLeft->mLeft->mContent == "else" ) {
          
      TokenPtr walkELSE = walk->mLeft->mRight;
      if ( walkELSE->mType == NIL )  // 條件成立但沒有沒辦法印東西
        throw new Exception( thisToken, "COND", ERROR_FORMAT );

      while ( walkELSE->mRight->mType != NIL )
        walkELSE = walkELSE->mRight;

      return EvalSExp( walkELSE->mLeft, level + 1 );

    } // if
    else if ( EvalSExp( walk->mLeft->mLeft, level + 1 )->mType == T ) {

      TokenPtr walkT = walk->mLeft->mRight;
      if ( walkT->mType == NIL )  // 條件成立但沒有沒辦法印東西
        throw new Exception( thisToken, "COND", ERROR_FORMAT );

      while ( walkT->mRight->mType != NIL )
        walkT = walkT->mRight;

      return EvalSExp( walkT->mLeft, level + 1 );

    } // else if
    else {  
      throw new Exception( thisToken, NO_RETURN_VAL );
    } // else
    
  } // if
  else {
    throw new Exception( thisToken, "COND", ERROR_FORMAT );
  } // else
} // COND()

TokenPtr CLEAN( TokenPtr thisToken, int level ) {
  if ( level == 0 ) {
    gUserDefineTable.clear();
    throw new Exception( CLEAN_TABLE );
  } // if
  else
    return thisToken;
} // CLEAN()

TokenPtr EvalSExp( TokenPtr thisToken, int level ) {
  // cout << "thisToken's content: " << thisToken->mContent << ", thisToken's type : " << thisToken->mType << endl;
  if ( thisToken->mType == LEFT_PAREN ) {
    if ( !IsPureList( thisToken ) ) {
      cout << "ERROR (non-list) : " << endl;
      return thisToken;
    } // if
    else { // 
      if ( thisToken->mLeft->mType == LEFT_PAREN ) {
        return EvalSExp( thisToken->mLeft, level + 1 );
      } // if
      else if ( thisToken->mLeft->mType != SYMBOL && thisToken->mLeft->mType != QUOTE ) {
        throw new Exception( thisToken->mLeft, APPLY_NON_FUNC ) ;
      } // if
      else {

        if ( IsKnownFunction( thisToken->mLeft ) ) 
          thisToken->mLeft = GetKnownFunction( thisToken->mLeft );

        if ( thisToken->mLeft->mContent == "cons" ) {
          return CONS( thisToken, level );
        } // if
        else if ( thisToken->mLeft->mContent == "list" ) {
          return LIST( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "quote" ) {
          return DeQuote( thisToken );
        } // else if
        else if ( thisToken->mLeft->mContent == "define" ) {
          return DEFINE( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "car" ) {
          return CAR( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "cdr" ) {
          return CDR( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "atom?" ) {
          return IS_ATOM( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "pair?" ) {
          return IS_PAIR( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "list?" ) {
          return IS_LIST( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "null?" ) {
          return IS_NULL( thisToken, level ) ;
        } // else if
        else if ( thisToken->mLeft->mContent == "integer?" ) {
          return IS_INTEGER( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "real?" ) {
          return IS_REAL( thisToken, level ) ;
        } // else if
        else if ( thisToken->mLeft->mContent == "number?" ) {
          return IS_NUMBER( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "string?" ) {
          return IS_STRING( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "boolean?" ) {
          return IS_BOOLEAN( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "symbol?" ) {
          return IS_SYMBOL( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "+" ) {
          return PLUS( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "-" ) {
          return MINUS( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "*" ) {
          return MUL( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "/" ) {
          return DIV( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "not" ) {
          return NOT( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "and" ) {
          return AND( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "or" ) {
          return OR( thisToken, level ) ;
        } // else if
        else if ( thisToken->mLeft->mContent == ">" ) {
          return MORE( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == ">=" ) {
          return MOREEQ( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "<" ) {
          return LESS( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "<=" ) {
          return LESSEQ( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "=" ) {
          return EQ( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "string-append" ) {
          return STRING_APPEND( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "string>?" ) {
          return STRING_MORE( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "string<?" ) {
          return STRING_LESS( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "string=?" ) {
          return STRING_EQ( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "eqv?" ) {
          return EQV( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "equal?" ) {
          return EQUAL( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "begin" ) {
          return BEGIN( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "if" ) {
          return IF( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "cond" ) {
          return COND( thisToken, level );
        } // else if
        else if ( thisToken->mLeft->mContent == "clean-environment" && level == 0 ) {
          return CLEAN( thisToken, level );
        } // else if
        else {
          if ( thisToken->mType == LEFT_PAREN && thisToken->mLeft->mType == SYMBOL && 
               thisToken->mLeft->mContent == "exit" && thisToken->mRight->mType == NIL ) {
            gExit = true;
            throw new Exception( EXIT );
          } // if
          else {
            throw new Exception( thisToken->mLeft, UNBOUND );
          } // else
        } // else
      } // else
    } // else
    
  } // if
  else if ( thisToken->mType == SYMBOL ) {
    
    if ( IsInternalCmd( thisToken ) ) {
      throw new Exception( thisToken, ONLYINTERELCMD );
    } // if
    else if ( IsKnownFunction( thisToken ) ) {
      thisToken = GetKnownFunction( thisToken ) ;
      if ( IsInternalCmd( thisToken ) )
        return EvalSExp( thisToken, 0 );
      return thisToken;
      // return EvalSExp( thisToken, 0 ) ;
    } // else if 
    else {
      throw new Exception( thisToken, UNBOUND ) ;
    } // else
  } // if
  else
    return thisToken;
} // EvalSExp()


int main() {
  
  cin >> uTestNum;

  cout << "Welcome to OurScheme!\n";

  TokenPtr thisToken = NULL; 
  
  while ( !gExit ) {
    cout << "\n> ";
    try {
      gSExpMode = false;
      gScanner.Initial();
      
      thisToken = ReadToken();

      if ( thisToken->mType == LEFT_PAREN || thisToken->mType == QUOTE ) {
        gSExpMode = true;
        gScanner.mOutputed = false;
        ReadSExp( thisToken, 0 );
      } // if

      try {


        thisToken = EvalSExp( thisToken, 0 );

        if ( thisToken != NULL ) {

          if ( thisToken->mLeft != NULL && thisToken->mRight != NULL ) {
            PrintSExp( thisToken, 0, false );
            cout << endl;
            // PrintSExpOneLine( thisToken );
          } // if
          else {
            cout << thisToken->mContent << endl;
          } // else

        } // if

      } catch ( Exception * eEval ) {

        cout << eEval->mErrorString ;

        
        if ( eEval->mPrint ) {
          if ( eEval->mErrorPtr->mLeft != NULL && eEval->mErrorPtr->mRight != NULL ) {
            PrintSExp( eEval->mErrorPtr, 0, false );
          } // if
          else
            cout << eEval->mErrorPtr->mContent << endl;
        } // if
        
      } // catch

      gScanner.mOutputed = true;

      gLine = 1;
      gColumn = 0;

    } catch ( Exception *exception  ) {

      cout << exception->mErrorString;
      if ( !gExit ) {
        cout << endl;
      } // if

      if ( gScanner.mCh != '\n' && gScanner.mCh != EOF ) {
        bool runToEndofLine = true;
        char c = '\0';
        while ( runToEndofLine ) {
          c = getchar();
          if ( c == '\n' || c == EOF )
            runToEndofLine = false;
        } // while
      } // if

      gLine = 1;
      gColumn = 0;

    } // catch

  } // while()

  cout << "\nThanks for using OurScheme!";

  return 0;

} // main()

