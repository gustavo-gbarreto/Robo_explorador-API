# Robô Explorador Espacial com ESP32 e Python

## 📝 Visão Geral do Projeto

Este projeto consiste em um sistema completo de Internet das Coisas (IoT) para a exploração de um planeta simulado. O sistema é composto por três partes principais:

1.  **Controle Remoto (Wokwi):** Um ESP32 simulado com um joystick para controlar os movimentos do robô.
2.  **Robô Físico (ESP32):** Um robô equipado com sensores (DHT22, LDR, PIR) que coleta dados do ambiente, calcula a "probabilidade de vida" e envia alertas.
3.  **Backend (Python/Flask):** Uma API que recebe os dados do robô via HTTP POST e os armazena em um banco de dados SQLite para consulta posterior.

Este repositório contém todos os códigos e instruções necessários para montar e executar o projeto completo.

---

## 🛠️ Componentes Utilizados

### Controle Remoto (Wokwi)
* ESP32
* Joystick Analógico
* LEDs (Verde e Vermelho) para indicar status
* Botão para desligamento de emergência

### Robô Físico
* ESP32
* Sensor de Temperatura e Umidade (DHT22)
* Fotorresistor (LDR) para medir luminosidade
* Sensor de Presença (PIR)
* 2x Motores de Rotação Contínua (Servos)
* LED Verde (Status: Operando)
* LED Vermelho (Status: Desligado ou Alerta)

---

## 🚀 Instruções de Execução

Siga os passos abaixo para replicar o projeto.

### 1. Como Montar o Robô

A montagem do circuito físico deve seguir o mapeamento de pinos definido no firmware (`firmware-robo-fisico/sketch.ino`).

| Componente              | Pino no ESP32 |
| ----------------------- | ------------- |
| Sensor DHT22 (DATA)     | GPIO 25       |
| Sensor PIR (OUT)        | GPIO 26       |
| Fotorresistor (LDR AO)  | GPIO 34       |
| LED Verde               | GPIO 12       |
| LED Vermelho            | GPIO 14       |
| Motor Esquerdo (ServoL) | GPIO 33       |
| Motor Direito (ServoR)  | GPIO 19       |

**Observações:**
* Alimente os sensores e motores com 3.3V/5V e GND, conforme a necessidade de cada componente.
* Certifique-se de que os motores estejam conectados corretamente para que os comandos de "Frente" e "Trás" funcionem como esperado.

### 2. Como Rodar o Backend Python

O backend é responsável por receber e armazenar os dados dos sensores. Ele pode ser executado localmente ou na nuvem (usando GitHub Codespaces).

**Pré-requisitos:**
* Python 3 instalado
* `pip` (gerenciador de pacotes do Python)

**Passos:**

1.  **Clone o repositório:**
    ```bash
    git clone [https://github.com/SEU-USUARIO/iot-esp32-robo-espacial.git](https://github.com/SEU-USUARIO/iot-esp32-robo-espacial.git)
    cd iot-esp32-robo-espacial/backend
    ```

2.  **Crie e ative um ambiente virtual (recomendado):**
    ```bash
    # Criar ambiente
    python -m venv venv

    # Ativar no Windows
    .\venv\Scripts\Activate

    # Ativar no macOS/Linux
    source venv/bin/activate
    ```

3.  **Instale as dependências:**
    ```bash
    pip install Flask
    ```

4.  **Execute a API:**
    ```bash
    python api_robo.py
    ```
    O servidor será iniciado na porta 5000. O banco de dados `dados_robo.db` será criado automaticamente no primeiro uso.

### 3. Como Consultar os Dados Salvos

Com o backend em execução, você pode consultar os dados de duas formas:

1.  **Via Navegador (GET):**
    * Abra seu navegador e acesse o endpoint `/leituras`.
    * **Se rodando localmente:** `http://127.0.0.1:5000/leituras`
    * **Se rodando no Codespaces:** Use a URL pública fornecida na aba "PORTS" e adicione `/leituras` no final.
    * O navegador exibirá as últimas 100 leituras em formato JSON.

2.  **Diretamente no Banco de Dados (SQLite):**
    * Você pode usar uma ferramenta como o **DB Browser for SQLite** para abrir o arquivo `dados_robo.db` e visualizar a tabela `leituras` de forma gráfica.
