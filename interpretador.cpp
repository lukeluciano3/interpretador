#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <sstream>
#include <vector>
#include <memory>

using namespace std;

// Estrutura que representa um nó de variável na árvore binária
struct NoVariavel {
    string nome;
    int valor;
    shared_ptr<NoVariavel> esquerda, direita;

    NoVariavel(string nome, int valor) : nome(nome), valor(valor), esquerda(nullptr), direita(nullptr) {}
};

// Classe que gerencia a árvore de variáveis
class ArvoreVariaveis {
private:
    shared_ptr<NoVariavel> raiz;

    // Função recursiva para inserir ou atualizar uma variável na árvore
    shared_ptr<NoVariavel> inserir(shared_ptr<NoVariavel> no, const string& nome, int valor) {
        if (!no) return make_shared<NoVariavel>(nome, valor);
        if (nome < no->nome) no->esquerda = inserir(no->esquerda, nome, valor);
        else if (nome > no->nome) no->direita = inserir(no->direita, nome, valor);
        else no->valor = valor;
        return no;
    }

    // Função recursiva para buscar o valor de uma variável na árvore
    int buscar(shared_ptr<NoVariavel> no, const string& nome) {
        if (!no) throw runtime_error("Variável não encontrada: " + nome);
        if (nome < no->nome) return buscar(no->esquerda, nome);
        else if (nome > no->nome) return buscar(no->direita, nome);
        return no->valor;
    }

public:
    // Define o valor de uma variável
    void definirVariavel(const string& nome, int valor) {
        raiz = inserir(raiz, nome, valor);
    }

    // Obtém o valor de uma variável
    int obterVariavel(const string& nome) {
        return buscar(raiz, nome);
    }
};

ArvoreVariaveis arvoreVariaveis;

// Função para dividir uma string com base em um delimitador
vector<string> dividir(const string& str, char delimitador) {
    vector<string> tokens;
    stringstream ss(str);
    string token;
    while (getline(ss, token, delimitador)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Função para remover espaços em branco no início e no final da string
string removerEspacos(const string& str) {
    size_t primeiro = str.find_first_not_of(' ');
    if (primeiro == string::npos) return "";
    size_t ultimo = str.find_last_not_of(' ');
    return str.substr(primeiro, ultimo - primeiro + 1);
}

// Função para avaliar uma expressão matemática
int avaliarExpressao(string expr) {
    stringstream ss(expr);
    string token;
    vector<int> termos;
    vector<char> operadores;

    // Separar os termos e operadores
    while (ss >> token) {
        token = removerEspacos(token);

        if (token == "+" || token == "-" || token == "*" || token == "/") {
            operadores.push_back(token[0]);
        } else {
            int termo = 0;
            try {
                if (isalpha(token[0])) {
                    // Se for uma variável, obter o seu valor
                    termo = arvoreVariaveis.obterVariavel(token);
                } else {
                    // Caso contrário, converter para número
                    termo = stoi(token);
                }
            } catch (const exception& e) {
                cerr << "Erro ao avaliar a expressão: " << e.what() << " para o token '" << token << "'" << endl;
                throw;
            }
            termos.push_back(termo);
        }
    }

    // Resolver multiplicação e divisão primeiro
    for (size_t i = 0; i < operadores.size(); ) {
        if (operadores[i] == '*' || operadores[i] == '/') {
            int esquerda = termos[i];
            int direita = termos[i + 1];
            int resultado = (operadores[i] == '*') ? esquerda * direita : esquerda / direita;

            termos[i] = resultado;
            termos.erase(termos.begin() + i + 1);
            operadores.erase(operadores.begin() + i);
        } else {
            i++;
        }
    }

    // Resolver adição e subtração
    int valor = termos[0];
    for (size_t i = 0; i < operadores.size(); i++) {
        if (operadores[i] == '+') {
            valor += termos[i + 1];
        } else if (operadores[i] == '-') {
            valor -= termos[i + 1];
        }
    }

    return valor;
}

// Função que executa uma linha de código BASIC
void executarLinha(const string& linha, size_t& linhaAtual, map<int, size_t>& rotulos, vector<string>& programa);

// Função que executa múltiplos comandos separados por ':' em uma única linha
void executarMultiplosComandos(string linha, size_t& linhaAtual, map<int, size_t>& rotulos, vector<string>& programa) {
    vector<string> comandos = dividir(linha, ':');
    for (const auto& comando : comandos) {
        string comandoAjustado = removerEspacos(comando);
        if (!comandoAjustado.empty()) {
            executarLinha(comandoAjustado, linhaAtual, rotulos, programa);
        }
    }
}

// Função que interpreta e executa uma linha específica do programa
void executarLinha(const string& linha, size_t& linhaAtual, map<int, size_t>& rotulos, vector<string>& programa) {
    vector<string> tokens = dividir(linha, ' ');
    if (tokens.empty()) return;

    if (tokens[0] == "PRINT") {
        string paraImprimir = linha.substr(6);
        vector<string> partes = dividir(paraImprimir, ';');

        for (size_t i = 0; i < partes.size(); ++i) {
            string parte = removerEspacos(partes[i]);

            size_t inicioAspas = parte.find('"');
            if (inicioAspas != string::npos) {
                size_t fimAspas = parte.find('"', inicioAspas + 1);
                if (fimAspas != string::npos) {
                    cout << parte.substr(inicioAspas + 1, fimAspas - inicioAspas - 1);
                }
            } else if (isalpha(parte[0])) {
                cout << arvoreVariaveis.obterVariavel(parte);
            } else {
                cout << avaliarExpressao(parte);
            }
        }
        cout << endl;
    } else if (tokens[0] == "LET") {
        // Atribui valor a uma variável
        string variavel = tokens[1];  
        string expressao = linha.substr(linha.find('=') + 1);  
        int resultado = avaliarExpressao(expressao);  
        arvoreVariaveis.definirVariavel(variavel, resultado);  
    } else if (tokens[0] == "INPUT") {
        size_t posAspas = linha.find('"');
        if (posAspas != string::npos) {
            size_t fimAspas = linha.find('"', posAspas + 1);
            if (fimAspas != string::npos) {
                string prompt = linha.substr(posAspas + 1, fimAspas - posAspas - 1);
                cout << prompt << " ";
            }
        }

        string variavel = tokens.back(); 
        int valor;
        cin >> valor;
        arvoreVariaveis.definirVariavel(variavel, valor);
    } else if (tokens[0] == "IF") {
        int lhs, rhs;
        try {
            lhs = avaliarExpressao(tokens[1]);
            rhs = avaliarExpressao(tokens[3]);
        } catch (const exception& e) {
            cerr << "Erro ao avaliar a condição IF: " << e.what() << endl;
            return;
        }

        bool condicaoVerdadeira = false;

        if (tokens[2] == "=") {
            condicaoVerdadeira = (lhs == rhs);
        } else if (tokens[2] == "<") {
            condicaoVerdadeira = (lhs < rhs);
        } else if (tokens[2] == ">") {
            condicaoVerdadeira = (lhs > rhs);
        }

        if (condicaoVerdadeira) {
            string aposThen = linha.substr(linha.find("THEN") + 4);
            executarMultiplosComandos(aposThen, linhaAtual, rotulos, programa);
        }

    } else if (tokens[0] == "GOTO") {
        try {
            int rotulo = stoi(tokens[1]);
            linhaAtual = rotulos.at(rotulo) - 1;
        } catch (const exception& e) {
            cerr << "Erro: rótulo de linha inválido no GOTO." << endl;
        }
    }
}

// Função principal que lê e executa o programa BASIC
int main() {
    ifstream arquivoBASIC("script.txt");
    if (!arquivoBASIC) {
        cerr << "Erro ao abrir o arquivo BASIC." << endl;
        return 1;
    }

    vector<string> programa;
    map<int, size_t> rotulos;

    string linha;
    size_t numeroLinha = 0;

    // Leitura do programa e detecção de rótulos
    while (getline(arquivoBASIC, linha)) {
        programa.push_back(linha);
        vector<string> tokens = dividir(linha, ' ');
        if (!tokens.empty() && isdigit(tokens[0][0])) {
            int rotulo = stoi(tokens[0]);
            rotulos[rotulo] = numeroLinha;
        }
        numeroLinha++;
    }

    size_t linhaAtual = 0;
    // Execução do programa linha por linha
    while (linhaAtual < programa.size()) {
        string atual = programa[linhaAtual];
        vector<string> tokens = dividir(atual, ' ');
        if (!tokens.empty() && isdigit(tokens[0][0])) {
            atual = atual.substr(atual.find(' ') + 1);
        }
        executarLinha(atual, linhaAtual, rotulos, programa);
        linhaAtual++;
    }

    return 0;
}
