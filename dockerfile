# 1. Imagem base do Python
# Usamos 'slim' para uma imagem menor e mais leve
FROM python:3.10-slim

# 2. Define o diretório de trabalho dentro do contêiner
WORKDIR /app

# 3. Copia o arquivo de requisitos
COPY requirements.txt requirements.txt

# 4. Instala as dependências do Python
#    --no-cache-dir: não guarda o cache para manter a imagem leve
#    -r requirements.txt: instala tudo que está no arquivo
RUN pip install --no-cache-dir -r requirements.txt

# 5. Copia o resto dos arquivos do projeto (o seu app.py)
COPY . .

# 6. Comando para rodar a aplicação quando o contêiner iniciar
#    Executa o app.py usando o python
CMD ["python", "app.py"]
