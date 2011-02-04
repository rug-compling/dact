#include <QColorDialog>
#include <QPaintEvent>

#include "DactColorWell.hh"

DactColorWell::DactColorWell(QWidget *parent)
:
QToolButton(parent),
d_swatch(16,16)
{
    updateSwatch(d_color);
    
    QObject::connect(this, SIGNAL(clicked()), this, SLOT(openColorDialog()));
    
    QObject::connect(&d_dialog, SIGNAL(currentColorChanged(QColor)), this, SLOT(updateSwatch(QColor)));
    QObject::connect(&d_dialog, SIGNAL(colorSelected(QColor)), this, SLOT(updateColor(QColor)));
}

QColor const &DactColorWell::color() const
{
    return d_color;
}

void DactColorWell::setColor(QColor const &color)
{
    d_color = color;
    updateSwatch(d_color);
}

void DactColorWell::updateColor(QColor const &color)
{
    setColor(color);
    emit colorSelected(d_color);
}

void DactColorWell::openColorDialog()
{
    d_dialog.open();
}

void DactColorWell::updateSwatch(QColor const &color)
{
    d_swatch.fill(color);
    d_icon = d_swatch;
    setIcon(d_icon);
}

