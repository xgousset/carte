#include "mapwidget.h"
#include <QGraphicsPixmapItem>
#include <QNetworkRequest>
#include <QDebug>
#include <cmath>
#include <QVBoxLayout>

MapWidget::MapWidget(QWidget *parent)
    : QGraphicsView(parent), zoomLevel(12)
{
    scene = new QGraphicsScene(this);
    setScene(scene);

    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &MapWidget::onTileDownloaded);

    // Ajouter un label de texte pour le débogage
    debugLabel = new QLabel(this);
    debugLabel->setText("Carte initialisée. En attente de chargement des tuiles...");
    debugLabel->setStyleSheet("QLabel { color: red; font-size: 16px; }");

    // Ajouter le label à la fenêtre
    QVBoxLayout *layout = new QVBoxLayout();
    layout->addWidget(debugLabel);
    layout->addWidget(this); // Ajouter la carte en dessous du label
    QWidget *container = new QWidget();
    container->setLayout(layout);
    setParent(container);

    setCenter(6.839349, 47.64263, zoomLevel); // Centrer sur les coordonnées WGS84
}

void MapWidget::setCenter(double latitude, double longitude, int zoom)
{
    zoomLevel = zoom;
    centerTile = latLonToTile(longitude, latitude, zoomLevel);
    centerOffset = QPointF(0, 0); // Ajuster si nécessaire

    // Mettre à jour le label avec les nouvelles coordonnées
    debugLabel->setText(QString("Centre de la carte : Lat %1, Lon %2, Zoom %3")
                            .arg(latitude)
                            .arg(longitude)
                            .arg(zoomLevel));

    updateTiles(); // Mettre à jour les tuiles
}

QPointF MapWidget::latLonToTile(double lat, double lon, int zoom)
{
    double latRad = lat * M_PI / 180.0;
    double n = pow(2.0, zoom);
    double xTile = n * ((lon + 180.0) / 360.0);
    double yTile = n * (1.0 - (log(tan(latRad) + 1.0 / cos(latRad)) / M_PI)) / 2.0;
    return QPointF(xTile, yTile);
}

QPointF MapWidget::tileToScreen(const QPointF &tilePos)
{
    double tileSize = 256.0; // Taille d'une tuile en pixels
    QPointF screenPos = (tilePos - centerTile) * tileSize + centerOffset;
    return screenPos;
}

void MapWidget::loadTile(int x, int y, int zoom)
{
    QString tileUrl = QString("https://tile.openstreetmap.org/%1/%2/%3.png").arg(zoom).arg(x).arg(y);
    QUrl url(tileUrl); // Créer un objet QUrl
    QNetworkRequest request(url); // Créer un objet QNetworkRequest avec l'URL

    // Ajouter un en-tête User-Agent
    request.setHeader(QNetworkRequest::UserAgentHeader, "MyQtMapApp/1.0");

    networkManager->get(request); // Passer l'objet QNetworkRequest à get
}

void MapWidget::onTileDownloaded(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        qDebug() << "Erreur réseau :" << reply->errorString();
        debugLabel->setText(QString("Erreur réseau : %1").arg(reply->errorString()));
        return;
    }

    QPixmap tile;
    tile.loadFromData(reply->readAll());
    if (tile.isNull()) {
        qDebug() << "Erreur : Impossible de charger la tuile.";
        debugLabel->setText("Erreur : Impossible de charger la tuile.");
        return;
    }

    QString url = reply->url().toString();
    QStringList parts = url.split('/');
    int zoom = parts[parts.size() - 3].toInt();
    int x = parts[parts.size() - 2].toInt();
    int y = parts[parts.size() - 1].split('.')[0].toInt();

    QPointF screenPos = tileToScreen(QPointF(x, y));
    QGraphicsPixmapItem *item = scene->addPixmap(tile);
    item->setPos(screenPos);

    // Mettre à jour le label avec la tuile chargée
    debugLabel->setText(QString("Tuile chargée : Zoom %1, X %2, Y %3").arg(zoom).arg(x).arg(y));
}



void MapWidget::centerScene()
{
    // Calculer la position centrale de la scène
    QRectF sceneRect = scene->itemsBoundingRect();
    QPointF center = sceneRect.center();

    // Centrer la vue sur la tuile centrale
    centerOn(center);
}

void MapWidget::updateTiles()
{
    // Nettoyer la scène avant de charger de nouvelles tuiles
    scene->clear();

    int tileX = static_cast<int>(centerTile.x());
    int tileY = static_cast<int>(centerTile.y());

    // Afficher les tuiles autour du centre
    for (int dx = -2; dx <= 2; ++dx) {
        for (int dy = -2; dy <= 2; ++dy) {
            int x = tileX + dx;
            int y = tileY + dy;
            loadTile(x, y, zoomLevel);
        }
    }

    // Centrer la scène sur la tuile centrale
    centerScene();
}

void MapWidget::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    centerScene(); // Recentrer la scène lors du redimensionnement
}

int MapWidget::getZoomLevel() const
{
    return zoomLevel; // Retourner le niveau de zoom actuel
}
