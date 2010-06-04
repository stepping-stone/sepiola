#ifndef EXTENDED_QPROCESS_HH
#define EXTENDED_QPROCESS_HH

#include <QProcess>

class ExtendedQProcess : public QProcess
{
public:
	void resetError();
	bool hasError();

private:
	QString noErrorString;
};

inline void ExtendedQProcess::resetError()
{
	// There's no way to reset error, so reset errorString only.
	setErrorString("");
	this->noErrorString = errorString();
}

inline bool ExtendedQProcess::hasError()
{
	return errorString() != this->noErrorString;
}

#endif

