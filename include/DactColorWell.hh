#ifndef DACTCOLORWELL_H
#define DACTCOLORWELL_H

#include <QColor>
#include <QIcon>
#include <QPixmap>
#include <QToolButton>


class DactColorWell : public QToolButton
{
    Q_OBJECT
    
public:
    DactColorWell(QWidget *parent = 0);
    void setColor(QColor const &color);
    QColor const &color() const;

signals:
    void colorSelected(QColor const &color);

private slots:
    void openColorDialog();
    void updateColor(QColor const &color);
    void updateSwatch(QColor const &color);

private:
    QColor d_color;
    QIcon d_icon;
    QPixmap d_swatch;
};

#endif