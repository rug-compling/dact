#include <QColorDialog>

#include "DactColorWell.hh"

DactColorWell::DactColorWell(QWidget *parent)
:
QToolButton(parent),
d_swatch(16,16)
{
    updateSwatch(d_color);
    
    QObject::connect(this, SIGNAL(clicked()), this, SLOT(openColorDialog()));
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
    updateColor(QColorDialog::getColor(d_color, this));
}

void DactColorWell::updateSwatch(QColor const &color)
{
    d_swatch.fill(color);
    d_icon = d_swatch;
    setIcon(d_icon);
}

