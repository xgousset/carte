#include "mainwindow.h"
#include "mapwidget.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QListWidget>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QDebug>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    // Création des widgets
    QWidget *centralWidget = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget); // Utiliser QHBoxLayout

    // Partie gauche : champ de recherche et liste des résultats
    QWidget *leftWidget = new QWidget(this);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftWidget);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    QLineEdit *searchInput = new QLineEdit(this);
    QPushButton *searchButton = new QPushButton("Search", this);
    searchLayout->addWidget(searchInput);
    searchLayout->addWidget(searchButton);
    leftLayout->addLayout(searchLayout);

    QListWidget *resultsList = new QListWidget(this);
    leftLayout->addWidget(resultsList);

    // Partie droite : carte
    MapWidget *mapWidget = new MapWidget(this);

    // Ajouter les deux parties à la mise en page principale
    mainLayout->addWidget(leftWidget, 1); // Partie gauche (1/3 de l'espace)
    mainLayout->addWidget(mapWidget, 2);  // Partie droite (2/3 de l'espace)

    setCentralWidget(centralWidget);

    // Initialisation de QNetworkAccessManager
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MainWindow::onSearchFinished);

    // Connexion du bouton de recherche
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::handleSearch);

    // Connexion de la sélection dans la liste des résultats
    connect(resultsList, &QListWidget::itemClicked, this, [this, mapWidget](QListWidgetItem *item) {


        QPointF coords = item->data(Qt::UserRole).toPointF(); // Récupérer les coordonnées

        //print les coordonnées pour debuguer
        qDebug() << coords;
        //print les coordonnées x et y pour debuguer
        qDebug() << coords.x();
        qDebug() << coords.y();
        mapWidget->setCenter(coords.y(), coords.x(), mapWidget->getZoomLevel()); // Mettre à jour la carte
    });
}

MainWindow::~MainWindow()
{
    delete networkManager;
}

void MainWindow::handleSearch()
{
    QLineEdit *searchInput = findChild<QLineEdit *>();
    if (!searchInput) {
        qDebug() << "Erreur : Impossible de trouver le champ de recherche.";
        return;
    }

    QString searchText = searchInput->text();
    if (searchText.isEmpty()) {
        return; // Ne rien faire si la zone de saisie est vide
    }

    // Construire l'URL de requête Nominatim
    QString url = QString("https://nominatim.openstreetmap.org/search?q=%1&format=json&polygon=0&addressdetails=0")
                      .arg(searchText);

    // Envoyer la requête HTTP
    networkManager->get(QNetworkRequest(QUrl(url)));
}

void MainWindow::onSearchFinished(QNetworkReply *reply)
//print la reply pour debuguer
{
    qDebug() << reply;
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Erreur réseau :" << reply->errorString();
        return;
    }

    // Lire la réponse JSON
    QByteArray responseData = reply->readAll();
    QJsonDocument jsonDoc = QJsonDocument::fromJson(responseData);
    if (jsonDoc.isNull()) {
        qDebug() << "Erreur : Impossible de parser la réponse JSON.";
        return;
    }

    QJsonArray resultsArray = jsonDoc.array();
    QListWidget *resultsList = findChild<QListWidget *>();
    if (!resultsList) {
        qDebug() << "Erreur : Impossible de trouver la liste des résultats.";
        return;
    }

    resultsList->clear(); // Vider la liste actuelle

    // Afficher les résultats dans la liste
    for (const QJsonValue &resultValue : resultsArray) {
        QJsonObject resultObject = resultValue.toObject();
        //print le resultObject pour debuguer
        qDebug() << resultObject;
        QString displayName = resultObject["display_name"].toString();
        double lat = resultObject["lat"].toString().toDouble();
        double lon = resultObject["lon"].toString().toDouble();
        fprintf(stdout, "lat: %f, lon: %f\n", lat, lon);


        // Ajouter le résultat à la liste
        QListWidgetItem *item = new QListWidgetItem(displayName, resultsList);
        item->setData(Qt::UserRole, QPointF(lat, lon)); // Stocker les coordonnées
        //print les points pour debuguer
    }
}
