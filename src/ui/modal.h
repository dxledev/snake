#pragma once
#include "core/theme.h"
#include <QPointer>
#include <QWidget>
#include <functional>
class QLabel;
class QScrollArea;
class QPushButton;
namespace snake {
class ModalMotion;
class ModalBackdrop;
class Modal : public QWidget {
  Q_OBJECT
public:
  Modal(QWidget *parent, QWidget *background);
  void open(const QString &title, const QString &body,
            const QStringList &actions);
  void setTheme(const Theme &theme);
  void dismiss();
  void refreshSnapshot();
signals:
  void chosen(int index);
  void dismissed();

protected:
  bool event(QEvent *event) override;
  void paintEvent(QPaintEvent *) override;
  void resizeEvent(QResizeEvent *) override;
  void hideEvent(QHideEvent *) override;
  void keyPressEvent(QKeyEvent *) override;
  void mousePressEvent(QMouseEvent *) override;
  bool eventFilter(QObject *object, QEvent *event) override;

private:
  QPointer<QWidget> background_, previousFocus_;
  QWidget *card_;
  QLabel *title_, *body_;
  ModalMotion *motion_;
  ModalBackdrop *backdrop_ = nullptr;
  bool closing_ = false, backgroundWasEnabled_ = true;
  int pendingChoice_ = -1;
  void beginDismiss(int choice);
  void finishDismiss();
  QScrollArea *bodyScroll_;
  QList<QPushButton *> buttons_;
  QPixmap snapshot_, backgroundFrame_;
  bool backgroundFrozen_ = false, backgroundUpdatesWereEnabled_ = true;
  void freezeBackground();
  void restoreBackgroundUpdates();
  void composeSnapshot(const QPixmap &blurred);
  Theme theme_;
  void centerCard();
};
} // namespace snake
