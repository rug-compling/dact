#include <QWheelEvent>

#include "DactTreeView.hh"

void DactTreeView::wheelEvent(QWheelEvent * event)
{
	// If the control modifier key isn't pressed, handle this wheel
	// even as any other event: pan the scene... probably.
	if (event->modifiers() != Qt::ControlModifier)
		return QGraphicsView::wheelEvent(event);
	
	// if it is not vertical scrolling, we ignore it.
	if (event->orientation() != Qt::Vertical)
		return;
	
	// according to the QWheelEvent docs
	qreal degree = event->delta() / 8; 
	
	// zoomFactor has to be something around 1.
	// NEVER BE ZERO OR YOU WILL RUIN THE MATRIX
	// Todo: adjust this formula. It works ok on my magic trackpad, but
	// I haven't tested it with a real mouse or anything.
	// Todo: maybe use the ZOOM_IN_FACTOR and ZOOM_OUT_FACTOR constants
	//qreal zoomFactor = 1.0 + degree / 360;
	
	qreal zoomFactor = 1.0 + degree / 180;
	
	if (zoomFactor > 2.0)
		zoomFactor = 2.0;
	else if (zoomFactor <= 0.0)
		zoomFactor = 0.1;
	
	// TODO: some kind of limit which prevents zooming in or out too far.
	// because being able to do that is silly.
	
	// Zoom in on position of the mouse (like Google Maps)
	setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
	scale(zoomFactor, zoomFactor);
	
	event->accept();
}
