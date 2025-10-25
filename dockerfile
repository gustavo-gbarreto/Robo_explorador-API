# 1. Imagem base
FROM python:3.9-slim

# 2. Define o diretório de trabalho
WORKDIR /app

# 3. Copia o arquivo de dependências
COPY requirements.txt .

# 4. Instala as dependências
RUN pip install --no-cache-dir -r requirements.txt

# 5. Copia todo o código da API para o contêiner
COPY . .

# 6. Expõe a porta que o Flask usa
EXPOSE 5000

# 7. Comando para rodar a API ao iniciar o contêiner
CMD ["python", "api_robo.py"]