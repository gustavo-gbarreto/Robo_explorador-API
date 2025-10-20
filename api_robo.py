import sqlite3
from flask import Flask, request, jsonify

app = Flask(__name__)

# --- CONFIGURAÇÃO DO BANCO DE DADOS ---
DATABASE_FILE = "dados_robo.db"

def init_db():
    """Cria a tabela no banco de dados, caso ela não exista."""
    conn = sqlite3.connect(DATABASE_FILE)
    cursor = conn.cursor()
    cursor.execute('''
        CREATE TABLE IF NOT EXISTS leituras (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp TEXT NOT NULL,
            temperatura_c REAL,
            umidade_pct REAL,
            luminosidade INTEGER,
            presenca INTEGER,
            probabilidade_vida REAL
        )
    ''')
    conn.commit()
    conn.close()

# --- ENDPOINTS DA API ---

@app.route('/', methods=['GET'])
def index():
    html_content = """
    <h1>API do Robô Explorador</h1>
    <p>Bem-vindo! Esta API foi criada para receber e armazenar os dados dos sensores do robô explorador.</p>
    <h2>Endpoints disponíveis:</h2>
    <ul>
        <li><code>POST /leituras</code>: Para enviar novos dados dos sensores (usado pelo ESP32).</li>
        <li><code>GET /leituras</code>: Para visualizar as últimas 100 leituras armazenadas.</li>
    </ul>
    """
    return html_content, 200

@app.route('/leituras', methods=['POST'])
def adicionar_leitura():
    """Recebe dados do ESP32 via POST e armazena no banco de dados."""
    dados = request.get_json()

    if not all(k in dados for k in ["timestamp", "temperatura_c", "umidade_pct", "luminosidade", "presenca", "probabilidade_vida"]):
        return jsonify({"erro": "Dados incompletos"}), 400

    try:
        conn = sqlite3.connect(DATABASE_FILE)
        cursor = conn.cursor()
        cursor.execute(
            "INSERT INTO leituras (timestamp, temperatura_c, umidade_pct, luminosidade, presenca, probabilidade_vida) VALUES (?, ?, ?, ?, ?, ?)",
            (
                dados['timestamp'],
                dados['temperatura_c'],
                dados['umidade_pct'],
                dados['luminosidade'],
                dados['presenca'],
                dados['probabilidade_vida']
            )
        )
        conn.commit()
        conn.close()
        return jsonify({"mensagem": "Leitura armazenada com sucesso!"}), 201
    except Exception as e:
        return jsonify({"erro": str(e)}), 500

@app.route('/leituras', methods=['GET'])
def obter_leituras():
    """Retorna as últimas 100 leituras do banco de dados."""
    try:
        conn = sqlite3.connect(DATABASE_FILE)
        conn.row_factory = sqlite3.Row 
        cursor = conn.cursor()
        cursor.execute("SELECT * FROM leituras ORDER BY id DESC LIMIT 100")
        
        leituras = [dict(row) for row in cursor.fetchall()]
        
        conn.close()
        return jsonify(leituras), 200
    except Exception as e:
        return jsonify({"erro": str(e)}), 500

# --- EXECUÇÃO DA APLICAÇÃO ---
if __name__ == '__main__':
    init_db()
    app.run(host='0.0.0.0', port=5000, debug=True)