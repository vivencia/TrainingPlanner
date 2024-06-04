#ifndef TPAPPLICATION_H
#define TPAPPLICATION_H

#include <QApplication>

class TPApplication : public QApplication
{

Q_OBJECT

public:
	TPApplication(int& argc, char** argv) : QApplication(argc, argv) {}
	bool event(QEvent* event) override;

signals:
	void openFileRequested(QString fileName);
};

#endif // TPAPPLICATION_H
