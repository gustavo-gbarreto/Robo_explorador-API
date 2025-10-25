import os
from flask import Flask, request, jsonify
from pymongo import MongoClient
from pymongo.errors import ConnectionFailure
from bson import json_util # Essencial para converter BSON do Mongo para JSON
import json

app = Flask(__name__)

# --- CONFIGURAÇÃO DO MONGODB ---

# Esta variável de ambiente é injetada pelo docker-compose.yml
# "mongo_db" é o nome do serviço do banco de dados no docker-compose
MONGO_URI = os.environ.get('MONGO_URI', "mongodb://root:example@mongo:27017/robo_db?authSource=admin")

try:
    # Tenta conectar ao MongoDB
    client = MongoClient(MONGO_URI, serverSelectionTimeoutMS=5000)
    # Testa a conexão
    client.admin.command('ping') 
    print("✅ Conectado ao MongoDB com sucesso!")
    
    # Define o banco de dados e a coleção
    db = client.get_default_database() # Pega o banco "robo_db" da URI
    leituras_collection = db.leituras

except ConnectionFailure as e:
    print(f"❌ Erro ao conectar ao MongoDB: {e}")
    # Se não puder conectar, o contêiner será encerrado
    exit(1)


# --- ENDPOINTS DA API ---

@app.route('/', methods=['GET'])
def index():
    """Retorna uma página inicial amigável."""
    return """
    <h1>API do Robô Explorador (Docker + MongoDB)</h1>
    <p>API está funcionando e conectada ao MongoDB.</p>
    <p>Use <code>GET /leituras</code> para ver os dados ou <code>POST /leituras</code> para enviar.</p>
    """

@app.route('/leituras', methods=['POST'])
def adicionar_leitura():
    """Recebe dados do ESP32 via POST e armazena no MongoDB."""
    dados = request.get_json()

    # Validação simples
    if not dados or "timestamp" not in dados:
        return jsonify({"erro": "Dados incompletos ou mal formatados"}), 400

    try:
        # Insere o documento JSON (dicionário Python) diretamente na coleção
        result = leituras_collection.insert_one(dados)
        # Retorna o ID gerado pelo MongoDB
        return jsonify({"mensagem": "Leitura armazenada com sucesso!", "id_inserido": str(result.inserted_id)}), 201
    except Exception as e:
        return jsonify({"erro": f"Erro ao inserir no banco: {str(e)}"}), 500

@app.route('/leituras', methods=['GET'])
def obter_leituras():
    """Retorna as últimas 100 leituras do banco de dados."""
    try:
        # Busca os dados, ordena pelo campo "_id" decrescente (mais novos primeiro)
        # O _id do Mongo contém um timestamp, então ordenar por ele é eficiente
        cursor = leituras_collection.find().sort("_id", -1).limit(100)
        
        # Converte os resultados BSON do MongoDB para uma lista JSON válida
        # json_util.dumps cria uma string JSON, json.loads converte de volta para lista/dict
        leituras_json = json.loads(json_util.dumps(cursor))
        
        return jsonify(leituras_json), 200
    except Exception as e:
        return jsonify({"erro": f"Erro ao buscar dados: {str(e)}"}), 500

# --- EXECUÇÃO DA APLICAÇÃO ---
if __name__ == '__main__':
    # '0.0.0.0' é crucial para que o Flask seja acessível de fora do contêiner Docker
    app.run(host='0.0.0.0', port=5000, debug=True)