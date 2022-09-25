/*__________________________________________________________________________________
|       Disciplina de Sistemas Embarcados - 2022-2
|       Prof. Eduardo Nunes dos Santos
| __________________________________________________________________________________
|
|		Lab 1
| __________________________________________________________________________________
*/
/*------------------------------------------------------------------------------
 *
 *      File includes
 *
 *------------------------------------------------------------------------------*/
#include <stdint.h>
#include <string>

#include <iostream>
using std::cout;
using std::cin;

#include "template.h"

/**
 * Main function.
 *
 * Essa funcao exibe na saida padrao a data atual, hora atual e caminho absoluto do 
 * arquivo principal do projeto. Depois requisita um numero do usuario e soma com o valor 5.8, 
 * exibindo o resultado da soma.
 *
 * @param[in] argc - not used, declared for compatibility
 * @param[in] argv - not used, declared for compatibility
 * @returns int    - not used, declared for compatibility
 */
int main(int argc, char ** argv)
{
    cout << "Lab 1 -  Mensagem inicial\n";
    cout << __DATE__;
    cout << "\n";
    cout << __TIME__;
    cout << "\n";
    cout << __FILE__;
    cout << "\n";
  
    float Num1;
    float Num2 = 5.8;
    float soma;
    
    cout << "Digite o número float:\n";
    cin >> Num1;
    
    soma = Num1 + Num2;
    
    cout << Num1;
    cout << '\n';
    cout << soma;
    

    return 0;
}
