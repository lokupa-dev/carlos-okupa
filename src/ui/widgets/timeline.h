#ifndef TIMELINE_H
#define TIMELINE_H

#include <QWidget>

class Timeline : public QWidget
{
    Q_OBJECT

public:
    explicit Timeline(QWidget *parent = nullptr);
    ~Timeline();

private:
    void setupUI();
};

#endif // TIMELINE_H
