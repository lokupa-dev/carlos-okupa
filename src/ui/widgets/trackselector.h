#ifndef TRACKSELECTOR_H
#define TRACKSELECTOR_H

#include <QWidget>
#include <QStringList>
#include <memory>

class QCheckBox;

class TrackSelector : public QWidget
{
    Q_OBJECT

public:
    explicit TrackSelector(QWidget *parent = nullptr);
    ~TrackSelector();

    QStringList selectedStems() const;

private slots:
    void onSelectAll();
    void onDeselectAll();

private:
    void setupUI();

    std::vector<std::unique_ptr<QCheckBox>> checkboxes;
};

#endif // TRACKSELECTOR_H
