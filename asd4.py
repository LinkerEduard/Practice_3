# Импортируем необходимые модули
import requests
import socket
import random
import string
from flask import Flask, request, redirect
from flask_cors import CORS

# Создаем объект приложения Flask
app = Flask(__name__)
CORS(app)

# Создаем функцию для генерации случайной строки из 6 символов
def generate_random_string():
    return "".join(random.choice(string.ascii_letters + string.digits) for _ in range(6))

# Создаем функцию для подключения к базе данных через сокет
def connect_to_database():
    # Создаем объект сокета
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    # Подключаемся к локальному хосту по порту 6379
    s.connect(("localhost", 6379))
    # Возвращаем объект сокета
    return s

# Создаем функцию для отправки запроса к базе данных
def send_query_to_database(query):
    # Получаем объект сокета
    s = connect_to_database()
    # Отправляем запрос в формате байтов
    s.sendall(query.encode())
    # Получаем ответ в формате байтов
    response = s.recv(1024)
    response=response.strip()
    # Закрываем сокет
    s.close()
    # Возвращаем ответ в формате строки
    return response.decode()

# Создаем конечную точку для сокращения ссылки
@app.route("/", methods=["POST"])
def shorten_url():
    # Получаем ссылку из тела запроса
    url = request.data.decode()
    # Проверяем, что ссылка начинается с http:// или https://
    if not url.startswith("http://") and not url.startswith("https://"):
        # Если нет, то добавляем http://
        url = "http://" + url
    # Проверяем, что ссылка валидна
    try:
        # Отправляем запрос к ссылке и получаем статус код
        status_code = requests.get(url).status_code
        # Если статус код не равен 200, то ссылка невалидна
        if (status_code != 200):
            # Возвращаем сообщение об ошибке
            return "Invalid URL"
    except:
        # Если возникло исключение, то ссылка невалидна
        # Возвращаем сообщение об ошибке
        return "Invalid URL"
    # Генерируем случайную строку для сокращения
    short = generate_random_string()
    # Проверяем, что ссылки не существует в базе данных
    temp_url = send_query_to_database(f"--file urls.txt --query HGET short_urls {url}")
    if temp_url=="False":
        # Добавляем сокращение и оригинальную ссылку в базу данных
        send_query_to_database(f"--file urls.txt --query HSET urls {short} {url}")
        send_query_to_database(f"--file urls.txt --query HSET short_urls {url} {short}")
        # Возвращаем сокращенную ссылку
        return f'{request.host_url}MDT/{short}'
    return f'{request.host_url}MDT/{short}'

# Создаем конечную точку для перехода по сокращенной ссылке
@app.route("/MDT/<short>", methods=["GET"])
def redirect_to_url(short):
    url = send_query_to_database(f"--file urls.txt --query HGET urls {short}")
    if url == "False":
        # Возвращаем сообщение об ошибке
        return "Invalid shortened URL"
    # Перенаправляем пользователя по оригинальной ссылке
    return redirect(url)

# Создаем конечную точку для отображения странички для браузера
@app.route("/page", methods=["GET"])
def show_page():
    # Возвращаем HTML-код странички
    return """
    <!DOCTYPE html>
    <html>
    <head>
        <title>Сокращатель ссылок</title>
        <style>
            /* Добавляем стили для элементов */
            h1 {
                text-align: center; /* Выравниваем заголовок по центру */
                font-family: Arial, sans-serif; /* Устанавливаем шрифт */
                color: #333333; /* Устанавливаем цвет текста */
            }
            p {
                text-align: center; /* Выравниваем абзацы по центру */
                font-family: Arial, sans-serif; /* Устанавливаем шрифт */
                color: #333333; /* Устанавливаем цвет текста */
            }
            input {
                display: block; /* Делаем поля для ввода блочными */
                margin: 10px auto; /* Добавляем отступы и выравниваем по центру */
                width: 80%; /* Устанавливаем ширину */
                height: 30px; /* Устанавливаем высоту */
                border: 2px solid #333333; /* Устанавливаем толщину и цвет границы */
                border-radius: 10px; /* Делаем углы поля для ввода скругленными */
                font-size: 20px; /* Устанавливаем размер шрифта */
            }
            button {
                display: block; /* Делаем кнопку блочной */
                margin: 10px auto; /* Добавляем отступы и выравниваем по центру */
                width: 30%; /* Устанавливаем ширину */
                height: 40px; /* Устанавливаем высоту */
                border: none; /* Убираем границу */
                border-radius: 10px; /* Делаем углы кнопки скругленными */
                background-color: #333333; /* Устанавливаем цвет фона */
                color: white; /* Устанавливаем цвет текста */
                font-size: 20px; /* Устанавливаем размер шрифта */
                cursor: pointer; /* Делаем курсор указателем при наведении на кнопку */
            }
        </style>
        <script>
            // Создаем функцию для отправки запроса на сокращение ссылки
            function shorten() {
                // Получаем элементы для ввода и вывода ссылок
                var input = document.getElementById("input");
                var output = document.getElementById("output");
                // Получаем ссылку из ввода
                var url = input.value;
                // Создаем объект XMLHttpRequest
                var xhr = new XMLHttpRequest();
                // Открываем соединение с сервером по методу POST и адресу /
                xhr.open("POST", "/");
                // Устанавливаем обработчик события загрузки ответа
                xhr.onload = function() {
                    // Если статус код равен 200, то запрос успешен
                    if (xhr.status == 200) {
                        // Получаем сокращенную ссылку из ответа
                        var short = xhr.responseText;
                        // Выводим сокращенную ссылку в элемент для вывода
                        output.value = short;
                    } else {
                        // Если статус код не равен 200, то запрос неуспешен
                        // Выводим сообщение об ошибке в элемент для вывода
                        output.value = "Error: " + xhr.statusText;
                    }
                };
                // Отправляем запрос с телом, содержащим ссылку
                xhr.send(url);
            }
        </script>
    </head>
    <body>
        <h1>Сокращатель ссылок</h1>
        <p>Введите ссылку, которую хотите сократить, и нажмите кнопку.</p>
        <input id="input" type="text" placeholder="http://example.com">
        <button onclick="shorten()">Сократить</button>
        <p>Сокращенная ссылка:</p>
        <input id="output" type="text" readonly>
    </body>
    </html>

    """

# Запускаем приложение на порту 5000
if __name__ == "__main__":
    app.run(port=5000)
