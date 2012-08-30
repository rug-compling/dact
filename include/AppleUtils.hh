#ifndef APPLE_UTILS_HH
#define APPLE_UTILS_HH
#include <QMainWindow>

#if defined(__APPLE__) && MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7

void enableFullScreenOnMac(QMainWindow const *);

void toggleFullScreenOnMac(QMainWindow const *);

#endif

#endif
