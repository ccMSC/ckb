#ifndef MPERFWIDGET_H
#define MPERFWIDGET_H

#include <QWidget>
#include <QSlider>
#include <QSpinBox>
#include <QCheckBox>
#include <QRadioButton>
#include <QSignalMapper>
#include "kbperf.h"
#include "colorbutton.h"

namespace Ui {
class MPerfWidget;
}

class MPerfWidget : public QWidget
{
    Q_OBJECT

public:
    explicit MPerfWidget(QWidget *parent = 0);
    ~MPerfWidget();

    void setPerf(KbPerf* newPerf);

    static const int DPI_COUNT = KbPerf::DPI_COUNT;
    const static int DPI_MIN = KbPerf::DPI_MIN, DPI_MAX = KbPerf::DPI_MAX;

private:
    Ui::MPerfWidget *ui;

    KbPerf* perf;

    struct DpiUi {
        ColorButton* indicator;
        QSlider* xSlider, *ySlider;
        QSpinBox* xBox, *yBox;
        QCheckBox* enableCheck;
    };
    DpiUi stages[DPI_COUNT];
    bool _xyLink;

    QSignalMapper buttonMapper;
    QSignalMapper sliderXMapper, sliderYMapper;
    QSignalMapper boxXMapper, boxYMapper;
    QSignalMapper enableMapper;

    // Hack: prevent recursive slot calls
    bool isSetting;

private slots:
    void colorChanged(int index);
    void sliderXMoved(int index);
    void sliderYMoved(int index);
    void boxXChanged(int index);
    void boxYChanged(int index);
    void enableChanged(int index);

    void on_xyBox_clicked(bool checked);
    void on_indicBox_clicked(bool checked);
    void on_aSnapBox_clicked(bool checked);
    void on_lHeightBox_activated(int index);
};

#endif // MPERFWIDGET_H
