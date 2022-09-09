// PL 10727129 10827107
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
class Parser {

  bool Command() {

    TokenPtr temp ;
    TokenPtr identToken ;
    TokenPtr peektoken ;
    float value ;

    peektoken = gScanner.PeekNextToken() ;

    if ( peektoken->mType == IDENT ) {

        identToken = gScanner.GetAToken() ;

        if ( identToken->mType == IDENT ) {

          peektoken = gScanner.PeekNextToken() ;

          if ( peektoken->mContent == ":=" ) {

            temp = gScanner.GetAToken() ;

            if ( ArithExp( value ) ) {

              temp = gScanner.GetAToken() ;

              if ( temp->mContent == ";" ) {

                identToken->mValue = value ;
                identToken->define = true ;
                return true ;

              } // if
              else throw new Exception( SYNTACTIC_ERR, temp->mContent ) ;

            } // if
            else return false ;
          } // if
          else if ( IDlessArithExpOrBexp( identToken ) ) {

            temp = gScanner.GetAToken() ;

            if ( temp->mContent == ";" ) return true ;
            else throw new Exception( SYNTACTIC_ERR, temp->mContent ) ;

          } // else if
          else throw new Exception( SYNTACTIC_ERR, temp->mContent ) ; 

        } // if
        else throw new Exception( SYNTACTIC_ERR, temp->mContent ) ; 

    } // if
    else if ( NOT_IDStartArithExpOrBexp() ) {

      temp = gScanner.GetAToken() ;

      if ( temp->mContent == ";" ) return true ;
      else throw new Exception( SYNTACTIC_ERR, temp->mContent ) ;

    } // else if
    else if ( peektoken->mType == QUIT ) {

        temp = gScanner.GetAToken() ;
        
        if ( temp->mType == QUIT ) return true ;
        else throw new Exception( SYNTACTIC_ERR, temp->mContent ) ;

    } // else if 
    else {

      temp = gScanner.GetAToken() ;
      throw new Exception( SYNTACTIC_ERR, temp->mContent ) ; 

    } // else
    
  } // Commannd  

  bool ArithExp( float & value ) {
    
    bool term1correct = false ;
    bool term2correct = false ;
    float term1Value ;
    float term2Value ;
    TokenPtr peekToken ;
    TokenPtr temp ;

    correect = true ;

    Term( term1correct, term1Value ) ;

    if ( !term1correct ) {
      correct = false ;
      return false
    } // if
    
    while ( correct ) {

      peektoken = gScanner.PeekNextToken() ;

      if ( peektoken->mContent == "-" || peektoken->mContent == "+" ) {

        temp = gScanner.GetAToken() ;
        Term( term2correct, term2Value ) ;

        if ( !term2correct ) {
          correct = false ;
          return false ;
        } // if
        else {
          if ( temp->mContent == "-" ) {
            term1Value = term1Value - term2Value ;
          } // if
          else if ( temp->mContent == "+" ) {
            term1Value = term1Value + term2Value ;
          } // else if
          else {
            cout << " ArithExp has error ! " << endl;
          } // else

        } // else
      } // if
      else {

        value = term1Value ;
        return true ; 

      } // else

    } // while

  } // ArithExp

  bool IDlessArithExpOrBexp( TokenPtr identToken ) {

    bool stop = false ;
    bool term1correct = false ;
    float term1Value ;
    bool factor1correct = false ;
    float factor1Value ;
    float tempValue ; // 在遇到boolOpf前的加總值
    float expValue ; // 做完ArithExp的加總值
    TokenPtr peekToken ;
    TokenPtr temp ;
    TokenPtr boolToken ; // boolean operator
    TokenPtr opToken ; // + - * / 

    if ( identToke->define == true ) { // 先判斷前面讀進來的IDENT是否被定義過了

      peekToken = gScanner.PeekNextToken() ;

      if ( peekToken->mContent == "+" ||  peekToken->mContent == "-" || peekToken->mContent == "*" || peekToken->mContent == "/") {
      
        opToken = gScanner.GetAToken() ;

        if ( opToken->mContent == "+" || opToken->mContent == "-" ) {

          Term( term1correct, term1Value ) ;  

        } // if
        else if ( opToken->mContent == "*" || opToken->mContent == "/" ) {

          Factor( factor1correct, factor1Value ) ;

        } // else if 
        else ;

      } // if

      if ( !term1correct || !factor1correct ) {
        stop = true ;
        return false ;
      } // if
    
      while( !stop ) {

        opToken = gScanner.GetAToken() ;

        if ( opToken->mContent == "+" ) {

          tempValue = tempValue + term1Value ;

        } // if
        else if ( opToken->mContent == "-" ) {

          tempValue = tempValue - term1Value ;

        } // else if 
        else if ( opToken->mContent == "*" ) {

          tempValue = tempValue * factor1Value ;

        } // else if 
        else if ( temopTokenp->mContent == "/" ) {

          tempValue = tempValue / factor1Value ;

        } // else if 
      
        peekToken = gScanner.PeekNextToken() ;

        if ( peekToken->mContent == "+" ||  peekToken->mContent == "-" || peekToken->mContent == "*" || peekToken->mContent == "/") {
        
          opToken = gScanner.GetAToken() ;

          if ( opToken->mContent == "+" || opToken->mContent == "-" ) {

            Term( term1correct, term1Value ) ;  

          } // if
          else if ( opToken->mContent == "*" || opToken->mContent == "/" ) {

            Factor( factor1correct, factor1Value ) ;
  
          } // else if 
          else ;

        } // if
   
        if ( !term1correct || !factor1correct ) {
          stop = true ;
        } // if

      } // while

      if ( NextIsBooleanOp() ) {

        boolToken = gScanner.GetAToken() ; // BooleanOperator

        if ( ArithExp( expValue ) ) {

          if ( boolToken->mContent == "=" ) {
  
            if ( tempValue == expValue ) return true ;
            else return false ;
    
          } // if
          else if ( boolToken->mContent == "<>") {

            if ( tempValue != expValue ) return true ;
            else return false ;
  
          } // else if
          else if ( boolToken->mContent == ">") {

            if ( tempValue > expValue ) return true ;
            else return false ;

          } // else if
          else if ( boolToken->mContent == "<") {

            if ( tempValue < expValue ) return true ;
            else return false ;

          } // else if
          else if ( boolToken->mContent == ">=") {
 
            if ( tempValue >= expValue ) return true ;
            else return false ;

          } // else if
          else if ( boolToken->mContent == "<=") {

            if ( tempValue <= expValue ) return true ;
            else return false ;
  
          } // else if
          else cout << " IDlessArithExpOrBexp has error ! " << endl ;

        } // if
        else return false ;

        if ( NextIsBooleanOp() ) {
          temp = gScanner.GetAToken() ;
          throw new Exception( SYNTACTIC_ERR, temp->mContent ) ; 
        } // if
        else return true ;
 
      } // if
      else return true ;  

    } // if
    else throw new Exception( SEMANTIC_ERR, identToken->mContent ) ; // IDENT沒被定義
    
  } // IDlessArithExpOrBexp

  bool NOT_IDStartArithExpOrBexp() {

    float value ;
    float tempValue ; 

    if ( NOT_ID_StartArithExp( value ) ) {

      if ( NextIsBooleanOp() ) {

        boolToken = gScanner.GetAToken() ; // BooleanOperator

        if ( ArithExp( tempValue ) ) {

          if ( boolToken->mContent == "=" ) {
  
            if ( value == tempVlaue ) return true ;
            else return false ;
    
          } // if
          else if ( boolToken->mContent == "<>") {

            if ( value != tempVlaue ) return true ;
            else return false ;
  
          } // else if
          else if ( boolToken->mContent == ">") {

            if ( value > tempVlaue ) return true ;
            else return false ;

          } // else if
          else if ( boolToken->mContent == "<") {

            if ( value < tempVlaue ) return true ;
            else return false ;

          } // else if
          else if ( boolToken->mContent == ">=") {
 
            if ( value >= tempVlaue ) return true ;
            else return false ;

          } // else if
          else if ( boolToken->mContent == "<=") {

            if ( value <= tempVlaue ) return true ;
            else return false ;
  
          } // else if
          else cout << " IDlessArithExpOrBexp has error ! " << endl ;

        } // if
        else return false ;

        if ( NextIsBooleanOp() ) {
          temp = gScanner.GetAToken() ;
          throw new Exception( SYNTACTIC_ERR, temp->mContent ) ; 
        } // if
        else return true ;
 
      } // if
      else return true ;  
    }
  } // NOT_IDStartArithExpOrBexp

  bool NextIsBooleanOp() { // 偷看下一個token是不是屬於BooleanOperator

    TokenPtr peektoken = gScanner.PeekNextToken() ;

    if ( peektoken->mContent == "=" || peektoken->mContent == "<>" || peektoken->mContent == ">" || 
      peektoken->mContent == ">=" || peektoken->mContent == "<=") {

      return true ;
    } // if 
    else {
      return false ;
    } // else

  } // CheckIsBooleanOp


};