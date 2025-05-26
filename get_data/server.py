#test: curl -X POST -F "file=@/home/anna/Pictures/LARIAT_bright.JPG" http://10.2.119.172:4000/upload
# potrzebujemy fist, palm, ok, peace (V) i stop

from flask import Flask, request
import os

app = Flask(__name__)

# Określ folder, w którym będą zapisywane zdjęcia
UPLOAD_FOLDER = 'data'
os.makedirs(UPLOAD_FOLDER, exist_ok=True)
app.config['UPLOAD_FOLDER'] = UPLOAD_FOLDER

@app.route('/upload', methods=['POST'])
def upload_file():
    if 'file' not in request.files:
        return 'No file part', 400
    file = request.files['file']
    if file.filename == '':
        return 'No selected file', 400
    if file:
        # Zapisz plik na serwerze
        print(f'Received file: {file.filename}')
        filepath = os.path.join(app.config['UPLOAD_FOLDER'], file.filename)
        file.save(filepath)
        return f'File uploaded successfully: {file.filename}', 200

if __name__ == '__main__':
    app.run(host='10.2.119.172', port=4000)
