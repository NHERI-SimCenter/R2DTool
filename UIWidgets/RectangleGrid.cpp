#include "RectangleGrid.h"
#include "NodeHandle.h"
#include "GridNode.h"
#include "SiteConfig.h"
#include "Site.h"
#include "VisualizationWidget.h"
#include "RuptureWidget.h"

#include <QObject>
#include <QDrag>
#include <QMimeData>
#include <QPainter>
#include <QPixmap>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QApplication>
#include <QRandomGenerator>
#include <QBitmap>
#include <QWidget>
#include <QDebug>

RectangleGrid::RectangleGrid(QObject* parent) : QObject(parent)
{
    GMSiteConfig = nullptr;

    setCacheMode(DeviceCoordinateCache);
    setZValue(-1);

    changingDimensions = false;
    updateConnectedWidgets = true;

    latMin = 0.0;
    lonMin = 0.0;
    latMax = 0.0;
    lonMax = 0.0;

    numDivisionsHoriz = 5;
    numDivisionsVertical = 5;

    color.setRgb(0,0,255,30);

    auto width = 150;
    auto height = 150;

    rectangleGeometry.setX(0);
    rectangleGeometry.setY(0);
    rectangleGeometry.setWidth(width);
    rectangleGeometry.setHeight(height);

    bottomLeftNode = new NodeHandle(this);
    bottomRightNode = new NodeHandle(this);
    topRightNode = new NodeHandle(this);
    topLeftNode = new NodeHandle(this);
    centerNode = new NodeHandle(this);

    centerNode->setColor(QColor(255,0,0,100));
    centerNode->setToolTip("Rupture Location");
    centerNode->setDiameter(20.0);

    connect(bottomLeftNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleBottomLeftCornerChanged);
    connect(bottomRightNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleBottomRightCornerChanged);
    connect(topRightNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleTopRightCornerChanged);
    connect(topLeftNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleTopLeftCornerChanged);
    connect(centerNode,&NodeHandle::positionChanged,this,&RectangleGrid::handleCenterNodeChanged);

    this->updateGeometry();
}


RectangleGrid::~RectangleGrid()
{

}


QRectF RectangleGrid::boundingRect() const
{
    qreal adjust = 5;
    return QRectF(rectangleGeometry.x() - adjust, rectangleGeometry.y() - adjust, rectangleGeometry.width() + 5 + adjust, rectangleGeometry.height() + 5 + adjust);
}


QPainterPath RectangleGrid::shape() const
{
    QPainterPath path;
    path.addRect(rectangleGeometry);
    return path;
}


void RectangleGrid::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setPen(Qt::NoPen);
    painter->setBrush(QBrush(color));
    painter->drawRect(rectangleGeometry);
}


void RectangleGrid::updateGeometry(void)
{
    auto bottomLeftPnt = rectangleGeometry.bottomLeft();
    auto topRightPnt = rectangleGeometry.topRight();
    auto centerPnt = rectangleGeometry.center();

    bottomLeftNode->setPos(bottomLeftPnt);
    bottomRightNode->setPos(rectangleGeometry.bottomRight());
    topRightNode->setPos(topRightPnt);
    topLeftNode->setPos(rectangleGeometry.topLeft());
    centerNode->setPos(centerPnt);

    if(GMSiteConfig && updateConnectedWidgets)
    {
        latMin = theVisWidget->getLatFromScreenPoint(bottomLeftPnt);
        latMax = theVisWidget->getLatFromScreenPoint(topRightPnt);

        lonMin = theVisWidget->getLongFromScreenPoint(bottomLeftPnt);
        lonMax = theVisWidget->getLongFromScreenPoint(topRightPnt);

        GMSiteConfig->siteGrid().latitude().set(latMin, latMax, numDivisionsHoriz);
        GMSiteConfig->siteGrid().longitude().set(lonMin, lonMax, numDivisionsVertical);

//        auto centerPointLat = theVisWidget->getLatFromScreenPoint(centerPnt);
//        auto centerPointLong = theVisWidget->getLongFromScreenPoint(centerPnt);

//        theRuptureWidget->setLocation(centerPointLat,centerPointLong);
    }

    emit geometryChanged();
}

QVector<GridNode *> RectangleGrid::getGridNodeVec() const
{
    return gridNodeVec;
}


size_t RectangleGrid::getNumDivisionsVertical() const
{
    return numDivisionsVertical;
}


void RectangleGrid::setNumDivisionsVertical(const size_t &value)
{
    numDivisionsVertical = value;
}


size_t RectangleGrid::getNumDivisionsHoriz() const
{
    return numDivisionsHoriz;
}


void RectangleGrid::setNumDivisionsHoriz(const size_t &value)
{
    numDivisionsHoriz = value;
}


//void RectangleGrid::setRuptureWidget(RuptureWidget *value)
//{
//    theRuptureWidget = value;
    
//    connect(&theRuptureWidget->getRuptureSource().location(), &RuptureLocation::latitudeChanged, this, &RectangleGrid::handleRuptureLocationChanged);
//    connect(&theRuptureWidget->getRuptureSource().location(), &RuptureLocation::longitudeChanged, this, &RectangleGrid::handleRuptureLocationChanged);
//}


void RectangleGrid::setVisualizationWidget(VisualizationWidget *value)
{
    theVisWidget = value;
}


void RectangleGrid::setGMSiteConfig(SiteConfig *value)
{
    GMSiteConfig = value;

    // Connect grid latitude
    connect(&GMSiteConfig->siteGrid().latitude(), &GridDivision::minChanged, this, &RectangleGrid::handleLatLonChanged);
    connect(&GMSiteConfig->siteGrid().latitude(), &GridDivision::maxChanged, this, &RectangleGrid::handleLatLonChanged);

    // Connect grid longitude
    connect(&GMSiteConfig->siteGrid().longitude(), &GridDivision::minChanged, this, &RectangleGrid::handleLatLonChanged);
    connect(&GMSiteConfig->siteGrid().longitude(), &GridDivision::maxChanged, this, &RectangleGrid::handleLatLonChanged);

    // Connect the grid discretization
    connect(&GMSiteConfig->siteGrid().latitude(), &GridDivision::divisionsChanged, this, &RectangleGrid::handleGridDivisionsChanged);
    connect(&GMSiteConfig->siteGrid().longitude(), &GridDivision::divisionsChanged, this, &RectangleGrid::handleGridDivisionsChanged);
}


void RectangleGrid::setPos(const QPoint& pos)
{
    this->rectangleGeometry.moveCenter(pos);
    this->updateGeometry();
}


void RectangleGrid::setWidth(const double& val)
{
    this->rectangleGeometry.setWidth(val);
    this->updateGeometry();
}


void RectangleGrid::setHeight(const double& val)
{
    this->rectangleGeometry.setHeight(val);
    this->updateGeometry();
}


QVariant RectangleGrid::itemChange(GraphicsItemChange change, const QVariant &value)
{
    switch (change) {
    case ItemPositionHasChanged:
        emit geometryChanged();
        break;
    default:
        break;
    };

    return QGraphicsItem::itemChange(change, value);
}


void RectangleGrid::mousePressEvent(QGraphicsSceneMouseEvent *)
{
}


void RectangleGrid::mouseReleaseEvent(QGraphicsSceneMouseEvent *)
{
}


NodeHandle *RectangleGrid::getCenterNode() const
{
    return centerNode;
}


NodeHandle *RectangleGrid::getTopLeftNode() const
{
    return topLeftNode;
}


NodeHandle *RectangleGrid::getTopRightNode() const
{
    return topRightNode;
}


NodeHandle *RectangleGrid::getBottomRightNode() const
{
    return bottomRightNode;
}


NodeHandle *RectangleGrid::getBottomLeftNode() const
{
    return bottomLeftNode;
}


void RectangleGrid::setTopLeftNode(NodeHandle *value)
{
    topLeftNode = value;
}


void RectangleGrid::setTopRightNode(NodeHandle *value)
{
    topRightNode = value;
}


void RectangleGrid::setTopRightNode(const double latitude, const double longitude)
{
    auto scrnPnt = theVisWidget->getScreenPointFromLatLong(latitude,longitude);

    this->handleTopRightCornerChanged(scrnPnt);
}


void RectangleGrid::setCenterNode(const double latitude, const double longitude)
{
    auto scrnPnt = theVisWidget->getScreenPointFromLatLong(latitude,longitude);

    this->handleCenterNodeChanged(scrnPnt);
}


void RectangleGrid::setBottomRightNode(NodeHandle *value)
{
    bottomRightNode = value;
}


void RectangleGrid::setBottomLeftNode(NodeHandle *value)
{
    bottomLeftNode = value;
}


void RectangleGrid::setBottomLeftNode(const double latitude, const double longitude)
{
    auto scrnPnt = theVisWidget->getScreenPointFromLatLong(latitude,longitude);

    this->handleBottomLeftCornerChanged(scrnPnt);
}


void RectangleGrid::handleBottomLeftCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setBottomLeft(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleBottomRightCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setBottomRight(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleTopLeftCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setTopLeft(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleTopRightCornerChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    changingDimensions = true;
    rectangleGeometry.setTopRight(pos.toPoint());
    this->updateGeometry();
    changingDimensions = false;
}


void RectangleGrid::handleCenterNodeChanged(const QPointF& pos)
{
    if(changingDimensions == true)
        return;

    this->rectangleGeometry.moveCenter(pos.toPoint());
    bottomLeftNode->setPos(rectangleGeometry.bottomLeft());
    bottomRightNode->setPos(rectangleGeometry.bottomRight());
    topRightNode->setPos(rectangleGeometry.topRight());
    topLeftNode->setPos(rectangleGeometry.topLeft());
}


void RectangleGrid::clearGrid()
{
    qDeleteAll(gridNodeVec);
    gridNodeVec.clear();
}


void RectangleGrid::createGrid()
{
    auto ni = numDivisionsHoriz;
    auto nj = numDivisionsVertical;

    gridNodeVec.reserve(ni*nj);

    for (size_t i=0; i<=ni; ++i)
    {
        for (size_t j=0;j<=nj; ++j)
        {

            // X COORDINATE
            std::function<double(void)> xCoordinatelambda = [=]
            {
                auto divisor = static_cast<double>(ni*nj);

                auto n1X = bottomLeftNode->pos().x();
                auto n2X = bottomRightNode->pos().x();
                auto n3X = topRightNode->pos().x();
                auto n4X = topLeftNode->pos().x();

                return static_cast<double>(i*j*n1X-i*j*n2X+i*j*n3X-i*j*n4X-i*n1X*nj+i*n2X*nj-j*n1X*ni+j*n4X*ni+n1X*ni*nj)/(divisor);
            };

            // Y COORDINATE
            std::function<double(void)> yCoordinatelambda = [=]
            {
                auto divisor = static_cast<double>(ni*nj);

                auto n1Y = bottomLeftNode->pos().y();
                auto n2Y = bottomRightNode->pos().y();
                auto n3Y = topRightNode->pos().y();
                auto n4Y = topLeftNode->pos().y();

                return static_cast<double>(i*j*n1Y-i*j*n2Y+i*j*n3Y-i*j*n4Y-i*n1Y*nj+i*n2Y*nj-j*n1Y*ni+j*n4Y*ni+n1Y*ni*nj)/(divisor);
            };

            auto x = xCoordinatelambda();
            auto y = yCoordinatelambda();

            // Create the grid node
            GridNode* newGridNode = new GridNode(this);

            newGridNode->setX(x);
            newGridNode->setY(y);

            newGridNode->setXPos(xCoordinatelambda);
            newGridNode->setYPos(yCoordinatelambda);

            gridNodeVec.push_back(newGridNode);

            connect(this,&RectangleGrid::geometryChanged,newGridNode,&GridNode::updateGeometry);
        }
    }
}


void RectangleGrid::handleGridDivisionsChanged(void)
{
    auto numDivH = static_cast<size_t>(GMSiteConfig->siteGrid().longitude().divisions());
    auto numDivV = static_cast<size_t>(GMSiteConfig->siteGrid().latitude().divisions());

    if(numDivV == this->numDivisionsVertical && numDivH == this->numDivisionsHoriz)
        return;

    this->numDivisionsVertical = numDivV;
    this->numDivisionsHoriz = numDivH;

    this->clearGrid();
    this->createGrid();
}


void RectangleGrid::handleLatLonChanged(void)
{
    if(changingDimensions == true)
        return;

    auto lat_Min = GMSiteConfig->siteGrid().latitude().min();
    auto lat_Max = GMSiteConfig->siteGrid().latitude().max();

    auto lon_Min = GMSiteConfig->siteGrid().longitude().min();
    auto lon_Max = GMSiteConfig->siteGrid().longitude().max();

    if(lat_Min != latMin || lon_Min != lonMin)
    {
        updateConnectedWidgets = false;
        latMin = lat_Min;
        lonMin = lon_Min;
        this->setBottomLeftNode(lat_Min,lon_Min);
        updateConnectedWidgets = true;
    }

    if(lat_Max != latMax || lon_Max != lonMax)
    {
        updateConnectedWidgets = false;
        latMax = lat_Max;
        lonMax = lon_Max;
        this->setTopRightNode(lat_Max,lon_Max);
        updateConnectedWidgets = true;
    }
}


//void RectangleGrid::handleRuptureLocationChanged(void)
//{
//    updateConnectedWidgets = false;
//    auto lat = theRuptureWidget->getRuptureSource().location().latitude();
//    auto lon = theRuptureWidget->getRuptureSource().location().longitude();
//    this->setCenterNode(lat,lon);
//    updateConnectedWidgets = true;

//}

