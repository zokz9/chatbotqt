#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QLabel>
#include <QDebug> // Для отладки

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // Включаем перенос текста в QLabel
    ui->label->setWordWrap(true); // Убедитесь, что у вас есть QLabel с objectName "label" в вашем UI

    // Инициализация менеджера сетевых запросов
    networkManager = new QNetworkAccessManager(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    try {
        // Получаем текст из QTextEdit
        QString userInput = ui->textEdit->toPlainText(); // Используем toPlainText() для QTextEdit

        // Если текст пустой, выводим сообщение и завершаем функцию
        if (userInput.isEmpty()) {
            ui->label->setText("Введите запрос!");
            return;
        }

        qDebug() << "Запрос отправлен: " << userInput; // Отладка

        // URL API OpenRouter
        QUrl url("https://openrouter.ai/api/v1/chat/completions");

        // Создание JSON-запроса
        QJsonObject json;
        json["model"] = "google/gemini-2.0-pro-exp-02-05:free";
        QJsonArray messages;
        QJsonObject message;
        message["role"] = "user";
        message["content"] = userInput;
        messages.append(message);
        json["messages"] = messages;

        // Преобразование JSON в QByteArray
        QJsonDocument doc(json);
        QByteArray data = doc.toJson();

        // Создание сетевого запроса
        QNetworkRequest request(url);
        request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
        request.setRawHeader("Authorization", "Bearer YOUR_API_KEY"); // Замените YOUR_API_KEY на ваш API-ключ

        // Отладка заголовков
        qDebug() << "Заголовки запроса:";
        qDebug() << "Content-Type:" << request.header(QNetworkRequest::ContentTypeHeader);
        qDebug() << "Authorization:" << request.rawHeader("Authorization");

        // Отправка POST-запроса
        QNetworkReply *reply = networkManager->post(request, data);

        // Подключение сигнала завершения запроса к слоту
        connect(reply, &QNetworkReply::finished, this, [this, reply]() {
            if (reply->error() == QNetworkReply::NoError) {
                // Чтение ответа
                QByteArray response = reply->readAll();
                QJsonDocument jsonResponse = QJsonDocument::fromJson(response);
                QJsonObject jsonObject = jsonResponse.object();

                // Извлечение текста ответа
                QString responseText = jsonObject["choices"].toArray()[0].toObject()["message"].toObject()["content"].toString();

                qDebug() << "Ответ получен: " << responseText; // Отладка

                // Удаление LaTeX-кода (если он всё ещё есть)
                responseText.replace("\\boxed{", "").replace("}", "").replace("\\text{", "").replace("}", "");

                // Запись ответа в QLabel
                ui->label->setText(responseText); // Убедитесь, что у вас есть QLabel с objectName "label" в вашем UI
            } else {
                // Обработка ошибки
                QString errorText = "Ошибка: " + reply->errorString();
                qDebug() << errorText; // Отладка
                ui->label->setText(errorText); // Вывод ошибки в QLabel
            }

            // Освобождение ресурсов
            reply->deleteLater();

            // Очистка QTextEdit после отправки запроса
            ui->textEdit->clear(); // Очищаем QTextEdit
        });
    } catch (const std::exception &e) {
        qDebug() << "Исключение: " << e.what();
        ui->label->setText("Произошла ошибка: " + QString(e.what()));
    }
}
