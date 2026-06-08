#include <QApplication>
#include <QIcon>
#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QStandardPaths>
#include <QDir>
#include <QtGlobal>
#include "AppWindow.h"

static bool       s_debugMode  = false;
static QFile*     s_logFile    = nullptr;
static QTextStream* s_logStream = nullptr;

static void messageHandler(QtMsgType type, const QMessageLogContext&, const QString& msg)
{
    // Qt emits this on GNOME Wayland sessions but the app runs fine on XCB — suppress it.
    if (msg.contains("Ignoring XDG_SESSION_TYPE"))
        return;

    if (!s_debugMode && type < QtWarningMsg)
        return;

    const char* level =
        type == QtDebugMsg    ? "DEBUG" :
        type == QtInfoMsg     ? "INFO"  :
        type == QtWarningMsg  ? "WARN"  :
        type == QtCriticalMsg ? "ERROR" : "FATAL";

    QString entry = QString("[%1] [%2] %3")
        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
        .arg(level)
        .arg(msg);

    if (s_logStream) {
        *s_logStream << entry << "\n";
        s_logStream->flush();
    }
    fprintf(stderr, "%s\n", entry.toLocal8Bit().constData());

    if (type == QtFatalMsg)
        abort();
}

int main(int argc, char *argv[])
{
    for (int i = 1; i < argc; ++i) {
        if (QString(argv[i]) == "--debug") {
            s_debugMode = true;
            break;
        }
    }

    qInstallMessageHandler(messageHandler);

    // On GNOME Wayland sessions Qt probes for a Wayland compositor before
    // falling back to XCB, which blocks the event loop briefly and produces
    // the "Ignoring XDG_SESSION_TYPE=wayland" warning. Setting the platform
    // explicitly skips that probe. Honour any value the user has already set.
    if (!qEnvironmentVariableIsSet("QT_QPA_PLATFORM"))
        qputenv("QT_QPA_PLATFORM", "xcb");

    QApplication app(argc, argv);
    app.setWindowIcon(QIcon(":/icons/appicon.png"));

    if (s_debugMode) {
        QString logDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QDir().mkpath(logDir);
        QString logPath = logDir + "/debug.log";
        s_logFile = new QFile(logPath);
        if (s_logFile->open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append)) {
            s_logStream = new QTextStream(s_logFile);
            fprintf(stderr, "Debug log: %s\n", logPath.toLocal8Bit().constData());
        }
        qDebug() << "=== Session started ===";
        qDebug() << "Qt version:" << QT_VERSION_STR;
        qDebug() << "Platform:" << app.platformName();
    }

    AppWindow window;
    window.show();
    int result = app.exec();

    delete s_logStream;
    s_logStream = nullptr;
    if (s_logFile) {
        s_logFile->close();
        delete s_logFile;
        s_logFile = nullptr;
    }
    return result;
}
