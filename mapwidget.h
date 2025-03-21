#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QGraphicsView>
#include <QGraphicsScene>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointF>
#include <QLabel>

class MapWidget : public QGraphicsView
{
    Q_OBJECT

public:
    explicit MapWidget(QWidget *parent = nullptr);
    void setCenter(double latitude, double longitude, int zoom);
    int getZoomLevel() const; // Ajouter cette méthode

protected:
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onTileDownloaded(QNetworkReply *reply);

private:
    QGraphicsScene *scene;
    QNetworkAccessManager *networkManager;
    int zoomLevel;
    QPointF centerTile;
    QPointF centerOffset;
    QLabel *debugLabel;

    QPointF latLonToTile(double lat, double lon, int zoom);
    QPointF tileToScreen(const QPointF &tilePos);
    void loadTile(int x, int y, int zoom);
    void updateTiles();
};

#endif // MAPWIDGET_H
