#include <iostream>
#include <vector>
#include <string>

int main(){
    std::vector<std::string> menu = {"Sair"};
    while(true){
        std::cout<<"Calculadora balística";
        for(int i = 0; i=menu.size(); i++){
            std::cout<< (i+1) << ". "<< menu[i] << std::endl;
        }
        int choice; 
        std::cout<<"Escolha: ";
        std::cin>>choice;
        if(choice == 1){
            break;
        }
    }
    
    return 0;
}
