#ifndef SCALELABEL_H
#define SCALELABEL_H

#include <QLabel>
#include <QMovie>

class ScaleLabel : public QLabel
{
  Q_OBJECT
public:
  explicit ScaleLabel(QWidget *parent = 0);
  ~ScaleLabel();

  void setScalableMovie(const QString &path);
  void setScalableImage(const QImage &image);
signals:

public slots:
protected:
  virtual void resizeEvent(QResizeEvent *event);
private:
  QSize m_OriginalMovieSize;
  QMovie *m_Movie;
  QPixmap *m_Pixmap;
  bool m_isMovie;
};

#endif // SCALELABEL_H
