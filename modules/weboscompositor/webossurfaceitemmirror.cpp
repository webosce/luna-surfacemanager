// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include "weboscompositorwindow.h"
#include "webossurfaceitem.h"
#include "webossurfaceitemmirror.h"

WebOSSurfaceItemMirror::WebOSSurfaceItemMirror()
        : QQuickItem()
{
    qDebug() << "WebOSSurfaceItemMirror's constructor called";

    // Allow all input by default
    setAcceptHoverEvents(true);
    setAcceptTouchEvents(true);
    setAcceptedMouseButtons(Qt::AllButtons);
}

WebOSSurfaceItemMirror::~WebOSSurfaceItemMirror()
{
    qDebug() << "WebOSSurfaceItemMirror's destructor called";

    setSourceItem(nullptr);
}

void WebOSSurfaceItemMirror::setSourceItem(WebOSSurfaceItem *sourceItem)
{
    qDebug() << "setSourceItem to replace" << m_sourceItem << "with" << sourceItem;

    if (m_sourceItem != sourceItem) {
        if (m_sourceItem) {
            disconnect(m_widthChangedConnection);
            disconnect(m_heightChangedConnection);
            disconnect(m_sourceDestroyedConnection);
            disconnect(m_mirrorDestroyedConnection);

            if (m_mirrorItem) {
                if (!m_sourceItem->removeMirrorItem(m_mirrorItem))
                    qCritical() << "Failed to remove mirror item";

                delete m_mirrorItem;
                m_mirrorItem = nullptr;
            }
        }

        if (sourceItem) {
            if (sourceItem->isMirrorItem() && sourceItem->mirrorSource()) {
                qDebug() << "Source item is already mirrored, use its mirror source" << sourceItem->mirrorSource();
                sourceItem = sourceItem->mirrorSource();
            }

            m_mirrorItem = sourceItem->createMirrorItem();
            if (m_mirrorItem) {
                m_mirrorItem->setParentItem(this);
                m_mirrorItem->setSizeFollowsSurface(false);
                m_mirrorItem->setSize(size());

                m_widthChangedConnection = connect(this, &QQuickItem::widthChanged, [this]() {
                    if (m_mirrorItem)
                        m_mirrorItem->setWidth(width());
                });
                m_heightChangedConnection = connect(this, &QQuickItem::heightChanged, [this]() {
                    if (m_mirrorItem)
                        m_mirrorItem->setHeight(height());
                });
                m_sourceDestroyedConnection = connect(sourceItem, &WebOSSurfaceItem::itemAboutToBeDestroyed, [this]() {
                    qDebug() << "Source(" << m_sourceItem << ")'s surface is about to be destroyed";
                    setSourceItem(nullptr);
                });
                m_mirrorDestroyedConnection = connect(m_mirrorItem, &WebOSSurfaceItem::itemAboutToBeDestroyed, [this]() {
                    qDebug() << "Mirror(" << m_mirrorItem << ")'s surface is about to be destroyed";
                    m_mirrorItem = nullptr;
                });
            } else {
                qWarning() << "Failed to start mirroring for" << sourceItem;
                return;
            }
        }

        m_sourceItem = sourceItem;
        emit sourceItemChanged();
    }
}

void WebOSSurfaceItemMirror::setClustered(bool clustered)
{
    if (m_clustered != clustered) {
        qDebug() << "Set clustered to" << clustered;
        m_clustered = clustered;
        emit clusteredChanged();
    }
}

void WebOSSurfaceItemMirror::setPropagateEvents(bool propagateEvents)
{
    if (m_propagateEvents != propagateEvents) {
        qDebug() << "Set propagateEvents to" << propagateEvents;
        m_propagateEvents = propagateEvents;
        emit propagateEventsChanged();
    }
}

void WebOSSurfaceItemMirror::hoverMoveEvent(QHoverEvent *event)
{
    if (!needToPropagate(event))
        return;

    QHoverEvent he(event->type(), translatePoint(event->pos()), translatePoint(event->oldPos()));
    QCoreApplication::sendEvent(m_sourceItem, &he);
}

void WebOSSurfaceItemMirror::hoverEnterEvent(QHoverEvent *event)
{
    if (!needToPropagate(event))
        return;

    QHoverEvent he(event->type(), translatePoint(event->pos()), translatePoint(event->oldPos()));
    QCoreApplication::sendEvent(m_sourceItem, &he);
}

void WebOSSurfaceItemMirror::hoverLeaveEvent(QHoverEvent *event)
{
    if (!needToPropagate(event))
        return;

    QHoverEvent he(event->type(), translatePoint(event->pos()), translatePoint(event->oldPos()));
    QCoreApplication::sendEvent(m_sourceItem, &he);
}

void WebOSSurfaceItemMirror::keyPressEvent(QKeyEvent *event)
{
    if (!needToPropagate(event))
        return;

    QCoreApplication::sendEvent(m_sourceItem, event);
}

void WebOSSurfaceItemMirror::keyReleaseEvent(QKeyEvent *event)
{
    if (!needToPropagate(event))
        return;

    QCoreApplication::sendEvent(m_sourceItem, event);
}

void WebOSSurfaceItemMirror::mouseMoveEvent(QMouseEvent *event)
{
    if (!needToPropagate(event))
        return;

    QMouseEvent me(event->type(), translatePoint(event->localPos()),
                   event->button(), event->buttons(), event->modifiers());
    QCoreApplication::sendEvent(m_sourceItem, &me);
}

void WebOSSurfaceItemMirror::mousePressEvent(QMouseEvent *event)
{
    if (!needToPropagate(event))
        return;

    QMouseEvent me(event->type(), translatePoint(event->localPos()),
                   event->button(), event->buttons(), event->modifiers());
    QCoreApplication::sendEvent(m_sourceItem, &me);
}

void WebOSSurfaceItemMirror::mouseReleaseEvent(QMouseEvent *event)
{
    if (!needToPropagate(event))
        return;

    QMouseEvent me(event->type(), translatePoint(event->localPos()),
                   event->button(), event->buttons(), event->modifiers());
    QCoreApplication::sendEvent(m_sourceItem, &me);
}

void WebOSSurfaceItemMirror::wheelEvent(QWheelEvent *event)
{
    if (!needToPropagate(event))
        return;

    QWheelEvent we(translatePoint(event->pos()), translatePoint(event->globalPos()),
                   event->pixelDelta(), event->angleDelta(),
                   event->buttons(), event->modifiers(),
                   event->phase(), event->inverted(), event->source());
    QCoreApplication::sendEvent(m_sourceItem, &we);
}

void WebOSSurfaceItemMirror::touchEvent(QTouchEvent *event)
{
    if (!needToPropagate(event))
        return;

    QList<QTouchEvent::TouchPoint> touchPoints;
    foreach (QTouchEvent::TouchPoint point, event->touchPoints()) {
        point.setPos(translatePoint(point.scenePos()));
        touchPoints.append(point);
    }

    QTouchEvent te(event->type(), event->device(),
                   event->modifiers(), event->touchPointStates(), touchPoints);
    QCoreApplication::sendEvent(m_sourceItem, &te);
}

bool WebOSSurfaceItemMirror::needToPropagate(QEvent *event)
{
    if (!m_propagateEvents)
        return false;

    if (!m_sourceItem) {
        qWarning() << "Failed to get sourceItem for" << this;
        return false;
    }

    if (!event) {
        qWarning() << "Event handle is invalid";
        return false;
    }

    return true;
}

QPointF WebOSSurfaceItemMirror::translatePoint(QPointF point)
{
    if (!m_clustered)
        return point;

    WebOSCompositorWindow *compositorWindow = static_cast<WebOSCompositorWindow *>(window());
    if (!compositorWindow) {
        qWarning() << "Compositor window handle is invalid";
        return point;
    }

    // Translate position based on current window's origin only for mouse event.
    // Currently, it's assumed all displays have the identical width and height in the cluster.
    if (point.x() > compositorWindow->outputGeometry().width())
        point.setX((int)point.x() % compositorWindow->outputGeometry().width());
    if (point.y() > compositorWindow->outputGeometry().height())
        point.setY((int)point.y() % compositorWindow->outputGeometry().height());

    point += compositorWindow->positionInCluster();

    QSize clusterSize = compositorWindow->clusterSize();
    point.setX(point.x() * m_sourceItem->width() / clusterSize.width());
    point.setY(point.y() * m_sourceItem->height() / clusterSize.height());

    return point;
}