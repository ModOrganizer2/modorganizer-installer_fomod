#include "scalelabel.h"
#include <QResizeEvent>

ScaleLabel::ScaleLabel(QWidget *parent)
  : QLabel(parent)
  , m_Movie(nullptr)
  , m_Pixmap(nullptr)
{
}

ScaleLabel::~ScaleLabel()
{
  if (m_Movie)
    delete m_Movie;
  if (m_Pixmap)
    delete m_Pixmap;
}

void ScaleLabel::setScalableMovie(const QString &path)
{
  if (m_Movie) {
    m_Movie->stop();
    delete m_Movie;
  }
  m_Movie = new QMovie(path);
  m_isMovie = true;
  setMovie(m_Movie);
  m_Movie->start();
  m_Movie->stop();
  m_OriginalMovieSize = m_Movie->currentImage().size();

  m_Movie->setScaledSize(m_OriginalMovieSize.scaled(size(), Qt::KeepAspectRatio));
  m_Movie->start();
}

void ScaleLabel::setScalableImage(const QImage &image)
{
  if (m_Pixmap) {
    delete m_Pixmap;
  }
  m_Pixmap = new QPixmap(QPixmap::fromImage(image));
  m_isMovie = false;
  setPixmap(m_Pixmap->scaled(size(), Qt::KeepAspectRatio));
}

void ScaleLabel::resizeEvent(QResizeEvent *event)
{
  if (m_isMovie) {
    if ((m_Movie != nullptr)) {
      m_Movie->stop();
      m_Movie->setScaledSize(m_OriginalMovieSize.scaled(event->size(), Qt::KeepAspectRatio));
      m_Movie->start();
    }
  } else {
    if ((m_Pixmap != nullptr) && (pixmap() != nullptr) && !pixmap()->isNull() && !m_Pixmap->isNull()) {
      setPixmap(m_Pixmap->scaled(event->size(), Qt::KeepAspectRatio));
    }
  }

}
